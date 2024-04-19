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


#include <database/noteactions.h>
#include <filter/string.h>
#include <filter/date.h>
#include <database/sqlite.h>


// Database resilience. 
// It is written to by a single user.
// No simultaneous writes are are rare.
// In case of corruption, the work done on the consultation notes is lost.
// Remove the database file, and re-run setup to correct the problem.


sqlite3 * Database_NoteActions::connect ()
{
  return database_sqlite_connect ("noteactions");
}


void Database_NoteActions::create ()
{
  sqlite3 * db = connect ();
  std::string sql = 
    "CREATE TABLE IF NOT EXISTS noteactions ("
    "  username text,"
    "  note integer,"
    "  timestamp integer,"
    "  action integer,"
    "  content text"
    ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


void Database_NoteActions::clear ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "DROP TABLE IF EXISTS noteactions;");
  database_sqlite_disconnect (db);
  create ();
}


void Database_NoteActions::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM;");
  database_sqlite_disconnect (db);
}


void Database_NoteActions::record (const std::string& username, int note, int action, const std::string& content)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO noteactions VALUES (");
  sql.add (username);
  sql.add (",");
  sql.add (note);
  sql.add (",");
  sql.add (filter::date::seconds_since_epoch ());
  sql.add (",");
  sql.add (action);
  sql.add (",");
  sql.add (content);
  sql.add (");");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


std::vector <int> Database_NoteActions::getNotes ()
{
  std::vector <int> notes;
  sqlite3 * db = connect ();
  std::vector <std::string> result = database_sqlite_query (db, "SELECT DISTINCT note FROM noteactions ORDER BY rowid;") ["note"];
  database_sqlite_disconnect (db);
  for (auto & note : result) {
    notes.push_back (filter::strings::convert_to_int (note));
  }
  return notes;
}


std::vector <Database_Note_Action> Database_NoteActions::getNoteData (int note)
{
  std::vector <Database_Note_Action> data;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid, username, timestamp, action, content FROM noteactions WHERE note =");
  sql.add (note);
  sql.add ("ORDER BY rowid;");
  sqlite3 * db = connect ();
  std::map <std::string, std::vector <std::string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  std::vector <std::string> rowids = result ["rowid"];
  std::vector <std::string> usernames = result ["username"];
  std::vector <std::string> timestamps = result ["timestamp"];
  std::vector <std::string> actions = result ["action"];
  std::vector <std::string> contents = result ["content"];
  for (unsigned int i = 0; i < rowids.size (); i++) {
    Database_Note_Action action = Database_Note_Action ();
    action.rowid = filter::strings::convert_to_int (rowids [i]);
    action.username = usernames [i];
    action.timestamp = filter::strings::convert_to_int (timestamps [i]);
    action.action = filter::strings::convert_to_int (actions [i]);
    action.content = contents [i];
    data.push_back (action);
  }
  return data;
}


// Update all actions for a note with identifier $old to $new.
void Database_NoteActions::updateNotes (int oldId, int newId)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE noteactions SET note =");
  sql.add (newId);
  sql.add ("WHERE note =");
  sql.add (oldId);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


void Database_NoteActions::erase (int rowid)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM noteactions where rowid =");
  sql.add (rowid);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


bool Database_NoteActions::exists (int note)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT note FROM noteactions where note =");
  sql.add (note);
  sql.add (";");
  sqlite3 * db = connect ();
  std::map <std::string, std::vector <std::string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  return !result.empty ();
}

