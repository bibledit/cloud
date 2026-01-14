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


#include <versification/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <webserver/request.h>
#include <database/versifications.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <assets/header.h>
#include <menu/logic.h>


std::string versification_index_url ()
{
  return "versification/index";
}


bool versification_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string versification_index (Webserver_Request& webserver_request)
{
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Versifications"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;

  Database_Versifications database_versifications = Database_Versifications();

  if (webserver_request.post_count("new")) {
    std::string name = webserver_request.post_get("entry");
    database_versifications.createSystem (name);
  }
  if (webserver_request.query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter the name for the new versification system"), "", "new", "");
    page += dialog_entry.run();
    return page;
  }

  if (webserver_request.query.count ("delete")) {
    std::string name = webserver_request.query ["delete"];
    std::string confirm = webserver_request.query ["confirm"];
    if (confirm == "yes") {
      database_versifications.erase (name);
    } else if (confirm == "cancel") {
    } else {
      Dialog_Yes dialog_yes = Dialog_Yes ("index", translate("Would you like to delete this versification system?"));
      dialog_yes.add_query ("delete", name);
      page += dialog_yes.run ();
      return page;
    }
  }

  std::stringstream systemblock;
  std::vector <std::string> systems = database_versifications.getSystems();
  for (auto & system : systems) {
    systemblock << "<p><a href=" << std::quoted ("system?name=" + system) << ">" << system << "</a></p>" << std::endl;
  }
  view.set_variable ("systemblock", systemblock.str());
  
  page += view.render ("versification", "index");
  
  page += assets_page::footer ();
  
  return page;
}

