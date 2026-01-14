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


#include <styles/sheetm.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
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
  return roles::access_control (webserver_request, roles::translator);
}


std::string styles_sheetm (Webserver_Request& webserver_request)
{
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Stylesheet"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (styles_indexm_url (), menu_logic_styles_indexm_text ());
  page = header.run ();
  
  Assets_View view;
  
  // The name of the stylesheet.
  const std::string name = webserver_request.query["name"];
  view.set_variable ("name", filter::string::escape_special_xml_characters (name));
  
  // Whether this user has write access to the stylesheet.
  const std::string& username = webserver_request.session_logic ()->get_username ();
  const int userlevel = webserver_request.session_logic ()->get_level ();
  bool write = database::styles::has_write_access (username, name);
  if (userlevel >= roles::admin) write = true;
  
  const std::string del = webserver_request.query["delete"];
  if (!del.empty())
    if (write) {
      database::styles::delete_marker (name, del);
    }

  pugi::xml_document html_block {};

  // List the styles v2 in the overview.
  {
    const auto get_and_sort_markers_v2 = [] (const auto& name) {
      std::vector<std::string> markers {database::styles::get_markers (name)};
      std::vector<std::string> sorted_markers{};
      for (const stylesv2::Style& style : stylesv2::styles) {
        const std::string& marker = style.marker;
        if (const auto iter = std::find(markers.cbegin(), markers.cend(), marker); iter != markers.cend()) {
          sorted_markers.push_back(marker);
          markers.erase(iter);
        }
      }
      sorted_markers.insert(sorted_markers.cend(), markers.cbegin(), markers.cend());
      return sorted_markers;
    };
    const std::vector<std::string> markers_v2 {get_and_sort_markers_v2 (name)};
    auto previous_category {stylesv2::Category::unknown};
    for (const auto& marker : markers_v2) {
      const stylesv2::Style* style {database::styles::get_marker_data (name, marker)};
      if (style->category != previous_category) {
        pugi::xml_node tr_node = html_block.append_child("tr");
        tr_node.append_child("td");
        pugi::xml_node td_node = tr_node.append_child("td");
        pugi::xml_node h_node = td_node.append_child("h3");
        std::stringstream ss {};
        ss << style->category;
        h_node.text().set(ss.str().c_str());
        previous_category = style->category;
      }
      pugi::xml_node tr_node = html_block.append_child("tr");
      {
        pugi::xml_node td_node = tr_node.append_child("td");
        pugi::xml_node a_node = td_node.append_child("a");
        const std::string href = "view?sheet=" + name + "&style=" + marker;
        a_node.append_attribute("href") = href.c_str();
        a_node.text().set(marker.c_str());
      }
      {
        pugi::xml_node td_node = tr_node.append_child("td");
        td_node.text().set(style->name.c_str());
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
  }
  
  // Generate the html and set it on the page.
  std::stringstream ss {};
  html_block.print (ss, "", pugi::format_raw);
  view.set_variable ("markerblock", ss.str());
  
  const std::string folder = filter_url_create_root_path ({database_logic_databases (), "styles", name});
  view.set_variable ("folder", folder);
  
  page += view.render ("styles", "sheetm");
  
  page += assets_page::footer ();
  
  return page;
}
