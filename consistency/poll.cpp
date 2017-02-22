/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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
#include <database/volatile.h>


string consistency_poll_url ()
{
  return "consistency/poll";
}


bool consistency_poll_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string consistency_poll (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  int id = convert_to_int (request->query ["id"]);
  Consistency_Logic consistency_logic = Consistency_Logic (webserver_request, id);
  string response = consistency_logic.response ();
  if (response != Database_Volatile::getValue (id, "response")) {
    Database_Volatile::setValue (id, "response", response);
    return response;
  }
  return "";
}
