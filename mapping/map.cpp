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


#include <mapping/map.h>
#include <mapping/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <webserver/request.h>
#include <database/mappings.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <assets/header.h>
#include <menu/logic.h>


string mapping_map_url ()
{
  return "mapping/map";
}


bool mapping_map_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string mapping_map (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Mappings database_mappings;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Verse mappings"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (mapping_index_url (), menu_logic_mapping_index_text ());

  page = header.run ();

  Assets_View view;
  string success;
  
  string name = request->query["name"];
  view.set_variable ("name", name);

  if (request->post.count ("submit")) {
    string data = request->post["data"];
    database_mappings.import (name, data);
    success = translate("The verse mapping was saved");
  }

  view.set_variable ("success", success);

  string data = database_mappings.output (name);
  view.set_variable ("data", data);
  
  page += view.render ("mapping", "map");
  
  page += Assets_Page::footer ();
  
  return page;
}
