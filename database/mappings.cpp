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


#include <database/mappings.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <database/books.h>


// This is a database for the verse mapping systems.
// Resilience: It is normally not written to, so corruption is unlikely.


constexpr const auto mappings {"mappings"};


Database_Mappings::Database_Mappings ()
{
}


Database_Mappings::~Database_Mappings ()
{
}


void Database_Mappings::create1 ()
{
  SqliteDatabase sql (mappings);
  sql.set_sql ("CREATE TABLE IF NOT EXISTS maps ("
               "name text,"
               "book integer,"
               "chapter integer,"
               "verse integer,"
               "origbook integer,"
               "origchapter integer,"
               "origverse integer"
               ");");
  sql.execute ();
  sql.set_sql ("DROP INDEX IF EXISTS bible;");
  sql.execute ();
  sql.set_sql ("DROP INDEX IF EXISTS original;");
  sql.execute ();
}


void Database_Mappings::create2 ()
{
  SqliteDatabase sql (mappings);
  sql.set_sql ("CREATE INDEX IF NOT EXISTS bible ON maps (name, book, chapter, verse);");
  sql.execute ();
  sql.set_sql ("CREATE INDEX IF NOT EXISTS original ON maps (name, book, chapter, verse);");
  sql.execute ();
}


// Import the default mappings that come with Bibledit.
void Database_Mappings::defaults ()
{
  const std::string folder = filter_url_create_root_path ({"mapping"});
  const std::vector <std::string> files = filter_url_scandir (folder);
  for (const auto& file : files) {
    std::string name (file);
    const std::string extension = filter_url_get_extension (name);
    if (extension != "txt")
      continue;
    name = name.substr (0, strlen (name.c_str()) - 4);
    name = filter::strings::replace ("_", " ", name);
    const std::string path = filter_url_create_path ({folder, file});
    const std::string data = filter_url_file_get_contents (path);
    import (name, data);
  }
}


void Database_Mappings::optimize ()
{
  SqliteDatabase sql (mappings);
  sql.set_sql ("VACUUM;");
  sql.execute();
}


void Database_Mappings::import (const std::string& name, const std::string& data)
{
  // Delete existing mapping with this name.
  erase (name);

  SqliteDatabase sql (mappings);

  // Begin a transaction for better speed.
  sql.set_sql ("BEGIN;");
  sql.execute();

  const std::vector <std::string> lines = filter::strings::explode (data, '\n');
  for (std::string line : lines) {
    
    // Each line looks like this: 
    // Haggai 2:15 = Haggai 2:14
    // At the left is the passage in this versification.
    // At the right is the passage in the original Hebrew and Greek versification.
    line = filter::strings::trim (line);
    if (line.empty ()) continue;

    // Cut the line into two: The two passages.
    std::vector <std::string> entry = filter::strings::explode (line, '=');
    if (entry.size() != 2) continue;
    
    std::string passage_string = filter::strings::trim (entry [0]);
    std::string original_string = filter::strings::trim (entry [1]);

    // Storage for further interpretation.
    std::vector <std::string> bits;
    
    // Split the passage entry on the colon (:) to get the verse.
    bits = filter::strings::explode(passage_string, ':');
    if (bits.size() != 2) continue;
    int passage_verse = filter::strings::convert_to_int(bits[1]);
    // Split the first bit on the spaces and get the last item as the chapter.
    bits = filter::strings::explode(bits[0], ' ');
    if (bits.size() < 2) continue;
    int passage_chapter = filter::strings::convert_to_int(bits[bits.size()-1]);
    // Remove the last bit so it remains with the book, and get that book.
    bits.pop_back();
    std::string passage_book_string = filter::strings::implode(bits, " ");
    int passage_book = static_cast<int>(database::books::get_id_from_english(passage_book_string));

    // Split the original entry on the colon (:) to get the verse.
    bits = filter::strings::explode(original_string, ':');
    if (bits.size() != 2) continue;
    int original_verse = filter::strings::convert_to_int(bits[1]);
    // Split the first bit on the spaces and get the last item as the chapter.
    bits = filter::strings::explode(bits[0], ' ');
    if (bits.size() < 2) continue;
    int original_chapter = filter::strings::convert_to_int(bits[bits.size()-1]);
    // Remove the last bit so it remains with the book, and get that book.
    bits.pop_back();
    std::string original_book_string = filter::strings::implode(bits, " ");
    int original_book = static_cast<int>(database::books::get_id_from_english(original_book_string));

    // Store it in the database.
    sql.clear();
    sql.add ("INSERT INTO maps VALUES (");
    sql.add (name);
    sql.add (",");
    sql.add (passage_book);
    sql.add (",");
    sql.add (passage_chapter);
    sql.add (",");
    sql.add (passage_verse);
    sql.add (",");
    sql.add (original_book);
    sql.add (",");
    sql.add (original_chapter);
    sql.add (",");
    sql.add (original_verse);
    sql.add (");");
    sql.execute ();
  }

  // Commit the transaction.
  sql.set_sql ("COMMIT;");
  sql.execute();
}


// Exports a mapping.
std::string Database_Mappings::output (const std::string& name)
{
  std::vector <std::string> data;
  SqliteDatabase sql (mappings);
  sql.add ("SELECT * FROM maps WHERE name =");
  sql.add (name);
  sql.add ("ORDER BY book ASC, chapter ASC, verse ASC;");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> books = result ["book"];
  const std::vector <std::string> chapters = result ["chapter"];
  const std::vector <std::string> verses = result ["verse"];
  const std::vector <std::string> origbooks = result ["origbook"];
  const std::vector <std::string> origchapters = result ["origchapter"];
  const std::vector <std::string> origverses = result ["origverse"];

  for (unsigned int i = 0; i < books.size (); i++) {
    const int book = filter::strings::convert_to_int (books [i]);
    const std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    const std::string chapter = chapters [i];
    const std::string verse = verses [i];
    const int origbook = filter::strings::convert_to_int (origbooks[i]);
    const std::string origbookname = database::books::get_english_from_id (static_cast<book_id>(origbook));
    const std::string origchapter = origchapters[i];
    const std::string origverse = origverses [i];
    const std::string item = bookname + " " + chapter + ":" + verse + " = " + origbookname + " " + origchapter + ":" + origverse;
    data.push_back (item);
  }
  return filter::strings::implode (data, "\n");
}


void Database_Mappings::create (const std::string& name)
{
  // Insert one entry, so the $name makes it into the database.
  SqliteDatabase sql (mappings);
  sql.add ("INSERT INTO maps VALUES (");
  sql.add (name);
  sql.add (", 1, 1, 1, 1, 1, 1);");
  sql.execute ();
}


void Database_Mappings::erase (const std::string& name)
{
  SqliteDatabase sql (mappings);
  sql.add ("DELETE FROM maps WHERE name =");
  sql.add (name);
  sql.add (";");
  sql.execute ();
}


// Returns the mapping names in the database.
std::vector <std::string> Database_Mappings::names ()
{
  // Get the names from the database.
  SqliteDatabase sql (mappings);
  sql.set_sql ("SELECT DISTINCT name FROM maps;");
  std::vector <std::string> names = sql.query ()["name"];

  // Ensure the original mapping is there too.
  if (find (names.begin (), names.end (), original ()) == names.end()) {
    names.push_back (original ());
  }

  sort (names.begin (), names.end());

  return names;
}


std::string Database_Mappings::original ()
{
  return "Hebrew Greek";
}


// This function translates a $book, $chapter, and $verse 
// in the $input versification to a $book, $chapter and $verse 
// in the $output versification.
// It returns an array with one passage in most cases.
// When the verses in the $input and $output versifications overlap,
// it may return an array with two passages.
std::vector <Passage> Database_Mappings::translate (const std::string& input, const std::string& output, int book, int chapter, int verse)
{
  // Care for situation that the input and output are the same.
  if (input == output) {
    Passage passage = Passage ("", book, chapter, std::to_string (verse));
    return {passage};
  }

  // Get the $input mapping for the passage from the database.
  // This maps the $input to the Hebrew/Greek versification system.
  // Skip this phase if the $input mapping is Hebrew / Greek.
  std::vector <Passage> origpassage;
  if (input != original ()) {
    SqliteDatabase sql (mappings);
    sql.add ("SELECT origbook, origchapter, origverse FROM maps WHERE name =");
    sql.add (input);
    sql.add ("AND book =");
    sql.add (book);
    sql.add ("AND chapter =");
    sql.add (chapter);
    sql.add ("AND verse =");
    sql.add (verse);
    sql.add (";");
    std::map <std::string, std::vector <std::string> > result = sql.query ();
    const std::vector <std::string> origbooks = result ["origbook"];
    const std::vector <std::string> origchapters = result ["origchapter"];
    const std::vector <std::string> origverses = result ["origverse"];
    for (unsigned int i = 0; i < origbooks.size (); i++) {
      const Passage passage ("", filter::strings::convert_to_int (origbooks [i]), filter::strings::convert_to_int (origchapters [i]), origverses [i]);
      origpassage.push_back (passage);
    }
  }
  
  // Check that the search yields a passage.
  // If there is none, it means that the $input passage is the same as in Hebrew/Greek.
  if (origpassage.empty ()) {
    Passage passage = Passage ("", book, chapter, std::to_string (verse));
    origpassage.push_back (passage);
  }

  // If the $output mapping is Hebrew/Greek, then we're done.
  if (output == original ()) {
    return origpassage;
  }
  
  // Get the $output mapping for the passage or two passages from the database.
  // This is a translation from Hebrew/Greek to the $output system.
  std::vector <Passage> targetpassage;
  for (const Passage& passage : origpassage) {
    const int origbook = passage.m_book;
    const int origchapter = passage.m_chapter;
    const int origverse = filter::strings::convert_to_int (passage.m_verse);
    SqliteDatabase sql (mappings);
    sql.add ("SELECT book, chapter, verse FROM maps WHERE name =");
    sql.add (output);
    sql.add ("AND origbook =");
    sql.add (origbook);
    sql.add ("AND origchapter =");
    sql.add (origchapter);
    sql.add ("AND origverse =");
    sql.add (origverse);
    sql.add (";");
    std::map <std::string, std::vector <std::string> > result = sql.query ();
    const std::vector <std::string> books = result ["book"];
    const std::vector <std::string> chapters = result ["chapter"];
    const std::vector <std::string> verses = result ["verse"];
    for (unsigned int i = 0; i < books.size (); i++) {
      Passage passage2 = Passage (std::string(), filter::strings::convert_to_int (books [i]), filter::strings::convert_to_int (chapters [i]), verses [i]);
      bool passageExists = false;
      for (auto& existingpassage : targetpassage) {
        if (existingpassage.equal (passage2)) passageExists = true;
      }
      if (!passageExists) targetpassage.push_back (passage2);
    }
  }
  
  // Check that the search yields a passage.
  // If none, it means that the $output passage is the same as in Hebrew/Greek.
  if (targetpassage.empty()) {
    targetpassage = origpassage;
  }
  
  // Result.
  return targetpassage;
}


/*

 To not overwrite updated verse mappings, 
 there could be one database with the defaults, 
 and another one on the system with the modifications.
 The updated mappings will be stored in the second database. 
 That means that if the name of a mapping occurs in a second database, 
 that it won't read it from the first database. 
 This enables the first database to be always updated, 
 while the system keeps reading from the second database.

 Another way of doing is to keep it as it is, and make the clients download updated mappings from the Cloud.
 And then to preserver existing mappings in the Cloud on update.
 
*/
