/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


std::string manage_privileges_url ()
{
  return "manage/privileges";
}


bool manage_privileges_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string manage_privileges (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("Read/write"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (manage_users_url (), menu_logic_manage_users_text ());
  page = header.run ();
  Assets_View view {};

  
  // Get the user and his/her level.
  std::string user {webserver_request.query["user"]};
  if (user.empty()) user = webserver_request.post["val1"];
  view.set_variable ("user", user);
  int level {0};
  access_logic::user_level (webserver_request, user, level);


  // Usernames for setting default new user privilege.
  std::set <std::string> defusers = access_logic::default_privilege_usernames ();

  
  bool privileges_updated {false};
  std::string checkbox {webserver_request.post ["checkbox"]};
  bool checked {filter::strings::convert_to_bool (webserver_request.post ["checked"])};
  bool state {false};
  
  
  // The privilege to view the Resources.
  if (checkbox == "viewresources") {
    DatabasePrivileges::set_feature (user, PRIVILEGE_VIEW_RESOURCES, checked);
    privileges_updated = true;
  }
  state = DatabasePrivileges::get_feature (user, PRIVILEGE_VIEW_RESOURCES);
  if (level >= access_logic::view_resources_role () && defusers.find (user) == defusers.end ()) {
    state = true;
    view.set_variable ("viewresourcesdisabled", filter::strings::get_disabled (true));
  }
  view.set_variable ("viewresourceschecked", filter::strings::get_checkbox_status (state));

  
  // Privilege to view the Consultation Notes.
  if (checkbox == "viewnotes") {
    DatabasePrivileges::set_feature (user, PRIVILEGE_VIEW_NOTES, checked);
    privileges_updated = true;
  }
  state = DatabasePrivileges::get_feature (user, PRIVILEGE_VIEW_NOTES);
  if (level >= access_logic::view_resources_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("viewnotesdisabled", filter::strings::get_disabled (true));
  }
  state = access_logic::privilege_view_notes (webserver_request, user);
  view.set_variable ("viewnoteschecked", filter::strings::get_checkbox_status (state));
  
  
  // Privilege to create and comment on Consultation Notes.
  if (checkbox == "createcommentnotes") {
    DatabasePrivileges::set_feature (user, PRIVILEGE_CREATE_COMMENT_NOTES, checked);
    privileges_updated = true;
  }
  state = DatabasePrivileges::get_feature (user, PRIVILEGE_CREATE_COMMENT_NOTES);
  if (level >= access_logic::view_resources_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("createcommentnotesdisabled", filter::strings::get_disabled (true));
  }
  state = access_logic::privilege_create_comment_notes (webserver_request, user);
  view.set_variable ("createcommentnoteschecked", filter::strings::get_checkbox_status (state));

  
  // Privilege to delete Consultation Notes.
  if (checkbox == "deletenotes") {
    webserver_request.database_config_user ()->set_privilege_delete_consultation_notes_for_user (user, checked);
  }
  state = DatabasePrivileges::get_feature (user, PRIVILEGE_CREATE_COMMENT_NOTES);
  if (level >= access_logic::delete_consultation_notes_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("deletenotesdisabled", filter::strings::get_disabled (true));
  }
  state = access_logic::privilege_delete_consultation_notes (webserver_request, user);
  view.set_variable ("deletenoteschecked", filter::strings::get_checkbox_status (state));

  
  // Privilege to use advanced mode.
  if (checkbox == "useadvancedmode") {
    webserver_request.database_config_user ()->set_privilege_use_advanced_mode_for_user (user, checked);
  }
  if (level >= access_logic::use_advanced_mode_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("useadvancedmodedisabled", filter::strings::get_disabled (true));
  }
  state = access_logic::privilege_use_advanced_mode (webserver_request, user);
  view.set_variable ("useadvancedmodechecked", filter::strings::get_checkbox_status (state));

  
  // Privilege to be able to edit and set stylesheets.
  if (checkbox == "editstylesheets") {
    webserver_request.database_config_user ()->set_privilege_set_stylesheets_for_user (user, checked);
  }
  if (level >= access_logic::set_stylesheets_role () && defusers.find (user) == defusers.end ()) {
    view.set_variable ("editstylesheetsdisabled", filter::strings::get_disabled (true));
  }
  state = access_logic::privilege_set_stylesheets (webserver_request, user);
  view.set_variable ("editstylesheetschecked", filter::strings::get_checkbox_status (state));

  
  if (privileges_updated) database_privileges_client_create (user, true);

  
  if (privileges_updated) return filter::strings::get_reload ();


  page += view.render ("manage", "privileges");
  
  page += assets_page::footer ();
  
  return page;
}
