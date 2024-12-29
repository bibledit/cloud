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


#include <search/all.h>
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
#include <notes/note.h>
#include <search/logic.h>
#include <menu/logic.h>


std::string search_all_url ()
{
  return "search/all";
}


bool search_all_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) 
    return true;
  auto [ read, write ] =  access_bible::any (webserver_request);
  return read;
}


std::string search_all (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Search"), webserver_request);
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();

  
  Assets_View view;
  
  
  // The query: The word or string to search for.
  // Put the query string into the search box.
  std::string queryString;
  if (webserver_request.query.count ("q")) {
    queryString = webserver_request.query ["q"];
  }
  
  
  view.set_variable ("query", queryString);

  
  // Clean the query string up.
  queryString = filter::strings::trim (queryString);
  
  
  // Generate search words for emphasizing the search passages.
  std::vector <std::string> queryWords = filter::strings::explode (queryString, ' ');
  
  
  Database_Notes database_notes (webserver_request);

  
  const std::string site_url = config::logic::site_url (webserver_request);

  
  std::vector <std::string> bibles = access_bible::bibles (webserver_request);


  // Search the notes.
  std::vector <int> identifiers = database_notes.search_notes (queryString, bibles);
  
  
  size_t note_count = identifiers.size();
  view.set_variable ("note_count", std::to_string (note_count));
  
  
  // Assemble the block of search results for the consultation notes.
  std::string notesblock;
  for (auto identifier : identifiers) {
    
    // The title.
    std::string summary = database_notes.get_summary (identifier);
    std::string verses = filter_passage_display_inline (database_notes.get_passages (identifier));
    std::string title = summary + " | " + verses;
    title = filter::strings::escape_special_xml_characters (title);
    
    // The url.
    std::string url = site_url + notes_note_url () + "?id=" + std::to_string (identifier);
    
    // The excerpt.
    std::string stext = database_notes.get_search_field (identifier);
    std::vector <std::string> vtext = filter::strings::explode (stext, '\n');
    std::string excerpt;
    // Go through each line of text separately.
    for (auto & line : vtext) {
      std::string markedLine = filter::strings::markup_words (queryWords, line);
      // If the line is marked up, add it to the excerpts.
      if (!excerpt.empty()) excerpt.append ("\n");
      if (markedLine != line) {
        excerpt.append ("<p style=\"margin-top: 0em\">" + markedLine + "</p>");
      }
    }
    
    // The html to display.
    std::string html = "<p style=\"margin-top: 0.75em; margin-bottom: 0em\"><a href=\"";
    html.append (url);
    html.append ("\">");
    html.append (title);
    html.append ("</a></p>");
    html.append (excerpt);
    if (!notesblock.empty ()) notesblock.append ("\n");
    notesblock.append (html);
  }
 
  
  // Display the search results for the notes.
  view.set_variable ("notesblock", notesblock);
  
  
  // Search the Bible text.
  std::vector <Passage> passages = search_logic_search_text (queryString, bibles);
  
  
  size_t textCount = passages.size ();
  view.set_variable ("textCount", std::to_string (textCount));
  
  
  // Assemble the search results for the Bible text.
  std::string textblock;
  for (auto & passage : passages) {
    std::string bible = passage.m_bible;
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    std::string verse = passage.m_verse;
    // The title plus link.
    std::string link = bible + " | " + filter_passage_link_for_opening_editor_at (book, chapter, verse);
    // The excerpt.
    std::string stext = search_logic_get_bible_verse_text (bible, book, chapter, filter::strings::convert_to_int (verse));
    std::vector <std::string> vtext = filter::strings::explode (stext, '\n');
    std::string excerpt;
    // Go through each line of text separately.
    for (auto & line : vtext) {
      std::string markedLine = filter::strings::markup_words (queryWords, line);
      if (markedLine != line) {
        // Store this bit of the excerpt.
        excerpt.append ("<p style=\"margin-top: 0em\">" + markedLine + "</p>\n");
      }
    }
    
    if (!textblock.empty ()) textblock.append ("\n");
    textblock.append ("<p style=\"margin-top: 0.75em; margin-bottom: 0em\">");
    textblock.append (link);
    textblock.append ("</p>");
    textblock.append (excerpt);
  }
  
  
  // Display the search results for the Bible text.
  view.set_variable ("textblock", textblock);
  
  
  page += view.render ("search", "all");
  
  
  page += assets_page::footer ();
  
  
  return page;
}
