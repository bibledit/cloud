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


#include <search/strongs.h>
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


std::string search_strongs_url ()
{
  return "search/strongs";
}


bool search_strongs_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string search_strongs (Webserver_Request& webserver_request)
{
  Database_Kjv database_kjv = Database_Kjv ();
  

  std::string bible = webserver_request.database_config_user()->getBible ();
  if (webserver_request.query.count ("b")) {
    bible = webserver_request.query ["b"];
  }
  
  
  if (webserver_request.query.count ("load")) {
    
    const int book = Ipc_Focus::getBook (webserver_request);
    const int chapter = Ipc_Focus::getChapter (webserver_request);
    const int verse = Ipc_Focus::getVerse (webserver_request);
    
    // Get Strong's numbers, plus English snippets.
    std::string searchtext {};
    std::vector <Database_Kjv_Item> details = database_kjv.getVerse (book, chapter, verse);
    for (unsigned int i = 0; i < details.size(); i++) {
      if (i) searchtext += " ";
      searchtext += details[i].strong;
      searchtext += " (";
      searchtext += details[i].english;
      searchtext += ")";
    }
    
    searchtext = filter::strings::trim (searchtext);
    
    return searchtext;
  }
  
  
  if (webserver_request.query.count ("words")) {
    
    std::string s_words {webserver_request.query ["words"]};
    s_words = filter::strings::trim (s_words);
    const std::vector <std::string> words {filter::strings::explode (s_words, ' ')};
    
    // Include items if there are no more search hits than 30% of the total number of verses in the KJV.
    const size_t maxcount = static_cast<size_t> (round (0.3 * 31102));
    
    // Store how often a verse occurs in an array.
    // The keys are the passages of the search results.
    // The values are how often the passages occur in the search results.
    std::map <int, int> passages {};
    
    for (const auto& strong : words) {
      
      // Skip short words.
      if (strong.length () < 2) continue;
      
      // Find out how often this word occurs in the Bible. Skip if too often.
      const std::vector <Passage> details {database_kjv.searchStrong (strong)};
      if (details.size() < 1) continue;
      if (details.size () > maxcount) continue;
      
      // Store the identifiers and their count.
      for (const auto& passage : details) {
        const int i_passage = filter_passage_to_integer (passage);
        if (passages.count (i_passage)) passages [i_passage]++;
        else passages [i_passage] = 1;
      }
      
    }
    
    // Sort on occurrence from high to low.
    // Skip identifiers that only occur once.
    std::vector <int> i_passages {};
    std::vector <int> counts {};
    for (const auto& element : passages) {
      int i_passage = element.first;
      const int count = element.second;
      if (count <= 1) continue;
      i_passages.push_back (i_passage);
      counts.push_back (count);
    }
    filter::strings::quick_sort (counts, i_passages, 0, static_cast<unsigned int>(counts.size()));
    reverse (i_passages.begin(), i_passages.end());

    // Output the passage identifiers to the browser.
    std::string output {};
    for (auto & i_passage : i_passages) {
      if (!output.empty ()) output.append ("\n");
      output.append (std::to_string (i_passage));
    }
    return output;
  }
  
  
  if (webserver_request.query.count ("id")) {
    const int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
    
    // Get the and passage for this identifier.
    const Passage passage = filter_integer_to_passage (id);
    const int book = passage.m_book;
    const int chapter = passage.m_chapter;
    const std::string verse = passage.m_verse;
    
    // Get the plain text.
    const std::string text = search_logic_get_bible_verse_text (bible, book, chapter, filter::strings::convert_to_int (verse));
    
    // Format it.
    const std::string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    const std::string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }
  
  
  std::string page {};
  
  Assets_Header header = Assets_Header (translate("Search"), webserver_request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view {};
  
  view.set_variable ("bible", bible);
  
  std::stringstream script {};
  script << "var searchBible = " << std::quoted(bible) << ";";
  view.set_variable ("script", script.str());

  page += view.render ("search", "strongs");
  
  page += assets_page::footer ();
  
  return page;
}
