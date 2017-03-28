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
#include <config/globals.h>
#include <webserver/request.h>


string assets_external_url ()
{
  return "assets/external";
}


string assets_external (void * webserver_request)
{
  // The request from the client.
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  // Whether a URL was POSTed, that is, whether it was clicked by the user.
  string href = request->post ["href"];
  if (!href.empty ()) {
    config_globals_external_url = request->post ["href"];
    return "";
  }

  // Wait for some time till a URL is available.
  int timer = 100;
  while (timer) {
    this_thread::sleep_for (chrono::milliseconds (100));
    timer--;
    if (!config_globals_external_url.empty ()) {
      href = config_globals_external_url;
      config_globals_external_url.clear ();
      timer = 0;
    }
  }
  
  // Return the URL, if it is there.
  return href;
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
