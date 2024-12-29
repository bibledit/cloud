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


#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <locale/translate.h>


// Schedule all Bibles for exports.
void export_logic::schedule_all ()
{
  tasks_logic_queue (task::export_all);
}


// Schedule a Bible book for export to text and basic USFM format.
// $bible: Bible.
// $book: book.
void export_logic::schedule_text_and_basic_usfm (const std::string& bible, bool log)
{
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    tasks_logic_queue (task::export_text_usfm, {bible, std::to_string (book), filter::strings::convert_to_string (log)});
  }
}


// Schedule a Bible for export to USFM format.
void export_logic::schedule_usfm (const std::string& bible, bool log)
{
  tasks_logic_queue (task::export_usfm, {bible, filter::strings::convert_to_string (log)});
}


// Schedule export to OpenDocument.
// $bible: Bible.
void export_logic::schedule_open_document (const std::string& bible, bool log)
{
  // Get the available books in the Bible.
  std::vector <int> books = database::bibles::get_books (bible);
  // Export the books, one OpenDocument file per book.
  for (auto book : books) {
    tasks_logic_queue (task::export_odt, {bible, std::to_string (book), filter::strings::convert_to_string (log)});
  }
  // Export the whole Bible to one OpenDocument file.
  tasks_logic_queue (task::export_odt, {bible, "0", filter::strings::convert_to_string (log)});
}


// Schedule creation info documents.
// $bible: Bible.
void export_logic::schedule_info (const std::string& bible, bool log)
{
  tasks_logic_queue (task::export_info, {bible, filter::strings::convert_to_string (log)});
}


// Schedule export to html.
// $bible: Bible.
void export_logic::schedule_html (const std::string& bible, bool log)
{
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    tasks_logic_queue (task::export_html, {bible, std::to_string (book), filter::strings::convert_to_string (log)});
  }
}


// Schedule export to web.
// $bible: Bible.
void export_logic::schedule_web (const std::string& bible, bool log)
{
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto book : books) {
    tasks_logic_queue (task::export_web_main, {bible, std::to_string (book), filter::strings::convert_to_string (log)});
  }
}


// Schedule export to web index.
// $bible: Bible.
void export_logic::schedule_web_index (const std::string& bible, bool log)
{
  tasks_logic_queue (task::export_web_index, {bible, filter::strings::convert_to_string (log)});
}


void export_logic::schedule_online_bible (const std::string& bible, bool log)
{
  tasks_logic_queue (task::export_online_bible, {bible, filter::strings::convert_to_string (log)});
}


void export_logic::schedule_e_sword (const std::string& bible, bool log)
{
  tasks_logic_queue (task::export_esword, {bible, filter::strings::convert_to_string (log)});
}


// The main exports directory.
std::string export_logic::main_directory ()
{
  return filter_url_create_root_path ({"exports"});
}


// A Bible's export directory.
std::string export_logic::bible_directory (const std::string& bible)
{
  return filter_url_create_path ({main_directory (), bible});
}


// Directory for the USFM.
// $type:
// 0: directory for the full USFM.
// 1: directory for the basic USFM.
// 2: root USFM directory.
std::string export_logic::usfm_directory (const std::string& bible, int type)
{
  std::string directory = filter_url_create_path ({bible_directory (bible), "usfm"});
  switch (type) {
    case 0: directory = filter_url_create_path ({directory, "full"}); break;
    case 1: directory = filter_url_create_path ({directory, "basic"}); break;
    default: break;
  }
  return directory;
}


std::string export_logic::web_directory (const std::string& bible)
{
  return filter_url_create_path ({bible_directory (bible), "web"});
}


std::string export_logic::web_back_link_directory (const std::string& bible)
{
  return "/exports/" + bible + "/web/";
}


// Provides the base book file name, e.g. 01_Genesis.
// Or 00_Bible for an entire Bible when $book = 0;
// Takes in account the order of the books, possibly modified by the user.
std::string export_logic::base_book_filename (const std::string& bible, int book)
{
  std::string filename;
  if (book) {
    // The file name has a number that indicates the defined order of the book.
    // See https://github.com/bibledit/cloud/issues/810
    // Localize the English book name: https://github.com/bibledit/cloud/issues/241
    std::vector <int> ordered_books = filter_passage_get_ordered_books (bible);
    std::vector<int>::iterator iterator;
    iterator = find(ordered_books.begin(), ordered_books.end(), book);
    if (iterator != ordered_books.end()) {
      const long order = iterator - ordered_books.begin() + 1;
      filename = filter::strings::fill (std::to_string (order), 2, '0');
      filename.append ("_");
    }
    filename.append (translate (database::books::get_english_from_id (static_cast<book_id>(book))));
  } else {
    // Whole Bible.
    filename = "00_" + translate ("Bible");
  }
  
  // Done.
  return filename;
}


