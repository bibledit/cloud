/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <database/logs.h>
#include <sync/logic.h>
#include <user/logic.h>


std::string sync_setup_url ()
{
  return "sync/setup";
}


std::string sync_setup (Webserver_Request& webserver_request)
{
  const std::string username = filter::string::hex2bin(webserver_request.query["user"]);
  const std::string password_hash = webserver_request.query ["pass"];

  constexpr const char* unrecognized_credentials {"Server does not recognize the credentials"};
  constexpr const char* require_secure {"Server requires a secure connection"};

  // Check the username presented by the client.
  if (!webserver_request.database_users ()->username_exists (username)) {
    if (user_logic_login_failure_check_okay()) {
      std::stringstream ss{};
      ss << "Failed client connection attempt with incorrect username " << std::quoted(username);
      Database_Logs::log(std::move(ss).str());
    }
    user_logic_login_failure_register ();
    return unrecognized_credentials;
  }

  // Check the password hash presented by the client.
  if (const std::string md5 = webserver_request.database_users()->get_md5(username);
      password_hash != md5) {
    if (user_logic_login_failure_check_okay()) {
      Database_Logs::log("Failed client connection attempt with incorrect password");
    }
    user_logic_login_failure_register ();
    return unrecognized_credentials;
  }

  // At this point the credentials are accepted.
  // To avoid credentials detection by brute force if the credentials were accepted,
  // give a response to the attacker as if the credentials were not accepted.
  if (!user_logic_login_failure_check_okay ()) {
    return unrecognized_credentials;
  }

  // When the Cloud enforces https, and the client uses http, inform the client to upgrade.
  Sync_Logic sync_logic (webserver_request);
  if (!sync_logic.security_okay ()) {
    webserver_request.response_code = 426;
    std::stringstream ss{};
    if (user_logic_login_failure_check_okay()) {
      ss << "Rejecting insecure client login with username " << std::quoted(username) << " - client should upgrade its Cloud connection to https";
      Database_Logs::log(std::move(ss).str());
    }
    return require_secure;
  }
  
  // Return the user level to the client.
  return std::to_string(webserver_request.database_users()->get_level(username));
}
