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
// This is a rugged and reliable system of storing data.
// Because no real database is used, no database can get corrupted.


string Database_Bibles::mainFolder ()
{
  return filter_url_create_root_path ({"bibles"});
}


string Database_Bibles::bibleFolder (string bible)
{
  return filter_url_create_path ({mainFolder (), bible});
}


string Database_Bibles::bookFolder (string bible, int book)
{
  return filter_url_create_path ({bibleFolder (bible), convert_to_string (book)});
}


string Database_Bibles::chapterFolder (string bible, int book, int chapter)
{
  return filter_url_create_path ({bookFolder (bible, book), convert_to_string (chapter)});
}


// Indonesian Cloud Free
// The root path for the Bibledit formatted TSI USFM directory tree.
// It mimics Bibledit's native filesystem database directory structure.
string Database_Bibles::icfURL ()
{
  return "https://raw.githubusercontent.com/aranggitoar/pengolah-usfm-tsi-untuk-bibledit/main/usfm-hasil/";
}


// Indonesian Cloud Free
// The directory name of the Bibledit formatted TSI USFM.
string Database_Bibles::icfBibleName ()
{
  return "AlkitabKita";
}


// Indonesian Cloud Free
// The book indexes of the Bibledit formatted TSI USFM.
vector <string> Database_Bibles::icfBooks ()
{
  // Taken from an index file of the Bibledit formatted TSI USFM directory.
  string url = icfURL () + icfBibleName () + "/index";

  // The fetched HTML is a string of indexes separated by commas.
  string error;
  string html = resource_logic_web_or_cache_get(url, error);
  vector <string> books;
  string delimiter = ",";

  // Find each comma from the original fetched HTML string,
  // push back the number before the comma into a vector,
  // then erase that number and comma from the original fetched HTML string.
  size_t pos = 0;
  while ((pos = html.find(delimiter)) != std::string::npos) {
    books.push_back (html.substr(0, pos));
    html.erase(0, pos + delimiter.length());
  }

  // Push back the last Bible book index.
  books.push_back (filter_string_trim (html));

  return books;
}


// Indonesian Cloud Free
// The chapter indexes of a chosen book of the Bibledit formatted TSI USFM.
vector <string> Database_Bibles::icfChapters (int book)
{
  // Taken from an index file of the chosen book from the Bibledit formatted
  // TSI USFM directory.
  string url = icfURL () + icfBibleName () + "/" + convert_to_string(book) + "/index";

  string error;
  // The fetched HTML is a string of chapter count of the chosen book.
  string html = resource_logic_web_or_cache_get(url, error);

  vector <string> chapters;

  // Iterate pushing back the iterated index until its equal to the fetched
  // chapter count.
  for (int i = 0; i < convert_to_int (html); i++) {
    chapters.push_back (convert_to_string(i));
  }

  return chapters;
}


// Returns an array with the available Bibles.
vector <string> Database_Bibles::getBibles ()
{
  vector <string> bibles = filter_url_scandir (mainFolder ());
  // Indonesian Cloud Free
  // Return only "AlkitabKita".
  if (config_logic_indonesian_cloud_free_simple ()) {
    bibles.clear ();
    bibles.push_back (icfBibleName ());
  }
  return bibles;
}


// Creates a new empty Bible. Returns its ID.
void Database_Bibles::createBible (string name)
{
  // Create the empty system.
  string folder = bibleFolder (name);
  filter_url_mkdir (folder);
  
  Database_State::setExport (name, 0, Export_Logic::export_needed);
}


// Deletes a Bible.
void Database_Bibles::deleteBible (string name)
{
  string path = bibleFolder (name);
  // Delete directory.
  filter_url_rmdir (path);
  // Just in case it was a regular file: Delete it.
  filter_url_unlink (path);
  Database_State::setExport (name, 0, Export_Logic::export_needed);
}


// Stores data of one chapter in Bible $name,
void Database_Bibles::storeChapter (string name, int book, int chapter_number, string chapter_text)
{
  string folder = chapterFolder (name, book, chapter_number);
  if (!file_or_dir_exists (folder)) filter_url_mkdir (folder);

  // Ensure that the data to be stored ends with a new line.
  if (!chapter_text.empty ()) {
    size_t pos = chapter_text.length () - 1;
    if (chapter_text.substr (pos, 1) != "\n") {
      chapter_text.append ("\n");
    }
  }
  // Increase the chapter identifier, and store the chapter data.
  int id = getChapterId (name, book, chapter_number);
  id++;
  string file = filter_url_create_path ({folder, convert_to_string (id)});
  filter_url_file_put_contents (file, chapter_text);

  // Update search fields.
  updateSearchFields (name, book, chapter_number);
  
  Database_State::setExport (name, 0, Export_Logic::export_needed);
}


void Database_Bibles::updateSearchFields (string name, int book, int chapter)
{
  search_logic_index_chapter (name, book, chapter);
}


// Returns an array with the available books in a Bible.
vector <int> Database_Bibles::getBooks (string bible)
{
  // Read the books from the database.
  string folder = bibleFolder (bible);
  vector <int> books;
  vector <string> files = filter_url_scandir (folder);
  // Indonesian Cloud Free
  // Read the books for TSI from the external database.
  if (config_logic_indonesian_cloud_free_simple ()) files = icfBooks ();
  for (string book : files) {
    if (filter_string_is_numeric (book)) books.push_back (convert_to_int (book));
  }

  // Sort the books according to the order defined in the books database.
  vector <int> order;
  for (auto & book : books) {
    order.push_back (Database_Books::getOrderFromId (book));
  }
  quick_sort (order, books, 0, static_cast<unsigned>(order.size()));

  // Result.
  return books;
}


void Database_Bibles::deleteBook (string bible, int book)
{
  string folder = bookFolder (bible, book);
  filter_url_rmdir (folder);
  Database_State::setExport (bible, 0, Export_Logic::export_needed);
}


// Returns an array with the available chapters in a $book in a Bible.
vector <int> Database_Bibles::getChapters (string bible, int book)
{
  // Read the chapters from the database.
  string folder = bookFolder (bible, book);
  vector <int> chapters;
  vector <string> files = filter_url_scandir (folder);
  // Indonesian Cloud Free
  // Read the chapters for TSI from the external database.
  if (config_logic_indonesian_cloud_free_simple ()) files = icfChapters (book);
  for (string file : files) {
    if (filter_string_is_numeric (file)) chapters.push_back (convert_to_int (file));
  }
  sort (chapters.begin (), chapters.end ());
  return chapters;
}


void Database_Bibles::deleteChapter (string bible, int book, int chapter)
{
  string folder = chapterFolder (bible, book, chapter);
  filter_url_rmdir (folder);
  Database_State::setExport (bible, 0, Export_Logic::export_needed);
}


// Gets the chapter data as a string.
string Database_Bibles::getChapter (string bible, int book, int chapter)
{
  // Indonesian Cloud Free
  // Read the chapter data for TSI from the external database.
  if (config_logic_indonesian_cloud_free_simple ()) {
    string filename = "100000001";
    string url = icfURL () + icfBibleName() + "/" + convert_to_string (book) +  "/" + convert_to_string (chapter) + "/" + filename;
    string error;
    string html = resource_logic_web_or_cache_get (url, error);
    return html;
  }
  // Read the chapter data from the database.
  string folder = chapterFolder (bible, book, chapter);
  vector <string> files = filter_url_scandir (folder);
  if (!files.empty ()) {
    string file = files [files.size () - 1];
    string data = filter_url_file_get_contents (filter_url_create_path ({folder, file}));
    // Remove trailing new line.
    data = filter_string_trim (data);
    return data;
  }
  return "";
}


// Gets the chapter id.
int Database_Bibles::getChapterId (string bible, int book, int chapter)
{
  string folder = chapterFolder (bible, book, chapter);
  vector <string> files = filter_url_scandir (folder);
  if (!files.empty ()) {
    string file = files [files.size() - 1];
    return convert_to_int (file);
  }
  return 100'000'000;
}


// Gets the chapter's time stamp in seconds since the Epoch.
int Database_Bibles::getChapterAge (string bible, int book, int chapter)
{
  string folder = chapterFolder (bible, book, chapter);
  vector <string> files = filter_url_scandir (folder);
  if (!files.empty ()) {
    string file = files [files.size() - 1];
    string path = filter_url_create_path ({folder, file});
    int time = filter_url_file_modification_time (path);
    int now = filter::date::seconds_since_epoch ();
    return now - time;
  }
  return 100'000'000;
}


void Database_Bibles::optimize ()
{
  // Go through all chapters in all books and all Ḃibles.
  vector <string> bibles = getBibles ();
  for (string bible : bibles) {
    vector <int> books = getBooks (bible);
    for (int book : books) {
      vector <int> chapters = getChapters (bible, book);
      for (int chapter : chapters) {
        string folder = chapterFolder (bible, book, chapter);
        // Read the files in the folder.
        vector <string> files = filter_url_scandir (folder);
        // Remove files with 0 size. so that in case a chapter was emptied by accident, 
        // it is removed now, effectually reverting the chapter to an earlier version.
        vector <string> files2;
        for (string file : files) {
          string path = filter_url_create_path ({folder, file});
          if (filter_url_filesize (path) == 0) {
            filter_url_unlink (path);
            Database_State::setExport (bible, 0, Export_Logic::export_needed);
          }
          else files2.push_back (file);
        }
        // Remove the three most recent files from the array, so they don't get deleted.
        // Because scandir sorts the files, the files to be kept are at the end.
        if (!files2.empty()) files2.pop_back ();
        if (!files2.empty()) files2.pop_back ();
        if (!files2.empty()) files2.pop_back ();
        // Remove the remaining files. These are the older versions.
        for (string file : files2) {
          string path = filter_url_create_path ({folder, file});
          filter_url_unlink (path);
        }
      }
    }
  }
}

