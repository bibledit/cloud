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


#include <database/versifications.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <database/books.h>
#include <database/logs.h>
#include <versification/logic.h>


// This is a database for the versification systems.
// Resilience: It is normally not written to, so corruption is unlikely.


// Versification information is also available from the Bible Organisational System.
// (This system contains versification information, no mapping data.)
// See http://freely-given.org/Software/BibleOrganisationalSystem/
// See https://github.com/openscriptures/BibleOrgSys/


sqlite3 * Database_Versifications::connect ()
{
  return database_sqlite_connect ("versifications");
}


void Database_Versifications::create ()
{
  sqlite3 * db = connect ();
  string sql;
  sql = 
    "CREATE TABLE IF NOT EXISTS names ("
    " system integer,"
    " name text"
    ");";
  database_sqlite_exec (db, sql);
  sql = 
    "CREATE TABLE IF NOT EXISTS data ("
    " system integer,"
    " book integer,"
    " chapter integer,"
    " verse integer"
    ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


void Database_Versifications::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM names;");
  database_sqlite_exec (db, "VACUUM data;");
  database_sqlite_disconnect (db);
}


// Import data.
void Database_Versifications::input (const string& contents, const string& name)
{
  // Delete old system if it is there, and create new one.
  erase (name);
  int id = createSystem (name);

  sqlite3 * db = connect ();
  database_sqlite_exec (db, "PRAGMA temp_store = MEMORY;");
  database_sqlite_exec (db, "PRAGMA synchronous = OFF;");
  database_sqlite_exec (db, "PRAGMA journal_mode = OFF;");
  database_sqlite_exec (db, "BEGIN;");
  
  vector <string> lines = filter_string_explode (contents, '\n');
  for (auto line : lines) {
    line = filter_string_trim (line);
    if (line.empty ()) continue;
    Passage passage = filter_passage_explode_passage (line);
    if ((passage.book == 0)
     || (passage.chapter == 0)
     || (passage.verse.empty ())) {
      Database_Logs::log ("Malformed versification entry: " + line);
      continue;
    }
    SqliteSQL sql = SqliteSQL ();
    sql.add ("INSERT INTO data (system, book, chapter, verse) VALUES (");
    sql.add (id);
    sql.add (",");
    sql.add (passage.book);
    sql.add (",");
    sql.add (passage.chapter);
    sql.add (",");
    sql.add (convert_to_int (passage.verse));
    sql.add (");");
    database_sqlite_exec (db, sql.sql);
  }
  
  database_sqlite_exec (db, "COMMIT;");
  database_sqlite_disconnect (db);
}


// Export data.
string Database_Versifications::output (const string& name)
{
  vector <string> lines;
  vector <Passage> versification_data = getBooksChaptersVerses (name);
  for (Passage & passage : versification_data) {
    string line = Database_Books::getEnglishFromId (passage.book);
    line.append (" ");
    line.append (convert_to_string (passage.chapter));
    line.append (":");
    line.append (passage.verse);
    lines.push_back (line);
  }
  return filter_string_implode (lines, "\n");
}


// Delete a versification system.
void Database_Versifications::erase (const string& name)
{
  int id = getID (name);

  SqliteSQL sql1 = SqliteSQL ();
  sql1.add ("DELETE FROM names WHERE system =");
  sql1.add (id);
  sql1.add (";");

  SqliteSQL sql2 = SqliteSQL ();
  sql2.add ("DELETE FROM data WHERE system =");
  sql2.add (id);
  sql2.add (";");

  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql1.sql);
  database_sqlite_exec (db, sql2.sql);
  database_sqlite_disconnect (db);
}


// Returns the ID for a named versification system.
int Database_Versifications::getID (const string& name)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT system FROM names WHERE name =");
  sql.add (name);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> systems = database_sqlite_query (db, sql.sql) ["system"];
  database_sqlite_disconnect (db);
  for (auto & id : systems) {
    return convert_to_int (id);
  }
  return 0;
}


// Creates a new empty versification system.
// Returns the new ID.
int Database_Versifications::createSystem (const string& name)
{
  // If the versification system already exists, return its ID.
  int id = getID (name);
  if (id > 0) {
    return id;
  }
  // Get the first free ID starting from 1000 (except when creating the default systems).
  id = 0;
  sqlite3 * db = connect ();
  vector <string> systems = database_sqlite_query (db, "SELECT system FROM names ORDER BY system DESC LIMIT 1;") ["system"];
  for (auto & system : systems) {
    id = convert_to_int (system);
  }
  id++;
  if (!creating_defaults) if (id < 1000) id = 1000;
  // Create the empty system.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO names VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (name);
  sql.add (");");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Return new ID.
  return id;
}


// Returns an array of the available versification systems.
vector <string> Database_Versifications::getSystems ()
{
  sqlite3 * db = connect ();
  vector <string> systems = database_sqlite_query (db, "SELECT name FROM names ORDER BY name ASC;") ["name"];
  database_sqlite_disconnect (db);
  return systems;
}


// Returns the books, chapters, verses for the given versification system.
vector <Passage> Database_Versifications::getBooksChaptersVerses (const string& name)
{
  vector <Passage> data;
  int id = getID (name);
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT book, chapter, verse FROM data WHERE system =");
  sql.add (id);
  sql.add ("ORDER BY book, chapter, verse ASC;");
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size (); i++) {
    Passage passage;
    passage.book = convert_to_int (books [i]);
    passage.chapter = convert_to_int (chapters [i]);
    passage.verse = verses [i];
    data.push_back (passage);
  }
  return data;
}


vector <int> Database_Versifications::getBooks (const string& name)
{
  vector <int> books;
  int id = getID (name);
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT book FROM data WHERE system =");
  sql.add (id);
  sql.add ("ORDER BY book ASC;");
  sqlite3 * db = connect ();
  vector <string> sbooks = database_sqlite_query (db, sql.sql) ["book"];
  database_sqlite_disconnect (db);
  for (auto & book : sbooks) {
    books.push_back (convert_to_int (book));
  }
  return books;
}


// This returns all the chapters in book of versification system name.
// include0: Includes chapter 0 also.
vector <int> Database_Versifications::getChapters (const string& name, int book, bool include0)
{
  vector <int> chapters;
  if (include0) chapters.push_back (0);
  int id = this->getID (name);
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT chapter FROM data WHERE system =");
  sql.add (id);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("ORDER BY chapter ASC;");
  sqlite3 * db = connect ();
  vector <string> schapters = database_sqlite_query (db, sql.sql) ["chapter"];
  database_sqlite_disconnect (db);
  for (auto & chapter : schapters) {
    chapters.push_back (convert_to_int (chapter));
  }
  return chapters;
}


vector <int> Database_Versifications::getVerses (const string& name, int book, int chapter)
{
  vector <int> verses;
  int id = getID (name);
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT verse FROM data WHERE system =");
  sql.add (id);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("ORDER BY verse ASC;");
  sqlite3 * db = connect ();
  vector <string> sverses = database_sqlite_query (db, sql.sql) ["verse"];
  database_sqlite_disconnect (db);
  for (auto & verse : sverses) {
    int maxverse = convert_to_int (verse);
    for (int i = 0; i <= maxverse; i++) {
      verses.push_back (i);
    }
  }
  // Put verse 0 in chapter 0.
  if (chapter == 0) verses.push_back (0);
  return verses;
}


void Database_Versifications::defaults ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "DELETE FROM names WHERE system < 1000;");
  database_sqlite_exec (db, "DELETE FROM data WHERE system < 1000;");
  database_sqlite_disconnect (db);

  creating_defaults = true;
  vector <string> names = versification_logic_names ();
  for (auto name : names) {
    string contents = versification_logic_data (name);
    input (contents, name);
  }
  creating_defaults = false;
}


// This returns all possible books in any versification system.
vector <int> Database_Versifications::getMaximumBooks ()
{
  vector <int> books;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT book FROM data ORDER BY book ASC;");
  sqlite3 * db = connect ();
  vector <string> sbooks = database_sqlite_query (db, sql.sql) ["book"];
  database_sqlite_disconnect (db);
  for (auto & book : sbooks) {
    books.push_back (convert_to_int (book));
  }
  return books;
}


// This returns all possible chapters in a book of any versification system.
vector <int> Database_Versifications::getMaximumChapters (int book)
{
  vector <int> chapters;
  chapters.push_back (0);
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT chapter FROM data WHERE book =");
  sql.add (book);
  sql.add ("ORDER BY chapter ASC;");
  sqlite3 * db = connect ();
  vector <string> schapters = database_sqlite_query (db, sql.sql) ["chapter"];
  database_sqlite_disconnect (db);
  for (auto & chapter : schapters) {
    chapters.push_back (convert_to_int (chapter));
  }
  return chapters;
}


// This returns all possible verses in a book / chapter of any versification system.
vector <int> Database_Versifications::getMaximumVerses (int book, int chapter)
{
  vector <int> verses;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT verse FROM data WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("ORDER BY verse ASC;");
  sqlite3 * db = connect ();
  vector <string> sverses = database_sqlite_query (db, sql.sql) ["verse"];
  database_sqlite_disconnect (db);
  for (auto & verse : sverses) {
    int maxverse = convert_to_int (verse);
    for (int i = 0; i <= maxverse; i++) {
      verses.push_back (i);
    }
  }
  // Put verse 0 in chapter 0.
  if (chapter == 0) verses.push_back (0);
  return verses;
}
