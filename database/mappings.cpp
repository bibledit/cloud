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


#include <database/mappings.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <database/books.h>


// This is a database for the verse mapping systems.
// Resilience: It is normally not written to, so corruption is unlikely.


Database_Mappings::Database_Mappings ()
{
}


Database_Mappings::~Database_Mappings ()
{
}


sqlite3 * Database_Mappings::connect ()
{
  return database_sqlite_connect ("mappings");
}


void Database_Mappings::create1 ()
{
  sqlite3 * db = connect ();
  string sql = 
    "CREATE TABLE IF NOT EXISTS maps ("
    "name text,"
    "book integer,"
    "chapter integer,"
    "verse integer,"
    "origbook integer,"
    "origchapter integer,"
    "origverse integer"
    ");";
  database_sqlite_exec (db, sql);
  sql = "DROP INDEX IF EXISTS bible;";
  database_sqlite_exec (db, sql);
  sql = "DROP INDEX IF EXISTS original;";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


void Database_Mappings::create2 ()
{
  string sql;
  sqlite3 * db = connect ();
  sql = "CREATE INDEX IF NOT EXISTS bible ON maps (name, book, chapter, verse);";
  database_sqlite_exec (db, sql);
  sql = "CREATE INDEX IF NOT EXISTS original ON maps (name, book, chapter, verse);";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


// Import the default mappings that come with Bibledit.
void Database_Mappings::defaults ()
{
  string folder = filter_url_create_root_path ("mapping");
  vector <string> files = filter_url_scandir (folder);
  for (auto & file : files) {
    string name (file);
    string extension = filter_url_get_extension (name);
    if (extension != "txt") continue;
    name = name.substr (0, strlen (name.c_str()) - 4);
    name = filter_string_str_replace ("_", " ", name);
    string path = filter_url_create_path (folder, file);
    string data = filter_url_file_get_contents (path);
    import (name, data);
  }
}


void Database_Mappings::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM maps;");
  database_sqlite_disconnect (db);
}


void Database_Mappings::import (const string& name, const string& data)
{
  // Delete existing mapping with this name.
  erase (name);

  sqlite3 * db = connect ();

  // Begin a transaction for better speed.
  database_sqlite_exec (db, "BEGIN;");
  
  Passage lastPassage = Passage ("", 1, 1, "1");
  Passage lastOriginal = Passage ("", 1, 1, "1");
  
  vector <string> lines = filter_string_explode (data, '\n');
  for (string line : lines) {
    
    // Each line looks like this: 
    // Haggai 2:15 = Haggai 2:14
    // At the left is the passage in this versification.
    // At the right is the passage in the original Hebrew and Greek versification.
    line = filter_string_trim (line);
    if (line.empty ()) continue;

    // Cut the line into two: The two passages.
    vector <string> entry = filter_string_explode (line, '=');
    if (entry.size() != 2) continue;
    
    string spassage = filter_string_trim (entry [0]);
    string soriginal = filter_string_trim (entry [1]);
    
    Passage passage = filter_passage_interpret_passage (lastPassage, spassage);
    lastPassage.book = passage.book;
    lastPassage.chapter = passage.chapter;
    lastPassage.verse = passage.verse;
    Passage original = filter_passage_interpret_passage (lastOriginal, soriginal);
    lastOriginal.book = original.book;
    lastOriginal.chapter = original.chapter;
    lastOriginal.verse = original.verse;

    int book        = passage.book;
    int chapter     = passage.chapter;
    int verse       = convert_to_int (passage.verse);
    int origbook    = original.book;
    int origchapter = original.chapter;
    int origverse   = convert_to_int (original.verse);

    SqliteSQL sql = SqliteSQL ();
    sql.add ("INSERT INTO maps VALUES (");
    sql.add (name);
    sql.add (",");
    sql.add (book);
    sql.add (",");
    sql.add (chapter);
    sql.add (",");
    sql.add (verse);
    sql.add (",");
    sql.add (origbook);
    sql.add (",");
    sql.add (origchapter);
    sql.add (",");
    sql.add (origverse);
    sql.add (");");
    database_sqlite_exec (db, sql.sql);
  }

  // Commit the transaction.
  database_sqlite_exec (db, "COMMIT;");

  database_sqlite_disconnect (db);
}


// Exports a mapping.
string Database_Mappings::output (const string& name)
{
  vector <string> data;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT * FROM maps WHERE name =");
  sql.add (name);
  sql.add ("ORDER BY book ASC, chapter ASC, verse ASC;");
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  vector <string> origbooks = result ["origbook"];
  vector <string> origchapters = result ["origchapter"];
  vector <string> origverses = result ["origverse"];

  for (unsigned int i = 0; i < books.size (); i++) {
    int book = convert_to_int (books [i]);
    string bookname = Database_Books::getEnglishFromId (book);
    string chapter = chapters [i];
    string verse = verses [i];
    int origbook = convert_to_int (origbooks[i]);
    string origbookname = Database_Books::getEnglishFromId (origbook);
    string origchapter = origchapters[i];
    string origverse = origverses [i];
    string item = bookname + " " + chapter + ":" + verse + " = " + origbookname + " " + origchapter + ":" + origverse;
    data.push_back (item);
  }
  return filter_string_implode (data, "\n");
}


void Database_Mappings::create (const string & name)
{
  // Insert one entry, so the $name makes it into the database.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO maps VALUES (");
  sql.add (name);
  sql.add (", 1, 1, 1, 1, 1, 1);");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


void Database_Mappings::erase (const string & name)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM maps WHERE name =");
  sql.add (name);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Returns the mapping names in the database.
vector <string> Database_Mappings::names ()
{
  // Get the names from the database.
  sqlite3 * db = connect ();
  vector <string> names = database_sqlite_query (db, "SELECT DISTINCT name FROM maps;")["name"];
  database_sqlite_disconnect (db);
  // Ensure the original mapping is there too.
  if (find (names.begin (), names.end (), original ()) == names.end()) {
    names.push_back (original ());
  }

  sort (names.begin (), names.end());

  return names;
}


string Database_Mappings::original ()
{
  return "Hebrew Greek";
}


// This function translates a $book, $chapter, and $verse 
// in the $input versification to a $book, $chapter and $verse 
// in the $output versification.
// It returns an array with one passage in most cases.
// When the verses in the $input and $output versifications overlap,
// it may return an array with two passages.
vector <Passage> Database_Mappings::translate (const string& input, const string& output, int book, int chapter, int verse)
{
  // Care for situation that the input and output are the same.
  if (input == output) {
    Passage passage = Passage ("", book, chapter, convert_to_string (verse));
    return {passage};
  }

  // Get the $input mapping for the passage from the database.
  // This maps the $input to the Hebrew/Greek versification system.
  // Skip this phase if the $input mapping is Hebrew / Greek.
  vector <Passage> origpassage;
  if (input != original ()) {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT origbook, origchapter, origverse FROM maps WHERE name =");
    sql.add (input);
    sql.add ("AND book =");
    sql.add (book);
    sql.add ("AND chapter =");
    sql.add (chapter);
    sql.add ("AND verse =");
    sql.add (verse);
    sql.add (";");
    sqlite3 * db = connect ();
    map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
    database_sqlite_disconnect (db);
    vector <string> origbooks = result ["origbook"];
    vector <string> origchapters = result ["origchapter"];
    vector <string> origverses = result ["origverse"];
    for (unsigned int i = 0; i < origbooks.size (); i++) {
      Passage passage = Passage ("", convert_to_int (origbooks [i]), convert_to_int (origchapters [i]), origverses [i]);
      origpassage.push_back (passage);
    }
  }
  
  // Check that the search yields a passage.
  // If there is none, it means that the $input passage is the same as in Hebrew/Greek.
  if (origpassage.empty ()) {
    Passage passage = Passage ("", book, chapter, convert_to_string (verse));
    origpassage.push_back (passage);
  }

  // If the $output mapping is Hebrew/Greek, then we're done.
  if (output == original ()) {
    return origpassage;
  }
  
  // Get the $output mapping for the passage or two passages from the database.
  // This is a translation from Hebrew/Greek to the $output system.
  vector <Passage> targetpassage;
  for (Passage & passage : origpassage) {
    int origbook = passage.book;
    int origchapter = passage.chapter;
    int origverse = convert_to_int (passage.verse);
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT book, chapter, verse FROM maps WHERE name =");
    sql.add (output);
    sql.add ("AND origbook =");
    sql.add (origbook);
    sql.add ("AND origchapter =");
    sql.add (origchapter);
    sql.add ("AND origverse =");
    sql.add (origverse);
    sql.add (";");
    sqlite3 * db = connect ();
    map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
    database_sqlite_disconnect (db);
    vector <string> books = result ["book"];
    vector <string> chapters = result ["chapter"];
    vector <string> verses = result ["verse"];
    for (unsigned int i = 0; i < books.size (); i++) {
      Passage passage = Passage ("", convert_to_int (books [i]), convert_to_int (chapters [i]), verses [i]);
      bool passageExists = false;
      for (auto & existingpassage : targetpassage) {
        if (existingpassage.equal (passage)) passageExists = true;
      }
      if (!passageExists) targetpassage.push_back (passage);
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