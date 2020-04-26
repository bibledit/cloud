/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


#include <edit/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/css.h>
#include <filter/url.h>
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
#include <config/globals.h>
#include <workspace/logic.h>


string edit_index_url ()
{
  return "edit/index";
}


bool edit_index_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string edit_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  bool touch = request->session_logic ()->touchEnabled ();

  
  if (request->query.count ("switchbook") && request->query.count ("switchchapter")) {
    int switchbook = convert_to_int (request->query ["switchbook"]);
    int switchchapter = convert_to_int (request->query ["switchchapter"]);
    int switchverse = 1;
    if (request->query.count ("switchverse")) switchverse = convert_to_int (request->query ["switchverse"]);
    Ipc_Focus::set (request, switchbook, switchchapter, switchverse);
    Navigation_Passage::recordHistory (request, switchbook, switchchapter, switchverse);
  }

  
  string page;
  
  
  Assets_Header header = Assets_Header (translate("Edit"), request);
  header.setNavigator ();
  header.setEditorStylesheet ();
  if (touch) header.jQueryTouchOn ();
  header.notifItOn ();
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  
  if (request->query.count ("changebible")) {
    string changebible = request->query ["changebible"];
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
      // Going to another Bible, ensure that the focused book exists there.
      int book = Ipc_Focus::getBook (request);
      vector <int> books = request->database_bibles()->getBooks (changebible);
      if (find (books.begin(), books.end(), book) == books.end()) {
        if (!books.empty ()) book = books [0];
        else book = 0;
        Ipc_Focus::set (request, book, 1, 1);
      }
    }
  }
  
  
  Assets_View view;
  
  
  // Active Bible, and check access.
  string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  if (request->query.count ("bible")) bible = access_bible_clamp (request, request->query ["bible"]);
  view.set_variable ("bible", bible);
  
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  

  int verticalCaretPosition = request->database_config_user ()->getVerticalCaretPosition ();
  string script =
  "var editorChapterLoaded = '" + locale_logic_text_loaded () + "';\n"
  "var editorWillSave = '" + locale_logic_text_will_save () + "';\n"
  "var editorChapterSaving = '" + locale_logic_text_saving () + "';\n"
  "var editorChapterSaved = '" + locale_logic_text_saved () + "';\n"
  "var editorChapterRetrying = '" + locale_logic_text_retrying () + "';\n"
  "var editorChapterReformat = '" + locale_logic_text_reformat () + "';\n"
  "var editorChapterVerseUpdatedLoaded = '" + locale_logic_text_reload () + "';\n"
  "var verticalCaretPosition = " + convert_to_string (verticalCaretPosition) + ";\n"
  "var editorForceReadOnly = " + convert_to_true_false (request->query.count (workspace_query_key_readonly ())) + ";\n";
  config_logic_swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  
  
  string clss = Filter_Css::getClass (bible);
  string font = Fonts_Logic::getTextFont (bible);
  int direction = Database_Config_Bible::getTextDirection (bible);
  int lineheight = Database_Config_Bible::getLineHeight (bible);
  int letterspacing = Database_Config_Bible::getLetterSpacing (bible);
  view.set_variable ("custom_class", clss);
  view.set_variable ("custom_css", Filter_Css::getCss (clss,
                                                       Fonts_Logic::getFontPath (font),
                                                       direction,
                                                       lineheight,
                                                       letterspacing));
  
 
  // In basic mode the editor has no controls and fewer indicators.
  // In basic mode, the user can just edit text, and cannot style it.
  bool basic_mode = config_logic_basic_mode (webserver_request);
  if (!basic_mode) view.enable_zone ("advancedmode");
  
  
  // Whether to enable fast Bible editor switching.
  if (!basic_mode && request->database_config_user ()->getFastEditorSwitchingAvailable ()) {
    view.enable_zone ("fastswitcheditor");
  }

  
  // Whether to enable the styles button.
  if (request->database_config_user ()->getEnableStylesButtonVisualEditors ()) {
    view.enable_zone ("stylesbutton");
  }
  
  
  page += view.render ("edit", "index");
  
  
  page += Assets_Page::footer ();
  
  
  return page;
}


/*
 Tests for the Bible editor:
 * Autosave on going to another passage.
 * Autosave on document unload.
 * Autosave shortly after any change.
 * Automatic reload when another user updates the chapter on the server.
 * Position caret at correct verse.
 * Scroll caret into view.
 */
