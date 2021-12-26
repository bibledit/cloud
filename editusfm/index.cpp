/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <editusfm/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <access/bible.h>
#include <database/config/bible.h>
#include <fonts/logic.h>
#include <navigation/passage.h>
#include <dialog/list.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <bb/logic.h>
#include <workspace/logic.h>


string editusfm_index_url ()
{
  return "editusfm/index";
}


bool editusfm_index_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string editusfm_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  bool touch = request->session_logic ()->touchEnabled ();

  
  if (request->query.count ("switchbook") && request->query.count ("switchchapter")) {
    string switchbook = request->query ["switchbook"];
    string switchchapter = request->query ["switchchapter"];
    int book = convert_to_int (switchbook);
    int chapter = convert_to_int (switchchapter);
    Ipc_Focus::set (request, book, chapter, 1);
    Navigation_Passage::record_history (request, book, chapter, 1);
  }
  
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Edit USFM"), request);
  header.setNavigator ();
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  if (touch) header.jQueryTouchOn ();
  header.notifItOn ();
  page = header.run ();
  
  if (request->query.count("changebible")) {
    string changebible = request->query["changebible"];
    if (changebible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to open in the editor"), "", "");
      vector <string> bibles = access_bible_bibles (request);
      for (auto & bible : bibles) {
        dialog_list.add_row (bible, "changebible", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (changebible);
    }
  }

  
  Assets_View view;


  // Get active Bible, and check read access to it.
  // If needed, change Bible to one it has read access to.
  string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  if (request->query.count ("bible")) bible = access_bible_clamp (request, request->query ["bible"]);
  view.set_variable ("bible", bible);
  
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  
  
  int verticalCaretPosition = request->database_config_user ()->getVerticalCaretPosition ();
  stringstream ss;
  ss << "var usfmEditorChapterLoaded = " << quoted(locale_logic_text_loaded ()) << ";" << endl;
  ss << "var usfmEditorWillSave = " << quoted(locale_logic_text_will_save ()) << ";" << endl;
  ss << "var usfmEditorChapterSaving = " << quoted(locale_logic_text_saving ()) << ";" << endl;
  ss << "var usfmEditorChapterSaved = " << quoted(locale_logic_text_saved ()) << ";" << endl;
  ss << "var usfmEditorChapterRetrying = " << quoted(locale_logic_text_retrying ()) << ";" << endl;
  ss << "var usfmEditorVerseUpdatedLoaded = " << quoted(locale_logic_text_reload ()) << ";" << endl;
  ss << "var usfmEditorWriteAccess = true;" << endl;
  ss << "var verticalCaretPosition = " << verticalCaretPosition << ";" << endl;
  string script = ss.str();
  config_logic_swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  

  string cls = Filter_Css::getClass (bible);
  string font = Fonts_Logic::getTextFont (bible);
  int current_theme_index = convert_to_int(request->database_config_user ()->getCurrentTheme ());
  int direction = Database_Config_Bible::getTextDirection (bible);
  int lineheight = Database_Config_Bible::getLineHeight (bible);
  int letterspacing = Database_Config_Bible::getLetterSpacing (bible);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", cls);
  view.set_variable ("custom_css", Filter_Css::getCss (cls,
                                                             Fonts_Logic::getFontPath (font),
                                                             direction,
                                                             lineheight,
                                                             letterspacing));
  
  // Whether to enable fast Bible editor switching.
  if (request->database_config_user ()->getFastEditorSwitchingAvailable ()) {
    view.enable_zone ("fastswitcheditor");
  }
  if (config_logic_indonesian_cloud_free ()) {
    view.enable_zone ("fastswitcheditor");
  }

  page += view.render ("editusfm", "index");
  
  
  page += Assets_Page::footer ();
  
  
  return page;
}


// Tests for the USFM editor:
// * Autosave on going to another passage.
// * Autosave on document unload.
// * Autosave shortly after any change.
