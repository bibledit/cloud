/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
using namespace std;


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
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  string page {};
  Assets_Header header = Assets_Header (translate("Read/write"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (manage_users_url (), menu_logic_manage_users_text ());
  page = header.run ();
  Assets_View view {};

  
  // Get the user and his/her level.
  string user {request->query["user"]};
  if (user.empty()) user = request->post["val1"];
  view.set_variable ("user", user);
  int level {0};
  access_logic::user_level (webserver_request, user, level);


  // Usernames for setting default new user privilege.
  set <string> defusers = {"defaultguest", "defaultmember", "defaulttranslator", "defaultconsultant", "defaultmanager"};

  
  bool privileges_updated {false};
  string checkbox {request->post ["checkbox"]};
  bool checked {convert_to_bool (request->post ["checked"])};
  bool state {false};
  
  
  // The privilege to view the Resources.
  if (checkbox == "viewresources") {
    Database_Privileges::setFeature (user, PRIVILEGE_VIEW_RESOURCES, checked);
    privileges_updated = true;
  }
  state = Database_Privileges::getFeature (user, PRIVILEGE_VIEW_RESOURCES);
  if (level >= access_logic::view_resources_role () && defusers.find (user) == defusers.end ()) {
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
  if (level >= access_logic::view_resources_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("viewnotesdisabled", get_disabled (true));
  }
  state = access_logic::privilege_view_notes (webserver_request, user);
  view.set_variable ("viewnoteschecked", get_checkbox_status (state));
  
  
  // Privilege to create and comment on Consultation Notes.
  if (checkbox == "createcommentnotes") {
    Database_Privileges::setFeature (user, PRIVILEGE_CREATE_COMMENT_NOTES, checked);
    privileges_updated = true;
  }
  state = Database_Privileges::getFeature (user, PRIVILEGE_CREATE_COMMENT_NOTES);
  if (level >= access_logic::view_resources_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("createcommentnotesdisabled", get_disabled (true));
  }
  state = access_logic::privilege_create_comment_notes (webserver_request, user);
  view.set_variable ("createcommentnoteschecked", get_checkbox_status (state));

  
  // Privilege to delete Consultation Notes.
  if (checkbox == "deletenotes") {
    request->database_config_user ()->setPrivilegeDeleteConsultationNotesForUser (user, checked);
  }
  state = Database_Privileges::getFeature (user, PRIVILEGE_CREATE_COMMENT_NOTES);
  if (level >= access_logic::delete_consultation_notes_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("deletenotesdisabled", get_disabled (true));
  }
  state = access_logic::privilege_delete_consultation_notes (webserver_request, user);
  view.set_variable ("deletenoteschecked", get_checkbox_status (state));

  
  // Privilege to use advanced mode.
  if (checkbox == "useadvancedmode") {
    request->database_config_user ()->setPrivilegeUseAdvancedModeForUser (user, checked);
  }
  if (level >= access_logic::use_advanced_mode_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("useadvancedmodedisabled", get_disabled (true));
  }
  state = access_logic::privilege_use_advanced_mode (webserver_request, user);
  view.set_variable ("useadvancedmodechecked", get_checkbox_status (state));

  
  // Privilege to be able to edit and set stylesheets.
  if (checkbox == "editstylesheets") {
    request->database_config_user ()->setPrivilegeSetStylesheetsForUser (user, checked);
  }
  if (level >= access_logic::set_stylesheets_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("editstylesheetsdisabled", get_disabled (true));
  }
  state = access_logic::privilege_set_stylesheets (webserver_request, user);
  view.set_variable ("editstylesheetschecked", get_checkbox_status (state));

  
  if (privileges_updated) database_privileges_client_create (user, true);

  
  if (privileges_updated) return get_reload ();


  page += view.render ("manage", "privileges");
  
  page += assets_page::footer ();
  
  return page;
}
