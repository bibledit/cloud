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


#include <database/imageresources.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <webserver/request.h>
#include <database/logic.h>


// Database resilience: 
// The data is stored as images and files in the filesystem.
// That should be resilient enough.


std::string Database_ImageResources::mainFolder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "imageresources"});
}


std::string Database_ImageResources::resourceFolder (const std::string& name)
{
  return filter_url_create_path ({mainFolder (), name});
}


std::string Database_ImageResources::imagePath (std::string name, std::string image)
{
  return filter_url_create_path ({resourceFolder (name), image});
}


std::string Database_ImageResources::databaseFile ()
{
  return "passages.sqlite";
}


std::string Database_ImageResources::database_path (const std::string& name)
{
  return filter_url_create_path ({resourceFolder (name), databaseFile ()});
}


std::vector <std::string> Database_ImageResources::names ()
{
  return filter_url_scandir (mainFolder ());
}


void Database_ImageResources::create (std::string name)
{
  // Create folder to store the images.
  std::string path = resourceFolder (name);
  filter_url_unlink (path);
  filter_url_mkdir (path);

  // Create the passages database.
  SqliteDatabase sql (database_path(name));
  sql.set_sql ("CREATE TABLE IF NOT EXISTS passages ("
               " start integer,"
               " end integer,"
               " image text"
               ");");
  sql.execute ();
}


void Database_ImageResources::erase (std::string name)
{
  std::string path = resourceFolder (name);
  // If a folder: Delete it.
  filter_url_rmdir (path);
  // If a file: Delete it.
  filter_url_unlink (path);
}


void Database_ImageResources::erase (std::string name, std::string image)
{
  filter_url_unlink (imagePath (name, image));
  SqliteDatabase sql (database_path(name));
  sql.add ("DELETE FROM passages WHERE image =");
  sql.add (image);
  sql.add (";");
  sql.execute ();
}


// Moves $file (path to an image file) into the database.
std::string Database_ImageResources::store (std::string name, std::string file)
{
  std::string folder = resourceFolder (name);
  std::string image = filter_url_basename (file);
  std::string path;
  bool exists = false;
  do {
    path = filter_url_create_path ({folder, image});
    exists = file_or_dir_exists (path);
    if (exists) image = filter::strings::replace (".", "0.", image);
  } while (exists);
  filter_url_rename (file, path);
  return image;
}


// Assign a passage range to the $image.
// It means that this image contains text for the passage range.
void Database_ImageResources::assign (std::string name, std::string image,
                                      int book1, int chapter1, int verse1,
                                      int book2, int chapter2, int verse2)
{
  SqliteDatabase sql (database_path(name));

  sql.clear();
  sql.add ("DELETE FROM passages WHERE image =");
  sql.add (image);
  sql.add (";");
  sql.execute ();
  
  sql.clear();
  sql.add ("INSERT INTO passages VALUES (");
  sql.add (filter_passage_to_integer (Passage ("", book1, chapter1, std::to_string (verse1))));
  sql.add (",");
  sql.add (filter_passage_to_integer (Passage ("", book2, chapter2, std::to_string (verse2))));
  sql.add (",");
  sql.add (image);
  sql.add (");");
  sql.execute ();
}


std::vector <std::string> Database_ImageResources::get (std::string name, int book, int chapter, int verse)
{
  int passage = filter_passage_to_integer (Passage ("", book, chapter, std::to_string (verse)));
  SqliteDatabase sql (database_path(name));
  sql.add ("SELECT image FROM passages WHERE start <=");
  sql.add (passage);
  sql.add ("AND end >=");
  sql.add (passage);
  sql.add ("ORDER BY start;");
  const std::vector <std::string> images = sql.query () ["image"];
  return images;
}


std::vector <std::string> Database_ImageResources::get (std::string name)
{
  // Get images from database, sorted on passage.
  SqliteDatabase sql (database_path(name));
  sql.add ("SELECT image FROM passages ORDER by start;");
  std::vector <std::string> images = sql.query () ["image"];
 
  // Get images from the folder.
  std::vector <std::string> files = filter_url_scandir (resourceFolder (name));
 
  // Files on disk, and not in the list from the database, add them.
  for (const auto& file : files) {
    if (!in_array (file, images)) {
      if (file != databaseFile ()) {
        images.push_back (file);
      }
    }
  }
  
  // Result.
  return images;
}


void Database_ImageResources::get (std::string name, std::string image,
                                   int & book1, int & chapter1, int & verse1,
                                   int & book2, int & chapter2, int & verse2)
{
  book1 = 0;
  chapter1 = 0;
  verse1 = 0;
  book2 = 0;
  chapter2 = 0;
  verse2 = 0;

  SqliteDatabase sql (database_path(name));
  sql.add ("SELECT start, end FROM passages WHERE image =");
  sql.add (image);
  sql.add ("ORDER by start;");
  std::map <std::string, std::vector <std::string> > results = sql.query ();
  const std::vector <std::string> start = results["start"];
  const std::vector <std::string> end   = results["end"];

  if (!start.empty ()) {
    Passage passage = filter_integer_to_passage (filter::strings::convert_to_int (start [0]));
    book1 = passage.m_book;
    chapter1 = passage.m_chapter;
    verse1 = filter::strings::convert_to_int (passage.m_verse);
  }
  
  if (!end.empty ()) {
    Passage passage = filter_integer_to_passage (filter::strings::convert_to_int (end [0]));
    book2 = passage.m_book;
    chapter2 = passage.m_chapter;
    verse2 = filter::strings::convert_to_int (passage.m_verse);
  }
}


std::string Database_ImageResources::get (std::string name, std::string image)
{
  std::string path = imagePath (name, image);
  return filter_url_file_get_contents (path);
}
