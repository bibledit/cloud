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


#include <access/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <database/privileges.h>


int access_logic_view_resources_role ()
{
  return Filter_Roles::consultant ();
}


bool access_logic_privilege_view_resources (void * webserver_request, string user)
{
  int level = 0;
  access_logic_user_level (webserver_request, user, level);
  if (level >= access_logic_view_resources_role ()) return true;
  return Database_Privileges::getFeature (user, PRIVILEGE_VIEW_RESOURCES);
}


int access_logic_view_notes_role ()
{
  return Filter_Roles::consultant ();
}


bool access_logic_privilege_view_notes (void * webserver_request, string user)
{
  int level = 0;
  access_logic_user_level (webserver_request, user, level);
  if (level >= access_logic_view_notes_role ()) return true;
  return Database_Privileges::getFeature (user, PRIVILEGE_VIEW_NOTES);
}


int access_logic_create_comment_notes_role ()
{
  return Filter_Roles::consultant ();
}


bool access_logic_privilege_create_comment_notes (void * webserver_request, string user)
{
  int level = 0;
  access_logic_user_level (webserver_request, user, level);
  if (level >= access_logic_create_comment_notes_role ()) return true;
  return Database_Privileges::getFeature (user, PRIVILEGE_CREATE_COMMENT_NOTES);
}


int access_logic_delete_consultation_notes_role ()
{
  return Filter_Roles::manager ();
}


bool access_logic_privilege_delete_consultation_notes (void * webserver_request, string user)
{
  int level = 0;
  access_logic_user_level (webserver_request, user, level);
  if (level >= access_logic_delete_consultation_notes_role ()) return true;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  return request->database_config_user ()->getPrivilegeDeleteConsultationNotesForUser (user);
}


int access_logic_use_advanced_mode_role ()
{
  return Filter_Roles::manager ();
}


bool access_logic_privilege_use_advanced_mode (void * webserver_request, string user)
{
  int level = 0;
  access_logic_user_level (webserver_request, user, level);
  if (level >= access_logic_use_advanced_mode_role ()) return true;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  return request->database_config_user ()->getPrivilegeUseAdvancedModeForUser (user);
}


void access_logic_user_level (void * webserver_request, string & user, int & level)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  if (user.empty ()) {
    // If no user is given, take the user from the session.
    user = request->session_logic ()->currentUser ();
    level = request->session_logic ()->currentLevel ();
  } else {
    // If a user is given, take the matching level from the database.
    level = request->database_users ()->get_level (user);
  }
}


void access_logic_create_client_files ()
{
  Database_Users database_users;
  vector <string> users = database_users.getUsers ();
  for (auto & user : users) {
    // Only maintain the privilege file if it does not yet exist,
    // to avoid unnecessary downloads by the clients.
    database_privileges_client_create (user, false);
  }
}
