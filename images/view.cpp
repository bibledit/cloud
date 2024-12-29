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


#include <images/view.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <journal/index.h>
#include <tasks/logic.h>
#include <journal/logic.h>
#include <menu/logic.h>
#include <database/bibleimages.h>


std::string images_view_url ()
{
  return "images/view";
}


bool images_view_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string images_view (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Bible image"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (images_view_url (), menu_logic_images_index_text ());
  page = header.run ();
  Assets_View view;
  std::string error, success;
  
  std::string image = webserver_request.query ["image"];

  view.set_variable ("image", image);
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("images", "view");
  page += assets_page::footer ();
  return page;
}
