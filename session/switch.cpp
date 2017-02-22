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


#include <session/switch.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>


const char * session_switch_url ()
{
  return "session/switch";
}


bool session_switch_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string session_switch (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string page;
  
  Assets_Header header = Assets_Header (translate ("Switch user"), webserver_request);
  page += header.run ();
  
  Assets_View view;
  
  string user = request->session_logic ()->currentUser ();
  view.set_variable ("user", user);

  page += view.render ("session", "switch");

  page += Assets_Page::footer ();

  return page;
}
