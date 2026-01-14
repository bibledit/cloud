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


#include <database/bibles.h>
#include <database/books.h>
#include <database/state.h>
#include <resource/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <search/logic.h>
#include <export/logic.h>


// This database stores its data in files in the filesystem.
// This is a robust and reliable way of storing data.
// Because no real database is used, no database can get corrupted.


namespace database::bibles {


static const std::string& main_folder ()
{
  static std::string folder {filter_url_create_root_path ({"bibles"})};
  return folder;
}


std::string bible_folder (const std::string& bible)
{
  return filter_url_create_path ({main_folder (), bible});
}


static std::string book_folder (const std::string& bible, const int book)
{
  return filter_url_create_path ({bible_folder (bible), std::to_string (book)});
}


static std::string chapter_folder (const std::string& bible, const int book, const int chapter)
{
  return filter_url_create_path ({book_folder (bible, book), std::to_string (chapter)});
}


// Returns a list of available Bibles.
std::vector <std::string> get_bibles ()
{
  return filter_url_scandir (main_folder ());
}


// Creates a new empty Bible. Returns its ID.
void create_bible (const std::string& bible)
{
  // Create the empty system.
  const std::string& folder = bible_folder (bible);
  filter_url_mkdir (folder);
  // Handle exporting it.
  Database_State::setExport (bible, 0, export_logic::export_needed);
}


// Deletes a Bible.
void delete_bible (const std::string& bible)
{
  const std::string& path = bible_folder (bible);
  // Delete directory.
  filter_url_rmdir (path);
  // Just in case it was a regular file: Delete it too.
  filter_url_unlink (path);
  // Handle exporting it.
  Database_State::setExport (bible, 0, export_logic::export_needed);
}


// Stores data of one chapter in Bible $bible.
void store_chapter (const std::string& bible, const int book, const int chapter_number, std::string chapter_text)
{
  const std::string& folder = chapter_folder (bible, book, chapter_number);
  if (!file_or_dir_exists (folder))
    filter_url_mkdir (folder);
  
  // Ensure that the data to be stored ends with a new line.
  if (!chapter_text.empty ()) {
    const size_t pos = chapter_text.length () - 1;
    if (chapter_text.substr (pos, 1) != "\n") {
      chapter_text.append ("\n");
    }
  }

  // Increase the chapter identifier, and store the chapter data.
  int id = get_chapter_id (bible, book, chapter_number);
  id++;
  const std::string file = filter_url_create_path ({folder, std::to_string (id)});
  filter_url_file_put_contents (file, chapter_text);
  
  // Update search fields.
  update_search_fields (bible, book, chapter_number);

  // Set flag for the exporter.
  Database_State::setExport (bible, 0, export_logic::export_needed);
}


void update_search_fields (const std::string& bible, const int book, const int chapter)
{
  search_logic_index_chapter (bible, book, chapter);
}


// Returns an array with the available books in a Bible.
std::vector <int> get_books (const std::string& bible)
{
  // Read the books from the database.
  const std::string folder = bible_folder (bible);
  const std::vector <std::string> files = filter_url_scandir (folder);
  std::vector <int> books {};
  for (const auto& book : files) {
    if (filter::string::is_numeric (book)) {
      books.push_back (filter::string::convert_to_int (book));
    }
  }
  
  // Sort the books according to the order defined in the books database.
  std::vector <int> order {};
  for (const auto book_number : books) {
    const book_id book_enum = static_cast<book_id>(book_number);
    order.push_back (database::books::get_order_from_id (book_enum));
  }
  filter::string::quick_sort (order, books, 0, static_cast<unsigned>(order.size()));
  
  // Result.
  return books;
}


void delete_book (const std::string& bible, int book)
{
  const std::string folder = book_folder (bible, book);
  filter_url_rmdir (folder);
  Database_State::setExport (bible, 0, export_logic::export_needed);
}


// Returns an array with the available chapters in a $book in a Bible.
std::vector <int> get_chapters (const std::string& bible, const int book)
{
  // Read the chapters from the database.
  const std::string folder = book_folder (bible, book);
  std::vector <int> chapters;
  const std::vector <std::string> files = filter_url_scandir (folder);
  for (const auto& file : files) {
    if (filter::string::is_numeric (file)) chapters.push_back (filter::string::convert_to_int (file));
  }
  std::sort (chapters.begin (), chapters.end ());
  return chapters;
}


void delete_chapter (const std::string& bible, int book, int chapter)
{
  const std::string folder = chapter_folder (bible, book, chapter);
  filter_url_rmdir (folder);
  Database_State::setExport (bible, 0, export_logic::export_needed);
}


// Gets the chapter data as a string.
std::string get_chapter (const std::string& bible, const int book, const int chapter)
{
  // Read the chapter data from the database.
  const std::string folder = chapter_folder (bible, book, chapter);
  const std::vector <std::string> files = filter_url_scandir (folder);
  if (!files.empty ()) {
    const std::string file = files.at(files.size() - 1);
    std::string data = filter_url_file_get_contents (filter_url_create_path ({folder, file}));
    // Remove trailing new line.
    data = filter::string::trim (data);
    return data;
  }
  return std::string();
}


// Gets the chapter id.
int get_chapter_id (const std::string& bible, const int book, const int chapter)
{
  const std::string folder = chapter_folder (bible, book, chapter);
  const std::vector <std::string> files = filter_url_scandir (folder);
  if (!files.empty ()) {
    const std::string file = files.at(files.size() - 1);
    return filter::string::convert_to_int (file);
  }
  return 100'000'000;
}


// Gets the chapter's time stamp in seconds since the Epoch.
int get_chapter_age (const std::string& bible, const int book, const int chapter)
{
  const std::string folder = chapter_folder (bible, book, chapter);
  const std::vector <std::string> files = filter_url_scandir (folder);
  if (!files.empty ()) {
    const std::string file = files.at(files.size() - 1);
    const std::string path = filter_url_create_path ({folder, file});
    const int time = filter_url_file_modification_time (path);
    const int now = filter::date::seconds_since_epoch ();
    return now - time;
  }
  return 100'000'000;
}


void optimize ()
{
  // Go through all chapters in all books and all Ḃibles.
  const std::vector <std::string> bibles = get_bibles ();
  for (const auto& bible : bibles) {
    const std::vector <int> books = get_books (bible);
    for (const int book : books) {
      const std::vector <int> chapters = get_chapters (bible, book);
      for (const int chapter : chapters) {
        const std::string folder = chapter_folder (bible, book, chapter);
        // Read the files in the folder.
        const std::vector <std::string> files = filter_url_scandir (folder);
        // Remove files with 0 size. so that in case a chapter was emptied by accident,
        // it is removed now, effectually reverting the chapter to an earlier version.
        std::vector <std::string> files2 {};
        for (const auto& file : files) {
          const std::string path = filter_url_create_path ({folder, file});
          if (filter_url_filesize (path) == 0) {
            filter_url_unlink (path);
            Database_State::setExport (bible, 0, export_logic::export_needed);
          }
          else files2.push_back (file);
        }
        // Remove the three most recent files from the list, so they don't get deleted.
        // Because scandir sorts the files, the files to be kept are at the end.
        if (!files2.empty()) files2.pop_back ();
        if (!files2.empty()) files2.pop_back ();
        if (!files2.empty()) files2.pop_back ();
        // Remove the remaining files. These are the older versions.
        for (const auto& file : files2) {
          const std::string path = filter_url_create_path ({folder, file});
          filter_url_unlink (path);
        }
      }
    }
  }
}


}
