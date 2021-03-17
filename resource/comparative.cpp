/*
 Copyright (©) 2003-2021 Teus Benschop.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <resource/comparative.h>
#include <styles/logic.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <dialog/color.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <database/volatile.h>
#include <access/user.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <styles/indexm.h>
#include <resource/logic.h>
#include <resource/organize.h>
#include <sync/logic.h>


string resource_comparative_url ()
{
  return "resource/comparative";
}


bool resource_comparative_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string resource_comparative (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Comparative Resource"), webserver_request);
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  Assets_View view;


  int userid = filter_string_user_identifier (webserver_request);
  string key = "comparative resource";
  

  // Start off with default comparative. Or keep comparative being constructed.
  bool clean_comparative = true;
  if (request->query.count ("title")) clean_comparative = false;
  if (request->query.count ("base")) clean_comparative = false;
  if (request->query.count ("update")) clean_comparative = false;
  if (request->query.count ("heartbeat")) clean_comparative = false;
  if (request->post.count ("entry")) clean_comparative = false;
  if (request->query.count ("add")) clean_comparative = false;
  if (clean_comparative) {
    Database_Volatile::setValue (userid, key, resource_logic_comparative_resource());
  }
 

  string comparative = Database_Volatile::getValue (userid, key);
  string title;
  string base;
  string update;
  if (!resource_logic_parse_comparative_resource (comparative, title, base, update)) {
    title = "Comparative resource";
    base = "none";
    update = "none";
    comparative = resource_logic_assemble_comparative_resource (title, base, update);
  }

  
  bool resource_edited = false;
  
  
  // The comparative's title.
  if (request->query.count ("title")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("comparative", translate("Please enter the title for the comparative resource"), title, "title", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("title")) {
    title = request->post["entry"];
    resource_edited = true;
  }

  
  // The comparative's base.
  if (request->query.count ("base")) {
    string value = request->query["base"];
    if (value.empty()) {
      Dialog_List dialog_list = Dialog_List ("comparative", translate("Select a resource to be used as a base resource."), translate ("The base resource is used as a starting point for the comparison."), "");
      dialog_list.add_query ("heartbeat", "heartbeat");
      vector <string> resources = resource_logic_get_names (webserver_request, true);
      for (auto & resource : resources) {
        dialog_list.add_row (resource, "base", resource);
      }
      page += dialog_list.run ();
      return page;
    } else {
      base = value;
      resource_edited = true;
    }
  }

  
  // The comparative's updated resource name.
  if (request->query.count ("update")) {
    string value = request->query["update"];
    if (value.empty()) {
      Dialog_List dialog_list = Dialog_List ("comparative", translate("Select a resource to be used as the updated resource."), translate ("The updated resource will be compared with the base resource."), "");
      dialog_list.add_query ("heartbeat", "heartbeat");
      vector <string> resources = resource_logic_get_names (webserver_request, true);
      for (auto & resource : resources) {
        dialog_list.add_row (resource, "update", resource);
      }
      page += dialog_list.run ();
      return page;
    } else {
      update = value;
      resource_edited = true;
    }
  }

  
  // Get and optionally save the new comparative.
  comparative = resource_logic_assemble_comparative_resource (title, base, update);
  if (resource_edited) {
    Database_Volatile::setValue (userid, key, comparative);
  }

  
  // Add it to the existing resources.
  if (request->query.count ("add")) {
    vector <string> resources = request->database_config_user()->getActiveResources ();
    resources.push_back (comparative);
    request->database_config_user()->setActiveResources (resources);
    request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
    redirect_browser (request, resource_organize_url ());
    return "";
  }
  

  // Set the building blocks of the comparative.
  view.set_variable ("title", title);
  view.set_variable ("base", base);
  view.set_variable ("update", update);
  
  
  page += view.render ("resource", "comparative");
  
  page += Assets_Page::footer ();
  
  return page;
}
