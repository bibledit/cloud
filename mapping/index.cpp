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


string mapping_index_url ()
{
  return "mapping/index";
}


bool mapping_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string mapping_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Mappings database_mappings;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Verse Mappings"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;
  string error;
  string success;

  // Create new verse mapping.
  if (request->query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Enter a name for the new verse mapping"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("new")) {
    string name = request->post ["entry"];
    vector <string> mappings = database_mappings.names ();
    if (find (mappings.begin(), mappings.end(), name) != mappings.end ()) {
      error = translate("This verse mapping already exists");
    } else {
      database_mappings.create (name);
    }
  }

  // Delete verse mapping.
  string name = request->query ["name"];
  if (request->query.count ("delete")) {
    string confirm = request->query ["confirm"];
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
  
  string mappingsblock;
  vector <string> mappings = database_mappings.names ();
  for (auto & mapping : mappings) {
    mappingsblock.append ("<p>");
    mappingsblock.append (mapping);
    mappingsblock.append (" ");
    mappingsblock.append ("<a href=\"map?name=" + mapping + "\">[translate(\"edit\")]</a>");
    mappingsblock.append ("</p>\n");
  }
  view.set_variable ("mappingsblock", mappingsblock);
  
  page += view.render ("mapping", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}
