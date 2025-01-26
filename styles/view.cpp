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


std::string styles_view_url ()
{
  return "styles/view";
}


bool styles_view_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
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

  
  database::styles1::Item marker_data = database::styles1::get_marker_data (sheet, style);
  
  
  // Whether the logged-in user has write access to the stylesheet.
  const std::string& username = webserver_request.session_logic ()->get_username ();
  const int userlevel = webserver_request.session_logic ()->get_level ();
  bool write = database::styles::has_write_access (username, sheet);
  if (userlevel >= Filter_Roles::admin ()) write = true;
  
  
  // Whether a style was edited.
  bool style_is_edited { false };

  
  // The style's name.
  std::string name = marker_data.name;
  if (webserver_request.query.count ("name")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter the name for the style"), name, "name", std::string());
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("name")) {
    name = webserver_request.post["entry"];
    if (write) {
      database::styles1::update_name (sheet, style, name);
      style_is_edited = true;
    }
  }
  view.set_variable ("name", filter::strings::escape_special_xml_characters (translate (name)));
  

  // The style's info.
  std::string info = marker_data.info;
  if (webserver_request.query.count ("info")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter the description for the style"), info, "info", std::string());
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("info")) {
    info = webserver_request.post["entry"];
    if (write) {
      database::styles1::update_info (sheet, style, info);
      style_is_edited = true;
    }
  }
  view.set_variable ("info", filter::strings::escape_special_xml_characters (translate (info)));
  
  
  // The style's category.
  std::string category = marker_data.category;
  if (webserver_request.query.count("category")) {
    category = webserver_request.query["category"];
    if (category.empty()) {
      Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change the category of this style?"),translate("Here are the various categories:"), translate("Please pick one."));
      dialog_list.add_query ("sheet", sheet);
      dialog_list.add_query ("style", style);
      dialog_list.add_row (styles_logic_category_text ("id"),  "category", "id");
      dialog_list.add_row (styles_logic_category_text ("ith"), "category", "ith");
      dialog_list.add_row (styles_logic_category_text ("ipp"), "category", "ipp");
      dialog_list.add_row (styles_logic_category_text ("ioe"), "category", "ioe");
      dialog_list.add_row (styles_logic_category_text ("t"),   "category", "t");
      dialog_list.add_row (styles_logic_category_text ("h"),   "category", "h");
      dialog_list.add_row (styles_logic_category_text ("cv"),  "category", "cv");
      dialog_list.add_row (styles_logic_category_text ("p"),   "category", "p");
      dialog_list.add_row (styles_logic_category_text ("l"),   "category", "l");
      dialog_list.add_row (styles_logic_category_text ("pe"),  "category", "pe");
      dialog_list.add_row (styles_logic_category_text ("te"),  "category", "te");
      dialog_list.add_row (styles_logic_category_text ("f"),   "category", "f");
      dialog_list.add_row (styles_logic_category_text ("x"),   "category", "x");
      dialog_list.add_row (styles_logic_category_text ("st"),  "category", "st");
      dialog_list.add_row (styles_logic_category_text ("cs"),  "category", "cs");
      dialog_list.add_row (styles_logic_category_text ("sb"),  "category", "sb");
      dialog_list.add_row (styles_logic_category_text ("sf"),  "category", "sf");
      dialog_list.add_row (styles_logic_category_text ("pm"),  "category", "pm");
      dialog_list.add_row (styles_logic_category_text (""),    "category", "");
      page += dialog_list.run ();
      return page;
    } else {
      if (write) {
        database::styles1::update_category (sheet, style, category);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("category", styles_logic_category_text(category));

  
  // The style's type.
  int type = marker_data.type;
  if (webserver_request.query.count ("type")) {
    const std::string s = webserver_request.query["type"];
    type = filter::strings::convert_to_int (s);
    if (s.empty()) {
      Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change the type of this style?"), translate("Here are the various types:"), translate("Please pick one."));
      dialog_list.add_query ("sheet", sheet);
      dialog_list.add_query ("style", style);
      for (int i = 0; i < 99; i++) {
        const std::string text = styles_logic_type_text (i);
        if (text.length () > 2) {
          dialog_list.add_row (text, "type", std::to_string (i));
        }
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write) {
        database::styles1::update_type (sheet, style, type);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("type", std::to_string (type));
  view.set_variable ("type_text", styles_logic_type_text (type));
  

  // The style's subtype.
  int subtype = marker_data.subtype;
  if (webserver_request.query.count ("subtype")) {
    const std::string s = webserver_request.query["subtype"];
    subtype = filter::strings::convert_to_int (s);
    if (s.empty()) {
      Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change the sub type of this style?"), std::string(), std::string());
      dialog_list.add_query ("sheet", sheet);
      dialog_list.add_query ("style", style);
      database::styles1::Item style_data = database::styles1::get_marker_data (sheet, style);
      int type2 = style_data.type;
      for (int i = 0; i < 99; i++) {
        std::string text = styles_logic_subtype_text (type2, i);
        if (text.length () > 2) {
          dialog_list.add_row (text, "subtype", std::to_string (i));
        }
      }
      page += dialog_list.run ();
      return page;
    } else {
      if (write) {
        database::styles1::update_sub_type (sheet, style, subtype);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("subtype", std::to_string (subtype));
  std::string subtype_text = styles_logic_subtype_text (type, subtype);
  view.set_variable ("subtype_text", subtype_text);
  if (subtype_text.length () > 2) view.enable_zone ("subtype_text");
  
  
  // The fontsize.
  if (styles_logic_fontsize_is_relevant (type, subtype)) view.enable_zone ("fontsize_relevant");
  float fontsize = marker_data.fontsize;
  if (webserver_request.query.count ("fontsize")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter a fontsize between 5 and 60 points"), filter::strings::convert_to_string (fontsize), "fontsize", translate ("The value to enter is just a number, e.g. 12."));
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("fontsize")) {
    const float fs = filter::strings::convert_to_float (webserver_request.post["entry"]);
    if ((fs >= 5) && (fs <= 60)) {
      fontsize = fs;
      if (write) {
        database::styles1::update_fontsize (sheet, style, fontsize);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("fontsize", filter::strings::convert_to_string (fontsize));


  // Italics, bold, underline, small caps relevance.
  if (styles_logic_italic_bold_underline_smallcaps_are_relevant (type, subtype)) {
    view.enable_zone ("ibus_relevant");
  }

  
  // Italic.
  int italic = marker_data.italic;
  if (webserver_request.query.count ("italic")) {
    const std::string s = webserver_request.query["italic"];
    if (s.empty()) {
      Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change whether this style is in italics?"), std::string(), std::string());
      dialog_list.add_query ("sheet", sheet);
      dialog_list.add_query ("style", style);
      database::styles1::Item style_item = database::styles1::get_marker_data (sheet, style);
      int last_value {ooitOn};
      if (styles_logic_italic_bold_underline_smallcaps_are_full (style_item.type, style_item.subtype))
        last_value = ooitToggle;
      for (int i = 0; i <= last_value; i++) {
        dialog_list.add_row (styles_logic_off_on_inherit_toggle_text (i), "italic", std::to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      italic = filter::strings::convert_to_int (s);
      if (write) {
        database::styles1::update_italic (sheet, style, italic);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("italic", styles_logic_off_on_inherit_toggle_text (italic));
  

  // Bold.
  int bold = marker_data.bold;
  if (webserver_request.query.count ("bold")) {
    const std::string s = webserver_request.query["bold"];
    if (s.empty()) {
      Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change whether this style is in bold?"), std::string(), std::string());
      dialog_list.add_query ("sheet", sheet);
      dialog_list.add_query ("style", style);
      database::styles1::Item style_data = database::styles1::get_marker_data (sheet, style);
      int last_value {ooitOn};
      if (styles_logic_italic_bold_underline_smallcaps_are_full (style_data.type, style_data.subtype))
        last_value = ooitToggle;
      for (int i = 0; i <= last_value; i++) {
        dialog_list.add_row (styles_logic_off_on_inherit_toggle_text (i), "bold", std::to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      bold = filter::strings::convert_to_int (s);
      if (write) {
        database::styles1::update_bold (sheet, style, bold);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("bold", styles_logic_off_on_inherit_toggle_text (bold));
  

  // Underline.
  int underline = marker_data.underline;
  if (webserver_request.query.count ("underline")) {
    const std::string s = webserver_request.query["underline"];
    if (s.empty()) {
      Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change whether this style is underlined?"), std::string(), std::string());
      dialog_list.add_query ("sheet", sheet);
      dialog_list.add_query ("style", style);
      database::styles1::Item style_data = database::styles1::get_marker_data (sheet, style);
      int last_value {ooitOn};
      if (styles_logic_italic_bold_underline_smallcaps_are_full (type, subtype))
        last_value = ooitToggle;
      for (int i = 0; i <= last_value; i++) {
        dialog_list.add_row (styles_logic_off_on_inherit_toggle_text (i), "underline", std::to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      underline = filter::strings::convert_to_int (s);
      if (write) {
        database::styles1::update_underline (sheet, style, underline);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("underline", styles_logic_off_on_inherit_toggle_text (underline));

  
  // Small caps.
  int smallcaps = marker_data.smallcaps;
  if (webserver_request.query.count ("smallcaps")) {
    const std::string s = webserver_request.query["smallcaps"];
    if (s.empty()) {
      Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change whether this style is in small caps?"), std::string(), std::string());
      dialog_list.add_query ("sheet", sheet);
      dialog_list.add_query ("style", style);
      database::styles1::Item style_data = database::styles1::get_marker_data (sheet, style);
      int last_value {ooitOn};
      if (styles_logic_italic_bold_underline_smallcaps_are_full (style_data.type, style_data.subtype))
        last_value = ooitToggle;
      for (int i = 0; i <= last_value; i++) {
        dialog_list.add_row (styles_logic_off_on_inherit_toggle_text (i), "smallcaps", std::to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      smallcaps = filter::strings::convert_to_int (s);
      if (write) {
        database::styles1::update_smallcaps (sheet, style, smallcaps);
        style_is_edited = true;
      }
    }
  }
  view.set_variable ("smallcaps", styles_logic_off_on_inherit_toggle_text (smallcaps));
  
  
  // Superscript.
  if (styles_logic_superscript_is_relevant (type, subtype)) view.enable_zone ("superscript_relevant");
  int superscript = marker_data.superscript;
  if (webserver_request.query.count ("superscript")) {
    superscript = filter::strings::convert_to_int (webserver_request.query["superscript"]);
    if (write) {
      database::styles1::update_superscript (sheet, style, superscript);
      style_is_edited = true;
    }
  }
  view.set_variable ("superscript_value", styles_logic_off_on_inherit_toggle_text (superscript));
  view.set_variable ("superscript_toggle", filter::strings::convert_to_string (!static_cast<bool> (superscript)));
  

  // Whether a list of the following paragraph treats are relevant.
  if (styles_logic_paragraph_treats_are_relevant (type, subtype)) view.enable_zone ("paragraph_treats_relevant");

  
  // Text alignment.
  int justification = marker_data.justification;
  if (webserver_request.query.count ("alignment")) {
    Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change the text alignment of this style?"), std::string(), std::string());
    dialog_list.add_query ("sheet", sheet);
    dialog_list.add_query ("style", style);
    for (int i = AlignmentLeft; i <= AlignmentJustify; i++) {
      dialog_list.add_row (styles_logic_alignment_text (i), "justification", std::to_string (i));
    }
    page += dialog_list.run ();
    return page;
  }
  if (webserver_request.query.count ("justification")) {
    justification = filter::strings::convert_to_int (webserver_request.query["justification"]);
    if (write) {
      database::styles1::update_justification (sheet, style, justification);
      style_is_edited = true;
    }
  }
  view.set_variable ("justification", styles_logic_alignment_text (justification));
  

  // Space before paragraph.
  float spacebefore = marker_data.spacebefore;
  if (webserver_request.query.count ("spacebefore")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter a space of between 0 and 100 mm before the paragraph"), filter::strings::convert_to_string (spacebefore), "spacebefore", translate ("This is the space before, or in other words, above the paragraph. The value to enter is just a number, e.g. 0."));
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("spacebefore")) {
    spacebefore = filter::strings::convert_to_float (webserver_request.post["entry"]);
    if (spacebefore < 0) spacebefore = 0;
    if (spacebefore > 100) spacebefore = 100;
    if (write) {
      database::styles1::update_space_before (sheet, style, spacebefore);
      style_is_edited = true;
    }
  }
  view.set_variable ("spacebefore", filter::strings::convert_to_string (spacebefore));
  

  // Space after paragraph.
  float spaceafter = marker_data.spaceafter;
  if (webserver_request.query.count ("spaceafter")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter a space of between 0 and 100 mm after the paragraph"), filter::strings::convert_to_string (spaceafter), "spaceafter", translate ("This is the space after, or in other words, below the paragraph. The value to enter is just a number, e.g. 0."));
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("spaceafter")) {
    spaceafter = filter::strings::convert_to_float (webserver_request.post["entry"]);
    if (spaceafter < 0) spaceafter = 0;
    if (spaceafter > 100) spaceafter = 100;
    if (write) {
      database::styles1::update_space_after (sheet, style, spaceafter);
      style_is_edited = true;
    }
  }
  view.set_variable ("spaceafter", filter::strings::convert_to_string (spaceafter));
  

  // Left margin.
  float leftmargin = marker_data.leftmargin;
  if (webserver_request.query.count ("leftmargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter a left margin of between -100 and 100 mm"), filter::strings::convert_to_string (leftmargin), "leftmargin", translate ("This is the left margin of the paragraph. The value to enter is just a number, e.g. 0."));
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("leftmargin")) {
    leftmargin = filter::strings::convert_to_float (webserver_request.post ["entry"]);
    if (leftmargin < 0) leftmargin = 0;
    if (leftmargin > 100) leftmargin = 100;
    if (write) {
      database::styles1::update_left_margin (sheet, style, leftmargin);
      style_is_edited = true;
    }
  }
  view.set_variable ("leftmargin", filter::strings::convert_to_string (leftmargin));

  
  // Right margin.
  float rightmargin = marker_data.rightmargin;
  if (webserver_request.query.count ("rightmargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter a right margin of between -100 and 100 mm"), filter::strings::convert_to_string (rightmargin), "rightmargin", translate ("This is the right margin of the paragraph. The value to enter is just a number, e.g. 0."));
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("rightmargin")) {
    rightmargin = filter::strings::convert_to_float (webserver_request.post["entry"]);
    if (rightmargin < -100) rightmargin = -100;
    if (rightmargin > 100) rightmargin = 100;
    if (write) {
      database::styles1::update_right_margin (sheet, style, rightmargin);
      style_is_edited = true;
    }
  }
  view.set_variable ("rightmargin", filter::strings::convert_to_string (rightmargin));
  
  
  // First line indent.
  float firstlineindent = marker_data.firstlineindent;
  if (webserver_request.query.count ("firstlineindent")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter a first line indent of between -100 and 100 mm"), filter::strings::convert_to_string (firstlineindent), "firstlineindent", translate ("This is the indent of the first line of the the paragraph. The value to enter is just a number, e.g. 0."));
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("firstlineindent")) {
    firstlineindent = filter::strings::convert_to_float (webserver_request.post["entry"]);
    if (firstlineindent < -100) firstlineindent = -100;
    if (firstlineindent > 100) firstlineindent = 100;
    if (write) {
      database::styles1::update_first_line_indent (sheet, style, firstlineindent);
      style_is_edited = true;
    }
  }
  view.set_variable ("firstlineindent", filter::strings::convert_to_string (firstlineindent));

  
  // Columns spanning.
  // This has been disabled in the GUI
  // so as not to raise the false expectation
  // that Bibledit supports columns in the exported output.
  /*
  if (styles_logic_columns_are_relevant (type, subtype)) view.enable_zone ("columns_relevant");
  bool spancolumns = marker_data.spancolumns;
  if (webserver_request.query.count ("spancolumns")) {
    spancolumns = filter::strings::convert_to_bool (webserver_request.query["spancolumns"]);
    if (write) database_styles.updateSpanColumns (sheet, style, spancolumns);
  }
  view.set_variable ("spancolumns", styles_logic_off_on_inherit_toggle_text (spancolumns));
  view.set_variable ("spancolumns_toggle", filter::strings::convert_to_string (!spancolumns));
   */

  
  // Color.
  if (styles_logic_color_is_relevant (type, subtype)) view.enable_zone ("color_relevant");
  
  std::string color = marker_data.color;
  if (webserver_request.query.count ("fgcolor")) {
    color = webserver_request.query["fgcolor"];
    if (color.find ("#") == std::string::npos) color.insert (0, "#");
    if (color.length () != 7) color = "#000000";
    if (write) {
      database::styles1::update_color (sheet, style, color);
      style_is_edited = true;
      if (style_is_edited) {};
    }
    return std::string();
  }
  view.set_variable ("color", color);

  std::string backgroundcolor {marker_data.backgroundcolor};
  if (webserver_request.query.count ("bgcolor")) {
    color = webserver_request.query["bgcolor"];
    if (color.find ("#") == std::string::npos) color.insert (0, "#");
    if (color.length () != 7) color = "#FFFFFF";
    if (write) {
      database::styles1::update_background_color (sheet, style, color);
      style_is_edited = true;
      if (style_is_edited) {};
    }
    return std::string();
  }
  view.set_variable ("backgroundcolor", backgroundcolor);


  // Whether to print this style.
  if (styles_logic_print_is_relevant (type, subtype)) view.enable_zone ("print_relevant");
  bool print {marker_data.print};
  if (webserver_request.query.count ("print")) {
    print = filter::strings::convert_to_bool (webserver_request.query["print"]);
    if (write) {
      database::styles1::update_print (sheet, style, print);
      style_is_edited = true;
    }
  }
  view.set_variable ("print", styles_logic_off_on_inherit_toggle_text (print));
  view.set_variable ("print_toggle", filter::strings::convert_to_string (!print));
  

  // Userbool1.
  std::string userbool1_function = styles_logic_get_userbool1_text (styles_logic_get_userbool1_function (type, subtype));
  if (userbool1_function.length () > 2) view.enable_zone ("userbool1_relevant");
  view.set_variable ("userbool1_function", userbool1_function);
  bool userbool1 {marker_data.userbool1};
  if (webserver_request.query.count ("userbool1")) {
    userbool1 = filter::strings::convert_to_bool (webserver_request.query["userbool1"]);
    if (write) {
      database::styles1::update_userbool1 (sheet, style, userbool1);
      style_is_edited = true;
    }
  }
  view.set_variable ("userbool1_value", styles_logic_off_on_inherit_toggle_text (userbool1));
  view.set_variable ("userbool1_toggle", filter::strings::convert_to_string (!userbool1));

  
  // Userbool2.
  std::string userbool2_function = styles_logic_get_userbool2_text (styles_logic_get_userbool2_function (type, subtype));
  if (userbool2_function.length () > 2) view.enable_zone ("userbool2_relevant");
  view.set_variable ("userbool2_function", userbool2_function);
  bool userbool2 {marker_data.userbool2};
  if (webserver_request.query.count ("userbool2")) {
    userbool2 = filter::strings::convert_to_bool (webserver_request.query["userbool2"]);
    if (write) {
      database::styles1::update_userbool2 (sheet, style, userbool2);
      style_is_edited = true;
    }
  }
  view.set_variable ("userbool2_value", styles_logic_off_on_inherit_toggle_text (userbool2));
  view.set_variable ("userbool2_toggle", filter::strings::convert_to_string (!userbool2));

  
  // Userbool3.
  std::string userbool3_function = styles_logic_get_userbool3_text (styles_logic_get_userbool3_function (type, subtype));
  if (userbool3_function.length () > 2) view.enable_zone ("userbool3_relevant");
  view.set_variable ("userbool3_function", userbool3_function);
  bool userbool3 {marker_data.userbool3};
  if (webserver_request.query.count ("userbool3")) {
    userbool3 = filter::strings::convert_to_bool (webserver_request.query["userbool3"]);
    if (write) {
      database::styles1::update_userbool3 (sheet, style, userbool3);
      style_is_edited = true;
    }
  }
  view.set_variable ("userbool3_value", styles_logic_off_on_inherit_toggle_text (userbool3));
  view.set_variable ("userbool3_toggle", filter::strings::convert_to_string (!userbool3));

  
  // Userint1.
  int userint1 = marker_data.userint1;
  switch (styles_logic_get_userint1_function (type, subtype)) {
    case UserInt1None :
      break;
    case UserInt1NoteNumbering :
      view.enable_zone ("userint1_notenumbering");
      if (webserver_request.query.count ("notenumbering")) {
        Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change the numbering of the note?"), std::string(), std::string());
        dialog_list.add_query ("sheet", sheet);
        dialog_list.add_query ("style", style);
        for (int i = NoteNumbering123; i <= NoteNumberingUser; i++) {
          dialog_list.add_row (styles_logic_note_numbering_text (i), "userint1", std::to_string (i));
        }
        page += dialog_list.run ();
        return page;
      }
      if (webserver_request.query.count ("userint1")) {
        userint1 = filter::strings::convert_to_int (webserver_request.query["userint1"]);
        if (write) {
          database::styles1::update_userint1 (sheet, style, userint1);
          style_is_edited = true;
        }
      }
      view.set_variable ("userint1", styles_logic_note_numbering_text (userint1));
      break;
    case UserInt1TableColumnNumber :
      view.enable_zone ("userint1_columnnumber");
      if (webserver_request.query.count ("userint1")) {
        Dialog_Entry dialog_entry = Dialog_Entry ("view", translate("Please enter a column number between 1 and 4"), std::to_string (userint1), "userint1", translate ("This is the column number for the style. The first columm is number 1."));
        dialog_entry.add_query ("sheet", sheet);
        dialog_entry.add_query ("style", style);
        page += dialog_entry.run ();
        return page;
      }
      if (webserver_request.post.count ("userint1")) {
        int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
        if ((value >= 1) && (value <= 4)) {
          userint1 = value;
          if (write) {
            database::styles1::update_userint1 (sheet, style, userint1);
            style_is_edited = true;
          }
        }
      }
      view.set_variable ("userint1", std::to_string (userint1));
      break;
    default: break;
  }
  
  
  // Userint2.
  int userint2 = marker_data.userint2;
  switch (styles_logic_get_userint2_function (type, subtype)) {
    case UserInt2None :
      break;
    case UserInt2NoteNumberingRestart :
      view.enable_zone ("userint2_notenumberingrestart");
      if (webserver_request.query.count ("notenumberingrestart")) {
        Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change when the note numbering restarts?"), std::string(), std::string());
        dialog_list.add_query ("sheet", sheet);
        dialog_list.add_query ("style", style);
        for (int i = NoteRestartNumberingNever; i <= NoteRestartNumberingEveryChapter; i++) {
          dialog_list.add_row (styles_logic_note_restart_numbering_text (i), "userint2", std::to_string (i));
        }
        page += dialog_list.run ();
        return page;
      }
      if (webserver_request.query.count ("userint2")) {
        userint2 = filter::strings::convert_to_int (webserver_request.query["userint2"]);
        if (write) {
          database::styles1::update_userint2 (sheet, style, userint2);
          style_is_edited = true;
        }
      }
      view.set_variable ("userint2", styles_logic_note_restart_numbering_text (userint2));
      break;
    case UserInt2EndnotePosition :
      view.enable_zone ("userint2_endnoteposition");
      if (webserver_request.query.count ("endnoteposition")) {
        Dialog_List dialog_list = Dialog_List ("view", translate("Would you like to change the position where to dump the endnotes?"), std::string(), std::string());
        dialog_list.add_query ("sheet", sheet);
        dialog_list.add_query ("style", style);
        for (int i = EndNotePositionAfterBook; i <= EndNotePositionAtMarker; i++) {
          dialog_list.add_row (styles_logic_end_note_position_text (i), "userint2", std::to_string(i));
        }
        page += dialog_list.run ();
        return page;
      }
      if (webserver_request.query.count ("userint2")) {
        userint2 = filter::strings::convert_to_int (webserver_request.query["userint2"]);
        if (write) {
          database::styles1::update_userint2 (sheet, style, userint2);
          style_is_edited = true;
        }
      }
      view.set_variable ("userint2", styles_logic_end_note_position_text (userint2));
      break;
    default: break;
  }

  
  // Userint3 not yet used.
  

  // Userstring1.
  std::string userstring1 {marker_data.userstring1};
  std::string userstring1_question {};
  std::string userstring1_help {};
  switch (styles_logic_get_userstring1_function (type, subtype)) {
    case UserString1None :
      break;
    case UserString1NoteNumberingSequence :
      if (userint1 == NoteNumberingUser) {
        view.enable_zone ("userstring1_numberingsequence");
        userstring1_question = translate("Please enter a new note numbering sequence");
        userstring1_help = translate("This gives a sequence for numbering the notes. When for example § † * is entered, the numbering goes like §, †, *, §, †, *, and so forth. Any sequence of characters can be used. Spaces should separate the characters.");
      }
      break;
    case UserString1WordListEntryAddition :
      view.enable_zone ("userstring1_wordlistaddition");
      userstring1_question = translate("Please enter a new addition to the word list entry");
      userstring1_help = translate("This given an optional string to be added after each definition in the body of text. In some Bibles the unusual words are marked with an asterisk and then explained in a glossary. If you would enter the asterisk here, or indeed any string, Bibledit would include this in the exported documents.");
      break;
    default: break;
  }
  if (webserver_request.query.count ("userstring1")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", userstring1_question, userstring1, "userstring1", userstring1_help);
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("userstring1")) {
    userstring1 = webserver_request.post["entry"];
    if (write) {
      database::styles1::update_userstring1 (sheet, style, userstring1);
      style_is_edited = true;
    }
  }
  if (userstring1.empty()) userstring1 = "--";
  view.set_variable ("userstring1", filter::strings::escape_special_xml_characters (userstring1));
  

  // Userstring2
  std::string userstring2 {marker_data.userstring2};
  std::string userstring2_question {};
  std::string userstring2_info {};
  switch (styles_logic_get_userstring2_function (type, subtype)) {
    case UserString2None :
      break;
    case UserString2DumpEndnotesHere :
      if (userint2 == EndNotePositionAtMarker) {
        view.enable_zone ("userstring2_dumpendnotes");
        userstring2_question = translate("Please enter a marker at which the endnotes should be dumped");
        userstring2_info = translate(R"(The marker is to be given without the backslash, e.g. "zendnotes".)");
      }
      break;
    default: break;
  }
  if (webserver_request.query.count ("userstring2")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("view", userstring2_question, userstring2, "userstring2", userstring2_info);
    dialog_entry.add_query ("sheet", sheet);
    dialog_entry.add_query ("style", style);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count("userstring2")) {
    userstring2 = webserver_request.post["entry"];
    if (write) {
      database::styles1::update_userstring2 (sheet, style, userstring2);
      style_is_edited = true;
    }
  }
  if (userstring2.empty()) userstring2 = "--";
  view.set_variable ("userstring2", filter::strings::escape_special_xml_characters (userstring2));
  
  // Recreate stylesheets after editing a style.
  if (style_is_edited) {
    styles_sheets_create_all ();
  }

  
  // Userstring3 not yet used.
  
  
  page += view.render ("styles", "view");
  
  page += assets_page::footer ();
  
  return page;
}
