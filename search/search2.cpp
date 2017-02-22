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


#include <search/search2.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/volatile.h>
#include <database/config/general.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <search/logic.h>
#include <menu/logic.h>


string search_search2_url ()
{
  return "search/search2";
}


bool search_search2_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string search_search2 (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string siteUrl = config_logic_site_url (webserver_request);
  
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("bible")) bible = request->query ["bible"];

  
  bool hit_is_set = request->query.count ("h");
  bool query_is_set = request->query.count ("q");
  int identifier = convert_to_int (request->query ["i"]);
  string query = request->query ["q"];
  string hit = request->query ["h"];

  
  // Get one search hit.
  if (hit_is_set) {
    
    
    // Retrieve the search parameters from the volatile database.
    string query = Database_Volatile::getValue (identifier, "query");
    //bool casesensitive = convert_to_bool (Database_Volatile::getValue (identifier, "casesensitive"));
    bool plaintext = convert_to_bool (Database_Volatile::getValue (identifier, "plaintext"));
    
    
    // Get the Bible and passage for this identifier.
    Passage details = Passage::decode (hit);
    string bible = details.bible;
    int book = details.book;
    int chapter = details.chapter;
    string verse = details.verse;
    
    
    // Get the plain text or USFM.
    string text;
    if (plaintext) {
      text = search_logic_get_bible_verse_text (bible, book, chapter, convert_to_int (verse));
    } else {
      text = search_logic_get_bible_verse_usfm (bible, book, chapter, convert_to_int (verse));
    }
    
    
    // Format it.
    string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    text =  filter_string_markup_words ({query}, text);
    string output = "<div>" + link + " " + text + "</div>";
    
    
    // Output to browser.
    return output;
  }
  

  // Perform the initial search.
  if (query_is_set) {
    
    
    // Get extra search parameters and store them all in the volatile database.
    bool casesensitive = (request->query ["c"] == "true");
    bool plaintext = (request->query ["p"] == "true");
    bool currentbook = (request->query ["b"] == "true");
    string sharing = request->query ["s"];
    Database_Volatile::setValue (identifier, "query", query);
    Database_Volatile::setValue (identifier, "casesensitive", convert_to_string (casesensitive));
    Database_Volatile::setValue (identifier, "plaintext", convert_to_string (plaintext));
    
    
    // Deal with case sensitivity.
    // Deal with whether to search the plain text, or the raw USFM.
    // Fetch the initial set of hits.
    vector <Passage> passages;
    if (plaintext) {
      if (casesensitive) {
        passages = search_logic_search_bible_text_case_sensitive (bible, query);
      } else {
        passages = search_logic_search_bible_text (bible, query);
      }
    } else {
      if (casesensitive) {
        passages = search_logic_search_bible_usfm_case_sensitive (bible, query);
      } else {
        passages = search_logic_search_bible_usfm (bible, query);
      }
    }
    
    
    // Deal with possible searching in the current book only.
    if (currentbook) {
      int book = Ipc_Focus::getBook (request);
      vector <Passage> bookpassages;
      for (auto & passage : passages) {
        if (book == passage.book) {
          bookpassages.push_back (passage);
        }
      }
      passages = bookpassages;
    }
    

    // Deal with how to share the results.
    vector <string> hits;
    for (auto & passage : passages) {
      hits.push_back (passage.encode ());
    }
    if (sharing != "load") {
      vector <string> loaded_hits = filter_string_explode (Database_Volatile::getValue (identifier, "hits"), '\n');
      if (sharing == "add") {
        hits.insert (hits.end(), loaded_hits.begin(), loaded_hits.end());
      }
      if (sharing == "remove") {
        hits = filter_string_array_diff (loaded_hits, hits);
      }
      if (sharing == "intersect") {
        hits = array_intersect (loaded_hits, hits);
      }
      hits = array_unique (hits);
    }


    // Generate one string from the hits.
    string output = filter_string_implode (hits, "\n");


    // Store search hits in the volatile database.
    Database_Volatile::setValue (identifier, "hits", output);


    // Output results.
    return output;
  }
  
  
  // Build the advanced search page.
  string page;
  Assets_Header header = Assets_Header (translate("Search"), request);
  header.setNavigator ();
  header.addBreadCrumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  Assets_View view;
  view.set_variable ("bible", bible);
  string script = "var searchBible = \"" + bible + "\";";
  view.set_variable ("script", script);
  page += view.render ("search", "search2");
  page += Assets_Page::footer ();
  return page;
}
