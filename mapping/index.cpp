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


std::string mapping_index_url ()
{
  return "mapping/index";
}


bool mapping_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string mapping_index (Webserver_Request& webserver_request)
{
  Database_Mappings database_mappings;
  
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Verse Mappings"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;
  std::string error;
  std::string success;

  // Create new verse mapping.
  if (webserver_request.query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Enter a name for the new verse mapping"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("new")) {
    std::string name = webserver_request.post ["entry"];
    std::vector <std::string> mappings = database_mappings.names ();
    if (find (mappings.begin(), mappings.end(), name) != mappings.end ()) {
      error = translate("This verse mapping already exists");
    } else {
      database_mappings.create (name);
    }
  }

  // Delete verse mapping.
  std::string name = webserver_request.query ["name"];
  if (webserver_request.query.count ("delete")) {
    std::string confirm = webserver_request.query ["confirm"];
    if (confirm == "") {
      Dialog_Yes dialog_yes = Dialog_Yes ("index", translate("Would you like to delete this verse mapping?"));
      dialog_yes.add_query ("name", name);
      dialog_yes.add_query ("delete", "1");
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      database_mappings.erase (name);
      success = translate ("The verse mapping was deleted");
    }
  }
  
  view.set_variable ("error", error);
  view.set_variable ("success", success);
  
  std::stringstream mappingsblock;
  std::vector <std::string> mappings = database_mappings.names ();
  for (auto & mapping : mappings) {
    mappingsblock << "<p>";
    mappingsblock << mapping;
    mappingsblock << " ";
    mappingsblock << "<a href=" << std::quoted("map?name=" + mapping) << ">[translate(" << std::quoted("edit") << "]</a>";
    mappingsblock << "</p>" << std::endl;
  }
  view.set_variable ("mappingsblock", mappingsblock.str());
  
  page += view.render ("mapping", "index");
  
  page += assets_page::footer ();
  
  return page;
}
