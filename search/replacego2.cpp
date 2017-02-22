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


#include <search/replacego2.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <text/text.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <bible/logic.h>
#include <search/logic.h>
#include <access/bible.h>


string search_replacego2_url ()
{
  return "search/replacego2";
}


bool search_replacego2_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string search_replacego2 (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string siteUrl = config_logic_site_url (webserver_request);
  
  
  // Get the action variables from the query.
  string id = request->query ["id"];
  string searchfor = request->query ["q"];
  string replacewith = request->query ["r"];
  bool casesensitive = (request->query ["c"] == "true");
  bool searchplain = (request->query ["p"] == "true");
  
  
  // Get Bible and passage for this identifier.
  Passage passage = Passage::decode (id);
  string bible = passage.bible;
  int book = passage.book;
  int chapter = passage.chapter;
  int verse = convert_to_int (passage.verse);
  
  
  // Check whether the user has write access to the book.
  string user = request->session_logic ()->currentUser ();
  bool write = access_bible_book_write (webserver_request, user, bible, book);

  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  // As a standard to compare against, get the plain text from the search database,
  // do the replacements, count the replacements, and then get the desired new plain text.
  // This only applies when searching/replacing in the plain text, not when doing it in the USFM.
  int standardReplacementCount = 0;
  string standardPlainText = search_logic_get_bible_verse_text (bible, book, chapter, convert_to_int (verse));
  if (searchplain) {
    if (casesensitive) {
      standardPlainText = filter_string_str_replace (searchfor, replacewith, standardPlainText, &standardReplacementCount);
    } else {
      vector <string> needles = filter_string_search_needles (searchfor, standardPlainText);
      for (auto & needle : needles) {
        standardPlainText = filter_string_str_replace (needle, replacewith, standardPlainText, &standardReplacementCount);
      }
    }
  }
  
  
  // Get the old chapter and verse USFM.
  string old_chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);
  string old_verse_usfm = usfm_get_verse_text (old_chapter_usfm, verse);
  
  
  // Do the replacing in the correct verse of the raw verse USFM.
  string new_verse_usfm = old_verse_usfm;
  int usfmReplacementCount = 0;
  if (casesensitive) {
    new_verse_usfm = filter_string_str_replace (searchfor, replacewith, new_verse_usfm, &usfmReplacementCount);
  } else {
    vector <string> needles = filter_string_search_needles (searchfor, new_verse_usfm);
    for (auto & needle : needles) {
      new_verse_usfm = filter_string_str_replace (needle, replacewith, new_verse_usfm, &usfmReplacementCount);
    }
  }

  
  // Create the updated chapter USFM as a string.
  string new_chapter_usfm = old_chapter_usfm;
  size_t pos = new_chapter_usfm.find (old_verse_usfm);
  if (pos != string::npos) {
    size_t length = old_verse_usfm.length ();
    new_chapter_usfm.erase (pos, length);
    new_chapter_usfm.insert (pos, new_verse_usfm);
  }

  
  // Text filter for getting the new plain text from the new USFM.
  // This is for search/replace in plain text, not in USFM.
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.text_text = new Text_Text ();
  filter_text.initializeHeadingsAndTextPerVerse (false);
  filter_text.addUsfmCode (new_chapter_usfm);
  filter_text.run (stylesheet);

  
  // Get the updated plain text of the correct verse of the updated USFM.
  // This is for search/replace in plain text, not in USFM.
  string updatedPlainText;
  map <int, string> texts = filter_text.getVersesText ();
  for (auto & element : texts) {
    int vs = element.first;
    string text = element.second;
    if (vs == verse) updatedPlainText.append (text + "\n");
  }
  map <int, string> headings = filter_text.verses_headings;
  for (auto & element : headings) {
    int vs = element.first;
    string heading = element.second;
    if (vs == verse) updatedPlainText.append (heading + "\n");
  }

  
  // Check that the standard and real number of replacements, and the standard and new texts, are the same.
  // This is for search/replace in plain text, not in USFM.
  bool replacementOkay = true;
  if (searchplain) {
    if (usfmReplacementCount != standardReplacementCount) replacementOkay = false;
    if (filter_string_trim (updatedPlainText) != filter_string_trim (standardPlainText)) replacementOkay = false;
  }
  
  
  // Generate success or failure icon.
  string icon;
  if (replacementOkay && write) {
    icon = "<span class=\"success\">✔</span>";
  } else {
    icon = "<span class=\"error\">" + emoji_wastebasket () + "</span>";
  }
  
  
  // Store the new chapter in the database on success.
  if (replacementOkay && write) {
    bible_logic_store_chapter (bible, book, chapter, new_chapter_usfm);
  }
  
  
  // Mark the new plain text.
  if (replacewith != "") {
    if (searchplain) {
      updatedPlainText = filter_string_markup_words ({replacewith}, updatedPlainText);
    }
  }
  
  
  // Clickable passage.
  string link = filter_passage_link_for_opening_editor_at (book, chapter, convert_to_string (verse));
  
  
  // Success or failure message.
  string msg;
  if (!write) {
    msg = locale_logic_text_no_privileges_modify_book ();
  } else if (replacementOkay) {
    msg = updatedPlainText;
  } else {
    msg = "<span class=\"error\">" + translate("This text could not be automatically replaced. Click the passage to do it manually.") + "</span>";
  }

  
  // Create output.
  string output = "<p>" + icon + " " + link + " " + msg + "</p>\n";
  
  
  // Output to browser.
  return output;
}
