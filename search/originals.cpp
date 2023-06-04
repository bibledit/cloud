/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <search/originals.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/bible.h>
#include <database/oshb.h>
#include <database/sblgnt.h>
#include <database/books.h>
#include <ipc/focus.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <access/bible.h>
using namespace std;


string search_originals_url ()
{
  return "search/originals";
}


bool search_originals_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


string search_originals (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);


  Database_OsHb database_oshb = Database_OsHb ();
  Database_Sblgnt database_sblgnt = Database_Sblgnt ();
  
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("b")) {
    bible = request->query ["b"];
  }
  

  if (request->query.count ("load")) {
    
    book_id book = static_cast<book_id>(Ipc_Focus::getBook (request));
    int chapter = Ipc_Focus::getChapter (request);
    int verse = Ipc_Focus::getVerse (request);
    
    book_type type = database::books::get_type (book);
    
    string classs{};
    
    // Get Hebrew or Greek words.
    string searchtext;
    vector <string> details;
    if (type == book_type::old_testament) {
      details = database_oshb.getVerse (static_cast<int>(book), chapter, verse);
      classs = "hebrew";
    }
    if (type == book_type::new_testament) {
      details = database_sblgnt.getVerse (static_cast<int>(book), chapter, verse);
      classs = "greek";
    }
    searchtext = filter::strings::implode (details, " ");
    searchtext = filter::strings::trim (searchtext);
    
    return classs + "\n" + searchtext;
  }
  
  
  if (request->query.count ("words")) {
    string words = request->query ["words"];
    
    words = filter::strings::trim (words);
    vector <string> v_words = filter::strings::explode (words, ' ');
    
    book_id book = static_cast<book_id>(Ipc_Focus::getBook (request));
    book_type type = database::books::get_type (book);
    
    // Include items if there are no more search hits
    // than 30% of the total number of verses in the Hebrew or Greek.
    size_t maxcount {0};
    if (type == book_type::old_testament) {
      maxcount = static_cast<size_t> (round (0.3 * 23145));
    }
    if (type == book_type::new_testament) {
      maxcount = static_cast<size_t> (round (0.3 * 7957));
    }
    
    
    // Store how often a verse occurs in an array.
    // The keys are the passages of the search results.
    // The values are how often the passages occur in the search results.
    map <int, int> passages {};
    
    for (const auto & word : v_words) {
      
      // Find out how often this word occurs in the Hebrew or Greek Bible. Skip if too often.
      vector <Passage> details {};
      if (type == book_type::old_testament) {
        details = database_oshb.searchHebrew (word);
      }
      if (type == book_type::new_testament) {
        details = database_sblgnt.searchGreek (word);
      }
      if (details.size() < 1) continue;
      if (details.size() > maxcount) continue;
      
      // Store the identifiers and their count.
      for (Passage & detail : details) {
        int passage = filter_passage_to_integer (detail);
        if (passages.count (passage)) passages [passage]++;
        else passages [passage] = 1;
      }
      
    }
    
    // Sort on occurrence from high to low.
    // Skip passages that only occur once.
    vector <int> v_passages;
    vector <int> counts;
    for (auto & element : passages) {
      int passage = element.first;
      int count = element.second;
      if (count <= 1) continue;
      v_passages.push_back (passage);
      counts.push_back (count);
    }
    filter::strings::quick_sort (counts, v_passages, 0, static_cast<unsigned> (counts.size()));
    reverse (v_passages.begin(), v_passages.end());

    
    // Output the passages to the browser.
    string output;
    for (auto & passage : v_passages) {
      if (!output.empty ()) output.append ("\n");
      output.append (filter::strings::convert_to_string (passage));
    }
    return output;
  }
  
  
  if (request->query.count ("id")) {
    int id = filter::strings::convert_to_int (request->query ["id"]);
    
    // Get the and passage for this identifier.
    Passage passage = filter_integer_to_passage (id);
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    string verse = passage.m_verse;
    
    // Get the plain text.
    string text = search_logic_get_bible_verse_text (bible, book, chapter, filter::strings::convert_to_int (verse));
    
    // Format it.
    string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }
  
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Search"), request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view;
  
  view.set_variable ("bible", bible);
  
  stringstream script {};
  script << "var searchBible = " << quoted(bible) << ";";
  view.set_variable ("script", script.str());

  page += view.render ("search", "originals");
  
  page += assets_page::footer ();
  
  return page;
}
