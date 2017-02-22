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


#include <database/bibles.h>
#include <database/books.h>
#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
#include <search/logic.h>
#include <export/logic.h>


// This database stores its data in files in the filesystem.
// This is a rugged and reliable system of storing data.
// Because no real database is used, no database can get corrupted.


string Database_Bibles::mainFolder ()
{
  return filter_url_create_root_path ("bibles");
}


string Database_Bibles::bibleFolder (string bible)
{
  return filter_url_create_path (mainFolder (), bible);
}


string Database_Bibles::bookFolder (string bible, int book)
{
  return filter_url_create_path (bibleFolder (bible), convert_to_string (book));
}


string Database_Bibles::chapterFolder (string bible, int book, int chapter)
{
  return filter_url_create_path (bookFolder (bible, book), convert_to_string (chapter));
}


// Returns an array with the available Bibles.
vector <string> Database_Bibles::getBibles ()
{
  vector <string> bibles = filter_url_scandir (mainFolder ());
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
  string file = filter_url_create_path (folder, convert_to_string (id));
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
  for (string book : files) {
    if (filter_string_is_numeric (book)) books.push_back (convert_to_int (book));
  }

  // Sort the books according to the sequence defined in the books database.
  vector <int> sequence;
  for (auto & book : books) {
    sequence.push_back (Database_Books::getSequenceFromId (book));
  }
  quick_sort (sequence, books, 0, sequence.size());

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
  string folder = chapterFolder (bible, book, chapter);
  vector <string> files = filter_url_scandir (folder);
  if (!files.empty ()) {
    string file = files [files.size () - 1];
    string data = filter_url_file_get_contents (filter_url_create_path (folder, file));
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
  return 100000000;
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
          string path = filter_url_create_path (folder, file);
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
          string path = filter_url_create_path (folder, file);
          filter_url_unlink (path);
        }
      }
    }
  }
}

