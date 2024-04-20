/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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
#include <dialog/list2.h>


std::string search_index_url ()
{
  return "search/index";
}


bool search_index_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) 
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string search_index (Webserver_Request& webserver_request)
{
  std::string bible = webserver_request.database_config_user()->getBible ();
  if (webserver_request.query.count ("b")) {
    bible = webserver_request.query ["b"];
  }
 

  const bool q_is_set = webserver_request.query.count ("q");
  const std::string q = webserver_request.query ["q"];
  

  if (webserver_request.query.count ("id")) {
    const std::string id = webserver_request.query ["id"];

    // Get the Bible and passage for this identifier.
    const Passage passage = Passage::decode (id);
    const std::string bible2 = passage.m_bible;
    const int book = passage.m_book;
    const int chapter = passage.m_chapter;
    const std::string verse = passage.m_verse;
    
    // Get the plain text.
    std::string text = search_logic_get_bible_verse_text (bible2, book, chapter, filter::strings::convert_to_int (verse));
    
    // Format it.
    const std::string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    text = filter::strings::markup_words ({q}, text);
    const std::string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }
  

  if (q_is_set) {
    // Search in the active Bible.
    const std::vector <Passage> passages = search_logic_search_bible_text (bible, q);
    // Output results.
    std::string output;
    for (const auto& passage : passages) {
      if (!output.empty ())
        output.append ("\n");
      output.append (passage.encode ());
    }
    return output;
  }
  
  
  // Set the user chosen Bible as the current Bible.
  if (webserver_request.post.count ("bibleselect")) {
    const std::string bibleselect = webserver_request.post ["bibleselect"];
    webserver_request.database_config_user ()->setBible (bibleselect);
    return std::string();
  }

  
  std::string page{};

  Assets_Header header = Assets_Header (translate("Search"), webserver_request);
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view{};
  
  {
    std::string bible_html;
    const std::vector <std::string> accessible_bibles = access_bible::bibles (webserver_request);
    for (const auto& selectable_bible : accessible_bibles) {
      bible_html = Options_To_Select::add_selection (selectable_bible, selectable_bible, bible_html);
    }
    view.set_variable ("bibleoptags", Options_To_Select::mark_selected (bible, bible_html));
  }
  view.set_variable ("bible", bible);
  
  std::stringstream script {};
  script << "var searchBible = " << std::quoted(bible) << ";";
  view.set_variable ("script", script.str());
  
  page += view.render ("search", "index");
  
  page += assets_page::footer ();
  
  return page;
}
