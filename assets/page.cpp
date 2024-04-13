/*
Copyright (©) 2003-2024 Teus Benschop.

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


#include <cstdlib>
#include <iostream>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/url.h>
#include <config/globals.h>


namespace assets_page {


std::string header (const std::string& title, Webserver_Request& webserver_request)
{
  Assets_Header header = Assets_Header (title, webserver_request);
  return header.run ();
}


std::string success (const std::string& message)
{
  Assets_View view {};
  view.set_variable ("message", message);
  return view.render ("assets", "success");
}


std::string error (const std::string& message)
{
  Assets_View view {};
  view.set_variable ("message", message);
  return view.render ("assets", "error");
}


std::string message (const std::string& message)
{
  Assets_View view {};
  view.set_variable ("message", message);
  return view.render ("assets", "message");
}


std::string footer ()
{
  std::string page {};
  Assets_View view {};
  page += view.render ("assets", "workspacewrapper_finish");
  page += view.render ("assets", "footer");
  page += view.render ("assets", "xhtml_finish");
  return page;
}


}
