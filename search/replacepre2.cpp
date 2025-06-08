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


#include <search/replacepre2.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/config/general.h>
#include <search/logic.h>
#include <access/bible.h>


std::string search_replacepre2_url ()
{
  return "search/replacepre2";
}


bool search_replacepre2_acl (Webserver_Request& webserver_request)
{
  if (roles::access_control (webserver_request, roles::translator))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string search_replacepre2 (Webserver_Request& webserver_request)
{
  std::string siteUrl = config::logic::site_url (webserver_request);
  
  
  // Get search variables from the query.
  std::string searchfor = webserver_request.query ["q"];
  std::string replacewith = webserver_request.query ["r"];
  bool casesensitive = (webserver_request.query ["c"] == "true");
  std::string id = webserver_request.query ["id"];
  bool searchplain = (webserver_request.query ["p"] == "true");
  
  
  // Get the Bible and passage for this identifier.
  Passage details = Passage::decode (id);
  std::string bible = details.m_bible;
  int book = details.m_book;
  int chapter = details.m_chapter;
  std::string verse = details.m_verse;
  
  
  // Get the plain text or the USFM.
  std::string text;
  if (searchplain) {
    text = search_logic_get_bible_verse_text (bible, book, chapter, filter::strings::convert_to_int (verse));
  } else {
    text = search_logic_get_bible_verse_usfm (bible, book, chapter, filter::strings::convert_to_int (verse));
  }
  
  // Clickable passage.
  std::string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
  
  
  std::string oldtext = filter::strings::markup_words ({searchfor}, text);
  

  std::string newtext (text);
  if (casesensitive) {
    newtext = filter::strings::replace (searchfor, replacewith, newtext);
  } else {
    std::vector <std::string> needles = filter::strings::search_needles (searchfor, text);
    for (auto & needle : needles) {
      newtext = filter::strings::replace (needle, replacewith, newtext);
    }
  }
  if (replacewith != "") newtext = filter::strings::markup_words ({replacewith}, newtext);
  
  
  // Check whether the user has write access to the book.
  const std::string& user = webserver_request.session_logic ()->get_username ();
  bool write = access_bible::book_write (webserver_request, user, bible, book);

  
  // Create output.
  std::string output;
  output.append ("<div id=\"" + id + "\">\n");
  output.append ("<p>");
  if (write) output.append ("<a href=\"replace\"> ✔ </a> <a href=\"delete\">" + filter::strings::emoji_wastebasket () + "</a> ");
  output.append (link);
  output.append ("</p>\n");
  output.append ("<p>" + oldtext + "</p>\n");
  output.append ("<p>");
  if (write) output.append (newtext);
  else output.append (locale_logic_text_no_privileges_modify_book ());
  output.append ("</p>\n");
  output.append ("</div>\n");
  
  
  // Output to browser.
  return output;
}
