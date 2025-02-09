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


#include <edit/preview.h>
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
#include <editor/usfm2html.h>


std::string edit_preview_url ()
{
  return "edit/preview";
}


bool edit_preview_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) 
    return true;
  const auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string edit_preview (Webserver_Request& webserver_request)
{
  const bool touch = webserver_request.session_logic ()->get_touch_enabled ();
  const bool timeout = webserver_request.query.count ("timeout");
  const std::string caller = webserver_request.query ["caller"];
  std::string bible = webserver_request.query ["bible"];

  std::string page;
  
  Assets_Header header = Assets_Header (translate("Preview"), webserver_request);
  header.set_navigator ();
  header.set_editor_stylesheet ();
  if (touch) header.jquery_touch_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  if (timeout) header.refresh (5, "../" + caller + "/index");
  page = header.run ();
  
  Assets_View view;

  // Check on whether the Bible was passed.
  // If so, write that to the active Bible in the user configuration.
  // More info: https://github.com/bibledit/cloud/issues/1003
  if (!bible.empty())
    webserver_request.database_config_user()->setBible(bible);

  // Get active Bible, and check read access to it.
  // If needed, change Bible to one it has read access to.
  bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());

  std::string cls = Filter_Css::getClass (bible);
  std::string font = fonts::logic::get_text_font (bible);
  int direction = database::config::bible::get_text_direction (bible);
  int lineheight = database::config::bible::get_line_height (bible);
  int letterspacing = database::config::bible::get_letter_spacing (bible);
  view.set_variable ("custom_class", cls);
  view.set_variable ("custom_css", Filter_Css::get_css (cls,
                                                       fonts::logic::get_font_path (font),
                                                       direction,
                                                       lineheight,
                                                       letterspacing));
  
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  
  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
  
  std::string usfm = database::bibles::get_chapter (bible, book, chapter);
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.set_preview();
  editor_usfm2html.run ();
  
  std::string html = editor_usfm2html.get ();
  view.set_variable ("html", html);
 
  if (timeout) {
    view.enable_zone ("timeout");
    view.set_variable ("caller", caller);
  }

  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  
  page += view.render ("edit", "preview");
  
  page += assets_page::footer ();
  
  return page;
}
