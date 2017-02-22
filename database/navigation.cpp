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


#include <database/navigation.h>
#include <filter/string.h>
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
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "DELETE FROM navigation;");
  database_sqlite_disconnect (db);
}


void Database_Navigation::record (int time, string user, int book, int chapter, int verse)
{
  // Clear any 'active' flags.
  SqliteSQL sql1 = SqliteSQL ();
  sql1.add ("UPDATE navigation SET active = 0 WHERE username =");
  sql1.add (user);
  sql1.add (";");

  // Remove too recent entries.
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


bool Database_Navigation::previousExists (const string& user)
{
  return (getPreviousId (user) != 0);
}


bool Database_Navigation::nextExists (const string& user)
{
  return (getNextId (user) != 0);
}


Passage Database_Navigation::getPrevious (const string& user)
{
  int id = getPreviousId (user);
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
  for (unsigned int i = 0; i < books.size(); i++) {
    Passage passage;
    passage.book = convert_to_int (books [i]);
    passage.chapter = convert_to_int (chapters [i]);
    passage.verse = verses [i];
    return passage;
  }
  return Passage ();
}


Passage Database_Navigation::getNext (const string& user)
{
  int id = getNextId (user);
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
  for (unsigned int i = 0; i < books.size(); i++) {
    Passage passage;
    passage.book = convert_to_int (books [i]);
    passage.chapter = convert_to_int (chapters [i]);
    passage.verse = verses [i];
    return passage;
  }
  return Passage ();
}


int Database_Navigation::getPreviousId (const string& user)
{
  // Get the ID of the active entry for the user.
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
  // If no active ID was found, return NULL.
  if (id == 0) return 0;

  // Get the ID of the entry just before the active entry.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid FROM navigation WHERE rowid <");
  sql.add (id);
  sql.add ("AND username =");
  sql.add (user);
  sql.add ("ORDER BY rowid DESC LIMIT 1;");
  sqlite3 * db = connect ();
  vector <string> ids = database_sqlite_query (db, sql.sql) ["rowid"];
  database_sqlite_disconnect (db);
  for (auto & s : ids) {
    return convert_to_int (s);
  }

  // Nothing found.
  return 0;
}


int Database_Navigation::getNextId (const string& user)
{
  // Get the ID of the active entry for the user.
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
  // If no active ID was found, return NULL.
  if (id == 0) return 0;

  // Get the ID of the entry just after the active entry.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid FROM navigation WHERE rowid >");
  sql.add (id);
  sql.add ("AND username =");
  sql.add (user);
  sql.add ("ORDER BY rowid ASC LIMIT 1;");
  sqlite3 * db = connect ();
  vector <string> ids = database_sqlite_query (db, sql.sql) ["rowid"];
  database_sqlite_disconnect (db);
  for (auto & s : ids) {
    return convert_to_int (s);
  }

  // Nothing found.
  return 0;
}

