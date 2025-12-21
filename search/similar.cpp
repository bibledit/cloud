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


#include <search/similar.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/temporal.h>
#include <database/config/bible.h>
#include <ipc/focus.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <access/bible.h>


std::string search_similar_url ()
{
  return "search/similar";
}


bool search_similar_acl (Webserver_Request& webserver_request)
{
  if (roles::access_control (webserver_request, roles::consultant))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string search_similar (Webserver_Request& webserver_request)
{
  const int myIdentifier = filter::strings::user_identifier (webserver_request);
  
  
  std::string bible = webserver_request.database_config_user()->get_bible ();
  if (webserver_request.query.count ("b")) {
    bible = webserver_request.query ["b"];
  }


  if (webserver_request.query.count ("load")) {
    const int book = Ipc_Focus::getBook (webserver_request);
    const int chapter = Ipc_Focus::getChapter (webserver_request);
    const int verse = Ipc_Focus::getVerse (webserver_request);
    // Text of the focused verse in the active Bible.
    // Remove all punctuation from it.
    std::string versetext = search_logic_get_bible_verse_text (bible, book, chapter, verse);
    std::vector <std::string> punctuation = filter::strings::explode (database::config::bible::get_sentence_structure_end_punctuation (bible), ' ');
    for (auto & sign : punctuation) {
      versetext = filter::strings::replace (sign, "", versetext);
    }
    punctuation = filter::strings::explode (database::config::bible::get_sentence_structure_middle_punctuation (bible), ' ');
    for (auto & sign : punctuation) {
      versetext = filter::strings::replace (sign, "", versetext);
    }
    versetext = filter::strings::trim (versetext);
    database::temporal::set_value (myIdentifier, "searchsimilar", versetext);
    return versetext;
  }
  
  
  if (webserver_request.query.count ("words")) {
    
    std::string words = webserver_request.query ["words"];
    words = filter::strings::trim (words);
    database::temporal::set_value (myIdentifier, "searchsimilar", words);
    std::vector <std::string> vwords = filter::strings::explode (words, ' ');
    
    // Include items if there are no more search hits than 30% of the total number of verses in the Bible.
    size_t maxcount = static_cast<size_t> (round (0.3 * search_logic_get_verse_count (bible)));
    
    // Store how often a verse occurs in an array.
    // The keys are the identifiers of the search results.
    // The values are how often the identifiers occur in the entire focused verse.
    std::map <int, int> identifiers;
    
    for (auto & word : vwords) {
      
      // Find out how often this word occurs in the Bible. Skip if too often.
      std::vector <Passage> passages = search_logic_search_bible_text (bible, word);
      if (passages.size () > maxcount) continue;
      
      // Store the identifiers and their count.
      for (auto & passage : passages) {
        int id = filter_passage_to_integer (passage);
        if (identifiers.count (id)) identifiers [id]++;
        else identifiers [id] = 1;
      }
      
    }
    
    // Sort on occurrence from high to low.
    // Skip identifiers that only occur once.
    std::vector <int> ids;
    std::vector <int> counts;
    for (auto & element : identifiers) {
      int id = element.first;
      int count = element.second;
      if (count <= 1) continue;
      ids.push_back (id);
      counts.push_back (count);
    }
    filter::strings::quick_sort (counts, ids, 0, static_cast<unsigned> (counts.size()));
    reverse (ids.begin(), ids.end());

    // Output the passage identifiers to the browser.
    std::string output;
    for (auto & id : ids) {
      if (!output.empty ()) output.append ("\n");
      output.append (std::to_string (id));
    }
    return output;
  }
  
  
  if (webserver_request.query.count ("id")) {
    int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
    
    // Get the Bible and passage for this identifier.
    Passage passage = filter_integer_to_passage (id);
    std::string bible2 = webserver_request.database_config_user()->get_bible ();
    // string bible = passage.bible;
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    std::string verse = passage.m_verse;
    
    // Get the plain text.
    std::string text = search_logic_get_bible_verse_text (bible2, book, chapter, filter::strings::convert_to_int (verse));
    
    // Get search words.
    std::vector <std::string> words = filter::strings::explode (database::temporal::get_value (myIdentifier, "searchsimilar"), ' ');
    
    // Format it.
    std::string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    text = filter::strings::markup_words (words, text);
    std::string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }

  
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Search"), webserver_request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view;
  
  view.set_variable ("bible", bible);
  
  std::stringstream script {};
  script << "var searchBible = " << std::quoted(bible) << ";";
  view.set_variable ("script", script.str());

  page += view.render ("search", "similar");
  
  page += assets_page::footer ();
  
  return page;
}
