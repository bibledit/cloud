/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


#include <export/odt.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/state.h>
#include <database/config/bible.h>
#include <database/bibleimages.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/text.h>
#include <filter/archive.h>
#include <filter/shell.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/sheets.h>


void export_odt_book (string bible, int book, bool log)
{
  // Create folders for the OpenDocument export.
  string directory = filter_url_create_path (Export_Logic::bibleDirectory (bible), "opendocument");
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  // Filenames for the various types of OpenDocument files.
  string basename = Export_Logic::baseBookFileName (book);
  string standardFilename = filter_url_create_path (directory, basename + "_standard.odt");
  string textOnlyFilename = filter_url_create_path (directory, basename + "_text_only.odt");
  string textAndCitationsFilename = filter_url_create_path (directory, basename + "_text_and_note_citations.odt");
  string notesFilename = filter_url_create_path (directory, basename + "_notes.odt");

  
  Database_Bibles database_bibles;
  Database_BibleImages database_bibleimages;
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new Odf_Text (bible);
  filter_text.odf_text_text_only = new Odf_Text (bible);
  filter_text.odf_text_text_and_note_citations = new Odf_Text (bible);
  filter_text.odf_text_notes = new Odf_Text (bible);
  
  
  if (book == 0) {
    // Load entire Bible, ordered.
    vector <int> books = filter_passage_get_ordered_books (bible);
    for (auto book : books) {
      vector <int> chapters = database_bibles.getChapters (bible, book);
      for (auto chapter : chapters) {
        string usfm = database_bibles.getChapter (bible, book, chapter);
        // Filter it.
        usfm = usfm_remove_w_attributes (usfm);
        usfm = filter_string_trim (usfm);
        // Use small chunks of USFM at a time for much better performance.
        filter_text.add_usfm_code (usfm);
      }
    }
  } else {
    // Load one book.
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string usfm = database_bibles.getChapter (bible, book, chapter);
      // Filter it.
      usfm = usfm_remove_w_attributes (usfm);
      usfm = filter_string_trim (usfm);
      // Use small chunks of USFM at a time for much better performance.
      filter_text.add_usfm_code (usfm);
    }
  }
  
  
  // Convert the USFM to OpenDocument.
  filter_text.run (stylesheet);
  
  
  // Save text files and optionally the images included in the text.
  filter_text.odf_text_standard->save (standardFilename);
  filter_text.odf_text_text_only->save (textOnlyFilename);
  filter_text.odf_text_text_and_note_citations->save (textAndCitationsFilename);
  filter_text.odf_text_notes->save (notesFilename);
  for (auto src : filter_text.image_sources) {
    string contents = database_bibleimages.get(src);
    string path = filter_url_create_path (directory, src);
    filter_url_file_put_contents(path, contents);
  }

    
  // Securing the OpenDocument export implies that the exported files are zipped and secured with a password.
  // It uses the external zip binary.
  bool secure = Database_Config_Bible::getSecureOdtExport (bible);
  string password = Database_Config_Bible::getExportPassword (bible);
  string basefile = filter_url_basename_cpp17 (standardFilename);
  filter_url_unlink_cpp17 (standardFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink_cpp17 (standardFilename);
  }
  basefile = filter_url_basename_cpp17 (textOnlyFilename);
  filter_url_unlink_cpp17 (textOnlyFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink_cpp17 (textOnlyFilename);
  }
  basefile = filter_url_basename_cpp17 (textAndCitationsFilename);
  filter_url_unlink_cpp17 (textAndCitationsFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink_cpp17 (textAndCitationsFilename);
  }
  basefile = filter_url_basename_cpp17 (notesFilename);
  filter_url_unlink_cpp17 (notesFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink_cpp17 (notesFilename);
  }
  
  
  // Clear the flag that indicated this export.
  Database_State::clearExport (bible, book, Export_Logic::export_opendocument);

  
  if (log) {
    string bookname;
    if (book) bookname = Database_Books::getEnglishFromId (book);
    else bookname = translate ("whole Bible");
    Database_Logs::log (translate("Exported to OpenDocument files") + " " + bible + " " + bookname, Filter_Roles::translator ());
  }
}
