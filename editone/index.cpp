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


#include <editone/index.h>
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
#include <dialog/select.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <bb/logic.h>
#include <config/globals.h>
#include <workspace/logic.h>
#include <demo/logic.h>


std::string editone_index_url ()
{
  return "editone/index";
}


bool editone_index_acl (Webserver_Request& webserver_request)
{
  // Default minimum role for getting access.
  const int minimum_role = roles::translator;
  if (roles::access_control (webserver_request, minimum_role))
    return true;
  const auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string editone_index (Webserver_Request& webserver_request)
{
  const bool touch = webserver_request.session_logic ()->get_touch_enabled ();
  
  if (webserver_request.query.count ("switchbook") && webserver_request.query.count ("switchchapter")) {
    const int switchbook = filter::strings::convert_to_int (webserver_request.query ["switchbook"]);
    const int switchchapter = filter::strings::convert_to_int (webserver_request.query ["switchchapter"]);
    Ipc_Focus::set (webserver_request, switchbook, switchchapter, 1);
    navigation_passage::record_history (webserver_request, switchbook, switchchapter, 1);
  }

  std::string page;
  
  Assets_Header header = Assets_Header (translate("Edit verse"), webserver_request);
  header.set_navigator ();
  header.set_editor_stylesheet ();
  if (touch) 
    header.jquery_touch_on ();
  header.notify_it_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  Assets_View view;
  
  // Get active Bible, and check read access to it.
  // Or if the user has used a query to preset the active Bible, get that preset Bible.
  // If needed, change the Bible to one it has read access to.
  // Set the chosen Bible on the Bible selector.
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible ());
  if (webserver_request.query.count ("bible")) 
    bible = access_bible::clamp (webserver_request, webserver_request.query ["bible"]);
  {
    constexpr const char* identification {"bibleselect"};
    if (webserver_request.post.count (identification)) {
      bible = webserver_request.post.at(identification);
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

  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", navigation_passage::code (bible));
  
  // Create the script, quote the strings to ensure it's legal Javascript.
  std::stringstream script_stream {};
  script_stream << "var oneverseEditorVerseLoaded = " << std::quoted(locale_logic_text_loaded ()) << ";\n";
  script_stream << "var oneverseEditorVerseUpdating = " << std::quoted(locale_logic_text_updating ()) << ";\n";
  script_stream << "var oneverseEditorVerseUpdated = " << std::quoted(locale_logic_text_updated ()) << ";\n";
  script_stream << "var oneverseEditorWillSave = " << std::quoted(locale_logic_text_will_save ()) << ";\n";
  script_stream << "var oneverseEditorVerseSaving = " << std::quoted(locale_logic_text_saving ()) << ";\n";
  script_stream << "var oneverseEditorVerseSaved = " << std::quoted(locale_logic_text_saved ()) << ";\n";
  script_stream << "var oneverseEditorVerseRetrying = " << std::quoted(locale_logic_text_retrying ()) << ";\n";
  script_stream << "var oneverseEditorVerseUpdatedLoaded = " << std::quoted(locale_logic_text_reload ()) << ";\n";
  int verticalCaretPosition = webserver_request.database_config_user ()->get_vertical_caret_position ();
  script_stream << "var verticalCaretPosition = " << verticalCaretPosition << ";\n";
  script_stream << "var verseSeparator = " << std::quoted(database::config::general::get_notes_verse_separator ()) << ";\n";
  std::string script {script_stream.str()};
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);

  const std::string custom_class = Filter_Css::getClass (bible);
  const std::string font = fonts::logic::get_text_font (bible);
  const int current_theme_index = webserver_request.database_config_user ()->get_current_theme ();
  const int direction = database::config::bible::get_text_direction (bible);
  const int lineheight = database::config::bible::get_line_height (bible);
  const int letterspacing = database::config::bible::get_letter_spacing (bible);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", custom_class);
  const std::string custom_css = Filter_Css::get_css (custom_class,
                                          fonts::logic::get_font_path (font),
                                          direction, lineheight, letterspacing);
  view.set_variable ("custom_css", custom_css);

  
  // Whether to enable fast Bible editor switching.
  if (webserver_request.database_config_user ()->get_fast_editor_switching_available ()) {
    view.enable_zone ("fastswitcheditor");
  }

  // Whether to enable the styles button.
  if (webserver_request.database_config_user ()->get_enable_styles_button_visual_editors ()) {
    view.enable_zone ("stylesbutton");
  }
  
  view.set_variable ("spellcheck", filter::strings::convert_to_true_false(webserver_request.database_config_user ()->get_enable_spell_check()));

  page.append (view.render ("editone", "index"));
  
  page.append (assets_page::footer ());
  
  return page;
}

// Tests for the editor:
// * Autosave on going to another passage.
// * Autosave on document unload.
// * Autosave shortly after any change.
// * Save the + sign of a note.
// * No loss of white space right after the verse number.
