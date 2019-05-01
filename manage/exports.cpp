/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


const char * manage_exports_url ()
{
  return "manage/exports";
}


bool manage_exports_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string space_href (string name)
{
  name = filter_string_str_replace ("-", "", name);
  name = filter_string_str_replace (" ", "", name);
  return name;
}


string manage_exports (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string page;
  Assets_Header header = Assets_Header (translate ("Export"), webserver_request);
  header.addBreadCrumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  if (request->query.count ("bible")) {
    string bible = request->query["bible"];
    if (bible == "") {
      Dialog_List dialog_list = Dialog_List ("exports", translate("Select a Bible"), "", "");
      vector <string> bibles = access_bible_bibles (webserver_request);
      for (auto bible : bibles) {
        dialog_list.add_row (bible, "bible", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (bible);
    }
  }
  
  
  string bible = access_bible_clamp (webserver_request, request->database_config_user()->getBible ());
  view.set_variable ("bible", bible);
  
  
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);
  
  
  if (request->query.count ("remove")) {
    string directory = Export_Logic::bibleDirectory (bible);
    filter_url_rmdir (directory);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
    view.set_variable ("success", translate("The export has been removed."));
  }
  
  
  if (checkbox == "web") {
    Database_Config_Bible::setExportWebDuringNight (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
    return "";
  }
  view.set_variable ("web", get_checkbox_status (Database_Config_Bible::getExportWebDuringNight (bible)));
  
  
  if (request->query.count ("webnow")) {
    Export_Logic::scheduleWeb (bible, true);
    Export_Logic::scheduleWebIndex (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to Web format."));
  }
  
  
  if (checkbox == "html") {
    Database_Config_Bible::setExportHtmlDuringNight (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("html", get_checkbox_status (Database_Config_Bible::getExportHtmlDuringNight (bible)));
  
  
  if (request->query.count ("htmlnow")) {
    Export_Logic::scheduleHtml (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to Html format."));
  }
  
  
  if (checkbox == "usfm") {
    Database_Config_Bible::setExportUsfmDuringNight (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("usfm", get_checkbox_status (Database_Config_Bible::getExportUsfmDuringNight (bible)));
 
  
  if (request->query.count ("usfmnow")) {
    Export_Logic::scheduleUsfm (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to USFM format."));
  }

  
  if (checkbox == "usfmsecure") {
    Database_Config_Bible::setSecureUsfmExport (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("usfmsecure", get_checkbox_status (Database_Config_Bible::getSecureUsfmExport (bible)));

                     
  if (checkbox == "text") {
    Database_Config_Bible::setExportTextDuringNight (bible, checked);
  }
  view.set_variable ("text", get_checkbox_status (Database_Config_Bible::getExportTextDuringNight (bible)));
  
  
  if (request->query.count ("textnow")) {
    Export_Logic::scheduleTextAndBasicUsfm (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to basic USFM format and text."));
  }
                       
                     
  if (checkbox == "odt") {
    Database_Config_Bible::setExportOdtDuringNight (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("odt", get_checkbox_status (Database_Config_Bible::getExportOdtDuringNight (bible)));

  
  if (request->query.count ("odtnow")) {
    Export_Logic::scheduleOpenDocument (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to OpenDocument format."));
  }

  
  if (checkbox == "dropcaps") {
    Database_Config_Bible::setExportChapterDropCapsFrames (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("dropcaps", get_checkbox_status (Database_Config_Bible::getExportChapterDropCapsFrames (bible)));

  
  if (request->query.count ("pagewidth")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter a page width in millimeters"), Database_Config_Bible::getPageWidth (bible), "pagewidth", translate ("The width of A4 is 210 mm. The width of Letter is 216 mm."));
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("pagewidth")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 30) && (value <= 500)) {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setPageWidth (bible, convert_to_string (value));
    }
  }
  view.set_variable ("pagewidth", Database_Config_Bible::getPageWidth (bible));

                     
  if (request->query.count ("pageheight")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter a page height in millimeters"), Database_Config_Bible::getPageHeight (bible), "pageheight", translate ("The height of A4 is 297 mm. The width of Letter is 279 mm."));
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("pageheight")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 40) && (value <= 600)) {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setPageHeight (bible, convert_to_string (value));
    }
  }
  view.set_variable ("pageheight", Database_Config_Bible::getPageHeight (bible));

  
  if (request->query.count ("innermargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an inner margin size in millimeters"), Database_Config_Bible::getInnerMargin (bible), "innermargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("innermargin")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setInnerMargin (bible, convert_to_string (value));
    }
  }
  view.set_variable ("innermargin", Database_Config_Bible::getInnerMargin (bible));


  if (request->query.count ("outermargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an outer margin size in millimeters"), Database_Config_Bible::getOuterMargin (bible), "outermargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("outermargin")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setOuterMargin (bible, convert_to_string (value));
    }
  }
  view.set_variable ("outermargin", Database_Config_Bible::getOuterMargin (bible));
  
  
  if (request->query.count ("topmargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an top margin size in millimeters"), Database_Config_Bible::getTopMargin (bible), "topmargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("topmargin")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setTopMargin (bible, convert_to_string (value));
    }
  }
  view.set_variable ("topmargin", Database_Config_Bible::getTopMargin (bible));


  if (request->query.count ("bottommargin")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("exports", translate("Please enter an bottom margin size in millimeters"), Database_Config_Bible::getBottomMargin (bible), "bottommargin", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("bottommargin")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setBottomMargin (bible, convert_to_string (value));
    }
  }
  view.set_variable ("bottommargin", Database_Config_Bible::getBottomMargin (bible));
  

  if (checkbox == "dateinheader") {
    Database_Config_Bible::setDateInHeader (bible, checked);
  }
  view.set_variable ("dateinheader", get_checkbox_status (Database_Config_Bible::getDateInHeader (bible)));
  
  
  if (checkbox == "odtsecure") {
    Database_Config_Bible::setSecureOdtExport (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("odtsecure", get_checkbox_status (Database_Config_Bible::getSecureOdtExport (bible)));

  
  vector <string> spaces = { " ", non_breaking_space_u00A0 (), en_space_u2002 (), figure_space_u2007 (), narrow_non_breaking_space_u202F () };
  if (request->query.count ("odtwhitespace")) {
    string odtwhitespace = request->query ["odtwhitespace"];
    for (auto space : spaces) {
      // Work with non-localized, English, space names.
      // Then it works across localizations.
      string href = space_href (locale_logic_space_get_name (space, true));
      if (odtwhitespace == href) {
        Database_Config_Bible::setOdtSpaceAfterVerse (bible, space);
      }
    }
  }
  string space_setting = Database_Config_Bible::getOdtSpaceAfterVerse (bible);
  for (auto space : spaces) {
    string name = locale_logic_space_get_name (space, true);
    string href = space_href (name);
    string cssclass;
    if (space == space_setting) {
      cssclass = "active";
    }
    name = locale_logic_space_get_name (space, false);
    view.add_iteration ("spaces", { make_pair ("space", href), make_pair ("class", cssclass), make_pair ("name", name) } );
  }
  
  
  if (request->post.count ("fontsubmit")) {
    string font = request->post["fontentry"];
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
    Database_Config_Bible::setExportFont (bible, font);
    view.set_variable ("success", translate("The font for securing exports was saved."));
  }
  view.set_variable ("font", Database_Config_Bible::getExportFont (bible));

  
  if (checkbox == "info") {
    Database_Config_Bible::setGenerateInfoDuringNight (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("info", get_checkbox_status (Database_Config_Bible::getGenerateInfoDuringNight (bible)));
                   
  
  if (request->query.count ("infonow")) {
    Export_Logic::scheduleInfo (bible, true);
    view.set_variable ("success", translate("The info documents are being generated."));
  }
  
                       
  if (checkbox == "esword") {
    Database_Config_Bible::setExportESwordDuringNight (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("esword", get_checkbox_status (Database_Config_Bible::getExportESwordDuringNight (bible)));
                     
                                          
  if (request->query.count ("eswordnow")) {
    Export_Logic::scheduleESword (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to e-Sword format."));
  }
  
                       
  if (checkbox == "onlinebible") {
    Database_Config_Bible::setExportOnlineBibleDuringNight (bible, checked);
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
  }
  view.set_variable ("onlinebible", get_checkbox_status (Database_Config_Bible::getExportOnlineBibleDuringNight (bible)));
  
  
  if (request->query.count ("onlinebiblenow")) {
    Export_Logic::scheduleOnlineBible (bible, true);
    view.set_variable ("success", translate("The Bible is being exported to Online Bible format."));
  }
  
  
  if (request->query.count ("sheet")) {
    string sheet = request->query["sheet"];
    if (sheet == "") {
      Dialog_List dialog_list = Dialog_List ("exports", translate("Would you like to use another stylesheet for the exports?"), "", "");
      vector <string> sheets = request->database_styles()->getSheets();
      for (auto sheet : sheets) {
        dialog_list.add_row (sheet, "sheet", sheet);
      }
      page += dialog_list.run ();
      return page;
    } else {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setExportStylesheet (bible, sheet);
    }
  }
  view.set_variable ("stylesheet", Database_Config_Bible::getExportStylesheet (bible));
                     
                     
  if (request->post.count ("passwordsubmit")) {
    string password = request->post["passwordentry"];
    Database_State::setExport (bible, 0, Export_Logic::export_needed);
    Database_Config_Bible::setExportPassword (bible, password);
    view.set_variable ("success", translate("The password for securing exports was saved."));
  }
  // If the password is needed, but not set, set a default password.
  if (Database_Config_Bible::getSecureUsfmExport (bible) || Database_Config_Bible::getSecureOdtExport (bible)) {
    if (Database_Config_Bible::getExportPassword (bible).empty ()) {
      Database_State::setExport (bible, 0, Export_Logic::export_needed);
      Database_Config_Bible::setExportPassword (bible, "password");
    }
  }
  view.set_variable ("password", Database_Config_Bible::getExportPassword (bible));

  
  if (request->query.count ("bibledropboxnow")) {
    string username = request->session_logic()->currentUser ();
    tasks_logic_queue (SUBMITBIBLEDROPBOX, { username, bible });
    string msg = translate("The Bible will be submitted to the Bible Drop Box.");
    msg.append (" ");
    msg.append (translate("You will receive email with further details."));
    view.set_variable ("success", msg);
  }
 
  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
  view.set_variable ("cloudlink", client_logic_link_to_cloud (manage_exports_url (), translate ("Go to Bibledit Cloud to submit the Bible there.")));
#else
  view.enable_zone ("cloud");
#endif
  view.set_variable ("external", assets_external_logic_link_addon ());
  
  
  page += view.render ("manage", "exports");
  page += Assets_Page::footer ();
  return page;
}
