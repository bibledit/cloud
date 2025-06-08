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


#include <consistency/poll.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <consistency/logic.h>
#include <database/temporal.h>


std::string consistency_poll_url ()
{
  return "consistency/poll";
}


bool consistency_poll_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string consistency_poll (Webserver_Request& webserver_request)
{
  const int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  Consistency_Logic consistency_logic = Consistency_Logic (webserver_request, id);
  const std::string response = consistency_logic.response ();
  if (response != database::temporal::get_value (id, "response")) {
    database::temporal::set_value (id, "response", response);
    return response;
  }
  return std::string();
}
