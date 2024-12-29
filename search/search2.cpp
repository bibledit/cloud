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


#include <search/search2.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/temporal.h>
#include <database/config/general.h>
#include <database/books.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <dialog/list2.h>


std::string search_search2_url ()
{
  return "search/search2";
}


bool search_search2_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string search_search2 (Webserver_Request& webserver_request)
{
  std::string siteUrl = config::logic::site_url (webserver_request);
  
  
  std::string bible = webserver_request.database_config_user()->getBible ();
  if (webserver_request.query.count ("bible")) bible = webserver_request.query ["bible"];

  
  bool hit_is_set = webserver_request.query.count ("h");
  bool query_is_set = webserver_request.query.count ("q");
  int identifier = filter::strings::convert_to_int (webserver_request.query ["i"]);
  std::string query = webserver_request.query ["q"];
  std::string hit = webserver_request.query ["h"];

  
  // Get one search hit.
  if (hit_is_set) {
    
    
    // Retrieve the search parameters from the volatile database.
    std::string query2 = database::temporal::get_value (identifier, "query");
    //bool casesensitive = filter::strings::convert_to_bool (database::temporal::get_value (identifier, "casesensitive"));
    bool plaintext = filter::strings::convert_to_bool (database::temporal::get_value (identifier, "plaintext"));
    
    
    // Get the Bible and passage for this identifier.
    Passage details = Passage::decode (hit);
    std::string bible2 = details.m_bible;
    int book = details.m_book;
    int chapter = details.m_chapter;
    std::string verse = details.m_verse;
    
    
    // Get the plain text or USFM.
    std::string text;
    if (plaintext) {
      text = search_logic_get_bible_verse_text (bible2, book, chapter, filter::strings::convert_to_int (verse));
    } else {
      text = search_logic_get_bible_verse_usfm (bible2, book, chapter, filter::strings::convert_to_int (verse));
    }
    
    
    // Format it.
    std::string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    text =  filter::strings::markup_words ({query2}, text);
    std::string output = "<div>" + link + " " + text + "</div>";
    
    
    // Output to browser.
    return output;
  }
  

  // Perform the initial search.
  if (query_is_set) {
    
    
    // Get extra search parameters and store them all in the volatile database.
    bool casesensitive = (webserver_request.query ["c"] == "true");
    bool plaintext = (webserver_request.query ["p"] == "true");
    std::string books = webserver_request.query ["b"];
    std::string sharing = webserver_request.query ["s"];
    database::temporal::set_value (identifier, "query", query);
    database::temporal::set_value (identifier, "casesensitive", filter::strings::convert_to_string (casesensitive));
    database::temporal::set_value (identifier, "plaintext", filter::strings::convert_to_string (plaintext));
    
    
    // Deal with case sensitivity.
    // Deal with whether to search the plain text, or the raw USFM.
    // Fetch the initial set of hits.
    std::vector <Passage> passages;
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
    if (books == "currentbook") {
      int book = Ipc_Focus::getBook (webserver_request);
      std::vector <Passage> bookpassages;
      for (auto & passage : passages) {
        if (book == passage.m_book) {
          bookpassages.push_back (passage);
        }
      }
      passages = bookpassages;
    }
    
    
    // Deal with possible searching in Old or New Testament only.
    bool otbooks = (books == "otbooks");
    bool ntbooks = (books == "ntbooks");
    if (otbooks || ntbooks) {
      std::vector <Passage> bookpassages;
      for (auto & passage : passages) {
        book_type type = database::books::get_type (static_cast<book_id>(passage.m_book));
        if (otbooks) if (type != book_type::old_testament) continue;
        if (ntbooks) if (type != book_type::new_testament) continue;
        bookpassages.push_back (passage);
      }
      passages = bookpassages;
    }
    

    // Deal with how to share the results.
    std::vector <std::string> hits;
    for (auto & passage : passages) {
      hits.push_back (passage.encode ());
    }
    if (sharing != "load") {
      std::vector <std::string> loaded_hits = filter::strings::explode (database::temporal::get_value (identifier, "hits"), '\n');
      if (sharing == "add") {
        hits.insert (hits.end(), loaded_hits.begin(), loaded_hits.end());
      }
      if (sharing == "remove") {
        hits = filter::strings::array_diff (loaded_hits, hits);
      }
      if (sharing == "intersect") {
        hits = array_intersect (loaded_hits, hits);
      }
      hits = filter::strings::array_unique (hits);
    }


    // Generate one string from the hits.
    std::string output = filter::strings::implode (hits, "\n");


    // Store search hits in the volatile database.
    database::temporal::set_value (identifier, "hits", output);


    // Output results.
    return output;
  }

  
  // Set the user chosen Bible as the current Bible.
  if (webserver_request.post.count ("bibleselect")) {
    std::string bibleselect = webserver_request.post ["bibleselect"];
    webserver_request.database_config_user ()->setBible (bibleselect);
    return std::string();
  }
  
  // Build the advanced search page.
  std::string page;
  Assets_Header header = Assets_Header (translate("Search"), webserver_request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  Assets_View view;
  {
    std::string bible_html;
    std::vector <std::string> accessible_bibles = access_bible::bibles (webserver_request);
    for (auto selectable_bible : accessible_bibles) {
      bible_html = Options_To_Select::add_selection (selectable_bible, selectable_bible, bible_html);
    }
    view.set_variable ("bibleoptags", Options_To_Select::mark_selected (bible, bible_html));
  }
  view.set_variable ("bible", bible);
  std::stringstream script {};
  script << "var searchBible = " << std::quoted(bible) << ";";
  view.set_variable ("script", script.str());
  page += view.render ("search", "search2");
  page += assets_page::footer ();
  return page;
}
