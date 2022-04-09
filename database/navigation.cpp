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


#include <database/navigation.h>
#include <filter/string.h>
#include <filter/date.h>
#include <database/sqlite.h>
#include <webserver/request.h>
#include <filter/passage.h>


// Database resilience: It is re-created every night 


sqlite3 * Database_Navigation::connect ()
{
  return database_sqlite_connect ("navigation");
}


void Database_Navigation::create ()
{
  sqlite3 * db = connect ();
  string sql = 
    "CREATE TABLE IF NOT EXISTS navigation ("
    "  timestamp integer,"
    "  username text,"
    "  book integer,"
    "  chapter integer,"
    "  verse integer,"
    "  active boolean"
    ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


void Database_Navigation::trim ()
{
  // Delete items older than, say, several weeks.
  int time = filter::date::seconds_since_epoch ();
  time -= (3600 * 24 * 14);
  SqliteSQL sql;
  sql.add ("DELETE FROM navigation WHERE timestamp <=");
  sql.add (time);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


void Database_Navigation::record (int time, string user, int book, int chapter, int verse)
{
  // Clear any 'active' flags.
  SqliteSQL sql1 = SqliteSQL ();
  sql1.add ("UPDATE navigation SET active = 0 WHERE username =");
  sql1.add (user);
  sql1.add (";");

  // Remove entries recorded less than several seconds ago.
  SqliteSQL sql2 = SqliteSQL ();
  int recent = time - 5;
  sql2.add ("DELETE FROM navigation WHERE timestamp >=");
  sql2.add (recent);
  sql2.add ("AND username =");
  sql2.add (user);
  sql2.add (";");

  // Record entry.
  SqliteSQL sql3 = SqliteSQL ();
  sql3.add ("INSERT INTO navigation VALUES (");
  sql3.add (time);
  sql3.add (",");
  sql3.add (user);
  sql3.add (",");
  sql3.add (book);
  sql3.add (",");
  sql3.add (chapter);
  sql3.add (",");
  sql3.add (verse);
  sql3.add (", 1);");

  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql1.sql);
  database_sqlite_exec (db, sql2.sql);
  database_sqlite_exec (db, sql3.sql);
  database_sqlite_disconnect (db);
}


bool Database_Navigation::previous_exists (const string& user)
{
  return (get_previous_id (user) != 0);
}


bool Database_Navigation::next_exists (const string& user)
{
  return (get_next_id (user) != 0);
}


Passage Database_Navigation::get_previous (const string& user)
{
  int id = get_previous_id (user);
  if (id == 0) return Passage ();

  // Update the 'active' flag.
  SqliteSQL sql1 = SqliteSQL ();
  sql1.add ("UPDATE navigation SET active = 0 WHERE username =");
  sql1.add (user);
  sql1.add (";");
  SqliteSQL sql2 = SqliteSQL ();
  sql2.add ("UPDATE navigation SET active = 1 WHERE rowid =");
  sql2.add (id);
  sql2.add (";");

  // Read the passage.
  map <string, vector <string> > result;
  SqliteSQL sql3 = SqliteSQL ();
  sql3.add ("SELECT book, chapter, verse FROM navigation WHERE rowid =");
  sql3.add (id);
  sql3.add (";");

  // Run all of the SQL at once, to minimize the database connection time.
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql1.sql);
  database_sqlite_exec (db, sql2.sql);
  result = database_sqlite_query (db, sql3.sql);
  database_sqlite_disconnect (db);
  
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  if (!books.empty()) {
    Passage passage;
    passage.book = convert_to_int (books [0]);
    passage.chapter = convert_to_int (chapters [0]);
    passage.verse = verses [0];
    return passage;
  }
  return Passage ();
}


Passage Database_Navigation::get_next (const string& user)
{
  int id = get_next_id (user);
  if (id == 0) return Passage ();

  // Update the 'active' flag.
  SqliteSQL sql1 = SqliteSQL ();
  sql1.add ("UPDATE navigation SET active = 0 WHERE username =");
  sql1.add (user);
  sql1.add (";");
  SqliteSQL sql2 = SqliteSQL ();
  sql2.add ("UPDATE navigation SET active = 1 WHERE rowid =");
  sql2.add (id);
  sql2.add (";");

  // Read the passage.
  map <string, vector <string> > result;
  SqliteSQL sql3 = SqliteSQL ();
  sql3.add ("SELECT book, chapter, verse FROM navigation WHERE rowid =");
  sql3.add (id);
  sql3.add (";");

  // Run all of the SQL at once.
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql1.sql);
  database_sqlite_exec (db, sql2.sql);
  result = database_sqlite_query (db, sql3.sql);
  database_sqlite_disconnect (db);
  
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  if (!books.empty()) {
    Passage passage;
    passage.book = convert_to_int (books [0]);
    passage.chapter = convert_to_int (chapters [0]);
    passage.verse = verses [0];
    return passage;
  }
  return Passage ();
}


int Database_Navigation::get_previous_id (const string& user)
{
  // Get the database row identifier of the active entry for the user.
  int id = 0;
  {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT rowid FROM navigation WHERE username =");
    sql.add (user);
    sql.add ("AND active = 1;");
    sqlite3 * db = connect ();
    vector <string> ids = database_sqlite_query (db, sql.sql) ["rowid"];
    for (auto & s : ids) {
      id = convert_to_int (s);
    }
    database_sqlite_disconnect (db);
  }
  // If no active row identifier was found, return zero.
  if (id == 0) return 0;

  // Get the database row identifier of the entry just before the active entry.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid FROM navigation WHERE rowid <");
  sql.add (id);
  sql.add ("AND username =");
  sql.add (user);
  sql.add ("ORDER BY rowid DESC LIMIT 1;");
  sqlite3 * db = connect ();
  vector <string> ids = database_sqlite_query (db, sql.sql) ["rowid"];
  database_sqlite_disconnect (db);
  if (!ids.empty()) {
    return convert_to_int (ids[0]);
  }

  // Nothing found.
  return 0;
}


int Database_Navigation::get_next_id (const string& user)
{
  // Get the database row identifier of the active entry for the user.
  int id = 0;
  {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT rowid FROM navigation WHERE username =");
    sql.add (user);
    sql.add ("AND active = 1;");
    sqlite3 * db = connect ();
    vector <string> ids = database_sqlite_query (db, sql.sql) ["rowid"];
    for (auto & s : ids) {
      id = convert_to_int (s);
    }
    database_sqlite_disconnect (db);
  }
  // If no active row identifier was found, return zero.
  if (id == 0) return 0;

  // Get the database row identifier of the entry just after the active entry.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid FROM navigation WHERE rowid >");
  sql.add (id);
  sql.add ("AND username =");
  sql.add (user);
  sql.add ("ORDER BY rowid ASC LIMIT 1;");
  sqlite3 * db = connect ();
  vector <string> ids = database_sqlite_query (db, sql.sql) ["rowid"];
  database_sqlite_disconnect (db);
  if (!ids.empty()) {
    return convert_to_int (ids[0]);
  }

  // Nothing found.
  return 0;
}


// The $user for whom to get the history.
// The $direction into which to get the history:
// * negative: Get the past history as if going back.
// * positive: Get the future history as if going forward.
vector <Passage> Database_Navigation::get_history (const string& user, int direction)
{
  vector <Passage> passages;
  
  int id = 0;
  if (direction > 0) id = get_next_id(user);
  if (direction < 0) id = get_previous_id (user);
  if (id) {

    // Read the passages history for this user.
    map <string, vector <string> > result;
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT book, chapter, verse FROM navigation WHERE rowid");
    if (direction > 0) sql.add (">=");
    if (direction < 0) sql.add ("<=");
    sql.add (id);
    sql.add ("AND username =");
    sql.add (user);

    // Order the results depending on getting the history forward or backward.
    sql.add ("ORDER BY rowid");
    if (direction > 0) sql.add ("ASC");
    if (direction < 0) sql.add ("DESC");
    sql.add (";");

    // Run the query on the database.
    sqlite3 * db = connect ();
    result = database_sqlite_query (db, sql.sql);
    database_sqlite_disconnect (db);

    // Assemble the results.
    vector <string> books = result ["book"];
    vector <string> chapters = result ["chapter"];
    vector <string> verses = result ["verse"];
    for (unsigned int i = 0; i < books.size(); i++) {
      Passage passage;
      passage.book = convert_to_int (books [i]);
      passage.chapter = convert_to_int (chapters [i]);
      passage.verse = verses [i];
      passages.push_back(passage);
    }
  }
  
  // Done.
  return passages;
}
