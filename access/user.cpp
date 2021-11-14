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


#include <access/user.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <filter/roles.h>


// This function returns users assigned to the logged-in user.
vector <string> access_user_assignees (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  string myuser = request->session_logic ()->currentUser ();
  int mylevel = request->session_logic ()->currentLevel ();

  // This holds the assignees.
  vector <string> assignees;

  // Process all users.
  vector <string> users = request->database_users ()->get_users ();
  sort (users.begin(), users.end());
  for (auto & user : users) {
    // Assignees should have a level less than or equal to mylevel.
    int level = request->database_users ()->get_level (user);
    if (level <= mylevel) {
      assignees.push_back (user);
    }
  }
  
  return assignees;
}
