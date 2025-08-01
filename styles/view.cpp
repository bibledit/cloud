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


#include <styles/view.h>
#include <styles/logic.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/select.h>
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
#include <assets/external.h>


std::string styles_view_url ()
{
  return "styles/view";
}


bool styles_view_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string styles_view (Webserver_Request& webserver_request)
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
    const stylesv2::Style* marker_data_ptr = database::styles::get_marker_data (sheet, style);
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
  if (userlevel >= roles::admin) write = true;


  if (webserver_request.query.count ("reset")) {
    if (write) {
      database::styles::reset_marker(sheet, style);
      styles_sheets_create_all();
      std::string url = filter_url_build_http_query(styles_view_url(), "sheet", sheet);
      url = filter_url_build_http_query(std::move(url), "style", style);
      redirect_browser (webserver_request, std::move(url));
    }
  }
  
  
  // Whether a style was edited.
  bool style_is_edited { false };
  
  
  // Function to replace an empty string with two hyphens.
  // This is needed to remain editable.
  const auto empty_to_dashes = [] (const auto value) -> std::string {
    if (!value.empty())
      return value;
    return "--";
  };
  
  
  // The style's name.
  if (webserver_request.query.count ("name")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter the name for the style"), marker_data.name, "name", std::string());
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("name")) {
    marker_data.name = webserver_request.post["entry"];
    style_is_edited = true;
  }
  view.set_variable ("name", empty_to_dashes(filter::strings::escape_special_xml_characters (translate (marker_data.name))));

  
  // The style's info.
  if (webserver_request.query.count ("info")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter the description for the style"), marker_data.info, "info", std::string());
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("info")) {
    marker_data.info = webserver_request.post["entry"];
    style_is_edited = true;
  }
  view.set_variable ("info", empty_to_dashes(filter::strings::escape_special_xml_characters (translate (marker_data.info))));

  
  // Handle toggle of checkbox.
  const std::string checkbox = webserver_request.post ["checkbox"];
  if (!checkbox.empty()) {
    const bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);
    sheet = webserver_request.post ["val1"];
    style = webserver_request.post ["val2"];
    marker_data = *(database::styles::get_marker_data (sheet, style));
    const stylesv2::Property property = stylesv2::property_value_to_enum (checkbox);
    marker_data.properties[property] = checked;
    style_is_edited = true;
  }

  
  // Enable the sections in the editor for the paragraph style properties.
  if (marker_data.paragraph) {
    view.enable_zone("paragraph");
    
    // Handle paragraph font size in points.
    if (const std::string fontsize = webserver_request.post ["fontsize"]; !fontsize.empty()) {
      marker_data.paragraph.value().font_size = std::clamp(filter::strings::convert_to_int(fontsize), 5, 50);
      style_is_edited = true;
    }
    view.set_variable("fontsize", std::to_string(marker_data.paragraph.value().font_size));

    // Handle paragraph italics.
    {
      constexpr const char* identification {"italic"};
      if (webserver_request.post.count (identification)) {
        marker_data.paragraph.value().italic = stylesv2::twostate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_two_states()) {
        states.emplace_back(twostate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = twostate_enum_to_value(marker_data.paragraph.value().italic),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }
    
    // Handle paragraph bold.
    {
      constexpr const char* identification {"bold"};
      if (webserver_request.post.count (identification)) {
        marker_data.paragraph.value().bold = stylesv2::twostate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_two_states()) {
        states.emplace_back(twostate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = twostate_enum_to_value(marker_data.paragraph.value().bold),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }

    // Handle paragraph underline.
    {
      constexpr const char* identification {"underline"};
      if (webserver_request.post.count (identification)) {
        marker_data.paragraph.value().underline = stylesv2::twostate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_two_states()) {
        states.emplace_back(twostate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = twostate_enum_to_value(marker_data.paragraph.value().underline),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }
    
    // Handle paragraph small caps.
    {
      constexpr const char* identification {"smallcaps"};
      if (webserver_request.post.count (identification)) {
        marker_data.paragraph.value().smallcaps = stylesv2::twostate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_two_states()) {
        states.emplace_back(twostate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = twostate_enum_to_value(marker_data.paragraph.value().smallcaps),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }

    // Handle text alignment.
    {
      constexpr const char* identification {"textalignment"};
      if (webserver_request.post.count (identification)) {
        marker_data.paragraph.value().text_alignment = stylesv2::textalignment_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> alignments;
      for (const auto alignment2 : stylesv2::get_text_alignments()) {
        alignments.emplace_back(textalignment_enum_to_value(alignment2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = alignments,
        .selected = textalignment_enum_to_value(marker_data.paragraph.value().text_alignment),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }

    // Handle space before in millimeters.
    if (const std::string space_before = webserver_request.post ["spacebefore"]; !space_before.empty()) {
      marker_data.paragraph.value().space_before = std::clamp(filter::strings::convert_to_float(space_before), 0.0f, 100.0f);
      style_is_edited = true;
    }
    // Precision = 1, i.e. one number after the digit.
    view.set_variable("spacebefore", filter::strings::convert_to_string(marker_data.paragraph.value().space_before, 1));

    // Handle space after in millimeters.
    if (const std::string space_after = webserver_request.post ["spaceafter"]; !space_after.empty()) {
      marker_data.paragraph.value().space_after = std::clamp(filter::strings::convert_to_float(space_after), 0.0f, 100.0f);
      style_is_edited = true;
    }
    // Precision = 1.
    view.set_variable("spaceafter", filter::strings::convert_to_string(marker_data.paragraph.value().space_after, 1));

    // Handle left margin in millimeters.
    if (const std::string left_margin = webserver_request.post ["leftmargin"]; !left_margin.empty()) {
      marker_data.paragraph.value().left_margin = std::clamp(filter::strings::convert_to_float(left_margin), 0.0f, 100.0f);
      style_is_edited = true;
    }
    // Precision = 1.
    view.set_variable("leftmargin", filter::strings::convert_to_string(marker_data.paragraph.value().left_margin, 1));

    // Handle right margin in millimeters.
    if (const std::string right_margin = webserver_request.post ["rightmargin"]; !right_margin.empty()) {
      marker_data.paragraph.value().right_margin = std::clamp(filter::strings::convert_to_float(right_margin), -100.0f, 100.0f);
      style_is_edited = true;
    }
    // Precision = 1.
    view.set_variable("rightmargin", filter::strings::convert_to_string(marker_data.paragraph.value().right_margin, 1));

    // Handle first line indent in millimeters.
    if (const std::string first_line_indent = webserver_request.post ["firstlineindent"]; !first_line_indent.empty()) {
      marker_data.paragraph.value().first_line_indent = std::clamp(filter::strings::convert_to_float(first_line_indent), -100.0f, 100.0f);
      style_is_edited = true;
    }
    // Precision = 1.
    view.set_variable("firstlineindent", filter::strings::convert_to_string(marker_data.paragraph.value().first_line_indent, 1));
  }
  
  
  // Enable the sections in the editor for the character style properties.
  if (marker_data.character) {
    view.enable_zone("character");
    bool enable_color = true;
    if (marker_data.type == stylesv2::Type::footnote_wrapper)
      enable_color = false;
    if (marker_data.type == stylesv2::Type::endnote_wrapper)
      enable_color = false;
    if (enable_color)
      view.enable_zone("character_color");

    // Handle character italics.
    {
      constexpr const char* identification {"italic"};
      if (webserver_request.post.count (identification)) {
        marker_data.character.value().italic = stylesv2::fourstate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_four_states()) {
        states.emplace_back(fourstate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = fourstate_enum_to_value(marker_data.character.value().italic),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }

    // Handle character bold.
    {
      constexpr const char* identification {"bold"};
      if (webserver_request.post.count (identification)) {
        marker_data.character.value().bold = stylesv2::fourstate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_four_states()) {
        states.emplace_back(fourstate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = fourstate_enum_to_value(marker_data.character.value().bold),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }

    // Handle character underline.
    {
      constexpr const char* identification {"underline"};
      if (webserver_request.post.count (identification)) {
        marker_data.character.value().underline = stylesv2::fourstate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_four_states()) {
        states.emplace_back(fourstate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = fourstate_enum_to_value(marker_data.character.value().underline),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }

    // Handle character small caps.
    {
      constexpr const char* identification {"smallcaps"};
      if (webserver_request.post.count (identification)) {
        marker_data.character.value().smallcaps = stylesv2::fourstate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_four_states()) {
        states.emplace_back(fourstate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = fourstate_enum_to_value(marker_data.character.value().smallcaps),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }

    // Handle character superscript.
    {
      constexpr const char* identification {"superscript"};
      if (webserver_request.post.count (identification)) {
        marker_data.character.value().superscript = stylesv2::twostate_value_to_enum(webserver_request.post.at(identification));
        style_is_edited = true;
      }
      std::vector<std::string> states;
      for (const auto state2 : stylesv2::get_two_states()) {
        states.emplace_back(twostate_enum_to_value(state2));
      }
      dialog::select::Settings settings {
        .identification = identification,
        .values = states,
        .selected = twostate_enum_to_value(marker_data.character.value().superscript),
        .parameters = { {"sheet", sheet}, {"style", style} },
      };
      dialog::select::Form form { .auto_submit = true };
      view.set_variable(identification, dialog::select::form(settings, form));
    }
    
    // Function to fix color input.
    const auto fix_color = [](std::string color, const char* value) {
      if (color.find ("#") == std::string::npos)
        color.insert (0, "#");
      if (color.length () != 7)
        color = value;
      return color;
    };

    // Handle foreground color.
    if (webserver_request.query.count ("fgcolor")) {
      std::string color = webserver_request.query["fgcolor"];
      color = fix_color(color, "#000000");
      marker_data.character.value().foreground_color = color;
      style_is_edited = true;
    }
    view.set_variable ("foregroundcolor", marker_data.character.value().foreground_color);
    
    // Handle background color..
    if (webserver_request.query.count ("bgcolor")) {
      std::string color = webserver_request.query["bgcolor"];
      color = fix_color(color, "#FFFFFF");
      marker_data.character.value().background_color = color;
      style_is_edited = true;
    }
    view.set_variable ("backgroundcolor", marker_data.character.value().background_color);
  }
  
  
  // Redirect the browser to a clean styles editor without any previous settings made via the URL.
  const auto redirect = [&webserver_request, &sheet, &style] () {
    std::string query = filter_url_build_http_query (styles_view_url (), "sheet", sheet);
    query = filter_url_build_http_query (query, "style", style);
    redirect_browser (webserver_request, query);
  };

  
  // Handle note numbering sequence.
  {
    const std::string note_numbering_sequence {stylesv2::property_enum_to_value (stylesv2::Property::note_numbering_sequence)};
    if (webserver_request.post.count(note_numbering_sequence)) {
      marker_data.properties[stylesv2::Property::note_numbering_sequence] = webserver_request.post[note_numbering_sequence];
      style_is_edited = true;
    }
    if (webserver_request.query.count("numerical")) {
      marker_data.properties[stylesv2::Property::note_numbering_sequence] = "1 2 3 4 5 6 7 8 9";
      style_is_edited = true;
      redirect();
    }
    if (webserver_request.query.count("alphabetical")) {
      marker_data.properties[stylesv2::Property::note_numbering_sequence] = "a b c d e f g h i j k l m n o p q r s t u v w x y z";
      style_is_edited = true;
      redirect();
    }
    view.set_variable (note_numbering_sequence, stylesv2::get_parameter<std::string>(&marker_data, stylesv2::Property::note_numbering_sequence));
  }

  
  // Handle footnote numbering restart.
  {
    constexpr const char* identification {"noterestart"};
    if (webserver_request.post.count (identification)) {
      marker_data.properties[stylesv2::Property::note_numbering_restart] = webserver_request.post.at(identification);
      style_is_edited = true;
    }
    const std::vector<std::string> values {
      stylesv2::notes_numbering_restart_never,
      stylesv2::notes_numbering_restart_book,
      stylesv2::notes_numbering_restart_chapter
    };
    dialog::select::Settings settings {
      .identification = identification,
      .values = values,
      .selected = stylesv2::get_parameter<std::string>(&marker_data, stylesv2::Property::note_numbering_restart),
      .parameters = { {"sheet", sheet}, {"style", style} },
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }

  
  // Handle endnotes dump location.
  {
    const std::string notes_dump {stylesv2::property_enum_to_value (stylesv2::Property::notes_dump)};
    if (webserver_request.post.count(notes_dump)) {
      marker_data.properties[stylesv2::Property::notes_dump] = stylesv2::validate_notes_dump(webserver_request.post[notes_dump]);
      style_is_edited = true;
    }
    if (webserver_request.query.count("afterbook")) {
      marker_data.properties[stylesv2::Property::notes_dump] = "book";
      style_is_edited = true;
      redirect();
    }
    if (webserver_request.query.count("veryend")) {
      marker_data.properties[stylesv2::Property::notes_dump] = "end";
      style_is_edited = true;
      redirect();
    }
    if (webserver_request.query.count("zendnotes")) {
      marker_data.properties[stylesv2::Property::notes_dump] = R"(\zendnotes)";
      style_is_edited = true;
      redirect();
    }
    view.set_variable (notes_dump, stylesv2::get_parameter<std::string>(&marker_data, stylesv2::Property::notes_dump));
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

  
  // Set the style's documentation.
  view.set_variable("doc", marker_data.doc.empty() ? "https://ubsicap.github.io/usfm" : marker_data.doc);
  view.set_variable("external", assets_external_logic_link_addon());

  
  // A style can be reset to its default values if the style's marker is among the default styles.
  if (std::find(stylesv2::styles.cbegin(), stylesv2::styles.cend(), style) != stylesv2::styles.cend())
    view.enable_zone("reset");
  else
    view.enable_zone("noreset");

  
  // If a style is edited, save it, and recreate cascaded stylesheets.
  if (style_is_edited) {
    if (write) {
      if (!marker_data.marker.empty())
        database::styles::save_style(sheet, marker_data);
      styles_sheets_create_all();
    }
  }


  page += view.render ("styles", "view");
  page += assets_page::footer ();
  return page;
}
