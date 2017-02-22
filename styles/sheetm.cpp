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


string styles_sheetm_url ()
{
  return "styles/sheetm";
}


bool styles_sheetm_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string styles_sheetm (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Stylesheet"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (styles_indexm_url (), menu_logic_styles_indexm_text ());
  page = header.run ();

  Assets_View view;
  
  string name = request->query["name"];
  view.set_variable ("name", filter_string_sanitize_html (name));

  Database_Styles database_styles;
  
  string username = request->session_logic ()->currentUser ();
  int userlevel = request->session_logic ()->currentLevel ();
  bool write = database_styles.hasWriteAccess (username, name);
  if (userlevel >= Filter_Roles::admin ()) write = true;

  if (request->post.count ("new")) {
    string newstyle = request->post["entry"];
    vector <string> existing_markers = database_styles.getMarkers (name);
    if (find (existing_markers.begin(), existing_markers.end(), newstyle) != existing_markers.end()) {
      page += Assets_Page::error (translate("This style already exists"));
    } else {
      database_styles.addMarker (name, newstyle);
      styles_sheets_create_all ();
      page += Assets_Page::success (translate("The style has been created"));
    }
  }
  if (request->query.count("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("sheetm", translate("Please enter the name for the new style"), "", "new", "");
    dialog_entry.add_query ("name", name);
    page += dialog_entry.run ();
    return page;
  }
  
  string del = request->query["delete"];
  if (del != "") {
    if (write) database_styles.deleteMarker (name, del);
  }

  vector <string> markerblock;
  map <string, string> markers_names = database_styles.getMarkersAndNames (name);
  for (auto & item : markers_names) {
    string marker = item.first;
    string marker_name = item.second;
    markerblock.push_back ("<tr>");
    markerblock.push_back ("<td><a href=\"view?sheet=" + name + "&style=" + marker + "\">" + marker + "</a></td>");
    markerblock.push_back ("<td>" + marker_name + "</td>");
    markerblock.push_back ("<td>[<a href=\"?name=" + name + "&delete=" + marker + "\">" + translate("delete") + "]</a></td>");
    markerblock.push_back ("</tr>");
  }
  view.set_variable ("markerblock", filter_string_implode (markerblock, "\n"));
  
  page += view.render ("styles", "sheetm");
  
  page += Assets_Page::footer ();
  
  return page;
}
