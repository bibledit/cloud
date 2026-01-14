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


#include <menu/index.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <menu/logic.h>


std::string menu_index_url ()
{
  return "menu/index";
}


bool menu_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::guest);
}


std::string menu_index (Webserver_Request& webserver_request)
{
  std::string item = webserver_request.query ["item"];
  item = menu_logic_click (item);
  redirect_browser (webserver_request, item);
  return std::string();
}
