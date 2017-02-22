/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <manage/privileges.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/privileges.h>
#include <access/logic.h>
#include <menu/logic.h>
#include <manage/users.h>


string manage_privileges_url ()
{
  return "manage/privileges";
}


bool manage_privileges_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string manage_privileges (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate("Read/write"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (manage_users_url (), menu_logic_manage_users_text ());
  page = header.run ();
  Assets_View view;

  
  // Get the user and his/her level.
  string user = request->query["user"];
  view.set_variable ("user", user);
  int level;
  access_logic_user_level (webserver_request, user, level);

  
  bool privileges_updated = false;
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);
  bool state;
  
  
  // The privilege to view the Resources.
  if (checkbox == "viewresources") {
    Database_Privileges::setFeature (user, PRIVILEGE_VIEW_RESOURCES, checked);
    privileges_updated = true;
  }
  state = Database_Privileges::getFeature (user, PRIVILEGE_VIEW_RESOURCES);
  if (level >= access_logic_view_resources_role ()) {
    state = true;
    view.set_variable ("viewresourcesdisabled", get_disabled (true));
  }
  view.set_variable ("viewresourceschecked", get_checkbox_status (state));

  
  // Privilege to view the Consultation Notes.
  if (checkbox == "viewnotes") {
    Database_Privileges::setFeature (user, PRIVILEGE_VIEW_NOTES, checked);
    privileges_updated = true;
  }
  state = Database_Privileges::getFeature (user, PRIVILEGE_VIEW_NOTES);
  if (level >= access_logic_view_resources_role ()) {
    view.set_variable ("viewnotesdisabled", get_disabled (true));
  }
  state = access_logic_privilege_view_notes (webserver_request, user);
  view.set_variable ("viewnoteschecked", get_checkbox_status (state));
  
  
  // Privilege to create and comment on Consultation Notes.
  if (checkbox == "createcommentnotes") {
    Database_Privileges::setFeature (user, PRIVILEGE_CREATE_COMMENT_NOTES, checked);
    privileges_updated = true;
  }
  state = Database_Privileges::getFeature (user, PRIVILEGE_CREATE_COMMENT_NOTES);
  if (level >= access_logic_view_resources_role ()) {
    view.set_variable ("createcommentnotesdisabled", get_disabled (true));
  }
  state = access_logic_privilege_create_comment_notes (webserver_request, user);
  view.set_variable ("createcommentnoteschecked", get_checkbox_status (state));

  
  if (privileges_updated) database_privileges_client_create (user, true);

  
  if (privileges_updated) return "";


  page += view.render ("manage", "privileges");
  
  page += Assets_Page::footer ();
  
  return page;
}
