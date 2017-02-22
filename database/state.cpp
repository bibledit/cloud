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


#include <database/state.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// Database resilience: It only contains state information.
// It is checked and optionally recreated every night.


void Database_State::create ()
{
  bool healthy_database = database_sqlite_healthy (name ());
  if (!healthy_database) {
    filter_url_unlink (database_sqlite_file (name ()));
  }

  sqlite3 * db = connect ();
  string sql;
  
  // On Android, this pragma prevents the following error: VACUUM; Unable to open database file.
  sql = "PRAGMA temp_store = MEMORY;";
  database_sqlite_exec (db, sql);
  
  sql =
    "CREATE TABLE IF NOT EXISTS notes ("
    " first integer,"
    " last integer,"
    " value text"
    ");";
  database_sqlite_exec (db, sql);
  
  sql = "DELETE FROM notes;";
  database_sqlite_exec (db, sql);
  
  // Here something weird was going on when doing a VACUUM at this stage.
  // On Android, it always would say this: VACUUM; Unable to open database file.
  // Testing on the existence of the database file, right before the VACUUM operation, showed that the database file did exist. The question is then: If the file exists, why does it fail to open it?
  // It also was tried to delay with 100 milliseconds before doing the VACUUM. But this made no difference. It would still give the error.
  // It also was tried to close the connection to the database, then open it again. This made no difference either.
  // It now does not VACUUM a newly created database, but only when it was created.
  // Later on, the PRAGMA as above was used to solve the issue.
  sql = "VACUUM;";
  database_sqlite_exec (db, sql);

  sql =
    "CREATE TABLE IF NOT EXISTS export ("
    " bible text,"
    " book integer,"
    " format integer"
  ");";
  database_sqlite_exec (db, sql);
  
  sql =
    "CREATE TABLE IF NOT EXISTS exported ("
    " bible text,"
    " book integer,"
    " state boolean"
    ");";
  database_sqlite_exec (db, sql);
  
  database_sqlite_disconnect (db);
}


// Stores a notes checksum for a range of notes.
void Database_State::putNotesChecksum (int first, int last, const string& checksum)
{
  sqlite3 * db = connect ();
  {
    // Remove possible existing range.
    SqliteSQL sql = SqliteSQL ();
    sql.add ("DELETE FROM notes WHERE first =");
    sql.add (first);
    sql.add ("AND last =");
    sql.add (last);
    sql.add (";");
    database_sqlite_exec (db, sql.sql);
  }
  {
    // Store the new checksum for the range.
    SqliteSQL sql = SqliteSQL ();
    sql.add ("INSERT INTO notes VALUES (");
    sql.add (first);
    sql.add (",");
    sql.add (last);
    sql.add (",");
    sql.add (checksum);
    sql.add (");");
    database_sqlite_exec (db, sql.sql);
  }
  database_sqlite_disconnect (db);
}


// Retrieves the checksum for a range of notes.
string Database_State::getNotesChecksum (int first, int last)
{
  // Receive the checksum for the exact range.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT value FROM notes WHERE first =");
  sql.add (first);
  sql.add ("AND last =");
  sql.add (last);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> values = database_sqlite_query (db, sql.sql)["value"];
  database_sqlite_disconnect (db);
  for (auto value : values) {
    return value;
  }
  return "";
}


// Erase the checksum for a note contained in any range.
void Database_State::eraseNoteChecksum (int identifier)
{
  // Remove ranges that contain the note identifier.
  sqlite3 * db = connect ();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM notes WHERE first <=");
  sql.add (identifier);
  sql.add ("AND last >=");
  sql.add (identifier);
  sql.add (";");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Flag export of $bible $book to $format.
void Database_State::setExport (const string & bible, int book, int format)
{
  if (getExport (bible, book, format)) return;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO export VALUES (");
  sql.add (bible);
  sql.add (",");
  sql.add (book);
  sql.add (",");
  sql.add (format);
  sql.add (");");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Get whether the $bible $book has been flagged for export in format $format.
bool Database_State::getExport (const string & bible, int book, int format)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT format FROM export WHERE bible =");
  sql.add (bible);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("AND format =");
  sql.add (format);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> values = database_sqlite_query (db, sql.sql)["format"];
  database_sqlite_disconnect (db);
  for (auto value : values) {
    return true;
  }
  return false;
}


// Clear the export flag for $bible $book to $format
void Database_State::clearExport (const string & bible, int book, int format)
{
  sqlite3 * db = connect ();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM export WHERE bible =");
  sql.add (bible);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("AND format =");
  sql.add (format);
  sql.add (";");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


const char * Database_State::name ()
{
  return "state";
}


sqlite3 * Database_State::connect ()
{
  return database_sqlite_connect (name ());
}
