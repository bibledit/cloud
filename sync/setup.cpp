/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


std::string sync_setup_url ()
{
  return "sync/setup";
}


std::string sync_setup (Webserver_Request& webserver_request)
{
  Sync_Logic sync_logic (webserver_request);
  
  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    webserver_request.response_code = 426;
    return std::string();
  }
  
  std::string page;
  
  std::string username = webserver_request.query ["user"];
  username = filter::strings::hex2bin (username);
  std::string password = webserver_request.query ["pass"];

  // Check the credentials of the client.
  if (webserver_request.database_users ()->usernameExists (username)) {
    std::string md5 = webserver_request.database_users ()->get_md5 (username);
    if (password == md5) {
      // Check brute force attack mitigation.
      if (user_logic_login_failure_check_okay ()) {
        // Return the level to the client.
        return filter::strings::convert_to_string (webserver_request.database_users ()->get_level (username));
      }
    }
  }
  
  // The credentials were not accepted.
  user_logic_login_failure_register ();
  return "Server does not recognize the credentials";
}
