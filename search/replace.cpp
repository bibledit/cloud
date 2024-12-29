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


#include <search/replace.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/config/general.h>
#include <access/bible.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <dialog/list2.h>


std::string search_replace_url ()
{
  return "search/replace";
}


bool search_replace_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string search_replace (Webserver_Request& webserver_request)
{
  const std::string siteUrl = config::logic::site_url (webserver_request);
  
  std::string bible = webserver_request.database_config_user()->getBible ();
  if (webserver_request.query.count ("b")) {
    bible = webserver_request.query ["b"];
  }
  
  std::string searchfor = webserver_request.query ["q"];
  std::string replacewith = webserver_request.query ["r"];
  std::string id = webserver_request.query ["id"];
  
  if (!id.empty ()) {
    
    // Get the Bible and passage for this identifier.
    Passage passage = Passage::decode (id);
    std::string bible2 = passage.m_bible;
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    std::string verse = passage.m_verse;
    
    // Get the plain text.
    std::string text = search_logic_get_bible_verse_text (bible2, book, chapter, filter::strings::convert_to_int (verse));
    
    // Format it.
    std::string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    std::string oldtext = text;
    std::string newtext = filter::strings::replace (searchfor, replacewith, text);
    if (replacewith != "") newtext = filter::strings::markup_words ({replacewith}, newtext);
    
    std::string output =
    "<div id=\"" + filter::strings::convert_to_string (id) + "\">\n"
    "<p><a href=\"replace\"> ✔ </a> <a href=\"delete\">" + filter::strings::emoji_wastebasket () + "</a> $link</p>\n"
    "<p>" + oldtext + "</p>\n"
    "<p>" + newtext + "</p>\n"
    "</div>\n";
    
    // Output to browser.
    return output;
  }

  // Set the user chosen Bible as the current Bible.
  if (webserver_request.post.count ("bibleselect")) {
    std::string bibleselect = webserver_request.post ["bibleselect"];
    webserver_request.database_config_user ()->setBible (bibleselect);
    return std::string();
  }
  
  std::string page;
  
  Assets_Header header = Assets_Header (translate("Replace"), webserver_request);
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
  
  page += view.render ("search", "replace");
  
  page += assets_page::footer ();
  
  return page;
}
