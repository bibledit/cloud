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


#include <read/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/css.h>
#include <filter/url.h>
#include <filter/indonesian.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <access/bible.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <fonts/logic.h>
#include <navigation/passage.h>
#include <dialog/list.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <bb/logic.h>
#include <config/globals.h>
#include <workspace/logic.h>


string read_index_url ()
{
  return "read/index";
}


bool read_index_acl (void * webserver_request)
{
  int role = Filter_Roles::translator ();
  if (config_logic_indonesian_cloud_free ()) {
    role = Filter_Roles::consultant ();
  }
  if (Filter_Roles::access_control (webserver_request, role)) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string read_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  bool touch = request->session_logic ()->touchEnabled ();
  
  if (request->query.count ("switchbook") && request->query.count ("switchchapter")) {
    int switchbook = convert_to_int (request->query ["switchbook"]);
    int switchchapter = convert_to_int (request->query ["switchchapter"]);
    Ipc_Focus::set (request, switchbook, switchchapter, 1);
    Navigation_Passage::recordHistory (request, switchbook, switchchapter, 1);
  }

  if (config_logic_indonesian_cloud_free ()) {
    // See issue https://github.com/bibledit/cloud/issues/503
    // Specific configuration for the Indonesian free Cloud instance.
    // The name of the default Bible in the Read tab will be AlkitabKita
    // (That means Our/Everyone's Translation.
    request->database_config_user()->setBible (filter_indonesian_alkitabkita_ourtranslation_name ());
  }

  string page;
  
  Assets_Header header = Assets_Header (translate("Edit verse"), request);
  header.setNavigator ();
  header.setEditorStylesheet ();
  if (touch) header.jQueryTouchOn ();
  header.notifItOn ();
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  Assets_View view;
  
  if (request->query.count ("changebible")) {
    string changebible = request->query ["changebible"];
    if (changebible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to read"), "", "");
      vector <string> bibles = access_bible_bibles (request);
      for (auto bible : bibles) {
        dialog_list.add_row (bible, "changebible", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (changebible);
    }
  }
  
  // Get active Bible, and check read access to it.
  // If needed, change Bible to one it has read access to.
  string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  if (request->query.count ("bible")) bible = access_bible_clamp (request, request->query ["bible"]);
  view.set_variable ("bible", bible);
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  
  int verticalCaretPosition = request->database_config_user ()->getVerticalCaretPosition ();
  string script =
  "var readchooseEditorVerseLoaded = '" + locale_logic_text_loaded () + "';\n"
  "var readchooseEditorVerseUpdating = '" + locale_logic_text_updating () + "';\n"
  "var readchooseEditorVerseUpdated = '" + locale_logic_text_updated () + "';\n"
  "var readchooseEditorWillSave = '" + locale_logic_text_will_save () + "';\n"
  "var readchooseEditorVerseSaving = '" + locale_logic_text_saving () + "';\n"
  "var readchooseEditorVerseSaved = '" + locale_logic_text_saved () + "';\n"
  "var readchooseEditorVerseRetrying = '" + locale_logic_text_retrying () + "';\n"
  "var readchooseEditorVerseUpdatedLoaded = '" + locale_logic_text_reload () + "';\n"
  "var verticalCaretPosition = " + convert_to_string (verticalCaretPosition) + ";\n"
  "var verseSeparator = '" + Database_Config_General::getNotesVerseSeparator () + "';\n";
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

  // Whether to enable the styles button.
  if (request->database_config_user ()->getEnableStylesButtonVisualEditors ()) {
    view.enable_zone ("stylesbutton");
  }
  
  page += view.render ("read", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}

