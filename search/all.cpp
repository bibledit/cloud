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


string search_all_url ()
{
  return "search/all";
}


bool search_all_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return read;
}


string search_all (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate("Search"), request);
  header.addBreadCrumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();

  
  Assets_View view;
  
  
  // The query: The word or string to search for.
  // Put the query string into the search box.
  string queryString;
  if (request->query.count ("q")) {
    queryString = request->query ["q"];
  }
  
  
  view.set_variable ("query", queryString);

  
  // Clean the query string up.
  queryString = filter_string_trim (queryString);
  
  
  // Generate search words for emphasizing the search passages.
  vector <string> queryWords = filter_string_explode (queryString, ' ');
  
  
  Database_Notes database_notes = Database_Notes (request);

  
  string siteUrl = config_logic_site_url (webserver_request);

  
  vector <string> bibles = access_bible_bibles (request);


  // Search the notes.
  vector <int> identifiers = database_notes.searchNotes (queryString, bibles);
  
  
  int noteCount = identifiers.size();
  view.set_variable ("noteCount", convert_to_string (noteCount));
  
  
  // Assemble the block of search results for the consultation notes.
  string notesblock;
  for (auto identifier : identifiers) {
    
    // The title.
    string summary = database_notes.getSummary (identifier);
    string verses = filter_passage_display_inline (database_notes.getPassages (identifier));
    string title = summary + " | " + verses;
    title = filter_string_sanitize_html (title);
    
    // The url.
    string url = siteUrl + notes_note_url () + "?id=" + convert_to_string (identifier);
    
    // The excerpt.
    string stext = database_notes.getSearchField (identifier);
    vector <string> vtext = filter_string_explode (stext, '\n');
    string excerpt;
    // Go through each line of text separately.
    for (auto & line : vtext) {
      string markedLine = filter_string_markup_words (queryWords, line);
      // If the line is marked up, add it to the excerpts.
      if (!excerpt.empty()) excerpt.append ("\n");
      if (markedLine != line) {
        excerpt.append ("<p style=\"margin-top: 0em\">" + markedLine + "</p>");
      }
    }
    
    // The html to display.
    string html = "<p style=\"margin-top: 0.75em; margin-bottom: 0em\"><a href=\"";
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
  vector <Passage> passages = search_logic_search_text (queryString, bibles);
  
  
  int textCount = passages.size ();
  view.set_variable ("textCount", convert_to_string (textCount));
  
  
  // Assemble the search results for the Bible text.
  string textblock;
  for (auto & passage : passages) {
    string bible = passage.bible;
    int book = passage.book;
    int chapter = passage.chapter;
    string verse = passage.verse;
    // The title plus link.
    string link = bible + " | " + filter_passage_link_for_opening_editor_at (book, chapter, verse);
    // The excerpt.
    string stext = search_logic_get_bible_verse_text (bible, book, chapter, convert_to_int (verse));
    vector <string> vtext = filter_string_explode (stext, '\n');
    string excerpt;
    // Go through each line of text separately.
    for (auto & line : vtext) {
      string markedLine = filter_string_markup_words (queryWords, line);
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
  
  
  page += Assets_Page::footer ();
  
  
  return page;
}
