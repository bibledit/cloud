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


#include <assets/external.h>
#include <config/globals.h>
#include <webserver/request.h>


std::string assets_external_url ()
{
  return "assets/external";
}


std::string assets_external (Webserver_Request& webserver_request)
{
  // Whether a URL was POSTed, that is, whether it was clicked by the user.
  std::string href = webserver_request.post_get("href");
  if (!href.empty ()) {
    config_globals_external_url = webserver_request.post_get("href");
    return std::string();
  }

  // Wait for some time till a URL is available.
  int timer {100};
  while (timer) {
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
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


std::string assets_external_logic_link_addon ()
{
  // Open an external link in an external browser on most clients.
  // Open an external link in a new tab in some situations.
#ifdef HAVE_CLOUD
  return R"(target="_blank")";
#endif
  return R"(class="external")";
}
