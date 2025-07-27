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


#include <read/index.h>
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
#include <database/cache.h>
#include <fonts/logic.h>
#include <navigation/passage.h>
#include <dialog/select.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <bb/logic.h>
#include <config/globals.h>
#include <workspace/logic.h>
#include <public/new.h>
#include <public/notes.h>


std::string read_index_url ()
{
  return "read/index";
}


bool read_index_acl (Webserver_Request& webserver_request)
{
  int role = roles::translator;
  if (roles::access_control (webserver_request, role))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string read_index (Webserver_Request& webserver_request)
{
  bool touch = webserver_request.session_logic ()->get_touch_enabled ();
  
  if (webserver_request.query.count ("switchbook") && webserver_request.query.count ("switchchapter")) {
    int switchbook = filter::strings::convert_to_int (webserver_request.query ["switchbook"]);
    int switchchapter = filter::strings::convert_to_int (webserver_request.query ["switchchapter"]);
    Ipc_Focus::set (webserver_request, switchbook, switchchapter, 1);
    navigation_passage::record_history (webserver_request, switchbook, switchchapter, 1);
  }

  // Set the user chosen Bible as the current Bible.
  if (webserver_request.post.count ("bibleselect")) {
    std::string bibleselect = webserver_request.post ["bibleselect"];
    webserver_request.database_config_user ()->set_bible (bibleselect);
    return std::string();
  }

  std::string page;
  
  Assets_Header header = Assets_Header (translate("Edit verse"), webserver_request);
  header.set_navigator ();
  header.set_editor_stylesheet ();
  if (touch) header.jquery_touch_on ();
  header.notify_it_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  Assets_View view;
  
  // Get active Bible, and check read access to it.
  // Or if the user has used a query to preset the active Bible, get that preset Bible.
  // If needed, change the Bible to one the user has read access to.
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible ());
  {
    constexpr const char* identification {"bible"};
    if (webserver_request.post.count (identification)) {
      bible = webserver_request.post.at(identification);
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
  
  // Create a script for Javascript. Quote string to get legal Javascript.
  std::stringstream script_stream {};
  script_stream << "var readchooseEditorVerseLoaded = " << std::quoted(locale_logic_text_loaded ()) << ";\n";
  script_stream << "var readchooseEditorVerseUpdating = " << std::quoted(locale_logic_text_updating ()) << ";\n";
  script_stream << "var readchooseEditorVerseUpdated = " << std::quoted(locale_logic_text_updated ()) << ";\n";
  script_stream << "var readchooseEditorWillSave = " << std::quoted(locale_logic_text_will_save ()) << ";\n";
  script_stream << "var readchooseEditorVerseSaving = " << std::quoted(locale_logic_text_saving ()) << ";\n";
  script_stream << "var readchooseEditorVerseSaved = " << std::quoted(locale_logic_text_saved ()) << ";\n";
  script_stream << "var readchooseEditorVerseRetrying = " << std::quoted(locale_logic_text_retrying ()) << ";\n";
  script_stream << "var readchooseEditorVerseUpdatedLoaded = " << std::quoted(locale_logic_text_reload ()) << ";\n";
  int verticalCaretPosition = webserver_request.database_config_user ()->get_vertical_caret_position ();
  script_stream << "var verticalCaretPosition = " << verticalCaretPosition << ";\n";
  script_stream << "var verseSeparator = " << std::quoted(database::config::general::get_notes_verse_separator ()) << ";\n";
  std::string script {script_stream.str()};
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);

  std::string cls = Filter_Css::getClass (bible);
  std::string font = fonts::logic::get_text_font (bible);
  int current_theme_index = webserver_request.database_config_user ()->get_current_theme ();
  int direction = database::config::bible::get_text_direction (bible);
  int lineheight = database::config::bible::get_line_height (bible);
  int letterspacing = database::config::bible::get_letter_spacing (bible);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", cls);
  view.set_variable ("custom_css", Filter_Css::get_css (cls,
                                                       fonts::logic::get_font_path (font),
                                                       direction,
                                                       lineheight,
                                                       letterspacing));
  
  // Whether to enable fast Bible editor switching.
  if (webserver_request.database_config_user ()->get_fast_editor_switching_available ()) {
    view.enable_zone ("fastswitcheditor");
  }

  // Whether to enable the styles button.
  if (webserver_request.database_config_user ()->get_enable_styles_button_visual_editors ()) {
    view.enable_zone ("stylesbutton");
  }

  // Enable one status by default.
  view.enable_zone ("onestatus");

  page += view.render ("read", "index");
  
  page += assets_page::footer ();
  
  return page;
}

