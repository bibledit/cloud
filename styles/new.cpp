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


#include <styles/new.h>
#include <styles/view.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/select.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/config/user.h>
#include <access/user.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <styles/indexm.h>
#include <database/logic.h>


std::string styles_new_url ()
{
  return "styles/new";
}


bool styles_new_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string styles_new (Webserver_Request& webserver_request)
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
  
  // Allowed characters in the style.
  constexpr const std::string_view allowed {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789-"};
  
  // Handle new style submission.
  if (webserver_request.post_count("style")) {
    const std::string new_style = webserver_request.post_get("style");
    const std::string base_style = webserver_request.post_get("base");
    const std::vector<std::string> markers = database::styles::get_markers(name);
    if (new_style.empty()) {
      page.append(assets_page::error (translate("Enter a name for the new style")));
    }
    else if (std::any_of(new_style.cbegin(), new_style.cend(), [&allowed](char c) {
      return allowed.find(c) == std::string::npos;
    }))
      page.append(assets_page::error (translate("Allowed characters for the style are:") + " " + std::string(allowed)));
    else if (base_style.empty()) {
      page.append(assets_page::error (translate("Select an existing style to base the new style on")));
    }
    else if (filter::string::in_array(new_style, markers)) {
      page.append(assets_page::error (translate("The style already exists in the stylesheet")));
    }
    else if (!write) {
      page.append(assets_page::error (translate("You don't have sufficient privileges to add a style to the stylesheet")));
    }
    else {
      // Create the marker.
      database::styles::add_marker (name, new_style, base_style);
      // Recreate all stylesheets.
      styles_sheets_create_all ();
      // Redirect to the page editing this style.
      const std::string location = filter_url_build_http_query(styles_view_url(), {
        {"sheet", name},
        {"style", new_style},
      });
      redirect_browser (webserver_request, location);
    }
  }
  
  // Generate the option html for all possible base styles and set it on the page.
  {
    std::vector<std::string> markers{database::styles::get_markers(stylesv2::standard_sheet())};
    markers.push_back(std::string());
    std::sort(markers.begin(), markers.end());
    for (const auto& marker : markers)
      view.add_iteration ("option", { std::pair ("style", marker) } );
  }
  
  page += view.render ("styles", "new");
  
  page += assets_page::footer ();
  
  return page;
}
