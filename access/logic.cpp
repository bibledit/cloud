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


#include <access/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <database/privileges.h>
#include <webserver/request.h>


namespace access_logic {


int view_resources_role ()
{
  return Filter_Roles::consultant ();
}


bool privilege_view_resources (Webserver_Request& webserver_request, std::string user)
{
  int level {0};
  user_level (webserver_request, user, level);
  if (level >= view_resources_role ()) return true;
  return DatabasePrivileges::get_feature (user, PRIVILEGE_VIEW_RESOURCES);
}


int view_notes_role ()
{
  return Filter_Roles::consultant ();
}


bool privilege_view_notes (Webserver_Request& webserver_request, std::string user)
{
  int level {0};
  user_level (webserver_request, user, level);
  if (level >= view_notes_role ()) return true;
  return DatabasePrivileges::get_feature (user, PRIVILEGE_VIEW_NOTES);
}


int create_comment_notes_role ()
{
  return Filter_Roles::consultant ();
}


bool privilege_create_comment_notes (Webserver_Request& webserver_request, std::string user)
{
  int level {0};
  user_level (webserver_request, user, level);
  if (level >= create_comment_notes_role ()) return true;
  return DatabasePrivileges::get_feature (user, PRIVILEGE_CREATE_COMMENT_NOTES);
}


int delete_consultation_notes_role ()
{
  return Filter_Roles::manager ();
}


bool privilege_delete_consultation_notes (Webserver_Request& webserver_request, std::string user)
{
  int level {0};
  user_level (webserver_request, user, level);
  if (level >= delete_consultation_notes_role ()) return true;
  return webserver_request.database_config_user ()->getPrivilegeDeleteConsultationNotesForUser (user);
}


int use_advanced_mode_role ()
{
  return Filter_Roles::manager ();
}


bool privilege_use_advanced_mode (Webserver_Request& webserver_request, std::string user)
{
  int level {0};
  user_level (webserver_request, user, level);
  if (level >= use_advanced_mode_role ()) return true;
  return webserver_request.database_config_user ()->getPrivilegeUseAdvancedModeForUser (user);
}


int set_stylesheets_role ()
{
  return Filter_Roles::manager ();
}


bool privilege_set_stylesheets (Webserver_Request& webserver_request, std::string user)
{
  int level {0};
  user_level (webserver_request, user, level);
  if (level >= set_stylesheets_role ()) return true;
  return webserver_request.database_config_user ()->getPrivilegeSetStylesheetsForUser (user);
}


void user_level (Webserver_Request& webserver_request, std::string& user, int& level)
{
  if (user.empty ()) {
    // If no user is given, take the user from the session.
    user = webserver_request.session_logic ()->get_username ();
    level = webserver_request.session_logic ()->get_level ();
  } else {
    // If a user is given, take the matching level from the database.
    level = webserver_request.database_users ()->get_level (user);
  }
}


void create_client_files ()
{
  Database_Users database_users;
  std::vector <std::string> users = database_users.get_users ();
  for (const auto& user : users) {
    // Only maintain the privilege file if it does not yet exist,
    // to avoid unnecessary downloads by the clients.
    database_privileges_client_create (user, false);
  }
}


std::set <std::string> default_privilege_usernames ()
{
  return {"defaultguest", "defaultmember", "defaultconsultant", "defaulttranslator", "defaultmanager"};
}


}
