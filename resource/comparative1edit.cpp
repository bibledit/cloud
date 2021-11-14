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


#include <resource/comparative1edit.h>
#include <resource/img.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/config/general.h>
#include <database/books.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <tasks/logic.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <client/logic.h>


string resource_comparative1edit_url ()
{
  return "resource/comparative1edit";
}


bool resource_comparative1edit_acl (void * webserver_request)
{
  return access_logic_privilege_view_resources (webserver_request);
}


string resource_comparative1edit (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  string page;
  Assets_Header header = Assets_Header (translate("User-defined resources"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;
  string error, success;
  
  
  string name = request->query ["name"];
  if (name.empty()) name = request->post ["val1"];
  view.set_variable ("name", name);

  
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);

  
  bool resource_edited = false;


  string title, base, update, remove, replace;
  bool diacritics = false, casefold = false, cache = false;
  {
    vector <string> resources = Database_Config_General::getComparativeResources ();
    for (auto resource : resources) {
      resource_logic_parse_comparative_resource (resource, &title, &base, &update, &remove, &replace, &diacritics, &casefold, &cache);
      if (title == name) break;
    }
  }

  
  // The comparative resource's base resource.
  if (request->query.count ("base")) {
    string value = request->query["base"];
    if (value.empty()) {
      Dialog_List dialog_list = Dialog_List ("comparative1edit", translate("Select a resource to be used as a base resource"), translate ("The base resource is used as a starting point for the comparison."), "");
      dialog_list.add_query ("name", name);
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
  
  
  // The comparative resource's updated resource.
  if (request->query.count ("update")) {
    string value = request->query["update"];
    if (value.empty()) {
      Dialog_List dialog_list = Dialog_List ("comparative1edit", translate("Select a resource to be used as the updated resource."), translate ("The updated resource will be compared with the base resource."), "");
      dialog_list.add_query ("name", name);
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
  
  
  // The characters to remove from both resources before doing a comparison.
  if (request->query.count ("remove")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("comparative1edit", translate("Enter or edit the characters to remove from the resources"), remove, "remove", "");
    dialog_entry.add_query ("name", name);
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("remove")) {
    remove = request->post ["entry"];
    resource_edited = true;
  }

  
  // The characters to search for and replace in both resources before doing a comparison.
  if (request->query.count ("replace")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("comparative1edit", translate("Enter or edit the search and replace sets"), replace, "replace", "");
    dialog_entry.add_query ("name", name);
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("replace")) {
    replace = request->post ["entry"];
    resource_edited = true;
  }

  
  // Whether to remove diacritics before doing the comparison.
  if (checkbox == "diacritics") {
    diacritics = checked;
    resource_edited = true;
  }

  
  // Whether to do case folding of the text before doing the comparison.
  if (checkbox == "casefold") {
    casefold = checked;
    resource_edited = true;
  }

  
  // Whether to cache the resource on client devices.
  if (checkbox == "cache") {
    cache = checked;
    resource_edited = true;
  }

  
  // Save the comparative resource if it was edited.
  if (resource_edited) {
    vector <string> resources = Database_Config_General::getComparativeResources ();
    error = translate ("Could not save");
    for (size_t i = 0; i < resources.size(); i++) {
      string title2;
      resource_logic_parse_comparative_resource (resources[i], &title2);
      if (title2 == title) {
        string resource = resource_logic_assemble_comparative_resource (title, base, update, remove, replace, diacritics, casefold, cache);
        resources[i] = resource;
        success = translate ("Saved");
        error.clear();
      }
    }
    Database_Config_General::setComparativeResources (resources);
    if (cache) client_logic_no_cache_resource_remove(title);
    else client_logic_no_cache_resource_add(title);
  }
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  view.set_variable ("title", title);
  view.set_variable ("base", base);
  view.set_variable ("update", update);
  view.set_variable ("remove", remove);
  view.set_variable ("replace", replace);
  view.set_variable ("diacritics", get_checkbox_status (diacritics));
  view.set_variable ("casefold", get_checkbox_status (casefold));
  view.set_variable ("cache", get_checkbox_status (cache));
  page += view.render ("resource", "comparative1edit");
  page += Assets_Page::footer ();
  return page;
}
