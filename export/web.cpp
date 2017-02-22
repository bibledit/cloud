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


#include <export/web.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/text.h>
#include <filter/css.h>
#include <fonts/logic.h>
#include <html/text.h>
#include <html/header.h>
#include <locale/translate.h>
#include <styles/sheets.h>


void export_web_book (string bible, int book, bool log)
{
  string directory = Export_Logic::webDirectory (bible);
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  Database_Bibles database_bibles;
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  // Copy font to the output directory.
  string font = Fonts_Logic::getTextFont (bible);
  if (!font.empty ()) {
    if (Fonts_Logic::fontExists (font)) {
      string fontpath = Fonts_Logic::getFontPath (font);
      string contents = filter_url_file_get_contents (fontpath);
      fontpath = filter_url_create_path (directory, font);
      filter_url_file_put_contents (fontpath, contents);
    }
  }
  
  
  string backLinkPath = Export_Logic::webBackLinkDirectory (bible);
  
  
  string bibleBookText = bible + " " + Database_Books::getEnglishFromId (book);
  
  
  // Web index file for the book.
  Html_Text html_text_rich_book_index (bibleBookText);
  Html_Header htmlHeader = Html_Header (&html_text_rich_book_index);
  htmlHeader.searchBackLink (backLinkPath + filter_url_html_file_name_bible ("", book), translate("Go back to") + " " + bibleBookText);
  htmlHeader.create ({ make_pair (bible, filter_url_html_file_name_bible ()), make_pair (Database_Books::getEnglishFromId (book), filter_url_html_file_name_bible ()) });
  html_text_rich_book_index.newParagraph ("navigationbar");
  html_text_rich_book_index.addText ("|");
  
  
  // Go through the chapters of this book.
  vector <int> chapters = database_bibles.getChapters (bible, book);
  for (auto chapter : chapters) {
    
    // The text filter for this chapter.
    Filter_Text filter_text_chapter = Filter_Text (bible);
    
    string usfm = database_bibles.getChapter (bible, book, chapter);
    usfm = filter_string_trim (usfm);
    // Use small chunks of USFM at a time for much better performance.
    filter_text_chapter.addUsfmCode (usfm);
    
    // Interlinked web data for one chapter.
    filter_text_chapter.html_text_linked = new Html_Text (translate("Bible"));
    filter_text_chapter.html_text_linked->customClass = Filter_Css::getClass (bible);
    
    // Create breadcrumbs for the chapter.
    Html_Header htmlHeader = Html_Header (filter_text_chapter.html_text_linked);
    htmlHeader.searchBackLink (backLinkPath + filter_url_html_file_name_bible ("", book, chapter), translate("Go back to") + " " + bibleBookText + " " + convert_to_string (chapter));
    htmlHeader.create ({ make_pair (bible, filter_url_html_file_name_bible ()), make_pair (Database_Books::getEnglishFromId (book), filter_url_html_file_name_bible ()), make_pair (convert_to_string (chapter), filter_url_html_file_name_bible ("", book))});
     
    // Create interlinked html for the chapter.
    filter_text_chapter.run (stylesheet);
    filter_text_chapter.html_text_linked->save (filter_url_html_file_name_bible (directory, book, chapter));
    
    html_text_rich_book_index.addLink (html_text_rich_book_index.currentPDomElement, filter_url_html_file_name_bible ("", book, chapter), "", convert_to_string (chapter), "", " " + convert_to_string (chapter) + " ");
    html_text_rich_book_index.addText ("|");
  }
  
  
  // Save the book index.
  html_text_rich_book_index.save (filter_url_html_file_name_bible (directory, book));
  
  
  // Clear the flag for this export.
  Database_State::clearExport (bible, book, Export_Logic::export_web);
  
  
  if (log) Database_Logs::log (translate("Exported to web") + ": " + bible + " " + Database_Books::getEnglishFromId (book), Filter_Roles::translator ());
}


void export_web_index (string bible, bool log)
{
  // Create folders for the web export.
  string directory = Export_Logic::webDirectory (bible);
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  // Filenames for the web file and stylesheet.
  string indexFile = filter_url_create_path (directory, "index.html");
  string index00 = filter_url_create_path (directory, "00_index.html");
  string filecss = filter_url_create_path (directory, "stylesheet.css");
  
  
  Database_Bibles database_bibles;
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  // Create stylesheet.
  Styles_Sheets styles_sheets;
  styles_sheets.create (stylesheet, filecss, false, bible);
  
  
  string backLinkPath = Export_Logic::webBackLinkDirectory (bible);
  
  
  // Main index file.
  Html_Text html_text_rich_bible_index (bible);
  // On top are the breadcrumbs, starting with a clickable Bible name.
  Html_Header htmlHeader = Html_Header (&html_text_rich_bible_index);
  htmlHeader.searchBackLink (backLinkPath + filter_url_html_file_name_bible (), translate("Go back to Bible"));
  htmlHeader.create ({ make_pair (bible, filter_url_html_file_name_bible ())});
  
  
  // Prepare for the list of books in de html index file.
  html_text_rich_bible_index.newParagraph ("navigationbar");
  html_text_rich_bible_index.addText (" |");
  
  
  // Go through the Bible books.
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    // Add this book to the main web index.
    html_text_rich_bible_index.addLink (html_text_rich_bible_index.currentPDomElement,  filter_url_html_file_name_bible ("", book), "", Database_Books::getEnglishFromId (book), "", " " + Database_Books::getEnglishFromId (book) + " ");
    html_text_rich_bible_index.addText ("|");
  }
  
  
  // Save index file for the interlinked web export.
  html_text_rich_bible_index.save (indexFile);
  html_text_rich_bible_index.save (index00);
  
  
  // Lens image supporting search.
  string lenspath = filter_url_create_root_path ("webbible", "lens.png");
  string contents = filter_url_file_get_contents (lenspath);
  lenspath = filter_url_create_path (directory, "lens.png");
  filter_url_file_put_contents (lenspath, contents);

  
  // Clear the flag that indicated this export.
  Database_State::clearExport (bible, 0, Export_Logic::export_web_index);

  
  if (log) Database_Logs::log (translate("Exported to web") + ": " + bible + " Index", Filter_Roles::translator ());
}
