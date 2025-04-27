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


#include <manage/exports.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/archive.h>
#include <filter/usfm.h>
#include <dialog/list.h>
#include <dialog/entry.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <export/logic.h>
#include <database/config/bible.h>
#include <database/state.h>
#include <client/logic.h>
#include <tasks/logic.h>
#include <menu/logic.h>
#include <assets/external.h>
#include <locale/logic.h>
#include <styles/logic.h>
#include <webserver/request.h>


const char * manage_exports_url ()
{
  return "manage/exports";
}


bool manage_exports_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string space_href (std::string name)
{
  name = filter::strings::replace ("-", "", name);
  name = filter::strings::replace (" ", "", name);
  return name;
}


std::string manage_exports (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate ("Export"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  if (webserver_request.query.count ("bible")) {
    std::string bible = webserver_request.query["bible"];
    if (bible.empty()) {
      Dialog_List dialog_list = Dialog_List ("exports", translate("Select a Bible"), "", "");
      std::vector <std::string> bibles = access_bible::bibles (webserver_request);
      for (const auto& bible2 : bibles) {
        dialog_list.add_row (bible2, "bible", bible2);
      }
      page += dialog_list.run ();
      return page;
    } else {
      webserver_request.database_config_user()->setBible (bible);
    }
  }
  
  
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  view.set_variable ("bible", bible);
  
  
  std::string checkbox = webserver_request.post ["checkbox"];
  bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);
  
  
  if (webserver_request.query.count ("remove")) {
    std::string directory = export_logic::bible_directory (bible);
    filter_url_rmdir (directory);
    Database_State::setExport (bible, 0, export_logic::export_needed);
    view.set_variable ("success", translate("The export has been removed."));
  }
  
  
  if (checkbox == "web") {
    database::config::bible::set_export_web_during_night (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
    return std::string();
  }
  view.set_variable ("web", filter::strings::get_checkbox_status (database::config::bible::get_export_web_during_night (bible)));
  
  
  if (webserver_request.query.count ("webnow")) {
    export_logic::schedule_web (bible, true);
    export_logic::schedule_web_index (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to Web format."));
  }
  
  
  if (webserver_request.post.count ("emailsubmit")) {
    std::string email = webserver_request.post["emailentry"];
    bool save = false;
    if (email.empty ()) {
      save = true;
      view.set_variable ("success", translate("The email address for the feedback link was removed."));
    } else {
      if (filter_url_email_is_valid (email)) {
        save = true;
        view.set_variable ("success", translate("The email address for the feedback link was saved."));
      } else {
        view.set_variable ("error", translate("The email address is not valid."));
      }
    }
    if (save) database::config::bible::set_export_feedback_email (bible, email);
  }
  view.set_variable ("email", database::config::bible::get_export_feedback_email (bible));

  
  if (checkbox == "html") {
    database::config::bible::set_export_hml_during_night (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("html", filter::strings::get_checkbox_status (database::config::bible::get_export_hml_during_night (bible)));
  
  
  if (webserver_request.query.count ("htmlnow")) {
    export_logic::schedule_html (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to Html format."));
  }

  
  if (checkbox == "htmlpopup") {
    database::config::bible::set_export_html_notes_on_hover (bible, checked);
  }
  view.set_variable ("htmlpopup", filter::strings::get_checkbox_status (database::config::bible::get_export_html_notes_on_hover (bible)));

  
  if (checkbox == "usfm") {
    database::config::bible::set_export_usfm_during_night (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("usfm", filter::strings::get_checkbox_status (database::config::bible::get_export_usfm_during_night (bible)));
 
  
  if (webserver_request.query.count ("usfmnow")) {
    export_logic::schedule_usfm (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to USFM format."));
  }

  
  if (checkbox == "usfmsecure") {
    database::config::bible::set_secure_usfm_export (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("usfmsecure", filter::strings::get_checkbox_status (database::config::bible::get_secure_usfm_export (bible)));

                     
  if (checkbox == "text") {
    database::config::bible::set_export_text_during_night (bible, checked);
  }
  view.set_variable ("text", filter::strings::get_checkbox_status (database::config::bible::get_export_text_during_night (bible)));
  
  
  if (webserver_request.query.count ("textnow")) {
    export_logic::schedule_text_and_basic_usfm (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to basic USFM format and text."));
  }
                       
                     
  if (checkbox == "odt") {
    database::config::bible::set_export_odt_during_night (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("odt", filter::strings::get_checkbox_status (database::config::bible::get_export_odt_during_night (bible)));

  
  if (webserver_request.query.count ("odtnow")) {
    export_logic::schedule_open_document (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to OpenDocument format."));
  }

  
  if (checkbox == "dropcaps") {
    database::config::bible::set_export_chapter_drop_caps_frames (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("dropcaps", filter::strings::get_checkbox_status (database::config::bible::get_export_chapter_drop_caps_frames (bible)));

  
  if (webserver_request.query.count ("pagewidth")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter a page width in millimeters"), database::config::bible::get_page_width (bible), "pagewidth", translate ("The width of A4 is 210 mm. The width of Letter is 216 mm."));
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("pagewidth")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 30) && (value <= 500)) {
      Database_State::setExport (bible, 0, export_logic::export_needed);
      database::config::bible::set_page_width (bible, std::to_string (value));
    }
  }
  view.set_variable ("pagewidth", database::config::bible::get_page_width (bible));

                     
  if (webserver_request.query.count ("pageheight")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter a page height in millimeters"), database::config::bible::get_page_height (bible), "pageheight", translate ("The height of A4 is 297 mm. The width of Letter is 279 mm."));
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("pageheight")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 40) && (value <= 600)) {
      Database_State::setExport (bible, 0, export_logic::export_needed);
      database::config::bible::set_page_height (bible, std::to_string (value));
    }
  }
  view.set_variable ("pageheight", database::config::bible::get_page_height (bible));

  
  if (webserver_request.query.count ("innermargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an inner margin size in millimeters"), database::config::bible::get_inner_margin (bible), "innermargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("innermargin")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, export_logic::export_needed);
      database::config::bible::set_inner_margin (bible, std::to_string (value));
    }
  }
  view.set_variable ("innermargin", database::config::bible::get_inner_margin (bible));


  if (webserver_request.query.count ("outermargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an outer margin size in millimeters"), database::config::bible::get_outer_margin (bible), "outermargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("outermargin")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, export_logic::export_needed);
      database::config::bible::set_outer_margin (bible, std::to_string (value));
    }
  }
  view.set_variable ("outermargin", database::config::bible::get_outer_margin (bible));
  
  
  if (webserver_request.query.count ("topmargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an top margin size in millimeters"), database::config::bible::get_top_margin (bible), "topmargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("topmargin")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, export_logic::export_needed);
      database::config::bible::set_top_margin (bible, std::to_string (value));
    }
  }
  view.set_variable ("topmargin", database::config::bible::get_top_margin (bible));


  if (webserver_request.query.count ("bottommargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an bottom margin size in millimeters"), database::config::bible::get_bottom_margin (bible), "bottommargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("bottommargin")) {
    int value = filter::strings::convert_to_int (webserver_request.post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, export_logic::export_needed);
      database::config::bible::set_bottom_margin (bible, std::to_string (value));
    }
  }
  view.set_variable ("bottommargin", database::config::bible::get_bottom_margin (bible));
  

  if (checkbox == "dateinheader") {
    database::config::bible::set_date_in_header (bible, checked);
  }
  view.set_variable ("dateinheader", filter::strings::get_checkbox_status (database::config::bible::get_date_in_header (bible)));
  
  
  if (checkbox == "odtsecure") {
    database::config::bible::set_secure_odt_export (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("odtsecure", filter::strings::get_checkbox_status (database::config::bible::get_secure_odt_export (bible)));

  
  std::vector <std::string> spaces = { " ", filter::strings::non_breaking_space_u00A0 (), filter::strings::en_space_u2002 (), filter::strings::figure_space_u2007 (), filter::strings::narrow_non_breaking_space_u202F () };
  if (webserver_request.query.count ("odtwhitespace")) {
    std::string odtwhitespace = webserver_request.query ["odtwhitespace"];
    for (auto space : spaces) {
      // Work with non-localized, English, space names.
      // Then it works across localizations.
      std::string href = space_href (locale_logic_space_get_name (space, true));
      if (odtwhitespace == href) {
        database::config::bible::set_odt_space_after_verse (bible, space);
      }
    }
  }
  std::string space_setting = database::config::bible::get_odt_space_after_verse (bible);
  for (auto space : spaces) {
    std::string name = locale_logic_space_get_name (space, true);
    std::string href = space_href (name);
    std::string cssclass;
    if (space == space_setting) {
      cssclass = "active";
    }
    name = locale_logic_space_get_name (space, false);
    view.add_iteration ("spaces", { std::pair ("space", href), std::pair ("class", cssclass), std::pair ("name", name) } );
  }

  
  if (checkbox == "odtqleft") {
    database::config::bible::set_odt_poetry_verses_left (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("odtqleft", filter::strings::get_checkbox_status (database::config::bible::get_odt_poetry_verses_left (bible)));
  {
    const std::vector <std::string> markers = database::styles::get_markers (stylesv2::standard_sheet ());
    std::vector <std::string> poetry_markers;
    for (const auto& marker : markers) {
      if (filter::usfm::is_standard_q_poetry (marker))
        poetry_markers.push_back(marker);
    }
    view.set_variable("poetrymarkers", filter::strings::implode(poetry_markers, " "));
  }

  
  if (webserver_request.post.count ("fontsubmit")) {
    std::string font = webserver_request.post["fontentry"];
    Database_State::setExport (bible, 0, export_logic::export_needed);
    database::config::bible::set_export_font (bible, font);
    view.set_variable ("success", translate("The font for securing exports was saved."));
  }
  view.set_variable ("font", database::config::bible::get_export_font (bible));

  
  if (checkbox == "autocaller") {
    database::config::bible::set_odt_automatic_note_caller (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("autocaller", filter::strings::get_checkbox_status (database::config::bible::get_odt_automatic_note_caller (bible)));

  
  if (checkbox == "info") {
    database::config::bible::set_generate_info_during_night (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("info", filter::strings::get_checkbox_status (database::config::bible::get_generate_info_during_night (bible)));
                   
  
  if (webserver_request.query.count ("infonow")) {
    export_logic::schedule_info (bible, true);
    view.set_variable ("success", translate("The info documents are being generated."));
  }
  
                       
  if (checkbox == "esword") {
    database::config::bible::set_export_e_sword_during_night (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("esword", filter::strings::get_checkbox_status (database::config::bible::get_export_e_sword_during_night (bible)));
                     
                                          
  if (webserver_request.query.count ("eswordnow")) {
    export_logic::schedule_e_sword (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to e-Sword format."));
  }
  
                       
  if (checkbox == "onlinebible") {
    database::config::bible::set_export_online_bible_during_night (bible, checked);
    Database_State::setExport (bible, 0, export_logic::export_needed);
  }
  view.set_variable ("onlinebible", filter::strings::get_checkbox_status (database::config::bible::get_export_online_bible_during_night (bible)));
  
  
  if (webserver_request.query.count ("onlinebiblenow")) {
    export_logic::schedule_online_bible (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to Online Bible format."));
  }
  
                     
  if (webserver_request.post.count ("passwordsubmit")) {
    std::string password = webserver_request.post["passwordentry"];
    Database_State::setExport (bible, 0, export_logic::export_needed);
    database::config::bible::set_export_password (bible, password);
    view.set_variable ("success", translate("The password for securing exports was saved."));
  }
  // If the password is needed, but not set, set a default password.
  if (database::config::bible::get_secure_usfm_export (bible) || database::config::bible::get_secure_odt_export (bible)) {
    if (database::config::bible::get_export_password (bible).empty ()) {
      Database_State::setExport (bible, 0, export_logic::export_needed);
      database::config::bible::set_export_password (bible, "password");
    }
  }
  view.set_variable ("password", database::config::bible::get_export_password (bible));
 
  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
  view.set_variable ("cloudlink", client_logic_link_to_cloud (manage_exports_url (), translate ("Go to Bibledit Cloud to submit the Bible there.")));
#else
  view.enable_zone ("cloud");
#endif
  view.set_variable ("external", assets_external_logic_link_addon ());
  
  
  page += view.render ("manage", "exports");
  page += assets_page::footer ();
  return page;
}
