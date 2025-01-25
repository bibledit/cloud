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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


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
      database::styles1::add_marker (name, newstyle);
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
  
  const std::string del = webserver_request.query["delete"];
  if (!del.empty())
    if (write)
      database::styles1::delete_marker (name, del);
  
  const std::map <std::string, std::string> markers_names = database_styles.getMarkersAndNames (name);
  pugi::xml_document document {};
  for (const auto& item : markers_names) {
    const std::string marker = item.first;
    const std::string marker_name = translate(item.second);
    pugi::xml_node tr_node = document.append_child("tr");
    {
      pugi::xml_node td_node = tr_node.append_child("td");
      pugi::xml_node a_node = td_node.append_child("a");
      const std::string href = "view?sheet=" + name + "&style=" + marker;
      a_node.append_attribute("href") = href.c_str();
      a_node.text().set(marker.c_str());
    }
    {
      pugi::xml_node td_node = tr_node.append_child("td");
      td_node.text().set(marker_name.c_str());
    }
    {
      pugi::xml_node td_node = tr_node.append_child("td");
      td_node.append_child("span").text().set("[");
      pugi::xml_node a_node = td_node.append_child("a");
      const std::string href = "?name=" + name + "&delete=" + marker;
      a_node.append_attribute("href") = href.c_str();
      a_node.text().set(translate("delete").c_str());
      td_node.append_child("span").text().set("]");
    }
  }
  {
    std::stringstream ss {};
    document.print (ss, "", pugi::format_raw);
    view.set_variable ("markerblock", ss.str());
  }
  
  std::string folder = filter_url_create_root_path ({database_logic_databases (), "styles", name});
  view.set_variable ("folder", folder);
  
  page += view.render ("styles", "sheetm");
  
  page += assets_page::footer ();
  
  return page;
}
