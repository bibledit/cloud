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


#include <export/textusfm.h>
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
#include <filter/archive.h>
#include <filter/shell.h>
#include <locale/translate.h>
#include <styles/sheets.h>


void export_text_usfm_book (string bible, int book, bool log)
{
  // Create folders for the clear text and the basic USFM exports.
  string usfmDirectory = Export_Logic::USFMdirectory (bible, 1);
  if (!file_or_dir_exists (usfmDirectory)) filter_url_mkdir (usfmDirectory);
  string textDirectory = filter_url_create_path (Export_Logic::bibleDirectory (bible), "text");
  if (!file_or_dir_exists (textDirectory)) filter_url_mkdir (textDirectory);
  
  
  // Filenames for text and usfm.
  string usfmFilename = filter_url_create_path (usfmDirectory, Export_Logic::baseBookFileName (book) + ".usfm");
  string textFilename = filter_url_create_path (textDirectory, Export_Logic::baseBookFileName (book) + ".txt");
  
  
  Database_Bibles database_bibles;
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  Filter_Text filter_text_book = Filter_Text (bible);
  filter_text_book.text_text = new Text_Text ();
  
  
  // Basic USFM.
  if (file_or_dir_exists (usfmFilename)) filter_url_unlink (usfmFilename);
  string basicUsfm = "\\id " + Database_Books::getUsfmFromId (book) + "\n";
  filter_url_file_put_contents_append (usfmFilename, basicUsfm);
  
  
  vector <int> chapters = database_bibles.getChapters (bible, book);
  for (auto chapter : chapters) {
    
    
    // The text filter for this chapter.
    Filter_Text filter_text_chapter = Filter_Text (bible);
    
    
    // Basic USFM for this chapter.
    filter_text_chapter.initializeHeadingsAndTextPerVerse (false);
    
    
    // Get the USFM code for the current chapter.
    string chapter_data = database_bibles.getChapter (bible, book, chapter);
    chapter_data = filter_string_trim (chapter_data);
    
    
    // Add the chapter's USFM code to the Text_* filter for the book, and for the chapter.
    // Use small chunks of USFM at a time. This provides much better performance.
    filter_text_book.addUsfmCode (chapter_data);
    filter_text_chapter.addUsfmCode (chapter_data);
    
    
    // Convert the chapter
    filter_text_chapter.run (stylesheet);
    
    
    // Deal with basic USFM.
    if (chapter > 0) {
      map <int, string> verses_text = filter_text_chapter.getVersesText ();
      basicUsfm = "\\c " + convert_to_string (chapter) + "\n";
      basicUsfm += "\\p\n";
      for (auto element : verses_text) {
        int verse = element.first;
        string text = element.second;
        basicUsfm += "\\v " + convert_to_string (verse) + " " + text + "\n";
      }
      filter_url_file_put_contents_append (usfmFilename, basicUsfm);
    }
    
    
  }
  
  
  // Convert the book.
  filter_text_book.run (stylesheet);
  
  
  // Save the text export.
  filter_text_book.text_text->save (textFilename);
  
  
  // Clear the flag that indicated this export.
  Database_State::clearExport (bible, book, Export_Logic::export_text_and_basic_usfm);

  
  if (log) Database_Logs::log (translate("Exported to basic USFM and text") + ": " + bible + " " + Database_Books::getEnglishFromId (book), Filter_Roles::translator ());
}
