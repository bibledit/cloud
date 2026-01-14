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


#include <webserver/request.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <index/index.h>


const char * session_logout_url ()
{
  return "session/logout";
}


bool session_logout_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::guest);
}


std::string session_logout (Webserver_Request& webserver_request)
{
  webserver_request.session_logic ()->logout ();
  redirect_browser (webserver_request, index_index_url ());
  return std::string();
}


