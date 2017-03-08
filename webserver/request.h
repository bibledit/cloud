/*
Copyright (©) 2003-2017 Teus Benschop.

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


#ifndef INCLUDED_WEBSERVER_REQUEST_H
#define INCLUDED_WEBSERVER_REQUEST_H


#include <config/libraries.h>
#include <session/logic.h>
#include <database/config/user.h>
#include <database/users.h>
#include <database/styles.h>
#include <database/bibles.h>
#include <database/check.h>
#include <database/ipc.h>


class Webserver_Request
{
public:
  Webserver_Request ();
  ~Webserver_Request ();
  bool secure; // Whether the connection runs via the secure server.
  string session_identifier; // The session identifier of the cookie sent by the browser.
  bool resend_cookie; // Whether to resend the cookie to the browser.
  string remote_address; // The browser's or client's remote IPv4 address.
  string get; // The page the browser requests via GET or via POST.
  bool is_post; // Whether it is a POST request.
  map <string, string> query; // The query from the browser, e.g. foo=bar&baz=qux, neatly arranged into a map.
  string user_agent; // The browser's user agent, e.g. Mozilla/x.0 (X11; Linux) ...
  string accept_language; // The browser's or client's Accept-Language header.
  string host; // The server's host as requested by the client.
  string content_type; // The content type of the browser request.
  int content_length; // The content length of the browser request.
  map <string, string> post; // The raw POST data from the browser, item by item.
  string if_none_match; // Header as received from the browser.
  string header; // Extra header to be sent back to the browser.
  string reply; // Body to be sent back to the browser.
  int response_code; // Response code to be sent to the browser.
  string etag; // The requested file's size for browser caching.
  string response_content_type; // The content type of the response.
  Session_Logic * session_logic ();
  Database_Config_User * database_config_user ();
  Database_Users * database_users ();
  Database_Styles * database_styles ();
  Database_Bibles * database_bibles ();
  Database_Check * database_check ();
  Database_Ipc * database_ipc ();
private:
  Session_Logic * session_logic_instance = NULL;
  Database_Config_User * database_config_user_instance = NULL;
  Database_Users * database_users_instance = NULL;
  Database_Styles * database_styles_instance = NULL;
  Database_Bibles * database_bibles_instance = NULL;
  Database_Check * database_check_instance = NULL;
  Database_Ipc * database_ipc_instance = NULL;
};


#endif
