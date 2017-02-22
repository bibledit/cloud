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


#include <database/usfmresources.h>
#include <filter/url.h>
#include <filter/string.h>


// Database resilience: 
// The data is stored as separate files in the filesystem.
// That is resilient enough.


string Database_UsfmResources::mainFolder ()
{
  return filter_url_create_root_path ("databases", "usfmresources");
}


string Database_UsfmResources::resourceFolder (const string& name)
{
  return filter_url_create_path (mainFolder (), name);
}


string Database_UsfmResources::bookFolder (const string& name, int book)
{
  return filter_url_create_path (resourceFolder (name), convert_to_string (book));
}


string Database_UsfmResources::chapterFile (const string& name, int book, int chapter)
{
  return filter_url_create_path (bookFolder (name, book), convert_to_string (chapter));
}


vector <string> Database_UsfmResources::getResources ()
{
  return filter_url_scandir (mainFolder ());
}


void Database_UsfmResources::deleteResource (const string& name)
{
  string path = resourceFolder (name);
  // If a folder: Delete it.
  filter_url_rmdir (path);
  // If a file: Delete it.
  filter_url_unlink (path);
}


void Database_UsfmResources::deleteBook (const string& name, int book)
{
  string path = bookFolder (name, book);
  // If a folder: Delete it.
  filter_url_rmdir (path);
  // If a file: Delete it.
  filter_url_unlink (path);
}


void Database_UsfmResources::deleteChapter (const string& name, int book, int chapter)
{
  filter_url_unlink (chapterFile (name, book, chapter));
}


void Database_UsfmResources::storeChapter (const string& name, int book, int chapter, const string& usfm)
{
  string file = chapterFile (name, book, chapter);
  string folder = filter_url_dirname (file);
  if (!file_or_dir_exists (folder)) filter_url_mkdir (folder);
  filter_url_file_put_contents (file, usfm);
}


vector <int> Database_UsfmResources::getBooks (const string& name)
{
  vector <int> books;
  vector <string> files = filter_url_scandir (resourceFolder (name));
  for (auto & book : files) books.push_back (convert_to_int (book));
  sort (books.begin (), books.end());
  return books;
}


vector <int> Database_UsfmResources::getChapters (const string& name, int book)
{
  vector <int> chapters;
  vector <string> folders = filter_url_scandir (bookFolder (name, book));
  for (auto & chapter : folders) chapters.push_back (convert_to_int (chapter));
  sort (chapters.begin(), chapters.end());
  return chapters;
}


string Database_UsfmResources::getUsfm (const string& name, int book, int chapter)
{
  string file = chapterFile (name, book, chapter);
  string usfm = filter_url_file_get_contents (file);
  return usfm;
}


int Database_UsfmResources::getSize (const string& name, int book, int chapter)
{
  string file = chapterFile (name, book, chapter);
  return filter_url_filesize (file);
}
