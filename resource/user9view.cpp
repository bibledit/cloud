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


string resource_user9view_url ()
{
  return "resource/user9view";
}


bool resource_user9view_acl (void * webserver_request)
{
  return access_logic_privilege_view_resources (webserver_request);
}


string resource_user9view (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate("User resources"), request);
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  Assets_View view;
  

  vector <string> resources = Database_UserResources::names ();
  vector <string> resourceblock;
  for (auto & resource : resources) {
    resourceblock.push_back ("<p>");
    resourceblock.push_back ("<a href=\"user1view?name=" + resource + "\">");
    resourceblock.push_back (resource);
    resourceblock.push_back ("</a>");
    resourceblock.push_back ("</p>");
  }
  view.set_variable ("resourceblock", filter_string_implode (resourceblock, "\n"));

   
  page += view.render ("resource", "user9view");
  page += Assets_Page::footer ();
  return page;
}
