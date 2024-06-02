/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <database/logic.h>


// Database resilience: 
// The data is stored as separate files in the filesystem.
// That is resilient enough.


std::string Database_UsfmResources::mainFolder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "usfmresources"});
}


std::string Database_UsfmResources::resourceFolder (const std::string& name)
{
  return filter_url_create_path ({mainFolder (), name});
}


std::string Database_UsfmResources::bookFolder (const std::string& name, int book)
{
  return filter_url_create_path ({resourceFolder (name), std::to_string (book)});
}


std::string Database_UsfmResources::chapterFile (const std::string& name, int book, int chapter)
{
  return filter_url_create_path ({bookFolder (name, book), std::to_string (chapter)});
}


std::vector <std::string> Database_UsfmResources::getResources ()
{
  return filter_url_scandir (mainFolder ());
}


void Database_UsfmResources::deleteResource (const std::string& name)
{
  std::string path = resourceFolder (name);
  // If a folder: Delete it.
  filter_url_rmdir (path);
  // If a file: Delete it.
  filter_url_unlink (path);
}


void Database_UsfmResources::deleteBook (const std::string& name, int book)
{
  std::string path = bookFolder (name, book);
  // If a folder: Delete it.
  filter_url_rmdir (path);
  // If a file: Delete it.
  filter_url_unlink (path);
}


void Database_UsfmResources::deleteChapter (const std::string& name, int book, int chapter)
{
  filter_url_unlink (chapterFile (name, book, chapter));
}


void Database_UsfmResources::storeChapter (const std::string& name, int book, int chapter, const std::string& usfm)
{
  std::string file = chapterFile (name, book, chapter);
  std::string folder = filter_url_dirname (file);
  if (!file_or_dir_exists (folder)) filter_url_mkdir (folder);
  filter_url_file_put_contents (file, usfm);
}


std::vector <int> Database_UsfmResources::getBooks (const std::string& name)
{
  std::vector <int> books;
  std::vector <std::string> files = filter_url_scandir (resourceFolder (name));
  for (auto & book : files) books.push_back (filter::strings::convert_to_int (book));
  sort (books.begin (), books.end());
  return books;
}


std::vector <int> Database_UsfmResources::getChapters (const std::string& name, int book)
{
  std::vector <int> chapters;
  std::vector <std::string> folders = filter_url_scandir (bookFolder (name, book));
  for (auto & chapter : folders) chapters.push_back (filter::strings::convert_to_int (chapter));
  sort (chapters.begin(), chapters.end());
  return chapters;
}


std::string Database_UsfmResources::getUsfm (const std::string& name, int book, int chapter)
{
  std::string file = chapterFile (name, book, chapter);
  std::string usfm = filter_url_file_get_contents (file);
  return usfm;
}


int Database_UsfmResources::getSize (const std::string& name, int book, int chapter)
{
  std::string file = chapterFile (name, book, chapter);
  return filter_url_filesize (file);
}
