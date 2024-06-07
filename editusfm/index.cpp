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
#include <dialog/list2.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <bb/logic.h>
#include <workspace/logic.h>


std::string editusfm_index_url ()
{
  return "editusfm/index";
}


bool editusfm_index_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string editusfm_index (Webserver_Request& webserver_request)
{
  const bool touch = webserver_request.session_logic ()->get_touch_enabled ();

  
  if (webserver_request.query.count ("switchbook") && webserver_request.query.count ("switchchapter")) {
    const std::string switchbook = webserver_request.query ["switchbook"];
    const std::string switchchapter = webserver_request.query ["switchchapter"];
    const int book = filter::strings::convert_to_int (switchbook);
    const int chapter = filter::strings::convert_to_int (switchchapter);
    Ipc_Focus::set (webserver_request, book, chapter, 1);
    Navigation_Passage::record_history (webserver_request, book, chapter, 1);
  }


  // Set the user chosen Bible as the current Bible.
  if (webserver_request.post.count ("bibleselect")) {
    const std::string bibleselect = webserver_request.post ["bibleselect"];
    webserver_request.database_config_user ()->setBible (bibleselect);
    return std::string();
  }
  
  
  std::string page{};
  
  Assets_Header header = Assets_Header (translate("Edit USFM"), webserver_request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  if (touch) 
    header.jquery_touch_on ();
  header.notify_it_on ();
  page = header.run ();
  Assets_View view;


  // Get active Bible, and check read access to it.
  // Or if the user have used query to preset the active Bible, get the preset Bible.
  // If needed, change Bible to one it has read access to.
  // Set the chosen Bible on the option HTML tag.
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  if (webserver_request.query.count ("bible"))
    bible = access_bible::clamp (webserver_request, webserver_request.query ["bible"]);
  std::string bible_html;
  const std::vector <std::string> bibles = access_bible::bibles (webserver_request);
  for (const auto& selectable_bible : bibles) {
    bible_html = Options_To_Select::add_selection (selectable_bible, selectable_bible, bible_html);
  }
  view.set_variable ("bibleoptags", Options_To_Select::mark_selected (bible, bible_html));
  view.set_variable ("bible", bible);
  
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  
  
  const int verticalCaretPosition = webserver_request.database_config_user ()->getVerticalCaretPosition ();
  std::stringstream ss{};
  ss << "var usfmEditorChapterLoaded = " << std::quoted(locale_logic_text_loaded ()) << ";" << std::endl;
  ss << "var usfmEditorWillSave = " << std::quoted(locale_logic_text_will_save ()) << ";" << std::endl;
  ss << "var usfmEditorChapterSaving = " << std::quoted(locale_logic_text_saving ()) << ";" << std::endl;
  ss << "var usfmEditorChapterSaved = " << std::quoted(locale_logic_text_saved ()) << ";" << std::endl;
  ss << "var usfmEditorChapterRetrying = " << std::quoted(locale_logic_text_retrying ()) << ";" << std::endl;
  ss << "var usfmEditorVerseUpdatedLoaded = " << std::quoted(locale_logic_text_reload ()) << ";" << std::endl;
  ss << "var usfmEditorWriteAccess = true;" << std::endl;
  ss << "var verticalCaretPosition = " << verticalCaretPosition << ";" << std::endl;
  std::string script = ss.str();
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  

  const std::string cls = Filter_Css::getClass (bible);
  const std::string font = fonts::logic::get_text_font (bible);
  const int current_theme_index = webserver_request.database_config_user ()->getCurrentTheme ();
  const int direction = database::config::bible::get_text_direction (bible);
  const int lineheight = database::config::bible::get_line_height (bible);
  const int letterspacing = database::config::bible::get_letter_spacing (bible);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", cls);
  view.set_variable ("custom_css", Filter_Css::get_css (cls, fonts::logic::get_font_path (font),
                                                        direction, lineheight, letterspacing));
  
  // Whether to enable fast Bible editor switching.
  if (webserver_request.database_config_user ()->getFastEditorSwitchingAvailable ()) {
    view.enable_zone ("fastswitcheditor");
  }
  
  // Whether to enable spell check in the editor.
  view.set_variable ("spellcheck", filter::strings::convert_to_true_false(webserver_request.database_config_user ()->get_enable_spell_check()));
  

  page.append (view.render ("editusfm", "index"));
  
  
  page.append (assets_page::footer ());
  
  
  return page;
}


// Tests for the USFM editor:
// * Autosave on going to another passage.
// * Autosave on document unload.
// * Autosave shortly after any change.
