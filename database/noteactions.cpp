/*
Copyright (©) 2003-2025 Teus Benschop.

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


constexpr const auto noteactions {"noteactions"};


void Database_NoteActions::create ()
{
  SqliteDatabase sql (noteactions);
  sql.set_sql ("CREATE TABLE IF NOT EXISTS noteactions ("
               " username text,"
               " note integer,"
               " timestamp integer,"
               " action integer,"
               " content text"
               ");");
  sql.execute ();
}


void Database_NoteActions::clear ()
{
  {
    SqliteDatabase sql (noteactions);
    sql.set_sql ("DROP TABLE IF EXISTS noteactions;");
    sql.execute();
  }
  create ();
}


void Database_NoteActions::optimize ()
{
  SqliteDatabase sql (noteactions);
  sql.set_sql ("VACUUM;");
  sql.execute ();
}


void Database_NoteActions::record (const std::string& username, int note, int action, const std::string& content)
{
  SqliteDatabase sql (noteactions);
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
  sql.execute ();
}


std::vector <int> Database_NoteActions::getNotes ()
{
  std::vector <int> notes;
  SqliteDatabase sql (noteactions);
  sql.set_sql ("SELECT DISTINCT note FROM noteactions ORDER BY rowid;");
  const std::vector <std::string> result = sql.query () ["note"];
  for (const auto& note : result) {
    notes.push_back (filter::strings::convert_to_int (note));
  }
  return notes;
}


std::vector <Database_Note_Action> Database_NoteActions::getNoteData (int note)
{
  std::vector <Database_Note_Action> data;
  SqliteDatabase sql (noteactions);
  sql.add ("SELECT rowid, username, timestamp, action, content FROM noteactions WHERE note =");
  sql.add (note);
  sql.add ("ORDER BY rowid;");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> rowids = result ["rowid"];
  const std::vector <std::string> usernames = result ["username"];
  const std::vector <std::string> timestamps = result ["timestamp"];
  const std::vector <std::string> actions = result ["action"];
  const std::vector <std::string> contents = result ["content"];
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
  SqliteDatabase sql (noteactions);
  sql.add ("UPDATE noteactions SET note =");
  sql.add (newId);
  sql.add ("WHERE note =");
  sql.add (oldId);
  sql.add (";");
  sql.execute ();
}


void Database_NoteActions::erase (int rowid)
{
  SqliteDatabase sql (noteactions);
  sql.add ("DELETE FROM noteactions where rowid =");
  sql.add (rowid);
  sql.add (";");
  sql.execute ();
}


bool Database_NoteActions::exists (int note)
{
  SqliteDatabase sql (noteactions);
  sql.add ("SELECT note FROM noteactions where note =");
  sql.add (note);
  sql.add (";");
  const std::map <std::string, std::vector <std::string> > result = sql.query ();
  return !result.empty ();
}
