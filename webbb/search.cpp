/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <webbb/search.h>
#include <assets/view.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <search/logic.h>


std::string webbible_search_url ()
{
  return "webbb/search";
}


bool webbible_search_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::guest);
}


std::string webbible_search (Webserver_Request& webserver_request)
{
  Assets_View view;
  
  
  // Get the URL and the text for the backlink.
  std::string backlinkUrl = webserver_request.query.count ("url") ? webserver_request.query["url"] : "";
  std::string backlinkText = webserver_request.query.count ("text") ? webserver_request.query["text"] : "";
  
  
  // The query: The word or string to search for.
  std::string queryString = webserver_request.query.count ("q") ? webserver_request.query["q"] : "";
  
  
  // Put the search query and the backlink into the search box.
  view.set_variable ("queryString", queryString);
  view.set_variable ("backlinkUrl", backlinkUrl);
  view.set_variable ("backlinkText", backlinkText);

  
  // Clean the query string up.
  queryString = filter::string::trim (queryString);
  
  
  // Generate search words for emphasizing the search hits.
  std::vector <std::string> queryWords = filter::string::explode (queryString, ' ');
  
  
  // Exported Bible.
  std::string exportedBible = filter_url_basename_web (filter_url_dirname_web (filter_url_dirname_web (backlinkUrl)));
  
  
  // Search the Bible text in the exported Bible.
  std::vector <Passage> passages = search_logic_search_text (queryString, {exportedBible});
  
  
  // Hit count.
  view.set_variable ("hitCount", std::to_string (passages.size ()));
  
  
  std::stringstream hitsblock;
  
  
  // Go through the search hits.
  for (auto & passage : passages) {
    
    
    // Get the passage of this search hit.
    if (passage.m_book == 0) continue;
    std::string bible = passage.m_bible;
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    std::string verse = passage.m_verse;
    
    
    // The title.
    std::string title = bible + " | " + filter_passage_display (book, chapter, verse);
    title = filter::string::escape_special_xml_characters (title);
    
    
    // The URL.
    std::string url = "/exports/" + bible + "/web/" + filter_url_html_file_name_bible (std::string(), book, chapter);
    
    
    // Output title and URL.
    hitsblock << "<p style=" << std::quoted ("margin-top: 0.75em; margin-bottom: 0em") << "><a href=" << std::quoted (url) << ">" << title << "</a></p>" << std::endl;
    
    
    // The excerpt.
    std::string text = search_logic_get_bible_verse_text (bible, book, chapter, filter::string::convert_to_int (verse));
    std::vector <std::string> v_text = filter::string::explode (text, '\n');
    // Go through each line of text separately.
    for (auto line : v_text) {
      std::string markedLine = filter::string::markup_words (queryWords, line);
      if (markedLine != line) {
        // Store this bit of the excerpt.
        hitsblock << "<p style=" << std::quoted ("margin-top: 0em; margin-bottom: 0em") << ">" << markedLine << "</p>" << std::endl;
      }
    }
  }
  
  view.set_variable ("hitsblock", hitsblock.str());
  
  return view.render ("webbb", "search");
}
