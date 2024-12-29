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


#include <resource/unload.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <access/logic.h>
#include <config/globals.h>


std::string resource_unload_url ()
{
  return "resource/unload";
}


bool resource_unload_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


std::string resource_unload (Webserver_Request& webserver_request)
{
  // The scroll position of the resource window.
  int position = filter::strings::convert_to_int (webserver_request.post ["position"]);
  if (position < 0) position = 0;
  if (position > 5000) position = 5000;
  // Store the position in volatile memory so it gets retained while the app is on,
  // and gets reset after app startup.
  const std::string& username = webserver_request.session_logic ()->get_username ();
  config_globals_resource_window_positions [username] = position;
  return std::string();
}
