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


#include <assets/external.h>
#include <filter/roles.h>
#include <config/globals.h>


string assets_external_url ()
{
  return "assets/external";
}


bool assets_external_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string assets_external (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  config_globals_external_url = request->post ["href"];
  return "";
}


string assets_external_logic_link_addon ()
{
  // Open an external link in an external browser on most clients.
  // Open an external link in a new tab in some situations.
  bool newtab = false;
#ifdef HAVE_CLOUD
  newtab = true;
#endif
#ifdef HAVE_CHROMEOS
  newtab = true;
#endif
  string addon;
  if (newtab) addon = "target=\"_blank\"";
  else addon = "class=\"external\"";
  // Done.
  return addon;
}
