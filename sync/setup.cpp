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


#include <sync/setup.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <database/config/general.h>
#include <database/users.h>
#include <sync/logic.h>
#include <user/logic.h>
using namespace std;


string sync_setup_url ()
{
  return "sync/setup";
}


string sync_setup (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Sync_Logic sync_logic = Sync_Logic (webserver_request);
  
  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    request->response_code = 426;
    return "";
  }
  
  string page;
  
  string username = request->query ["user"];
  username = hex2bin (username);
  string password = request->query ["pass"];

  // Check the credentials of the client.
  if (request->database_users ()->usernameExists (username)) {
    string md5 = request->database_users ()->get_md5 (username);
    if (password == md5) {
      // Check brute force attack mitigation.
      if (user_logic_login_failure_check_okay ()) {
        // Return the level to the client.
        return convert_to_string (request->database_users ()->get_level (username));
      }
    }
  }
  
  // The credentials were not accepted.
  user_logic_login_failure_register ();
  return "Server does not recognize the credentials";
}
