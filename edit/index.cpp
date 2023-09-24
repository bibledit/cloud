/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
#include <database/config/general.h>
#include <fonts/logic.h>
#include <navigation/passage.h>
#include <dialog/list.h>
#include <dialog/list2.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <bb/logic.h>
#include <config/globals.h>
#include <workspace/logic.h>
using namespace std;


string edit_index_url ()
{
  return "edit/index";
}


bool edit_index_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


string edit_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  bool touch = request->session_logic ()->touchEnabled ();

  
  if (request->query.count ("switchbook") && request->query.count ("switchchapter")) {
    int switchbook = filter::strings::convert_to_int (request->query ["switchbook"]);
    int switchchapter = filter::strings::convert_to_int (request->query ["switchchapter"]);
    int switchverse = 1;
    if (request->query.count ("switchverse")) switchverse = filter::strings::convert_to_int (request->query ["switchverse"]);
    Ipc_Focus::set (request, switchbook, switchchapter, switchverse);
    Navigation_Passage::record_history (request, switchbook, switchchapter, switchverse);
  }


  // Set the user chosen Bible as the current Bible.
  if (request->post.count ("bibleselect")) {
    string bibleselect = request->post ["bibleselect"];
    request->database_config_user ()->setBible (bibleselect);
    // Going to another Bible, ensure that the focused book exists there.
    int book = Ipc_Focus::getBook (request);
    vector <int> books = request->database_bibles()->get_books (bibleselect);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books [0];
      else book = 0;
      Ipc_Focus::set (request, book, 1, 1);
    }
    return string();
  }

  
  string page;
  
  
  Assets_Header header = Assets_Header (translate("Edit"), request);
  header.set_navigator ();
  header.set_editor_stylesheet ();
  if (touch) header.jquery_touch_on ();
  header.notify_it_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  
  Assets_View view;
  
  
  // Active Bible, and check access.
  // Or if the user have used query to preset the active Bible, get the preset Bible.
  // Set the chosen Bible on the option HTML tag.
  string bible = access_bible::clamp (request, request->database_config_user()->getBible ());
  if (request->query.count ("bible")) bible = access_bible::clamp (request, request->query ["bible"]);
  string bible_html;
  vector <string> bibles = access_bible::bibles (request);
  for (auto selectable_bible : bibles) {
    bible_html = Options_To_Select::add_selection (selectable_bible, selectable_bible, bible_html);
  }
  view.set_variable ("bibleoptags", Options_To_Select::mark_selected (bible, bible_html));
  view.set_variable ("bible", bible);
  
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  

  // Create the script.
  // Quote the text to be sure it's a legal Javascript string.
  // https://github.com/bibledit/cloud/issues/900
  stringstream script_stream {};
  script_stream << "var editorChapterLoaded = " << quoted(locale_logic_text_loaded ()) << ";\n";
  script_stream << "var editorChapterUpdating = " << quoted(locale_logic_text_updating ()) << ";\n";
  script_stream << "var editorChapterUpdated = " << quoted(locale_logic_text_updated ()) << ";\n";
  script_stream << "var editorWillSave = " << quoted(locale_logic_text_will_save ()) << ";\n";
  script_stream << "var editorChapterSaving = " << quoted(locale_logic_text_saving ()) << ";\n";
  script_stream << "var editorChapterSaved = " << quoted(locale_logic_text_saved ()) << ";\n";
  script_stream << "var editorChapterRetrying = " << quoted(locale_logic_text_retrying ()) << ";\n";
  script_stream << "var editorChapterVerseUpdatedLoaded = " << quoted(locale_logic_text_reload ()) << ";\n";
  script_stream << "var verticalCaretPosition = " << request->database_config_user ()->getVerticalCaretPosition () << ";\n";
  script_stream << "var verseSeparator = " << quoted(Database_Config_General::getNotesVerseSeparator ()) << ";\n";
  string script = script_stream.str();
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  
  
  string clss = Filter_Css::getClass (bible);
  string font = fonts::logic::get_text_font (bible);
  int current_theme_index = request->database_config_user ()->getCurrentTheme ();
  int direction = Database_Config_Bible::getTextDirection (bible);
  int lineheight = Database_Config_Bible::getLineHeight (bible);
  int letterspacing = Database_Config_Bible::getLetterSpacing (bible);
  string versebeam_current_theme = Filter_Css::theme_picker (current_theme_index, 5);
  if (versebeam_current_theme == "") versebeam_current_theme = "versebeam";
  view.set_variable ("versebeam_theme_color", versebeam_current_theme);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", clss);
  view.set_variable ("custom_css", Filter_Css::get_css (clss,
                                                       fonts::logic::get_font_path (font),
                                                       direction,
                                                       lineheight,
                                                       letterspacing));
  
 
  // In basic mode the editor has no controls and fewer indicators.
  // In basic mode, the user can just edit text, and cannot style it.
  bool basic_mode = config::logic::basic_mode (webserver_request);
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
  
  
  page += assets_page::footer ();
  
  
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
