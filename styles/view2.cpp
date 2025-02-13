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


#include <styles/view2.h>
#include <styles/logic.h>
#include <stylesv2/logic.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <dialog/color.h>
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


std::string styles_view2_url ()
{
  return "styles/view2";
}


bool styles_view2_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string styles_view2 (Webserver_Request& webserver_request)
{
  std::string page {};
  
  Assets_Header header = Assets_Header (translate("Style"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (styles_indexm_url (), menu_logic_styles_indexm_text ());
  page = header.run ();
  
  Assets_View view {};


  std::string sheet = webserver_request.query ["sheet"];
  view.set_variable ("sheet", filter::strings::escape_special_xml_characters (sheet));
  
  
  std::string style = webserver_request.query ["style"];
  view.set_variable ("style", filter::strings::escape_special_xml_characters (style));

  
  // Get the data for the marker.
  // If no data was found, use a default style with the marker set to the requested marker.
  // It means that the marker data is always set with something.
  stylesv2::Style marker_data;
  {
    const stylesv2::Style* marker_data_ptr = database::styles2::get_marker_data (sheet, style);
    if (marker_data_ptr) {
      marker_data = *marker_data_ptr;
    } else {
      const stylesv2::Style default_style;
      marker_data = default_style;
      marker_data.marker = style;
    }
  }

  
  view.set_variable ("type", stylesv2::type_enum_to_value(marker_data.type, true));
  
  
  // Whether the logged-in user has write access to the stylesheet.
  const std::string& username = webserver_request.session_logic ()->get_username ();
  const int userlevel = webserver_request.session_logic ()->get_level ();
  bool write = database::styles::has_write_access (username, sheet);
  if (userlevel >= Filter_Roles::admin ()) write = true;

  
  // Whether a style was edited.
  bool style_is_edited { false };
  
  
  // The style's name.
  if (webserver_request.query.count ("name")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view2", translate("Please enter the name for the style"), marker_data.name, "name", std::string());
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("name")) {
    marker_data.name = webserver_request.post["entry"];
    style_is_edited = true;
  }
  view.set_variable ("name", filter::strings::escape_special_xml_characters (translate (marker_data.name)));

  
  // The style's info.
  if (webserver_request.query.count ("info")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view2", translate("Please enter the description for the style"), marker_data.info, "info", std::string());
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("info")) {
    marker_data.info = webserver_request.post["entry"];
    style_is_edited = true;
  }
  view.set_variable ("info", filter::strings::escape_special_xml_characters (translate (marker_data.info)));

  
  // Handle toggle of checkbox.
  const std::string checkbox = webserver_request.post ["checkbox"];
  if (!checkbox.empty()) {
    const bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);
    sheet = webserver_request.post ["val1"];
    style = webserver_request.post ["val2"];
    marker_data = *(database::styles2::get_marker_data (sheet, style));
    const stylesv2::Property property = stylesv2::property_value_to_enum (checkbox);
    marker_data.properties[property] = checked;
    style_is_edited = true;
  }

  
  // Enable the section(s) in the editor for the capabilities.
  // Set the values correctly for in the html page.
  for (const auto& [property, parameter] : marker_data.properties) {
    const std::string enum_value = property_enum_to_value(property);
    view.enable_zone(enum_value);
    if (std::holds_alternative<bool>(parameter))
      view.set_variable (enum_value, filter::strings::get_checkbox_status(std::get<bool>(parameter)));
    if (std::holds_alternative<int>(parameter))
      view.set_variable (enum_value, std::to_string(std::get<int>(parameter)));
    if (std::holds_alternative<std::string>(parameter))
      view.set_variable (enum_value, std::get<std::string>(parameter));
  }

  
  // If a style is edited, save it, and recreate cascaded stylesheets.
  if (style_is_edited) {
    if (write) {
      if (!marker_data.marker.empty())
        database::styles2::save_style(sheet, marker_data);
      styles_sheets_create_all ();
    }
  }


  page += view.render ("styles", "view2");
  page += assets_page::footer ();
  return page;
}
