/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <search/replace.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/config/general.h>
#include <access/bible.h>
#include <search/logic.h>
#include <menu/logic.h>


string search_replace_url ()
{
  return "search/replace";
}


bool search_replace_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string search_replace (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string siteUrl = config_logic_site_url (webserver_request);
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("b")) {
    bible = request->query ["b"];
  }
  
  string searchfor = request->query ["q"];
  string replacewith = request->query ["r"];
  string id = request->query ["id"];
  
  if (!id.empty ()) {
    
    // Get the Bible and passage for this identifier.
    Passage passage = Passage::decode (id);
    string bible = passage.bible;
    int book = passage.book;
    int chapter = passage.chapter;
    string verse = passage.verse;
    
    // Get the plain text.
    string text = search_logic_get_bible_verse_text (bible, book, chapter, convert_to_int (verse));
    
    // Format it.
    string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    string oldtext = text;
    string newtext = filter_string_str_replace (searchfor, replacewith, text);
    if (replacewith != "") newtext = filter_string_markup_words ({replacewith}, newtext);
    
    string output =
    "<div id=\"" + convert_to_string (id) + "\">\n"
    "<p><a href=\"replace\"> ✔ </a> <a href=\"delete\">" + emoji_wastebasket () + "</a> $link</p>\n"
    "<p>" + oldtext + "</p>\n"
    "<p>" + newtext + "</p>\n"
    "</div>\n";
    
    // Output to browser.
    return output;
  }
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Replace"), request);
  header.addBreadCrumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view;

  view.set_variable ("bible", bible);
  
  string script = "var searchBible = \"" + bible + "\";";
  view.set_variable ("script", script);
  
  page += view.render ("search", "replace");
  
  page += Assets_Page::footer ();
  
  return page;
}
