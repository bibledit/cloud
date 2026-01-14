/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <filter/passage.h>
#include <locale/translate.h>
#include <styles/sheets.h>


void export_odt_book (std::string bible, int book, bool log)
{
  // Create folders for the OpenDocument export.
  std::string directory = filter_url_create_path ({export_logic::bible_directory (bible), "opendocument"});
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  
  
  // Filenames for the various types of OpenDocument files.
  std::string basename = export_logic::base_book_filename (bible, book);
  std::string standardFilename = filter_url_create_path ({directory, basename + "_standard.odt"});
  std::string textOnlyFilename = filter_url_create_path ({directory, basename + "_text_only.odt"});
  std::string textAndCitationsFilename = filter_url_create_path ({directory, basename + "_text_and_note_citations.odt"});
  std::string notesFilename = filter_url_create_path ({directory, basename + "_notes.odt"});

  
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  
  
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.odf_text_text_only = new odf_text (bible);
  filter_text.odf_text_text_and_note_citations = new odf_text (bible);
  filter_text.odf_text_notes = new odf_text (bible);
  
  
  if (book == 0) {
    // Load entire Bible, ordered.
    std::vector <int> books = filter_passage_get_ordered_books (bible);
    for (auto book2 : books) {
      std::vector <int> chapters = database::bibles::get_chapters (bible, book2);
      for (auto chapter : chapters) {
        std::string usfm = database::bibles::get_chapter (bible, book2, chapter);
        usfm = filter::string::trim (usfm);
        // Use small chunks of USFM at a time for much better performance.
        filter_text.add_usfm_code (usfm);
      }
    }
  } else {
    // Load one book.
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto chapter : chapters) {
      std::string usfm = database::bibles::get_chapter (bible, book, chapter);
      usfm = filter::string::trim (usfm);
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
    std::string contents = database::bible_images::get(src);
    std::string path = filter_url_create_path ({directory, src});
    filter_url_file_put_contents(path, contents);
  }

    
  // Securing the OpenDocument export implies that the exported files are zipped and secured with a password.
  // It uses the external zip binary.
  bool secure = database::config::bible::get_secure_odt_export (bible);
  std::string password = database::config::bible::get_export_password (bible);
  std::string basefile = filter_url_basename (standardFilename);
  filter_url_unlink (standardFilename + ".zip");
  if (secure) {
    filter::shell::run (directory, filter::shell::get_executable(filter::shell::Executable::zip), {"-P", password, basefile + ".zip", basefile}, nullptr, nullptr);
    filter_url_unlink (standardFilename);
  }
  basefile = filter_url_basename (textOnlyFilename);
  filter_url_unlink (textOnlyFilename + ".zip");
  if (secure) {
    filter::shell::run (directory, filter::shell::get_executable(filter::shell::Executable::zip), {"-P", password, basefile + ".zip", basefile}, nullptr, nullptr);
    filter_url_unlink (textOnlyFilename);
  }
  basefile = filter_url_basename (textAndCitationsFilename);
  filter_url_unlink (textAndCitationsFilename + ".zip");
  if (secure) {
    filter::shell::run (directory, filter::shell::get_executable(filter::shell::Executable::zip), {"-P", password, basefile + ".zip", basefile}, nullptr, nullptr);
    filter_url_unlink (textAndCitationsFilename);
  }
  basefile = filter_url_basename (notesFilename);
  filter_url_unlink (notesFilename + ".zip");
  if (secure) {
    filter::shell::run (directory, filter::shell::get_executable(filter::shell::Executable::zip), {"-P", password, basefile + ".zip", basefile}, nullptr, nullptr);
    filter_url_unlink (notesFilename);
  }
  
  
  // Clear the flag that indicated this export.
  Database_State::clearExport (bible, book, export_logic::export_opendocument);

  
  if (log) {
    std::string bookname;
    if (book) bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    else bookname = translate ("whole Bible");
    Database_Logs::log (translate("Exported to OpenDocument files") + " " + bible + " " + bookname, roles::translator);
  }
}
