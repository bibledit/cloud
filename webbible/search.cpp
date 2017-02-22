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


#include <webbible/search.h>
#include <assets/view.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <search/logic.h>


string webbible_search_url ()
{
  return "webbible/search";
}


bool webbible_search_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string webbible_search (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  Assets_View view;
  
  
  // Get the URL and the text for the backlink.
  string backlinkUrl = request->query.count ("url") ? request->query["url"] : "";
  string backlinkText = request->query.count ("text") ? request->query["text"] : "";
  
  
  // The query: The word or string to search for.
  string queryString = request->query.count ("q") ? request->query["q"] : "";
  
  
  // Put the search query and the backlink into the search box.
  view.set_variable ("queryString", queryString);
  view.set_variable ("backlinkUrl", backlinkUrl);
  view.set_variable ("backlinkText", backlinkText);

  
  // Clean the query string up.
  queryString = filter_string_trim (queryString);
  
  
  // Generate search words for emphasizing the search hits.
  vector <string> queryWords = filter_string_explode (queryString, ' ');
  
  
  // Exported Bible.
  string exportedBible = filter_url_basename_web (filter_url_dirname_web (filter_url_dirname_web (backlinkUrl)));
  
  
  // Search the Bible text in the exported Bible.
  vector <Passage> passages = search_logic_search_text (queryString, {exportedBible});
  
  
  // Hit count.
  view.set_variable ("hitCount", convert_to_string (passages.size ()));
  
  
  string hitsblock;
  
  
  // Go through the search hits.
  for (auto & passage : passages) {
    
    
    // Get the passage of this search hit.
    if (passage.book == 0) continue;
    string bible = passage.bible;
    int book = passage.book;
    int chapter = passage.chapter;
    string verse = passage.verse;
    
    
    // The title.
    string title = bible + " | " + filter_passage_display (book, chapter, verse);
    title = filter_string_sanitize_html (title);
    
    
    // The URL.
    string url = "/exports/" + bible + "/web/" + filter_url_html_file_name_bible ("", book, chapter);
    
    
    // Output title and URL.
    hitsblock.append ("<p style=\"margin-top: 0.75em; margin-bottom: 0em\"><a href=\"" + url + "\">" + title + "</a></p>\n");
    
    
    // The excerpt.
    string text = search_logic_get_bible_verse_text (bible, book, chapter, convert_to_int (verse));
    vector <string> v_text = filter_string_explode (text, '\n');
    // Go through each line of text separately.
    for (auto line : v_text) {
      string markedLine = filter_string_markup_words (queryWords, line);
      if (markedLine != line) {
        // Store this bit of the excerpt.
        hitsblock.append ("<p style=\"margin-top: 0em; margin-bottom: 0em\">" + markedLine + "</p>\n");
      }
    }
  }
  
  view.set_variable ("hitsblock", hitsblock);
  
  return view.render ("webbible", "search");
}
