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


#include <edit/styles.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <editor/styles.h>
#include <access/bible.h>


std::string edit_styles_url ()
{
  return "edit/styles";
}


bool edit_styles_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string edit_styles (Webserver_Request& webserver_request)
{
  if (webserver_request.query.count ("style")) {
    std::string style = webserver_request.query["style"];
    Editor_Styles::record_usage (webserver_request, style);
    std::string action = Editor_Styles::get_action (webserver_request, style);
    return style + "\n" + action;
  }
  
  
  if (webserver_request.query.count ("all")) {
    return Editor_Styles::get_all (webserver_request);
  }
  
  
  return Editor_Styles::get_recently_used (webserver_request);
}

