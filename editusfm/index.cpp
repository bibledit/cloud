/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <dialog/select.h>
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
  if (roles::access_control (webserver_request, roles::translator))
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
    const int book = filter::string::convert_to_int (switchbook);
    const int chapter = filter::string::convert_to_int (switchchapter);
    ipc_focus::set_passage (webserver_request, book, chapter, 1);
    navigation_passage::record_history (webserver_request, book, chapter, 1);
  }


  std::string page{};
  
  Assets_Header header = Assets_Header (translate("Edit USFM"), webserver_request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  header.notify_on ();
  header.set_focus_group(ipc_focus::get_focus_group(webserver_request));
  page = header.run ();
  Assets_View view;


  // Get active Bible, and check read access to it.
  // Or if the user has used a query to preset the active Bible, get that preset Bible.
  // If needed, change Bible to one it has read access to.
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible ());
  if (webserver_request.query.count ("bible"))
    bible = access_bible::clamp (webserver_request, webserver_request.query ["bible"]);
  {
    constexpr const char* identification {"bibleselect"};
    if (webserver_request.post_count(identification)) {
      bible = webserver_request.post_get(identification);
      webserver_request.database_config_user ()->set_bible (bible);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = access_bible::bibles (webserver_request),
      .selected = bible,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  view.set_variable ("bible", bible);
  
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", navigation_passage::code (bible));
  
  
  const int verticalCaretPosition = webserver_request.database_config_user ()->get_vertical_caret_position ();
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
  const int current_theme_index = webserver_request.database_config_user ()->get_current_theme ();
  const int direction = database::config::bible::get_text_direction (bible);
  const int lineheight = database::config::bible::get_line_height (bible);
  const int letterspacing = database::config::bible::get_letter_spacing (bible);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", cls);
  view.set_variable ("custom_css", Filter_Css::get_css (cls, fonts::logic::get_font_path (font),
                                                        direction, lineheight, letterspacing));
  
  // Whether to enable fast Bible editor switching.
  if (webserver_request.database_config_user ()->get_fast_editor_switching_available ()) {
    view.enable_zone ("fastswitcheditor");
  }
  
  // Whether to enable spell check in the editor.
  view.set_variable ("spellcheck", filter::string::convert_to_true_false(webserver_request.database_config_user ()->get_enable_spell_check()));
  

  page.append (view.render ("editusfm", "index"));
  
  
  page.append (assets_page::footer ());
  
  
  return page;
}


// Tests for the USFM editor:
// * Autosave on going to another passage.
// * Autosave on document unload.
// * Autosave shortly after any change.
