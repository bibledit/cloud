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


std::string search_strong_url ()
{
  return "search/strong";
}


bool search_strong_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


std::string search_strong (Webserver_Request& webserver_request)
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
    std::string html = "<table>\n";
    std::vector <Database_Kjv_Item> details = database_kjv.getVerse (book, chapter, verse);
    for (auto & detail : details) {
      std::string strong = detail.strong;
      std::string english = detail.english;
      html += "<tr><td><a href=\"" + strong + "\">" + strong + "</a></td><td>" + english + "</td></tr>\n";
    }
    html += "</table>\n";
    
    return html;
  }
  
  
  if (webserver_request.query.count ("strong")) {
    
    std::string strong = webserver_request.query ["strong"];
    strong = filter::strings::trim (strong);
    
    std::vector <int> passages;
    
    std::vector <Passage> details = database_kjv.searchStrong (strong);
    
    for (auto & passage : details) {
      int i_passage = filter_passage_to_integer (passage);
      passages.push_back (i_passage);
    }
    
    passages = filter::strings::array_unique (passages);
    sort (passages.begin(), passages.end());
    
    std::string output;
    for (auto & passage : passages) {
      if (!output.empty()) output.append ("\n");
      output.append (filter::strings::convert_to_string (passage));
    }
    return output;
  }
  
  
  if (webserver_request.query.count ("id")) {
    int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
    
    // Get the and passage for this identifier.
    Passage passage = filter_integer_to_passage (id);
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    std::string verse = passage.m_verse;
    
    // Get the plain text.
    std::string text = search_logic_get_bible_verse_text (bible, book, chapter, filter::strings::convert_to_int (verse));
    
    // Format it.
    std::string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
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

  page += view.render ("search", "strong");
  
  page += assets_page::footer ();
  
  return page;
}
