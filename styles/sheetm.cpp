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


#include <styles/sheetm.h>
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
#include <styles/sheets.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <styles/indexm.h>
#include <database/logic.h>


std::string styles_sheetm_url ()
{
  return "styles/sheetm";
}


bool styles_sheetm_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string styles_sheetm (Webserver_Request& webserver_request)
{
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Stylesheet"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (styles_indexm_url (), menu_logic_styles_indexm_text ());
  page = header.run ();

  Assets_View view;
  
  std::string name = webserver_request.query["name"];
  view.set_variable ("name", filter::strings::escape_special_xml_characters (name));

  Database_Styles database_styles;
  
  const std::string& username = webserver_request.session_logic ()->get_username ();
  int userlevel = webserver_request.session_logic ()->get_level ();
  bool write = database_styles.hasWriteAccess (username, name);
  if (userlevel >= Filter_Roles::admin ()) write = true;

  if (webserver_request.post.count ("new")) {
    std::string newstyle = webserver_request.post["entry"];
    std::vector <std::string> existing_markers = database_styles.getMarkers (name);
    if (find (existing_markers.begin(), existing_markers.end(), newstyle) != existing_markers.end()) {
      page += assets_page::error (translate("This style already exists"));
    } else {
      database_styles.addMarker (name, newstyle);
      styles_sheets_create_all ();
      page += assets_page::success (translate("The style has been created"));
    }
  }
  if (webserver_request.query.count("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("sheetm", translate("Please enter the name for the new style"), "", "new", "");
    dialog_entry.add_query ("name", name);
    page += dialog_entry.run ();
    return page;
  }
  
  std::string del = webserver_request.query["delete"];
  if (del != "") {
    if (write) database_styles.deleteMarker (name, del);
  }

  std::stringstream markerblock;
  std::map <std::string, std::string> markers_names = database_styles.getMarkersAndNames (name);
  for (auto & item : markers_names) {
    std::string marker = item.first;
    std::string marker_name = item.second;
    marker_name = translate (marker_name);
    markerblock << "<tr>";
    markerblock << R"(<td><a href=")" << "view?sheet=" << name << "&style=" << marker << R"(">)"  << marker << "</a></td>";
    markerblock << "<td>" << marker_name << "</td>";
    markerblock << R"(<td>[<a href=")" << "?name=" << name << "&delete=" << marker << R"(">)" << translate("delete") << "]</a></td>";
    markerblock << "</tr>";
  }
  view.set_variable ("markerblock", markerblock.str());
  
  std::string folder = filter_url_create_root_path ({database_logic_databases (), "styles", name});
  view.set_variable ("folder", folder);

  page += view.render ("styles", "sheetm");
  
  page += assets_page::footer ();
  
  return page;
}
