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


#include <search/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <access/bible.h>


string search_index_url ()
{
  return "search/index";
}


bool search_index_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string search_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("b")) {
    bible = request->query ["b"];
  }
 

  bool q_is_set = request->query.count ("q");
  string q = request->query ["q"];
  

  if (request->query.count ("id")) {
    string id = request->query ["id"];

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
    text =  filter_string_markup_words ({q}, text);
    string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }
  

  if (q_is_set) {
    // Search in the active Bible.
    vector <Passage> passages = search_logic_search_bible_text (bible, q);
    // Output results.
    string output;
    for (auto & passage : passages) {
      if (!output.empty ()) output.append ("\n");
      output.append (passage.encode ());
    }
    return output;
  }

  
  string page;

  Assets_Header header = Assets_Header (translate("Search"), request);
  header.addBreadCrumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view;
  
  view.set_variable ("bible", bible);
  
  string script = "var searchBible = \"" + bible + "\";";
  view.set_variable ("script", script);
  
  page += view.render ("search", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}
