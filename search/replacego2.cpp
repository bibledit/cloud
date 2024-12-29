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
#include <bb/logic.h>
#include <search/logic.h>
#include <access/bible.h>


std::string search_replacego2_url ()
{
  return "search/replacego2";
}


bool search_replacego2_acl (Webserver_Request& webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ()))
    return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string search_replacego2 (Webserver_Request& webserver_request)
{
  std::string siteUrl = config::logic::site_url (webserver_request);
  
  
  // Get the action variables from the query.
  std::string id = webserver_request.query ["id"];
  std::string searchfor = webserver_request.query ["q"];
  std::string replacewith = webserver_request.query ["r"];
  bool casesensitive = (webserver_request.query ["c"] == "true");
  bool searchplain = (webserver_request.query ["p"] == "true");
  
  
  // Get Bible and passage for this identifier.
  Passage passage = Passage::decode (id);
  std::string bible = passage.m_bible;
  int book = passage.m_book;
  int chapter = passage.m_chapter;
  int verse = filter::strings::convert_to_int (passage.m_verse);
  
  
  // Check whether the user has write access to the book.
  const std::string& user = webserver_request.session_logic ()->get_username ();
  bool write = access_bible::book_write (webserver_request, user, bible, book);

  
  // Get the old chapter and verse USFM.
  std::string old_chapter_usfm = database::bibles::get_chapter (bible, book, chapter);
  std::string old_verse_usfm = filter::usfm::get_verse_text (old_chapter_usfm, verse);
  
  
  // As a standard to compare against, get the plain text from the search database,
  // do the replacements, count the replacements, and then get the desired new plain text.
  // This only applies when searching/replacing in the plain text, not when doing it in the USFM.
  int standardReplacementCount = 0;
  std::string standardPlainText = search_logic_plain_replace_verse_text (old_verse_usfm);
  if (searchplain) {
    if (casesensitive) {
      standardPlainText = filter::strings::replace (searchfor, replacewith, standardPlainText, &standardReplacementCount);
    } else {
      std::vector <std::string> needles = filter::strings::search_needles (searchfor, standardPlainText);
      for (auto & needle : needles) {
        standardPlainText = filter::strings::replace (needle, replacewith, standardPlainText, &standardReplacementCount);
      }
    }
  }
  
  
  // Do the replacing in the correct verse of the raw verse USFM.
  std::string new_verse_usfm (old_verse_usfm);
  int usfmReplacementCount = 0;
  if (casesensitive) {
    new_verse_usfm = filter::strings::replace (searchfor, replacewith, new_verse_usfm, &usfmReplacementCount);
  } else {
    std::vector <std::string> needles = filter::strings::search_needles (searchfor, new_verse_usfm);
    for (auto & needle : needles) {
      new_verse_usfm = filter::strings::replace (needle, replacewith, new_verse_usfm, &usfmReplacementCount);
    }
  }

  
  // Create the updated chapter USFM as a string.
  std::string new_chapter_usfm = old_chapter_usfm;
  size_t pos = new_chapter_usfm.find (old_verse_usfm);
  if (pos != std::string::npos) {
    size_t length = old_verse_usfm.length ();
    new_chapter_usfm.erase (pos, length);
    new_chapter_usfm.insert (pos, new_verse_usfm);
  }

  
  // Get the updated plain text of the correct verse of the updated USFM.
  // This is for search/replace in plain text, not in USFM.
  std::string updatedPlainText = search_logic_plain_replace_verse_text (new_verse_usfm);

  
  // Check that the standard and real number of replacements, and the standard and new texts, are the same.
  // This is for search/replace in plain text, not in USFM.
  bool replacementOkay = true;
  if (searchplain) {
    if (usfmReplacementCount != standardReplacementCount) replacementOkay = false;
    if (filter::strings::trim (updatedPlainText) != filter::strings::trim (standardPlainText)) replacementOkay = false;
  }
  
  
  // Generate success or failure icon.
  std::string icon;
  if (replacementOkay && write) {
    icon = "<span class=\"success\">✔</span>";
  } else {
    icon = "<span class=\"error\">" + filter::strings::emoji_wastebasket () + "</span>";
  }
  
  
  // Store the new chapter in the database on success.
  if (replacementOkay && write) {
    bible_logic::store_chapter (bible, book, chapter, new_chapter_usfm);
  }
  
  
  // Mark the new plain text.
  if (replacewith != "") {
    if (searchplain) {
      updatedPlainText = filter::strings::markup_words ({replacewith}, updatedPlainText);
    }
  }
  
  
  // Clickable passage.
  std::string link = filter_passage_link_for_opening_editor_at (book, chapter, std::to_string (verse));
  
  
  // Success or failure message.
  std::string msg;
  if (!write) {
    msg = locale_logic_text_no_privileges_modify_book ();
  } else if (replacementOkay) {
    msg = updatedPlainText;
  } else {
    msg = "<span class=\"error\">" + translate("This text could not be automatically replaced. Click the passage to do it manually.") + "</span>";
  }

  
  // Create output.
  std::string output = "<p>" + icon + " " + link + " " + msg + "</p>\n";
  
  
  // Output to browser.
  return output;
}
