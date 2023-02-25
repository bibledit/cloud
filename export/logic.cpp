/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
using namespace std;


// Schedule all Bibles for exports.
void export_logic::schedule_all ()
{
  tasks_logic_queue (EXPORTALL);
}


// Schedule a Bible book for export to text and basic USFM format.
// $bible: Bible.
// $book: book.
void export_logic::schedule_text_and_basic_usfm (const string & bible, bool log)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    tasks_logic_queue (EXPORTTEXTUSFM, {bible, convert_to_string (book), convert_to_string (log)});
  }
}


// Schedule a Bible for export to USFM format.
void export_logic::schedule_usfm (const string & bible, bool log)
{
  tasks_logic_queue (EXPORTUSFM, {bible, convert_to_string (log)});
}


// Schedule export to OpenDocument.
// $bible: Bible.
void export_logic::schedule_open_document (const string & bible, bool log)
{
  // Get the available books in the Bible.
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  // Export the books, one OpenDocument file per book.
  for (auto book : books) {
    tasks_logic_queue (EXPORTODT, {bible, convert_to_string (book), convert_to_string (log)});
  }
  // Export the whole Bible to one OpenDocument file.
  tasks_logic_queue (EXPORTODT, {bible, "0", convert_to_string (log)});
}


// Schedule creation info documents.
// $bible: Bible.
void export_logic::schedule_info (const string & bible, bool log)
{
  tasks_logic_queue (EXPORTINFO, {bible, convert_to_string (log)});
}


// Schedule export to html.
// $bible: Bible.
void export_logic::schedule_html (const string & bible, bool log)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    tasks_logic_queue (EXPORTHTML, {bible, convert_to_string (book), convert_to_string (log)});
  }
}


// Schedule export to web.
// $bible: Bible.
void export_logic::schedule_web (const string & bible, bool log)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    tasks_logic_queue (EXPORTWEBMAIN, {bible, convert_to_string (book), convert_to_string (log)});
  }
}


// Schedule export to web index.
// $bible: Bible.
void export_logic::schedule_web_index (const string & bible, bool log)
{
  tasks_logic_queue (EXPORTWEBINDEX, {bible, convert_to_string (log)});
}


void export_logic::schedule_online_bible (const string & bible, bool log)
{
  tasks_logic_queue (EXPORTONLINEBIBLE, {bible, convert_to_string (log)});
}


void export_logic::schedule_e_sword (const string & bible, bool log)
{
  tasks_logic_queue (EXPORTESWORD, {bible, convert_to_string (log)});
}


// The main exports directory.
string export_logic::main_directory ()
{
  return filter_url_create_root_path ({"exports"});
}


// A Bible's export directory.
string export_logic::bible_directory (const string & bible)
{
  return filter_url_create_path ({main_directory (), bible});
}


// Directory for the USFM.
// $type:
// 0: directory for the full USFM.
// 1: directory for the basic USFM.
// 2: root USFM directory.
string export_logic::usfm_directory (const string & bible, int type)
{
  string directory = filter_url_create_path ({bible_directory (bible), "usfm"});
  switch (type) {
    case 0: directory = filter_url_create_path ({directory, "full"}); break;
    case 1: directory = filter_url_create_path ({directory, "basic"}); break;
    default: break;
  }
  return directory;
}


string export_logic::web_directory (const string & bible)
{
  return filter_url_create_path ({bible_directory (bible), "web"});
}


string export_logic::web_back_link_directory (const string & bible)
{
  return "/exports/" + bible + "/web/";
}


// Provides the base book file name, e.g. 01_Genesis.
// Or 00_Bible for an entire Bible when $book = 0;
// Takes in account the order of the books, possibly modified by the user.
string export_logic::base_book_filename (const string & bible, int book)
{
  string filename;
  if (book) {
    // The file name has a number that indicates the defined order of the book.
    // See https://github.com/bibledit/cloud/issues/810
    // Localize the English book name: https://github.com/bibledit/cloud/issues/241
    vector <int> ordered_books = filter_passage_get_ordered_books (bible);
    vector<int>::iterator iterator;
    iterator = find(ordered_books.begin(), ordered_books.end(), book);
    if (iterator != ordered_books.end()) {
      long order = iterator - ordered_books.begin() + 1;
      filename = filter_string_fill (to_string (order), 2, '0');
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


