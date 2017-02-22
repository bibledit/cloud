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


#include <search/strong.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/bible.h>
#include <database/kjv.h>
#include <ipc/focus.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <access/bible.h>


string search_strong_url ()
{
  return "search/strong";
}


bool search_strong_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string search_strong (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;


  Database_Kjv database_kjv = Database_Kjv ();
  
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("b")) {
    bible = request->query ["b"];
  }
  
  
  if (request->query.count ("load")) {

    int book = Ipc_Focus::getBook (request);
    int chapter = Ipc_Focus::getChapter (request);
    int verse = Ipc_Focus::getVerse (request);
    
    // Get Strong's numbers, plus English snippets.
    string html = "<table>\n";
    vector <Database_Kjv_Item> details = database_kjv.getVerse (book, chapter, verse);
    for (auto & detail : details) {
      string strong = detail.strong;
      string english = detail.english;
      html += "<tr><td><a href=\"" + strong + "\">" + strong + "</a></td><td>" + english + "</td></tr>\n";
    }
    html += "</table>\n";
    
    return html;
  }
  
  
  if (request->query.count ("strong")) {
    
    string strong = request->query ["strong"];
    strong = filter_string_trim (strong);
    
    vector <int> passages;
    
    vector <Passage> details = database_kjv.searchStrong (strong);
    
    for (auto & passage : details) {
      int i_passage = filter_passage_to_integer (passage);
      passages.push_back (i_passage);
    }
    
    passages = array_unique (passages);
    sort (passages.begin(), passages.end());
    
    string output;
    for (auto & passage : passages) {
      if (!output.empty()) output.append ("\n");
      output.append (convert_to_string (passage));
    }
    return output;
  }
  
  
  if (request->query.count ("id")) {
    int id = convert_to_int (request->query ["id"]);
    
    // Get the and passage for this identifier.
    Passage passage = filter_integer_to_passage (id);
    int book = passage.book;
    int chapter = passage.chapter;
    string verse = passage.verse;
    
    // Get the plain text.
    string text = search_logic_get_bible_verse_text (bible, book, chapter, convert_to_int (verse));
    
    // Format it.
    string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }
  
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Search"), request);
  header.setNavigator ();
  header.addBreadCrumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view;
  
  view.set_variable ("bible", bible);
  
  string script = "var searchBible = \"" + bible + "\";";
  view.set_variable ("script", script);

  page += view.render ("search", "strong");
  
  page += Assets_Page::footer ();
  
  return page;
}
