/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


std::string edit_index_url ()
{
  return "edit/index";
}


bool edit_index_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) 
    return true;
  const auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string edit_index (Webserver_Request& webserver_request)
{
  const bool touch = webserver_request.session_logic ()->get_touch_enabled ();

  
  if (webserver_request.query.count ("switchbook") && webserver_request.query.count ("switchchapter")) {
    const int switchbook = filter::strings::convert_to_int (webserver_request.query ["switchbook"]);
    const int switchchapter = filter::strings::convert_to_int (webserver_request.query ["switchchapter"]);
    int switchverse = 1;
    if (webserver_request.query.count ("switchverse")) 
      switchverse = filter::strings::convert_to_int (webserver_request.query ["switchverse"]);
    Ipc_Focus::set (webserver_request, switchbook, switchchapter, switchverse);
    Navigation_Passage::record_history (webserver_request, switchbook, switchchapter, switchverse);
  }


  // Set the user chosen Bible as the current Bible.
  if (webserver_request.post.count ("bibleselect")) {
    const std::string bibleselect = webserver_request.post ["bibleselect"];
    webserver_request.database_config_user ()->setBible (bibleselect);
    // Going to another Bible, ensure that the focused book exists there.
    int book = Ipc_Focus::getBook (webserver_request);
    const std::vector <int> books = database::bibles::get_books (bibleselect);
    if (find (books.begin(), books.end(), book) == books.end()) {
      if (!books.empty ()) book = books [0];
      else book = 0;
      Ipc_Focus::set (webserver_request, book, 1, 1);
    }
    return std::string();
  }

  
  std::string page{};
  
  
  Assets_Header header = Assets_Header (translate("Edit"), webserver_request);
  header.set_navigator ();
  header.set_editor_stylesheet ();
  if (touch) header.jquery_touch_on ();
  header.notify_it_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  
  Assets_View view{};
  
  
  // Active Bible, and check access.
  // Or if the user have used query to preset the active Bible, get the preset Bible.
  // Set the chosen Bible on the option HTML tag.
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  if (webserver_request.query.count ("bible"))
    bible = access_bible::clamp (webserver_request, webserver_request.query ["bible"]);
  std::string bible_html{};
  const std::vector <std::string> bibles = access_bible::bibles (webserver_request);
  for (const auto& selectable_bible : bibles) {
    bible_html = Options_To_Select::add_selection (selectable_bible, selectable_bible, bible_html);
  }
  view.set_variable ("bibleoptags", Options_To_Select::mark_selected (bible, bible_html));
  view.set_variable ("bible", bible);
  
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  

  // Create the script.
  // Quote the text to be sure it's a legal Javascript string.
  // https://github.com/bibledit/cloud/issues/900
  std::stringstream script_stream {};
  script_stream << "var editorChapterLoaded = " << std::quoted(locale_logic_text_loaded ()) << ";\n";
  script_stream << "var editorChapterUpdating = " << std::quoted(locale_logic_text_updating ()) << ";\n";
  script_stream << "var editorChapterUpdated = " << std::quoted(locale_logic_text_updated ()) << ";\n";
  script_stream << "var editorWillSave = " << std::quoted(locale_logic_text_will_save ()) << ";\n";
  script_stream << "var editorChapterSaving = " << std::quoted(locale_logic_text_saving ()) << ";\n";
  script_stream << "var editorChapterSaved = " << std::quoted(locale_logic_text_saved ()) << ";\n";
  script_stream << "var editorChapterRetrying = " << std::quoted(locale_logic_text_retrying ()) << ";\n";
  script_stream << "var editorChapterVerseUpdatedLoaded = " << std::quoted(locale_logic_text_reload ()) << ";\n";
  script_stream << "var verticalCaretPosition = " << webserver_request.database_config_user ()->getVerticalCaretPosition () << ";\n";
  script_stream << "var verseSeparator = " << std::quoted(database::config::general::get_notes_verse_separator ()) << ";\n";
  std::string script = script_stream.str();
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  
  
  const std::string clss = Filter_Css::getClass (bible);
  const std::string font = fonts::logic::get_text_font (bible);
  const int current_theme_index = webserver_request.database_config_user ()->getCurrentTheme ();
  const int direction = database::config::bible::get_text_direction (bible);
  const int lineheight = database::config::bible::get_line_height (bible);
  const int letterspacing = database::config::bible::get_letter_spacing (bible);
  std::string versebeam_current_theme = Filter_Css::theme_picker (current_theme_index, 5);
  if (versebeam_current_theme.empty())
    versebeam_current_theme = "versebeam";
  view.set_variable ("versebeam_theme_color", versebeam_current_theme);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", clss);
  view.set_variable ("custom_css", Filter_Css::get_css (clss, fonts::logic::get_font_path (font),
                                                        direction, lineheight, letterspacing));
  
 
  // In basic mode the editor has no controls and fewer indicators.
  // In basic mode, the user can just edit text, and cannot style it.
  const bool basic_mode = config::logic::basic_mode (webserver_request);
  if (!basic_mode) view.enable_zone ("advancedmode");
  
  
  // Whether to enable fast Bible editor switching.
  if (!basic_mode && webserver_request.database_config_user ()->getFastEditorSwitchingAvailable ()) {
    view.enable_zone ("fastswitcheditor");
  }

  
  // Whether to enable the styles button.
  if (webserver_request.database_config_user ()->getEnableStylesButtonVisualEditors ()) {
    view.enable_zone ("stylesbutton");
  }
  
  
  // Whether to enable spell check.
  view.set_variable ("spellcheck", filter::strings::convert_to_true_false(webserver_request.database_config_user ()->get_enable_spell_check()));

  
  page.append (view.render ("edit", "index"));
  
  
  page.append (assets_page::footer ());
  
  
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
