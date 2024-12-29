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


#include <consistency/input.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <consistency/logic.h>
#include <database/temporal.h>


std::string consistency_input_url ()
{
  return "consistency/input";
}


bool consistency_input_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string consistency_input (Webserver_Request& webserver_request)
{
  const int id = filter::strings::convert_to_int (webserver_request.post ["id"]);
  const std::string passages = webserver_request.post ["passages"];
  const std::string translations = webserver_request.post ["translations"];
  database::temporal::set_value (id, "passages", passages);
  database::temporal::set_value (id, "translations", translations);
  Consistency_Logic consistency_logic (webserver_request, id);
  const std::string response = consistency_logic.response ();
  database::temporal::set_value (id, "response", response);
  return response;
}
