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


#pragma once

#include <config/libraries.h>
#include <session/logic.h>
#include <database/config/user.h>
#include <database/users.h>
#include <database/styles.h>
#include <database/bibles.h>
#include <database/ipc.h>

class Webserver_Request
{
public:
  Webserver_Request ();
  ~Webserver_Request ();
  Webserver_Request(const Webserver_Request&) = delete;
  Webserver_Request operator=(const Webserver_Request&) = delete;
  // Whether the connection runs via the secure server.
  bool secure {false};
  // The session identifier of the cookie sent by the browser.
  std::string session_identifier {};
   // Whether to resend the cookie to the browser.
  bool resend_cookie {false};
   // The browser's or client's remote IPv4 or IPv6 address.
  std::string remote_address {};
  // The page the browser requests via GET or via POST.
  std::string get {};
   // Whether it is a POST request.
  bool is_post {false};
   // The query from the browser, e.g. foo=bar&baz=qux, neatly arranged into a map.
  std::map <std::string, std::string> query {};
   // The browser's user agent, e.g. Mozilla/x.0 (X11; Linux) ...
  std::string user_agent {};
   // The browser's or client's Accept-Language header.
  std::string accept_language {};
   // The server's host as requested by the client.
  std::string host {};
   // The content type of the browser request.
  std::string content_type {};
   // The content length of the browser request.
  int content_length {0};
   // The raw POST data from the browser, item by item.
  std::map <std::string, std::string> post {};
   // Header as received from the browser.
  std::string if_none_match {};
   // Extra header to be sent back to the browser.
  std::string header {};
   // Body to be sent back to the browser.
  std::string reply {};
   // Response code to be sent to the browser.
  int response_code {0};
   // The requested file's size for browser caching.
  std::string etag {};
   // The content type of the response.
  std::string response_content_type {};
   // The path of the file to copy from disk straight to the network without loading it in memory.
  std::string stream_file {};
  // Extra objects.
  Session_Logic * session_logic ();
  Database_Config_User * database_config_user ();
  Database_Users * database_users ();
  Database_Styles * database_styles ();
  Database_Ipc * database_ipc ();
private:
  Session_Logic * session_logic_instance { nullptr };
  Database_Config_User * database_config_user_instance { nullptr };
  Database_Users * database_users_instance { nullptr };
  Database_Styles * database_styles_instance { nullptr };
  Database_Ipc * database_ipc_instance { nullptr };
};
