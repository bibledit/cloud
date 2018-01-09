/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


string edit_preview_url ()
{
  return "edit/preview";
}


bool edit_preview_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string edit_preview (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  bool touch = request->session_logic ()->touchEnabled ();
  bool timeout = request->query.count ("timeout");
  string caller = request->query ["caller"];

  string page;
  
  Assets_Header header = Assets_Header (translate("Preview"), request);
  header.setNavigator ();
  header.setEditorStylesheet ();
  if (touch) header.jQueryTouchOn ();
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  if (timeout) header.refresh (5, "../" + caller + "/index");
  page = header.run ();
  
  Assets_View view;
  
  // Get active Bible, and check read access to it.
  // If needed, change Bible to one it has read access to.
  string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  
  string cls = Filter_Css::getClass (bible);
  string font = Fonts_Logic::getTextFont (bible);
  int direction = Database_Config_Bible::getTextDirection (bible);
  int lineheight = Database_Config_Bible::getLineHeight (bible);
  int letterspacing = Database_Config_Bible::getLetterSpacing (bible);
  view.set_variable ("custom_class", cls);
  view.set_variable ("custom_css", Filter_Css::getCss (cls,
                                                       Fonts_Logic::getFontPath (font),
                                                       direction,
                                                       lineheight,
                                                       letterspacing));
  
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  
  string usfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesheet);
  editor_usfm2html.run ();
  
  string html = editor_usfm2html.get ();
  view.set_variable ("html", html);
 
  if (timeout) {
    view.enable_zone ("timeout");
    view.set_variable ("caller", caller);
  }
  
  page += view.render ("edit", "preview");
  
  page += Assets_Page::footer ();
  
  return page;
}
