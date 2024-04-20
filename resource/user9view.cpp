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


#include <resource/user9view.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <menu/logic.h>
#include <database/userresources.h>
#include <access/logic.h>


std::string resource_user9view_url ()
{
  return "resource/user9view";
}


bool resource_user9view_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


std::string resource_user9view (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("User resources"), webserver_request);
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  Assets_View view;
  

  std::vector <std::string> resources = Database_UserResources::names ();
  std::vector <std::string> resourceblock;
  for (auto & resource : resources) {
    resourceblock.push_back ("<p>");
    resourceblock.push_back ("<a href=\"user1view?name=" + resource + "\">");
    resourceblock.push_back (resource);
    resourceblock.push_back ("</a>");
    resourceblock.push_back ("</p>");
  }
  view.set_variable ("resourceblock", filter::strings::implode (resourceblock, "\n"));

   
  page += view.render ("resource", "user9view");
  page += assets_page::footer ();
  return page;
}
