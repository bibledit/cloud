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


string versification_index_url ()
{
  return "versification/index";
}


bool versification_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string versification_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Versifications"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;

  Database_Versifications database_versifications = Database_Versifications();

  if (request->post.count ("new")) {
    string name = request->post["entry"];
    database_versifications.createSystem (name);
  }
  if (request->query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter the name for the new versification system"), "", "new", "");
    page += dialog_entry.run();
    return page;
  }

  if (request->query.count ("delete")) {
    string name = request->query ["delete"];
    string confirm = request->query ["confirm"];
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

  string systemblock;
  vector <string> systems = database_versifications.getSystems();
  for (auto & system : systems) {
    systemblock.append ("<p><a href=\"system?name=" + system + "\">" + system + "</a></p>\n");
  }
  view.set_variable ("systemblock", systemblock);
  
  page += view.render ("versification", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}

