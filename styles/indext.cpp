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


#include <styles/indext.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <access/user.h>
#include <locale/translate.h>
#include <assets/header.h>
#include <menu/logic.h>


string styles_indext_url ()
{
  return "styles/indext";
}


bool styles_indext_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string styles_indext (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string page;

  Assets_Header header = Assets_Header (translate("Styles"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (menu_logic_settings_styles_menu (), menu_logic_styles_text ());
  page = header.run ();

  Assets_View view;

  Database_Config_User database_config_user = Database_Config_User (webserver_request);
   
  if (request->query.count ("sheet")) {
  string sheet = request->query["sheet"];
    
  if (sheet == "") {
    Dialog_List dialog_list = Dialog_List ("indext", translate("Would you like to switch to another stylesheet?"), "", "");
    Database_Styles database_styles = Database_Styles();
    vector <string> sheets = database_styles.getSheets();
    for (auto& name : sheets) {
      dialog_list.add_row (name, "sheet", name);
    }
    page += dialog_list.run ();
    return page;
  } else {
    database_config_user.setStylesheet (sheet);
    }
  }
   
  string stylesheet = database_config_user.getStylesheet();
  view.set_variable ("stylesheet", filter_string_sanitize_html (stylesheet));
  
  page += view.render ("styles", "indext");

  page += Assets_Page::footer ();

  return page;
}
