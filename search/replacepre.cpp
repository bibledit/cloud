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


#include <search/replacepre.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/config/general.h>
#include <search/logic.h>
#include <access/bible.h>


string search_replacepre_url ()
{
  return "search/replacepre";
}


bool search_replacepre_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string search_replacepre (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string siteUrl = config_logic_site_url (webserver_request);
  
  
  // Get search variables from the query.
  string searchfor = request->query ["q"];
  string replacewith = request->query ["r"];
  bool casesensitive = (request->query ["c"] == "true");
  string id = request->query ["id"];
  
  
  // Get the Bible and passage for this identifier.
  Passage passage = Passage::decode (id);
  string bible = passage.bible;
  int book = passage.book;
  int chapter = passage.chapter;
  string verse = passage.verse;
  
  
  // Get the plain text.
  string text = search_logic_get_bible_verse_text (bible, book, chapter, convert_to_int (verse));
  
  
  // Clickable passage.
  string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
  
  
  string oldtext = filter_string_markup_words ({searchfor}, text);
  

  string newtext;
  if (casesensitive) {
    newtext = filter_string_str_replace (searchfor, replacewith, text);
  } else {
    newtext = text;
    vector <string> needles = filter_string_search_needles (searchfor, text);
    for (auto & needle : needles) {
      newtext = filter_string_str_replace (needle, replacewith, newtext);
    }
  }
  if (replacewith != "") newtext = filter_string_markup_words ({replacewith}, newtext);
  
  
  // Check whether the user has write access to the book.
  string user = request->session_logic ()->currentUser ();
  bool write = access_bible_book_write (webserver_request, user, bible, book);

  
  // Create output.
  string output;
  output.append ("<div id=\"" + id + "\">\n");
  output.append ("<p>");
  if (write) output.append ("<a href=\"replace\"> ✔ </a> <a href=\"delete\">" + emoji_wastebasket () + "</a> ");
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
