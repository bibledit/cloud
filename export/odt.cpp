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


#include <export/odt.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/state.h>
#include <database/config/bible.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/text.h>
#include <filter/archive.h>
#include <filter/shell.h>
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
  
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new Odf_Text (bible);
  filter_text.odf_text_text_only = new Odf_Text (bible);
  filter_text.odf_text_text_and_note_citations = new Odf_Text (bible);
  filter_text.odf_text_notes = new Odf_Text (bible);
  
  
  if (book == 0) {
    // Load entire Bible.
    vector <int> books = database_bibles.getBooks (bible);
    for (auto book : books) {
      vector <int> chapters = database_bibles.getChapters (bible, book);
      for (auto chapter : chapters) {
        string usfm = database_bibles.getChapter (bible, book, chapter);
        usfm = filter_string_trim (usfm);
        // Use small chunks of USFM at a time for much better performance.
        filter_text.addUsfmCode (usfm);
      }
    }
  } else {
    // Load one book.
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      string usfm = database_bibles.getChapter (bible, book, chapter);
      usfm = filter_string_trim (usfm);
      // Use small chunks of USFM at a time for much better performance.
      filter_text.addUsfmCode (usfm);
    }
  }
  
  
  // Convert the USFM to OpenDocument.
  filter_text.run (stylesheet);
  
  
  // Save files.
  filter_text.odf_text_standard->save (standardFilename);
  filter_text.odf_text_text_only->save (textOnlyFilename);
  filter_text.odf_text_text_and_note_citations->save (textAndCitationsFilename);
  filter_text.odf_text_notes->save (notesFilename);
  
  
  // Securing the OpenDocument export implies that the exported files are zipped and secured with a password.
  // It uses the external zip binary.
  bool secure = Database_Config_Bible::getSecureOdtExport (bible);
  string password = Database_Config_Bible::getExportPassword (bible);
  string basefile = filter_url_basename (standardFilename);
  filter_url_unlink (standardFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink (standardFilename);
  }
  basefile = filter_url_basename (textOnlyFilename);
  filter_url_unlink (textOnlyFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink (textOnlyFilename);
  }
  basefile = filter_url_basename (textAndCitationsFilename);
  filter_url_unlink (textAndCitationsFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink (textAndCitationsFilename);
  }
  basefile = filter_url_basename (notesFilename);
  filter_url_unlink (notesFilename + ".zip");
  if (secure) {
    filter_shell_run (directory, "zip", {"-P", password, basefile + ".zip", basefile}, NULL, NULL);
    filter_url_unlink (notesFilename);
  }
  
  
  // Clear the flag that indicated this export.
  Database_State::clearExport (bible, book, Export_Logic::export_opendocument);

  
  if (log) Database_Logs::log (translate("Exported to OpenDocument files") + " " + bible + " " + Database_Books::getEnglishFromId (book), Filter_Roles::translator ());
}
