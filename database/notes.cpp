/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <database/notes.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <filter/md5.h>
#include <notes/logic.h>
#include <locale/translate.h>
#include <locale/translate.h>
#include <config/globals.h>
#include <database/sqlite.h>
#include <database/logs.h>
#include <database/state.h>
#include <trash/handler.h>
#include <webserver/request.h>
#include <jsonxx/jsonxx.h>


using namespace jsonxx;


// Database resilience.
// The notes are stored in the plain filesystem for robustness.
// A database can easily be corrupted. The filesystem is more robust.
// The notes database works like this:
// * All read operations are done from the filesystem.
// * All write operations first go to the file system, then to the database.
// * All search and locate operations work through the database.
// * Connections to the database are alive as short as possible.


/*
 Storing notes, previously being done as separate files, each file took up a default space,
 has now become more efficient, that is, takes up less space,
 since now each note is stored as a separate JSON file.
 On a Linux server, one notes took 32 kbytes, and a lot of that space is wasted.
 Since now one notes is stored in one file, it takes only 4 kbytes.
 That is a difference of 8 times.
*/


Database_Notes::Database_Notes (void * webserver_request_in)
{
  webserver_request = webserver_request_in;
}


sqlite3 * Database_Notes::connect ()
{
  return database_sqlite_connect ("notes");
}


sqlite3 * Database_Notes::connect_checksums ()
{
  return database_sqlite_connect ("notes_checksums");
}


void Database_Notes::create_v12 ()
{
  // Create the main database and table.
  sqlite3 * db = connect ();
  string sql;
  sql = 
    "CREATE TABLE IF NOT EXISTS notes ("
    " id integer primary key autoincrement,"
    " identifier integer NOT NULL,"
    " modified integer NOT NULL,"
    " assigned text,"
    " subscriptions text,"
    " bible text,"
    " passage text,"
    " status text,"
    " severity integer,"
    " summary text,"
    " contents text,"
    " cleantext text"
    ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);

  // Create the database and table for the checksums.
  // A general reason for having this separate is robustness.
  // A specific reason for this is that when the main notes database is being repaired,
  // and several clients keep reading it, it may disrupt the repair.
  db = connect_checksums ();
  sql = 
    "CREATE TABLE IF NOT EXISTS checksums ("
    " identifier integer,"
    " checksum checksum"
    ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);

  // Enter the standard statuses in the list of translatable strings.
#ifdef NONE
  translate("New");
  translate("Pending");
  translate("In progress");
  translate("Done");
  translate("Reopened");
#endif
  
  // Enter the standard severities in the list of translatable strings.
#ifdef NONE
  translate("Wish");
  translate("Minor");
  translate("Normal");
  translate("Important");
  translate("Major");
  translate("Critical");
#endif
}


string Database_Notes::database_path_v12 ()
{
  return filter_url_create_root_path ("databases", "notes.sqlite");
}


string Database_Notes::checksums_database_path_v12 ()
{
  return filter_url_create_root_path ("databases", "notes_checksums.sqlite");
}


// Returns whether the notes database is healthy, as a boolean.
bool Database_Notes::healthy_v12 ()
{
  return database_sqlite_healthy (database_path_v12 ());
}


// Returns whether the notes checksums database is healthy, as a boolean.
bool Database_Notes::checksums_healthy_v12 ()
{
  return database_sqlite_healthy (checksums_database_path_v12 ());
}


// Does a checkup on the health of the main database.
// Optionally recreates it.
// Returns true if to be synced, else false.
bool Database_Notes::checkup_v12 ()
{
  if (healthy_v12 ()) return false;
  filter_url_unlink (database_path_v12 ());
  create_v12 ();
  return true;
}


// Does a checkup on the health of the checksums database.
// Optionally recreates it.
// Returns true if to synced, else false.
bool Database_Notes::checkup_checksums_v12 ()
{
  if (checksums_healthy_v12 ()) return false;
  filter_url_unlink (checksums_database_path_v12 ());
  create_v12 ();
  return true;
}


void Database_Notes::trim_v12 ()
{
  // Clean empty directories.
  string message = "Deleting empty notes folder ";
  string main_folder = main_folder_v12 ();
  vector <string> bits1 = filter_url_scandir (main_folder);
  for (auto bit1 : bits1) {
    if (bit1.length () == 3) {
      string folder = filter_url_create_path (main_folder, bit1);
      vector <string> bits2 = filter_url_scandir (folder);
      if (bits2.empty ()) {
        Database_Logs::log (message + folder);
        remove (folder.c_str ());
      }
      for (auto bit2 : bits2) {
        if (bit2.length () == 3) {
          string folder = filter_url_create_path (main_folder, bit1, bit2);
          vector <string> bits3 = filter_url_scandir (folder);
          if (bits3.empty ()) {
            Database_Logs::log (message + folder);
            remove (folder.c_str());
          }
        }
      }
    }
  }
}


void Database_Notes::trim_server_v12 ()
{
  // Notes expiry.
  touch_marked_for_deletion_v12 ();
  /// Storage for notes to be deleted.
  vector <int> identifiers;
  // Deal with old notes storage.
  identifiers = get_due_for_deletion_v1 ();
  for (auto & identifier : identifiers) {
    trash_consultation_note (webserver_request, identifier);
    erase_v12 (identifier);
  }
  // Deal with new notes storage in JSON.
  identifiers = get_due_for_deletion_v2 ();
  for (auto & identifier : identifiers) {
    trash_consultation_note (webserver_request, identifier);
    erase_v12 (identifier);
  }
}


void Database_Notes::optimize_v12 ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM notes;");
  database_sqlite_disconnect (db);
}


void Database_Notes::sync_v12 ()
{
  string main_folder = main_folder_v12 ();

  // List of notes in the filesystem.
  vector <int> identifiers;

  vector <string> bits1 = filter_url_scandir (main_folder);
  for (auto & bit1 : bits1) {
    // Bit 1 / 2 / 3 may start with a 0, so conversion to int cannot be used, rather use a length of 3.
    // It used conversion to int before to determine it was a real note,
    // with the result that it missed 10% of the notes, which subsequently got deleted, oops!
    if (bit1.length () == 3) {
      vector <string> bits2 = filter_url_scandir (filter_url_create_path (main_folder, bit1));
      for (auto & bit2 : bits2) {
        // Old storage mechanism, e.g. folder "425".
        if (bit2.length () == 3) {
          vector <string> bits3 = filter_url_scandir (filter_url_create_path (main_folder, bit1, bit2));
          for (auto & bit3 : bits3) {
            if (bit3.length () == 3) {
              int identifier = convert_to_int (bit1 + bit2 + bit3);
              identifiers.push_back (identifier);
              update_database_v1 (identifier);
              update_search_fields_v12 (identifier);
              update_checksum_v1 (identifier);
            }
          }
        }
        // New JSON storage mechanism, e.g. file "894093.json".
        if ((bit2.length () == 11) && bit2.find (".json") != string::npos) {
          int identifier = convert_to_int (bit1 + bit2.substr (0,6));
          identifiers.push_back (identifier);
          update_database_v2 (identifier);
          update_search_fields_v12 (identifier);
          update_checksum_v2 (identifier);
        }
      }
    }
  }

  // Get all identifiers in the main notes index.
  sqlite3 * db = connect ();
  vector <int> database_identifiers;
  vector <string> result = database_sqlite_query (db, "SELECT identifier FROM notes;") ["identifier"];
  for (auto & id : result) {
    database_identifiers.push_back (convert_to_int (id));
  }
  database_sqlite_disconnect (db);

  // Any note identifiers in the main index, and not in the filesystem, remove them.
  for (auto id : database_identifiers) {
    if (find (identifiers.begin(), identifiers.end(), id) == identifiers.end()) {
      trash_consultation_note (webserver_request, id);
      erase_v12 (id);
    }
  }
  
  // Get all identifiers in the checksums database.
  db = connect_checksums ();
  database_identifiers.clear ();
  result = database_sqlite_query (db, "SELECT identifier FROM checksums;") ["identifier"];
  for (auto & id : result) {
    database_identifiers.push_back (convert_to_int (id));
  }
  database_sqlite_disconnect (db);

  // Any note identifiers in the checksums database, and not in the filesystem, remove them.
  for (auto id : database_identifiers) {
    if (find (identifiers.begin(), identifiers.end(), id) == identifiers.end()) {
      delete_checksum_v12 (id);
    }
  }
  
}


void Database_Notes::update_database_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    update_database_v1 (identifier);
  } else {
    update_database_v2 (identifier);
  }
}


void Database_Notes::update_database_v1 (int identifier)
{
  // Read the relevant values from the filesystem.
  int modified = get_modified_v1 (identifier);
  
  string file = assigned_file_v1 (identifier);
  string assigned = filter_url_file_get_contents (file);
  
  file = subscriptions_file_v1 (identifier);
  string subscriptions = filter_url_file_get_contents (file);
  
  string bible = get_bible_v1 (identifier);
  
  string passage = get_raw_passage_v1 (identifier);
  
  string status = get_raw_status_v1 (identifier);
  
  int severity = get_raw_severity_v1 (identifier);
  
  string summary = get_summary_v1 (identifier);
  
  string contents = get_contents_v1 (identifier);

  // Sync the values to the database.
  update_database_internal (identifier, modified, assigned, subscriptions, bible, passage, status, severity, summary, contents);
}


void Database_Notes::update_database_v2 (int identifier)
{
  // Read the relevant values from the filesystem.
  int modified = get_modified_v2 (identifier);
  string assigned = get_field_v2 (identifier, assigned_key_v2 ());
  string subscriptions = get_field_v2 (identifier, subscriptions_key_v2 ());
  string bible = get_bible_v2 (identifier);
  string passage = get_raw_passage_v2 (identifier);
  string status = get_raw_status_v2 (identifier);
  int severity = get_raw_severity_v2 (identifier);
  string summary = get_summary_v2 (identifier);
  string contents = get_contents_v2 (identifier);
  
  // Sync the values to the database.
  update_database_internal (identifier, modified, assigned, subscriptions, bible, passage, status, severity, summary, contents);
}


void Database_Notes::update_database_internal (int identifier, int modified, string assigned, string subscriptions, string bible, string passage, string status, int severity, string summary, string contents)
{
  // Read the relevant values from the database.
  // If all the values in the database are the same as the values in the filesystem,
  // it means that the database is already in sync with the filesystem.
  // Bail out in that case.
  sqlite3 * db = connect ();
  bool database_in_sync = true;
  bool record_in_database = false;
  SqliteSQL sql;
  sql.add ("SELECT modified, assigned, subscriptions, bible, passage, status, severity, summary, contents FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> vmodified = result ["modified"];
  vector <string> vassigned = result ["assigned"];
  vector <string> vsubscriptions = result ["subscriptions"];
  vector <string> vbible = result ["bible"];
  vector <string> vpassage = result ["passage"];
  vector <string> vstatus = result ["status"];
  vector <string> vseverity = result ["severity"];
  vector <string> vsummary = result ["summary"];
  vector <string> vcontents = result ["contents"];
  for (unsigned int i = 0; i < vmodified.size(); i++) {
    record_in_database = true;
    if (modified != convert_to_int (vmodified[i])) database_in_sync = false;
    if (assigned != vassigned[i]) database_in_sync = false;
    if (subscriptions != vsubscriptions[i]) database_in_sync = false;
    if (bible != vbible [i]) database_in_sync = false;
    if (passage != vpassage [i]) database_in_sync = false;
    if (status != vstatus [i]) database_in_sync = false;
    if (severity != convert_to_int (vseverity [i])) database_in_sync = false;
    if (summary != vsummary [i]) database_in_sync = false;
    if (contents != vcontents [i]) database_in_sync = false;
  }
  if (database_in_sync && record_in_database) return;
  
  // At this stage, the index needs to be brought in sync with the filesystem.
  db = connect ();
  
  sql.clear ();
  sql.add ("DELETE FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  database_sqlite_exec (db, sql.sql);
  
  sql.clear ();
  sql.add ("INSERT INTO notes (identifier, modified, assigned, subscriptions, bible, passage, status, severity, summary, contents) VALUES (");
  sql.add (identifier);
  sql.add (",");
  sql.add (modified);
  sql.add (",");
  sql.add (assigned);
  sql.add (",");
  sql.add (subscriptions);
  sql.add (",");
  sql.add (bible);
  sql.add (",");
  sql.add (passage);
  sql.add (",");
  sql.add (status);
  sql.add (",");
  sql.add (severity);
  sql.add (",");
  sql.add (summary);
  sql.add (",");
  sql.add (contents);
  sql.add (")");
  database_sqlite_exec (db, sql.sql);
  
  database_sqlite_disconnect (db);
}


string Database_Notes::main_folder_v12 ()
{
  return filter_url_create_root_path ("consultations");
}


string Database_Notes::note_folder_v1 (int identifier)
{
  // The maximum number of folders a folder may contain is constrained by the filesystem.
  // To overcome this, the notes will be stored in a deep folder structure.
  string sidentifier = convert_to_string (identifier);
  string bit1 = sidentifier.substr (0, 3);
  string bit2 = sidentifier.substr (3, 3);
  string bit3 = sidentifier.substr (6, 3);
  return filter_url_create_path (main_folder_v12 (), bit1, bit2, bit3);
}


string Database_Notes::note_file_v2 (int identifier)
{
  // The maximum number of folders a folder may contain is constrained by the filesystem.
  // To overcome this, the notes will be stored in a folder structure.
  string sidentifier = convert_to_string (identifier);
  string folder = sidentifier.substr (0, 3);
  string file = sidentifier.substr (3, 6) + ".json";
  return filter_url_create_path (main_folder_v12 (), folder, file);
}


string Database_Notes::bible_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "bible");
}


string Database_Notes::passage_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "passage");
}


string Database_Notes::status_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "status");
}


string Database_Notes::severity_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "severity");
}


string Database_Notes::modified_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "modified");
}


string Database_Notes::summary_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "summary");
}


string Database_Notes::contents_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "contents");
}


string Database_Notes::subscriptions_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "subscriptions");
}


string Database_Notes::assigned_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "assigned");
}


string Database_Notes::public_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "public");
}


string Database_Notes::expiry_file_v1 (int identifier)
{
  return filter_url_create_path (note_folder_v1 (identifier), "expiry");
}


// This checks whether the note identifier exists.
// It works for the old way of storing notes in many files,
// and for the new way of storing notes in JSON.
bool Database_Notes::identifier_exists_v12 (int identifier)
{
  if (file_or_dir_exists (note_file_v2 (identifier))) return true;
  if (file_or_dir_exists (note_folder_v1 (identifier))) return true;
  return false;
}


// Update a note's identifier.
// new_identifier is the value given to the note identifier by identifier.
void Database_Notes::set_identifier_v12 (int identifier, int new_identifier)
{
  if (is_v1 (identifier)) {
    set_identifier_v1 (identifier, new_identifier);
  } else {
    set_identifier_v2 (identifier, new_identifier);
  }
}


void Database_Notes::set_identifier_v1 (int identifier, int new_identifier)
{
  // Move data on the filesystem.
  erase_v12 (new_identifier);
  string file = note_folder_v1 (identifier);
  string newfile = note_folder_v1 (new_identifier);
  filter_url_mkdir (filter_url_dirname (newfile));
  filter_url_rename (file, newfile);
  
  // Update main notes database.
  sqlite3 * db = connect ();
  SqliteSQL sql;
  sql.add ("UPDATE notes SET identifier =");
  sql.add (new_identifier);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
  // Update checksums database.
  db = connect_checksums ();
  sql.clear ();
  sql.add ("UPDATE checksums SET identifier =");
  sql.add (new_identifier);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // The range-based one also.
  Database_State::eraseNoteChecksum (identifier);
}


// Update a note's identifier.
// $new_identifier is the value given to the note $identifier.
void Database_Notes::set_identifier_v2 (int identifier, int new_identifier)
{
  // Move data on the filesystem.
  erase_v12 (new_identifier);
  string path = note_file_v2 (identifier);
  string json = filter_url_file_get_contents (path);
  path = note_file_v2 (new_identifier);
  string folder = filter_url_dirname (path);
  filter_url_mkdir (folder);
  filter_url_file_put_contents (path, json);
  
  // Update main notes database.
  sqlite3 * db = connect ();
  SqliteSQL sql;
  sql.add ("UPDATE notes SET identifier =");
  sql.add (new_identifier);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
  // Update checksums database.
  db = connect_checksums ();
  sql.clear ();
  sql.add ("UPDATE checksums SET identifier =");
  sql.add (new_identifier);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
  // Update the range-based checksum also.
  Database_State::eraseNoteChecksum (identifier);
  
  // Remove old identifier that was copied to the new.
  erase_v12 (identifier);
}


// Gets new unique note identifier.
// Works for the old and new storage system.
int Database_Notes::get_new_unique_identifier_v12 ()
{
  int identifier = 0;
  do {
    identifier = filter_string_rand (Notes_Logic::lowNoteIdentifier, Notes_Logic::highNoteIdentifier);
  } while (identifier_exists_v12 (identifier));
  return identifier;
}


vector <int> Database_Notes::get_identifiers_v12 ()
{
  sqlite3 * db = connect ();
  vector <int> identifiers;
  vector <string> result = database_sqlite_query (db, "SELECT identifier FROM notes;") ["identifier"];
  for (auto & id : result) {
    identifiers.push_back (convert_to_int (id));
  }
  database_sqlite_disconnect (db);
  return identifiers;
}


string Database_Notes::assemble_contents_v12 (int identifier, string contents)
{
  string new_contents;
  if (is_v1 (identifier)) new_contents = get_contents_v1 (identifier);
  else new_contents = get_contents_v2 (identifier);
  int time = filter_date_seconds_since_epoch ();
  string datetime = convert_to_string (filter_date_numerical_month_day (time)) + "/" + convert_to_string (filter_date_numerical_month (time)) + "/" + convert_to_string (filter_date_numerical_year (time));
  string user = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  
  new_contents.append ("\n");
  new_contents.append ("<p>");
  new_contents.append (user);
  new_contents.append (" (");
  new_contents.append (datetime);
  new_contents.append ("):</p>\n");
  if (contents == "<br>") contents.clear();
  vector <string> lines = filter_string_explode (contents, '\n');
  for (auto line : lines) {
    line = filter_string_trim (line);
    new_contents.append ("<p>");
    new_contents.append (line);
    new_contents.append ("</p>\n");
  }
  return new_contents;
}


// Store a new note into the database.
// bible: The notes's Bible.
// book, chapter, verse: The note's passage.
// summary: The note's summary.
// contents: The note's contents.
// raw: Import contents as it is.
// It returns the identifier of this new note.
int Database_Notes::store_new_note_v1 (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw)
{
  // Create a new identifier.
  int identifier = get_new_unique_identifier_v12 ();

  // Passage.
  string passage = encode_passage_v12 (book, chapter, verse);
  
  string status = "New";
  int severity = 2;

  // If the summary is not given, take the first line of the contents as the summary.
  if (summary == "") {
    // The notes editor does not put new lines at each line, but instead <div>s. Handle these also.
    summary = filter_string_str_replace ("<", "\n", contents);
    vector <string> bits = filter_string_explode (summary, '\n');
    if (!bits.empty ()) summary = bits [0];
  }

  // Assemble contents.
  if (!raw) contents = assemble_contents_v12 (identifier, contents);
  if ((contents.empty()) && (summary.empty())) return 0;
  
  // Store the note in the file system.
  filter_url_mkdir (note_folder_v1 (identifier));
  filter_url_file_put_contents (bible_file_v1 (identifier), bible);
  filter_url_file_put_contents (passage_file_v1 (identifier), passage);
  filter_url_file_put_contents (status_file_v1 (identifier), status);
  filter_url_file_put_contents (severity_file_v1 (identifier), convert_to_string (severity));
  filter_url_file_put_contents (summary_file_v1 (identifier), summary);
  set_raw_contents_v1 (identifier, contents);
  
  // Store new default note into the database.
  sqlite3 * db = connect ();
  SqliteSQL sql;
  sql.add ("INSERT INTO notes (identifier, modified, assigned, subscriptions, bible, passage, status, severity, summary, contents) VALUES (");
  sql.add (identifier);
  sql.add (", 0, '', '',");
  sql.add (bible);
  sql.add (",");
  sql.add (passage);
  sql.add (",");
  sql.add (status);
  sql.add (",");
  sql.add (severity);
  sql.add (",");
  sql.add (summary);
  sql.add (",");
  sql.add (contents);
  sql.add (")");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);

  // Updates.
  update_search_fields_v12 (identifier);
  note_modified_actions_v12 (identifier);

  // Return this new note´s identifier.
  return identifier;
}


// Store a new consultation note into the database and in JSON.
// bible: The notes's Bible.
// book, chapter, verse: The note's passage.
// summary: The note's summary.
// contents: The note's contents.
// raw: Import contents as it is.
// It returns the identifier of this new note.
int Database_Notes::store_new_note_v2 (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw)
{
  // Create a new identifier.
  int identifier = get_new_unique_identifier_v12 ();
  
  // Passage.
  string passage = encode_passage_v12 (book, chapter, verse);
  
  string status = "New";
  int severity = 2;
  
  // If the summary is not given, take the first line of the contents as the summary.
  if (summary == "") {
    // The notes editor does not put new lines at each line, but instead <div>s. Handle these also.
    summary = filter_string_str_replace ("<", "\n", contents);
    vector <string> bits = filter_string_explode (summary, '\n');
    if (!bits.empty ()) summary = bits [0];
  }
  
  // Assemble contents.
  if (!raw) contents = assemble_contents_v12 (identifier, contents);
  if ((contents.empty()) && (summary.empty())) return 0;
  
  // Store the JSON representation of the note in the file system.
  string path = note_file_v2 (identifier);
  string folder = filter_url_dirname (path);
  filter_url_mkdir (folder);
  Object note;
  note << bible_key_v2 () << bible;
  note << passage_key_v2 () << passage;
  note << status_key_v2 () << status;
  note << severity_key_v2 () << convert_to_string (severity);
  note << summary_key_v2 () << summary;
  note << contents_key_v2 () << contents;
  string json = note.json ();
  filter_url_file_put_contents (path, json);
  
  // Store new default note into the database.
  sqlite3 * db = connect ();
  SqliteSQL sql;
  sql.add ("INSERT INTO notes (identifier, modified, assigned, subscriptions, bible, passage, status, severity, summary, contents) VALUES (");
  sql.add (identifier);
  sql.add (", 0, '', '',");
  sql.add (bible);
  sql.add (",");
  sql.add (passage);
  sql.add (",");
  sql.add (status);
  sql.add (",");
  sql.add (severity);
  sql.add (",");
  sql.add (summary);
  sql.add (",");
  sql.add (contents);
  sql.add (")");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
  // Updates.
  update_search_fields_v12 (identifier);
  note_modified_actions_v12 (identifier);
  
  // Return this new note´s identifier.
  return identifier;
}


// Returns an array of note identifiers selected.
// bibles: Array of Bible names the user has read access to.
// book, chapter, verse, passage_selector: These are related and can limit the selection.
// edit_selector: Optionally constrains selection based on modification time.
// non_edit_selector: Optionally constrains selection based on modification time.
// status_selector: Optionally constrains selection based on note status.
// bible_selector: Optionally constrains the selection, based on the note's Bible.
// assignment_selector: Optionally constrains the selection based on a note being assigned to somebody.
// subscription_selector: Optionally limits the selection based on a note's subscription.
// severity_selector: Optionally limits the selection, based on a note's severity.
// text_selector: Optionally limits the selection to notes that contains certain text. Used for searching notes.
// search_text: Works with text_selector, contains the text to search for.
// limit: If >= 0, it indicates the starting limit for the selection.
vector <int> Database_Notes::select_notes_v12 (vector <string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const string& status_selector, string bible_selector, string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const string& search_text, int limit)
{
  string username = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  vector <int> identifiers;
  // SQL SELECT statement.
  string query = notes_select_identifier ();
  // SQL optional fulltext search statement sorted on relevance.
  if (text_selector == 1) {
    query.append (notes_optional_fulltext_search_relevance_statement (search_text));
  }
  // SQL FROM ... WHERE statement.
  query.append (notes_from_where_statement ());
  // Consider passage selector.
  string passage;
  switch (passage_selector) {
    case 0:
      // Select notes that refer to the current verse.
      // It means that the book, the chapter, and the verse, should match.
      passage = encode_passage_v12 (book, chapter, verse);
      query.append (" AND passage LIKE '%" + passage + "%' ");
      break;
    case 1:
      // Select notes that refer to the current chapter.
      // It means that the book and the chapter should match.
      passage = encode_passage_v12 (book, chapter, -1);
      query.append (" AND passage LIKE '%" + passage + "%' ");
      break;
    case 2:
      // Select notes that refer to the current book.
      // It means that the book should match.
      passage = encode_passage_v12 (book, -1, -1);
      query.append (" AND passage LIKE '%" + passage + "%' ");
      break;
    case 3:
      // Select notes that refer to any passage: No constraint to apply here.
      break;
  }
  // Consider edit selector.
  int time = 0;
  switch (edit_selector) {
    case 0:
      // Select notes that have been edited at any time. Apply no constraint.
      time = 0;
      break;
    case 1:
      // Select notes that have been edited during the last 30 days.
      time = filter_date_seconds_since_epoch () - 30 * 24 * 3600;
      break;
    case 2:
      // Select notes that have been edited during the last 7 days.
      time = filter_date_seconds_since_epoch () - 7 * 24 * 3600;
      break;
    case 3:
      // Select notes that have been edited since yesterday.
      time = filter_date_seconds_since_epoch () - 1 * 24 * 3600 - filter_date_numerical_hour (filter_date_seconds_since_epoch ()) * 3600;
      break;
    case 4:
      // Select notes that have been edited today.
      time = filter_date_seconds_since_epoch () - filter_date_numerical_hour (filter_date_seconds_since_epoch ()) * 3600;
      break;
  }
  if (time != 0) {
    query.append (" AND modified >= ");
    query.append (convert_to_string (time));
    query.append (" ");
  }
  // Consider non-edit selector.
  int nonedit = 0;
  switch (non_edit_selector) {
    case 0:
      // Select notes that have not been edited at any time. Apply no constraint.
      nonedit = 0;
      break;
    case 1:
      // Select notes that have not been edited for a day.
      nonedit = filter_date_seconds_since_epoch () - 1 * 24 * 3600;
      break;
    case 2:
      // Select notes that have not been edited for two days.
      nonedit = filter_date_seconds_since_epoch () - 2 * 24 * 3600;
      break;
    case 3:
      // Select notes that have not been edited for a week.
      nonedit = filter_date_seconds_since_epoch () - 7 * 24 * 3600;
      break;
    case 4:
      // Select notes that have not been edited for a month.
      nonedit = filter_date_seconds_since_epoch () - 30 * 24 * 3600;
      break;
    case 5:
      // Select notes that have not been edited for a year.
      nonedit = filter_date_seconds_since_epoch () - 365 * 24 * 3600;
      break;
  }
  if (nonedit != 0) {
    query.append (" AND modified <= ");
    query.append (convert_to_string (nonedit));
    query.append (" ");
  }
  // Consider status constraint.
  if (status_selector != "") {
    query.append (" AND status = '");
    query.append (database_sqlite_no_sql_injection (status_selector));
    query.append ("' ");
  }
  // Consider two different Bible constraints:
  // 1. The vector of bibles: "bibles".
  //    This contains all the Bibles a user has access to, so only notes that refer to any Bible in this lot are going to be selected.
  // 2. The string "bible_selector".
  //    If this is left empty, then it selects notes that refer to Bibles in the vector above.
  //    If this contains a Bible, then it selects notes that refer to this Bible.
  // In addition to the above two selectors, it always selects note that refer to any Bible.
  if (!bible_selector.empty()) {
    bibles.clear ();
    bibles.push_back (bible_selector);
  }
  if (!bibles.empty ()) {
    query.append (" AND (bible = '' ");
    for (auto bible : bibles) {
      bible = database_sqlite_no_sql_injection (bible);
      query.append (" OR bible = '");
      query.append (bible);
      query.append ("' ");
    }
    query.append (" ) ");
  }
  // Consider note assignment constraints.
  if (assignment_selector != "") {
    assignment_selector = database_sqlite_no_sql_injection (assignment_selector);
    query.append (" AND assigned LIKE '% ");
    query.append (assignment_selector);
    query.append (" %' ");
  }
  // Consider note subscription constraints.
  if (subscription_selector) {
    query.append (" AND subscriptions LIKE '% ");
    query.append (username);
    query.append (" %' ");
  }
  // Consider the note severity.
  if (severity_selector != -1) {
    query.append (" AND severity = ");
    query.append (convert_to_string (severity_selector));
    query.append (" ");
  }
  // Consider text contained in notes.
  if (text_selector == 1) {
    query.append (notes_optional_fulltext_search_statement (search_text));
  }
  if (text_selector == 1) {
    // If searching in fulltext mode, notes get ordered on relevance of search hits.
    query.append (notes_order_by_relevance_statement ());
  } else {
    // Notes get ordered by the passage they refer to. It is a rough method and better ordering is needed.
    query.append (" ORDER BY ABS (passage) ");
  }
  // Limit the selection if a limit is given.
  if (limit >= 0) {
    query.append (" LIMIT ");
    query.append (convert_to_string (limit));
    query.append (", 50 ");
  }
  query.append (";");

  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, query) ["identifier"];
  database_sqlite_disconnect (db);
  for (auto & id : result) {
    identifiers.push_back (convert_to_int (id));
  }
  return identifiers;
}


string Database_Notes::get_summary_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_summary_v1 (identifier);
  } else {
    return get_summary_v2 (identifier);
  }
}


string Database_Notes::get_summary_v1 (int identifier)
{
  string file = summary_file_v1 (identifier);
  return filter_url_file_get_contents (file);
}


string Database_Notes::get_summary_v2 (int identifier)
{
  return get_field_v2 (identifier, summary_key_v2 ());
}


void Database_Notes::set_summary_v12 (int identifier, const string& summary)
{
  if (is_v1 (identifier)) {
    set_summary_v1 (identifier, summary);
  } else {
    set_summary_v2 (identifier, summary);
  }
}


void Database_Notes::set_summary_v1 (int identifier, const string& summary)
{
  // Store authoritative copy in the filesystem.
  string file = summary_file_v1 (identifier);
  filter_url_file_put_contents (file, summary);
  // Update the shadow database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET summary =");
  sql.add (summary);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Update the search data in the database.
  update_search_fields_v12 (identifier);
  // Update checksum.
  update_checksum_v1 (identifier);
}


void Database_Notes::set_summary_v2 (int identifier, string summary)
{
  // Store authoritative copy in the filesystem.
  set_field_v2 (identifier, summary_key_v2 (), summary);
  // Update the shadow database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET summary =");
  sql.add (summary);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Update the search data in the database.
  update_search_fields_v12 (identifier);
  // Update checksum.
  update_checksum_v2 (identifier);
}


string Database_Notes::get_contents_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_contents_v1 (identifier);
  } else {
    return get_contents_v2 (identifier);
  }
}


string Database_Notes::get_contents_v1 (int identifier)
{
  string file = contents_file_v1 (identifier);
  return filter_url_file_get_contents (file);
}


string Database_Notes::get_contents_v2 (int identifier)
{
  return get_field_v2 (identifier, contents_key_v2 ());
}


void Database_Notes::set_raw_contents_v1 (int identifier, const string& contents)
{
  string file = contents_file_v1 (identifier);
  filter_url_file_put_contents (file, contents);
}


void Database_Notes::set_raw_contents_v2 (int identifier, const string& contents)
{
  set_field_v2 (identifier, contents_key_v2 (), contents);
}


void Database_Notes::set_contents_v12 (int identifier, const string& contents)
{
  if (is_v1 (identifier)) {
    set_contents_v1 (identifier, contents);
  } else {
    set_contents_v2 (identifier, contents);
  }
}


void Database_Notes::set_contents_v1 (int identifier, const string& contents)
{
  // Store in file system.
  set_raw_contents_v1 (identifier, contents);
  // Update database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET contents =");
  sql.add (contents);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Update search system.
  update_search_fields_v12 (identifier);
  // Update checksum.
  update_checksum_v1 (identifier);
}


void Database_Notes::set_contents_v2 (int identifier, const string& contents)
{
  // Store in file system.
  set_raw_contents_v2 (identifier, contents);
  // Update database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET contents =");
  sql.add (contents);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Update search system.
  update_search_fields_v12 (identifier);
  // Update checksum.
  update_checksum_v2 (identifier);
}


// Erases a note stored in the old and in the new format.
void Database_Notes::erase_v12 (int identifier)
{
  // Delete old storage from filesystem.
  string folder = note_folder_v1 (identifier);
  filter_url_rmdir (folder);
  // Delete new storage from filesystem.
  string path = note_file_v2 (identifier);
  filter_url_unlink (path);
  // Update databases as well.
  delete_checksum_v12 (identifier);
  SqliteSQL sql;
  sql.add ("DELETE FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Add a comment to an exiting note identified by identifier.
void Database_Notes::add_comment_v12 (int identifier, const string& comment)
{
  if (is_v1 (identifier)) {
    add_comment_v1 (identifier, comment);
  } else {
    add_comment_v2 (identifier, comment);
  }
}


// Add a comment to an exiting note identified by identifier.
void Database_Notes::add_comment_v1 (int identifier, const string& comment)
{
  // Assemble the new content and store it.
  // This updates the search database also.
  string contents = assemble_contents_v12 (identifier, comment);
  set_contents_v1 (identifier, contents);
  
  // Some triggers.
  note_modified_actions_v12 (identifier);
  unmark_for_deletion_v1 (identifier);
  
  // Update shadow database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET contents =");
  sql.add (contents);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Add a comment to an exiting note identified by identifier.
void Database_Notes::add_comment_v2 (int identifier, const string& comment)
{
  // Assemble the new content and store it.
  // This updates the search database also.
  string contents = assemble_contents_v12 (identifier, comment);
  set_contents_v2 (identifier, contents);
  
  // Some triggers.
  note_modified_actions_v12 (identifier);
  unmark_for_deletion_v2 (identifier);
  
  // Update shadow database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET contents =");
  sql.add (contents);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Subscribe the current user to the note identified by identifier.
void Database_Notes::subscribe_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    subscribe_v1 (identifier);
  } else {
    subscribe_v2 (identifier);
  }
}


// Subscribe the current user to the note identified by identifier.
void Database_Notes::subscribe_v1 (int identifier)
{
  string user = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  subscribe_user_v1 (identifier, user);
}


// Subscribe the current user to the note identified by identifier.
void Database_Notes::subscribe_v2 (int identifier)
{
  string user = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  subscribe_user_v2 (identifier, user);
}


// Subscribe the user to the note identified by identifier.
void Database_Notes::subscribe_user_v12 (int identifier, const string& user)
{
  if (is_v1 (identifier)) {
    subscribe_user_v1 (identifier, user);
  } else {
    subscribe_user_v2 (identifier, user);
  }
}


// Subscribe the user to the note identified by identifier.
void Database_Notes::subscribe_user_v1 (int identifier, const string& user)
{
  // If the user already is subscribed to the note, bail out.
  vector <string> subscribers = get_subscribers_v1 (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) != subscribers.end()) return;
  // Subscribe user.
  subscribers.push_back (user);
  set_subscribers_v1 (identifier, subscribers);
}


// Subscribe the user to the note identified by identifier.
void Database_Notes::subscribe_user_v2 (int identifier, const string& user)
{
  // If the user already is subscribed to the note, bail out.
  vector <string> subscribers = get_subscribers_v2 (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) != subscribers.end()) return;
  // Subscribe user.
  subscribers.push_back (user);
  set_subscribers_v2 (identifier, subscribers);
}


// Returns an array with the subscribers to the note identified by identifier.
vector <string> Database_Notes::get_subscribers_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_subscribers_v1 (identifier);
  } else {
    return get_subscribers_v2 (identifier);
  }
}


// Returns an array with the subscribers to the note identified by identifier.
vector <string> Database_Notes::get_subscribers_v1 (int identifier)
{
  string file = subscriptions_file_v1 (identifier);
  string contents = filter_url_file_get_contents (file);
  if (contents.empty()) return {};
  vector <string> subscribers = filter_string_explode (contents, '\n');
  for (auto & subscriber : subscribers) {
    subscriber = filter_string_trim (subscriber);
  }
  return subscribers;
}


// Returns an array with the subscribers to the note identified by identifier.
vector <string> Database_Notes::get_subscribers_v2 (int identifier)
{
  string contents = get_raw_subscriptions_v2 (identifier);
  if (contents.empty()) return {};
  vector <string> subscribers = filter_string_explode (contents, '\n');
  for (auto & subscriber : subscribers) {
    subscriber = filter_string_trim (subscriber);
  }
  return subscribers;
}


string Database_Notes::get_raw_subscriptions_v2 (int identifier)
{
  return get_field_v2 (identifier, subscriptions_key_v2 ());
}


void Database_Notes::set_raw_subscriptions_v2 (int identifier, const string& subscriptions)
{
  // Store them in the filesystem.
  set_field_v2 (identifier, subscriptions_key_v2 (), subscriptions);
  
  // Store them in the database as well.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET subscriptions =");
  sql.add (subscriptions);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


void Database_Notes::set_subscribers_v12 (int identifier, vector <string> subscribers)
{
  if (is_v1 (identifier)) {
    set_subscribers_v1 (identifier, subscribers);
  } else {
    set_subscribers_v2 (identifier, subscribers);
  }
}

void Database_Notes::set_subscribers_v1 (int identifier, vector <string> subscribers)
{
  // Add a space at both sides of the subscriber to allow for easier note selection based on note assignment.
  for (auto & subscriber : subscribers) {
    subscriber.insert (0, " ");
    subscriber.append (" ");
  }
  string subscriberstring = filter_string_implode (subscribers, "\n");
  
  // Store them in the filesystem, and remove the file if there's no data to store.
  string file = subscriptions_file_v1 (identifier);
  if (subscriberstring.empty ()) filter_url_unlink (file);
  else filter_url_file_put_contents (file, subscriberstring);
  
  // Store them in the database as well.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET subscriptions =");
  sql.add (subscriberstring);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);

  // Checksum.
  update_checksum_v1 (identifier);
}


void Database_Notes::set_subscribers_v2 (int identifier, vector <string> subscribers)
{
  // Add a space at both sides of the subscriber to allow for easier note selection based on note assignment.
  for (auto & subscriber : subscribers) {
    subscriber.insert (0, " ");
    subscriber.append (" ");
  }
  string subscriberstring = filter_string_implode (subscribers, "\n");
  
  // Store them to file and in the database.
  set_raw_subscriptions_v2 (identifier, subscriberstring);
  
  // Checksum.
  update_checksum_v2 (identifier);
}


// Returns true if user is subscribed to the note identified by identifier.
bool Database_Notes::is_subscribed_v12 (int identifier, const string& user)
{
  if (is_v1 (identifier)) {
    return is_subscribed_v1 (identifier, user);
  } else {
    return is_subscribed_v2 (identifier, user);
  }
}


// Returns true if user is subscribed to the note identified by identifier.
bool Database_Notes::is_subscribed_v1 (int identifier, const string& user)
{
  vector <string> subscribers = get_subscribers_v1 (identifier);
  return find (subscribers.begin(), subscribers.end(), user) != subscribers.end();
}


// Returns true if user is subscribed to the note identified by identifier.
bool Database_Notes::is_subscribed_v2 (int identifier, const string& user)
{
  vector <string> subscribers = get_subscribers_v2 (identifier);
  return find (subscribers.begin(), subscribers.end(), user) != subscribers.end();
}


// Unsubscribes the currently logged in user from the note identified by identifier.
void Database_Notes::unsubscribe_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    unsubscribe_v1 (identifier);
  } else {
    unsubscribe_v2 (identifier);
  }
}


// Unsubscribes the currently logged in user from the note identified by identifier.
void Database_Notes::unsubscribe_v1 (int identifier)
{
  string user = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  unsubscribe_user_v1 (identifier, user);
}


// Unsubscribes the currently logged in user from the note identified by identifier.
void Database_Notes::unsubscribe_v2 (int identifier)
{
  string user = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  unsubscribe_user_v2 (identifier, user);
}


// Unsubscribes user from the note identified by identifier.
void Database_Notes::unsubscribe_user_v12 (int identifier, const string& user)
{
  if (is_v1 (identifier)) {
    unsubscribe_user_v1 (identifier, user);
  } else {
    unsubscribe_user_v2 (identifier, user);
  }
}


// Unsubscribes user from the note identified by identifier.
void Database_Notes::unsubscribe_user_v1 (int identifier, const string& user)
{
  // If the user is not subscribed to the note, bail out.
  vector <string> subscribers = get_subscribers_v1 (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) == subscribers.end()) return;
  // Unsubscribe user.
  subscribers.erase (remove (subscribers.begin(), subscribers.end(), user), subscribers.end());
  set_subscribers_v1 (identifier, subscribers);
}


// Unsubscribes user from the note identified by identifier.
void Database_Notes::unsubscribe_user_v2 (int identifier, const string& user)
{
  // If the user is not subscribed to the note, bail out.
  vector <string> subscribers = get_subscribers_v2 (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) == subscribers.end()) return;
  // Unsubscribe user.
  subscribers.erase (remove (subscribers.begin(), subscribers.end(), user), subscribers.end());
  set_subscribers_v2 (identifier, subscribers);
}


string Database_Notes::get_raw_assigned_v2 (int identifier)
{
  // Get the asssignees from the filesystem.
  return get_field_v2 (identifier, assigned_key_v2 ());
}


void Database_Notes::set_raw_assigned_v2 (int identifier, const string& assigned)
{
  // Store the assignees in the filesystem.
  set_field_v2 (identifier, assigned_key_v2 (), assigned);
  
  // Store the assignees in the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET assigned =");
  sql.add (assigned);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Returns an array with all assignees to the notes selection.
// These are the usernames to which one or more notes have been assigned.
// This means that if no notes have been assigned to anybody, it will return an empty array.
// Normally the authoritative copy of the notes is stored in the file system.
// But as retrieving the assignees from the file system would be slow, 
// this function retrieves them from the database.
// Normally the database is in sync with the filesystem.
vector <string> Database_Notes::get_all_assignees_v12 (const vector <string>& bibles)
{
  set <string> unique_assignees;
  SqliteSQL sql;
  sql.add ("SELECT DISTINCT assigned FROM notes WHERE bible = ''");
  for (auto & bible : bibles) {
    sql.add ("OR bible =");
    sql.add (bible);
  }
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["assigned"];
  for (auto & item : result) {
    if (item.empty ()) continue;
    vector <string> names = filter_string_explode (item, '\n');
    for (auto & name : names) unique_assignees.insert (name);
  }
  database_sqlite_disconnect (db);
  
  vector <string> assignees (unique_assignees.begin(), unique_assignees.end());
  for (auto & assignee : assignees) {
    assignee = filter_string_trim (assignee);
  }
  return assignees;
}


// Returns an array with the assignees to the note identified by identifier.
vector <string> Database_Notes::get_assignees_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_assignees_v1 (identifier);
  } else {
    return get_assignees_v2 (identifier);
  }
}

// Returns an array with the assignees to the note identified by identifier.
vector <string> Database_Notes::get_assignees_v1 (int identifier)
{
  // Get the asssignees from the filesystem.
  string file = assigned_file_v1 (identifier);
  string assignees = filter_url_file_get_contents (file);
  return get_assignees_internal_v12 (assignees);
}


// Returns an array with the assignees to the note identified by identifier.
vector <string> Database_Notes::get_assignees_v2 (int identifier)
{
  // Get the asssignees from the filesystem.
  string assignees = get_raw_assigned_v2 (identifier);
  return get_assignees_internal_v12 (assignees);
}


vector <string> Database_Notes::get_assignees_internal_v12 (string assignees)
{
  if (assignees.empty ()) return {};
  vector <string> assignees_vector = filter_string_explode (assignees, '\n');
  // Remove the padding space at both sides of the assignee.
  for (auto & assignee : assignees_vector) {
    assignee = filter_string_trim (assignee);
  }
  return assignees_vector;
}


// Sets the note's assignees.
// identifier : note identifier.
// assignees : array of user names.
void Database_Notes::set_assignees_v12 (int identifier, vector <string> assignees)
{
  if (is_v1 (identifier)) {
    set_assignees_v1 (identifier, assignees);
  } else {
    set_assignees_v2 (identifier, assignees);
  }
}


// Sets the note's assignees.
// identifier : note identifier.
// assignees : array of user names.
void Database_Notes::set_assignees_v1 (int identifier, vector <string> assignees)
{
  // Add a space at both sides of the assignee to allow for easier note selection based on note assignment.
  for (auto & assignee : assignees) {
    assignee.insert (0, " ");
    assignee.append (" ");
  }
  string assignees_string = filter_string_implode (assignees, "\n");
  
  // Store the assignees in the filesystem, or remove the file if there's no data to store.
  string file = assigned_file_v1 (identifier);
  if (assignees_string.empty ()) filter_url_unlink (file);
  else filter_url_file_put_contents (file, assignees_string);
  
  // Store the assignees in the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET assigned =");
  sql.add (assignees_string);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
  note_modified_actions_v12 (identifier);
}


// Sets the note's assignees.
// identifier : note identifier.
// assignees : array of user names.
void Database_Notes::set_assignees_v2 (int identifier, vector <string> assignees)
{
  // Add a space at both sides of the assignee to allow for easier note selection based on note assignment.
  for (auto & assignee : assignees) {
    assignee.insert (0, " ");
    assignee.append (" ");
  }
  string assignees_string = filter_string_implode (assignees, "\n");
  set_raw_assigned_v2 (identifier, assignees_string);
  note_modified_actions_v12 (identifier);
}


// Assign the note identified by identifier to user.
void Database_Notes::assign_user_v12 (int identifier, const string& user)
{
  if (is_v1 (identifier)) {
    assign_user_v1 (identifier, user);
  } else {
    assign_user_v2 (identifier, user);
  }
}


// Assign the note identified by identifier to user.
void Database_Notes::assign_user_v1 (int identifier, const string& user)
{
  // If the note already is assigned to the user, bail out.
  vector <string> assignees = get_assignees_v1 (identifier);
  if (find (assignees.begin (), assignees.end(), user) != assignees.end()) return;
  // Assign the note to the user.
  assignees.push_back (user);
  // Store the whole lot.
  set_assignees_v1 (identifier, assignees);
}


// Assign the note identified by identifier to user.
void Database_Notes::assign_user_v2 (int identifier, const string& user)
{
  // If the note already is assigned to the user, bail out.
  vector <string> assignees = get_assignees_v2 (identifier);
  if (find (assignees.begin (), assignees.end(), user) != assignees.end()) return;
  // Assign the note to the user.
  assignees.push_back (user);
  // Store the whole lot.
  set_assignees_v2 (identifier, assignees);
}


// Returns true if the note identified by identifier has been assigned to user.
bool Database_Notes::is_assigned_v12 (int identifier, const string& user)
{
  if (is_v1 (identifier)) {
    return is_assigned_v1 (identifier, user);
  } else {
    return is_assigned_v2 (identifier, user);
  }
}


// Returns true if the note identified by identifier has been assigned to user.
bool Database_Notes::is_assigned_v1 (int identifier, const string& user)
{
  vector <string> assignees = get_assignees_v1 (identifier);
  return find (assignees.begin(), assignees.end(), user) != assignees.end();
}


// Returns true if the note identified by identifier has been assigned to user.
bool Database_Notes::is_assigned_v2 (int identifier, const string& user)
{
  vector <string> assignees = get_assignees_v2 (identifier);
  return find (assignees.begin(), assignees.end(), user) != assignees.end();
}


// Unassigns user from the note identified by identifier.
void Database_Notes::unassign_user_v12 (int identifier, const string& user)
{
  if (is_v1 (identifier)) {
    unassign_user_v1 (identifier, user);
  } else {
    unassign_user_v2 (identifier, user);
  }
}


// Unassigns user from the note identified by identifier.
void Database_Notes::unassign_user_v1 (int identifier, const string& user)
{
  // If the note is not assigned to the user, bail out.
  vector <string> assignees = get_assignees_v1 (identifier);
  if (find (assignees.begin(), assignees.end(), user) == assignees.end()) return;
  // Remove assigned user.
  assignees.erase (remove (assignees.begin(), assignees.end(), user), assignees.end());
  set_assignees_v1 (identifier, assignees);
}


// Unassigns user from the note identified by identifier.
void Database_Notes::unassign_user_v2 (int identifier, const string& user)
{
  // If the note is not assigned to the user, bail out.
  vector <string> assignees = get_assignees_v2 (identifier);
  if (find (assignees.begin(), assignees.end(), user) == assignees.end()) return;
  // Remove assigned user.
  assignees.erase (remove (assignees.begin(), assignees.end(), user), assignees.end());
  set_assignees_v2 (identifier, assignees);
}


string Database_Notes::get_bible_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_bible_v1 (identifier);
  } else {
    return get_bible_v2 (identifier);
  }
}


string Database_Notes::get_bible_v1 (int identifier)
{
  string file = bible_file_v1 (identifier);
  return filter_url_file_get_contents (file);
}


string Database_Notes::get_bible_v2 (int identifier)
{
  return get_field_v2 (identifier, bible_key_v2 ());
}


void Database_Notes::set_bible_v12 (int identifier, const string& bible)
{
  if (is_v1 (identifier)) {
    set_bible_v1 (identifier, bible);
  } else {
    set_bible_v2 (identifier, bible);
  }
}


void Database_Notes::set_bible_v1 (int identifier, const string& bible)
{
  // Write the bible to the filesystem, or remove the bible in case there's no data to store.
  string file = bible_file_v1 (identifier);
  if (bible.empty ()) filter_url_unlink (file);
  else filter_url_file_put_contents (file, bible);
  
  // Update the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET bible =");
  sql.add (bible);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
  note_modified_actions_v12 (identifier);
}


void Database_Notes::set_bible_v2 (int identifier, const string& bible)
{
  // Write the bible to the filesystem.
  set_field_v2 (identifier, bible_key_v2 (), bible);
  
  // Update the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET bible =");
  sql.add (bible);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
  note_modified_actions_v12 (identifier);
}


vector <string> Database_Notes::get_all_bibles_v12 ()
{
  vector <string> bibles;
  sqlite3 * db = connect ();
  vector <int> identifiers;
  vector <string> result = database_sqlite_query (db, "SELECT DISTINCT bible FROM notes;") ["bible"];
  for (auto & bible : result) {
    if (bible.empty ()) continue;
    bibles.push_back (bible);
  }
  database_sqlite_disconnect (db);
  return bibles;
}


// Encodes the book, chapter and verse, like to, e.g.: "40.5.13",
// and returns this as a string.
// The chapter and the verse can be negative, in which case they won't be included.
string Database_Notes::encode_passage_v12 (int book, int chapter, int verse)
{
  // Space before and after the passage enables notes selection on passage.
  // Special way of encoding, as done below, is to enable note selection on book / chapter / verse.
  string passage;
  passage.append (" ");
  passage.append (convert_to_string (book));
  passage.append (".");
  // Whether to include the chapter number.
  if (chapter >= 0) {
    passage.append (convert_to_string (chapter));
    passage.append (".");
    // Inclusion of verse, also depends on chapter inclusion.
    if (verse >= 0) {
      passage.append (convert_to_string (verse));
      passage.append (" ");
    }
  }
  return passage;
}


// Takes the passage as a string, and returns an object with book, chapter, and verse.
Passage Database_Notes::decode_passage_v12 (string passage)
{
  passage = filter_string_trim (passage);
  Passage decodedpassage = Passage ();
  vector <string> lines = filter_string_explode (passage, '.');
  if (lines.size() > 0) decodedpassage.book = convert_to_int (lines[0]);
  if (lines.size() > 1) decodedpassage.chapter = convert_to_int (lines[1]);
  if (lines.size() > 2) decodedpassage.verse = lines[2];
  return decodedpassage;
}


// Returns the raw passage text of the note identified by identifier.
string Database_Notes::get_raw_passage_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_raw_passage_v1 (identifier);
  } else {
    return get_raw_passage_v2 (identifier);
  }
}


// Returns the raw passage text of the note identified by identifier.
string Database_Notes::get_raw_passage_v1 (int identifier)
{
  string file = passage_file_v1 (identifier);
  return filter_url_file_get_contents (file);
}


// Returns the raw passage text of the note identified by identifier.
string Database_Notes::get_raw_passage_v2 (int identifier)
{
  return get_field_v2 (identifier, passage_key_v2 ());
}


// Returns an array with the passages that the note identified by identifier refers to.
// Each passages is an array (book, chapter, verse).
vector <Passage> Database_Notes::get_passages_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_passages_v1 (identifier);
  } else {
    return get_passages_v2 (identifier);
  }
}


// Returns an array with the passages that the note identified by identifier refers to.
// Each passages is an array (book, chapter, verse).
vector <Passage> Database_Notes::get_passages_v1 (int identifier)
{
  string contents = get_raw_passage_v1 (identifier);
  if (contents.empty()) return {};
  vector <string> lines = filter_string_explode (contents, '\n');
  vector <Passage> passages;
  for (auto & line : lines) {
    if (line.empty()) continue;
    Passage passage = decode_passage_v12 (line);
    passages.push_back (passage);
  }
  return passages;
}


// Returns an array with the passages that the note identified by identifier refers to.
// Each passages is an array (book, chapter, verse).
vector <Passage> Database_Notes::get_passages_v2 (int identifier)
{
  string contents = get_raw_passage_v2 (identifier);
  if (contents.empty()) return {};
  vector <string> lines = filter_string_explode (contents, '\n');
  vector <Passage> passages;
  for (auto & line : lines) {
    if (line.empty()) continue;
    Passage passage = decode_passage_v12 (line);
    passages.push_back (passage);
  }
  return passages;
}


// Set the passages for note identifier.
// passages is an array of an array (book, chapter, verse) passages.
// import: If true, just write passages, no further actions.
void Database_Notes::set_passages_v12 (int identifier, const vector <Passage>& passages, bool import)
{
  if (is_v1 (identifier)) {
    set_passages_v1 (identifier, passages, import);
  } else {
    set_passages_v2 (identifier, passages, import);
  }
}


// Set the passages for note identifier.
// passages is an array of an array (book, chapter, verse) passages.
// import: If true, just write passages, no further actions.
void Database_Notes::set_passages_v1 (int identifier, const vector <Passage>& passages, bool import)
{
  // Format the passages.
  string line;
  for (auto & passage : passages) {
    if (!line.empty ()) line.append ("\n");
    line.append (encode_passage_v12 (passage.book, passage.chapter, convert_to_int (passage.verse)));
  }
  // Store it.
  set_raw_passage_v1 (identifier, line);

  // Update index.
  index_raw_passage_v12 (identifier, line);

  if (!import) note_modified_actions_v12 (identifier);
}


// Set the passages for note identifier.
// passages is an array of an array (book, chapter, verse) passages.
// import: If true, just write passages, no further actions.
void Database_Notes::set_passages_v2 (int identifier, const vector <Passage>& passages, bool import)
{
  // Format the passages.
  string line;
  for (auto & passage : passages) {
    if (!line.empty ()) line.append ("\n");
    line.append (encode_passage_v12 (passage.book, passage.chapter, convert_to_int (passage.verse)));
  }
  // Store it.
  set_raw_passage_v2 (identifier, line);
  
  // Update index.
  index_raw_passage_v12 (identifier, line);

  if (!import) note_modified_actions_v12 (identifier);
}


// Sets the raw $passage(s) for a note $identifier.
// The reason for having this function is this:
// There is a slight difference in adding a new line or not to the passage
// between Bibledit as it was written in PHP,
// and Bibledit as it is now written in C++.
// Due to this difference, when a client downloads a note from the server,
// it should download the exact passage file contents as it is on the server,
// so as to prevent keeping to download the same notes over and over,
// due to the above mentioned difference in adding a new line or not.
void Database_Notes::set_raw_passage_v12 (int identifier, const string& passage)
{
  if (is_v1 (identifier)) {
    set_raw_passage_v1 (identifier, passage);
  } else {
    set_raw_passage_v2 (identifier, passage);
  }
}


// Sets the raw $passage(s) for a note $identifier.
// The reason for having this function is this:
// There is a slight difference in adding a new line or not to the passage
// between Bibledit as it was written in PHP,
// and Bibledit as it is now written in C++.
// Due to this difference, when a client downloads a note from the server,
// it should download the exact passage file contents as it is on the server,
// so as to prevent keeping to download the same notes over and over,
// due to the above mentioned difference in adding a new line or not.
void Database_Notes::set_raw_passage_v1 (int identifier, const string& passage)
{
  // Store the authoritative copy in the filesystem.
  string file = passage_file_v1 (identifier);
  filter_url_file_put_contents (file, passage);
}


// Sets the raw $passage(s) for a note $identifier.
// The reason for having this function is this:
// There is a slight difference in adding a new line or not to the passage
// between Bibledit as it was written in PHP,
// and Bibledit as it is now written in C++.
// Due to this difference, when a client downloads a note from the server,
// it should download the exact passage file contents as it is on the server,
// so as to prevent keeping to download the same notes over and over,
// due to the above mentioned difference in adding a new line or not.
void Database_Notes::set_raw_passage_v2 (int identifier, const string& passage)
{
  // Store the authoritative copy in the filesystem.
  set_field_v2 (identifier, passage_key_v2 (), passage);
}


void Database_Notes::index_raw_passage_v12 (int identifier, const string& passage)
{
  // Update the search index database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET passage =");
  sql.add (passage);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  
}


// Gets the raw status of a note.
// Returns it as a string.
string Database_Notes::get_raw_status_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_raw_status_v1 (identifier);
  } else {
    return get_raw_status_v2 (identifier);
  }
}


// Gets the raw status of a note.
// Returns it as a string.
string Database_Notes::get_raw_status_v1 (int identifier)
{
  string file = status_file_v1 (identifier);
  return filter_url_file_get_contents (file);
}


// Gets the raw status of a note.
// Returns it as a string.
string Database_Notes::get_raw_status_v2 (int identifier)
{
  return get_field_v2 (identifier, status_key_v2 ());
}


// Gets the localized status of a note.
// Returns it as a string.
string Database_Notes::get_status_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_status_v1 (identifier);
  } else {
    return get_status_v2 (identifier);
  }
}


// Gets the localized status of a note.
// Returns it as a string.
string Database_Notes::get_status_v1 (int identifier)
{
  string status = get_raw_status_v1 (identifier);
  // Localize status if possible.
  status = translate (status.c_str());
  // Return status.
  return status;
}


// Gets the localized status of a note.
// Returns it as a string.
string Database_Notes::get_status_v2 (int identifier)
{
  string status = get_raw_status_v2 (identifier);
  // Localize status if possible.
  status = translate (status.c_str());
  // Return status.
  return status;
}


// Sets the status of the note identified by identifier.
// status is a string.
// import: Just write the status, and skip any logic.
void Database_Notes::set_status_v12 (int identifier, const string& status, bool import)
{
  if (is_v1 (identifier)) {
    set_status_v1 (identifier, status, import);
  } else {
    set_status_v2 (identifier, status, import);
  }
}


// Sets the status of the note identified by identifier.
// status is a string.
// import: Just write the status, and skip any logic.
void Database_Notes::set_status_v1 (int identifier, const string& status, bool import)
{
  // Store the authoritative copy in the filesystem.
  string file = status_file_v1 (identifier);
  filter_url_file_put_contents (file, status);
  
  if (!import) note_modified_actions_v12(identifier);
  
  // Store a copy in the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET status =");
  sql.add (status);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Sets the status of the note identified by identifier.
// status is a string.
// import: Just write the status, and skip any logic.
void Database_Notes::set_status_v2 (int identifier, const string& status, bool import)
{
  // Store the authoritative copy in the filesystem.
  set_field_v2 (identifier, status_key_v2 (), status);
  
  if (!import) note_modified_actions_v12 (identifier);
  
  // Store a copy in the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET status =");
  sql.add (status);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Gets an array of array with the possible statuses of consultation notes,
// both raw and localized versions.
vector <Database_Notes_Text> Database_Notes::get_possible_statuses_v12 ()
{
  // Get an array with the statuses used in the database, ordered by occurrence, most often used ones first.
  string query = "SELECT status, COUNT(status) AS occurrences FROM notes GROUP BY status ORDER BY occurrences DESC;";
  sqlite3 * db = connect ();
  vector <string> statuses = database_sqlite_query (db, query) ["status"];
  database_sqlite_disconnect (db);
  // Ensure the standard statuses are there too.
  vector <string> standard_statuses = {"New", "Pending", "In progress", "Done", "Reopened"};
  for (auto & standard_status : standard_statuses) {
    if (find (statuses.begin(), statuses.end(), standard_status) == statuses.end()) {
      statuses.push_back (standard_status);
    }
  }
  // Localize the results.
  vector <Database_Notes_Text> localized_statuses;
  for (auto & status : statuses) {
    string localization = translate (status.c_str());
    Database_Notes_Text localized_status;
    localized_status.raw = status;
    localized_status.localized = localization;
    localized_statuses.push_back (localized_status);
  }
  // Return result.
  return localized_statuses;
}


vector <string> Database_Notes::standard_severities_v12 ()
{
  return {"Wish", "Minor", "Normal", "Important", "Major", "Critical"};
}


// Returns the severity of a note as a number.
int Database_Notes::get_raw_severity_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_raw_severity_v1 (identifier);
  } else {
    return get_raw_severity_v2 (identifier);
  }
}


// Returns the severity of a note as a number.
int Database_Notes::get_raw_severity_v1 (int identifier)
{
  string file = severity_file_v1 (identifier);
  string severity = filter_url_file_get_contents (file);
  if (severity.empty ()) return 2;
  return convert_to_int (severity);
}


// Returns the severity of a note as a number.
int Database_Notes::get_raw_severity_v2 (int identifier)
{
  string severity = get_field_v2 (identifier, severity_key_v2 ());
  if (severity.empty ()) return 2;
  return convert_to_int (severity);
}


// Returns the severity of a note as a localized string.
string Database_Notes::get_severity_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_severity_v1 (identifier);
  } else {
    return get_severity_v2 (identifier);
  }
}


// Returns the severity of a note as a localized string.
string Database_Notes::get_severity_v1 (int identifier)
{
  int severity = get_raw_severity_v1 (identifier);
  vector <string> standard = standard_severities_v12 ();
  string severitystring;
  if ((severity >= 0) && (severity < (int)standard.size())) severitystring = standard [severity];
  if (severitystring.empty()) severitystring = "Normal";
  severitystring = translate (severitystring.c_str());
  return severitystring;
}


// Returns the severity of a note as a localized string.
string Database_Notes::get_severity_v2 (int identifier)
{
  int severity = get_raw_severity_v2 (identifier);
  vector <string> standard = standard_severities_v12 ();
  string severitystring;
  if ((severity >= 0) && (severity < (int)standard.size())) severitystring = standard [severity];
  if (severitystring.empty()) severitystring = "Normal";
  severitystring = translate (severitystring.c_str());
  return severitystring;
}


// Sets the severity of the note identified by identifier.
// severity is a number.
void Database_Notes::set_raw_severity_v12 (int identifier, int severity)
{
  if (is_v1 (identifier)) {
    set_raw_severity_v1 (identifier, severity);
  } else {
    set_raw_severity_v2 (identifier, severity);
  }
}


// Sets the severity of the note identified by identifier.
// severity is a number.
void Database_Notes::set_raw_severity_v1 (int identifier, int severity)
{
  // Update the file system.
  string file = severity_file_v1 (identifier);
  filter_url_file_put_contents (file, convert_to_string (severity));
  
  note_modified_actions_v12 (identifier);
  
  // Update the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET severity =");
  sql.add (severity);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Sets the severity of the note identified by identifier.
// severity is a number.
void Database_Notes::set_raw_severity_v2 (int identifier, int severity)
{
  // Update the file system.
  set_field_v2 (identifier, severity_key_v2 (), convert_to_string (severity));
  
  note_modified_actions_v12 (identifier);
  
  // Update the database also.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET severity =");
  sql.add (severity);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Gets an array with the possible severities.
vector <Database_Notes_Text> Database_Notes::get_possible_severities_v12 ()
{
  vector <string> standard = standard_severities_v12 ();
  vector <Database_Notes_Text> severities;
  for (size_t i = 0; i < standard.size(); i++) {
    Database_Notes_Text severity;
    severity.raw = convert_to_string (i);
    severity.localized = translate (standard[i].c_str());
    severities.push_back (severity);
  }
  return severities;
}


int Database_Notes::get_modified_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_modified_v1 (identifier);
  } else {
    return get_modified_v2 (identifier);
  }
}


int Database_Notes::get_modified_v1 (int identifier)
{
  string file = modified_file_v1 (identifier);
  string modified = filter_url_file_get_contents (file);
  if (modified.empty ()) return 0;
  return convert_to_int (modified);
}


int Database_Notes::get_modified_v2 (int identifier)
{
  string modified = get_field_v2 (identifier, modified_key_v2 ());
  if (modified.empty ()) return 0;
  return convert_to_int (modified);
}


void Database_Notes::set_modified_v12 (int identifier, int time)
{
  if (is_v1 (identifier)) {
    set_modified_v1 (identifier, time);
  } else {
    set_modified_v2 (identifier, time);
  }
}

void Database_Notes::set_modified_v1 (int identifier, int time)
{
  // Update the filesystem.
  string file = modified_file_v1 (identifier);
  filter_url_file_put_contents (file, convert_to_string (time));
  // Update the database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET modified =");
  sql.add (time);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Update checksum.
  update_checksum_v1 (identifier);
}


void Database_Notes::set_modified_v2 (int identifier, int time)
{
  // Update the filesystem.
  set_field_v2 (identifier, modified_key_v2 (), convert_to_string (time));
  // Update the database.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET modified =");
  sql.add (time);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Update checksum.
  update_checksum_v2 (identifier);
}


bool Database_Notes::get_public_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return get_public_v1 (identifier);
  } else {
    return get_public_v2 (identifier);
  }
}


bool Database_Notes::get_public_v1 (int identifier)
{
  string file = public_file_v1 (identifier);
  return file_or_dir_exists (file);
}


bool Database_Notes::get_public_v2 (int identifier)
{
  string value = get_field_v2 (identifier, public_key_v2 ());
  return convert_to_bool (value);
}


void Database_Notes::set_public_v12 (int identifier, bool value)
{
  if (is_v1 (identifier)) {
    set_public_v1 (identifier, value);
  } else {
    set_public_v2 (identifier, value);
  }
}


void Database_Notes::set_public_v1 (int identifier, bool value)
{
  string file = public_file_v1 (identifier);
  if (value) {
    filter_url_file_put_contents (file, "");
  } else {
    filter_url_unlink (file);
  }
}


void Database_Notes::set_public_v2 (int identifier, bool value)
{
  set_field_v2 (identifier, public_key_v2 (), convert_to_string (value));
}


// Takes actions when a note has been edited.
void Database_Notes::note_modified_actions_v12 (int identifier)
{
  // Update 'modified' field.
  if (is_v1 (identifier)) {
    set_modified_v1 (identifier, filter_date_seconds_since_epoch());
  } else {
    set_modified_v2 (identifier, filter_date_seconds_since_epoch());
  }
}


void Database_Notes::update_search_fields_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    update_search_fields_v1 (identifier);
  } else {
    update_search_fields_v2 (identifier);
  }
}


void Database_Notes::update_search_fields_v1 (int identifier)
{
  // The search field is a combination of the summary and content converted to clean text.
  // It enables us to search with wildcards before and after the search query.
  string noteSummary = get_summary_v1 (identifier);
  string noteContents = get_contents_v1 (identifier);
  string cleanText = noteSummary + "\n" + filter_string_html2text (noteContents);
  // Bail out if the search field is already up to date.
  if (cleanText == get_search_field_v12 (identifier)) return;
  // Update the field.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET cleantext =");
  sql.add (cleanText);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


void Database_Notes::update_search_fields_v2 (int identifier)
{
  // The search field is a combination of the summary and content converted to clean text.
  // It enables us to search with wildcards before and after the search query.
  string noteSummary = get_summary_v2 (identifier);
  string noteContents = get_contents_v2 (identifier);
  string cleanText = noteSummary + "\n" + filter_string_html2text (noteContents);
  // Bail out if the search field is already up to date.
  if (cleanText == get_search_field_v12 (identifier)) return;
  // Update the field.
  SqliteSQL sql;
  sql.add ("UPDATE notes SET cleantext =");
  sql.add (cleanText);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


string Database_Notes::get_search_field_v12 (int identifier)
{
  SqliteSQL sql;
  sql.add ("SELECT cleantext FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["cleantext"];
  database_sqlite_disconnect (db);
  string value;
  for (auto & cleantext : result) {
    value = cleantext;
  }
  return value;
}


// Searches the notes.
// Returns an array of note identifiers.
// search: Contains the text to search for.
// bibles: Array of Bibles the notes should refer to.
vector <int> Database_Notes::search_notes_v12 (string search, const vector <string> & bibles)
{
  vector <int> identifiers;

  search = filter_string_trim (search);
  if (search == "") return identifiers;

  // SQL SELECT statement.
  string query = notes_select_identifier ();

  // SQL fulltext search statement sorted on relevance.
  query.append (notes_optional_fulltext_search_relevance_statement (search));

  // SQL FROM ... WHERE statement.
  query.append (notes_from_where_statement ());

  // Consider text contained in notes.
  query.append (notes_optional_fulltext_search_statement (search));

  // Consider Bible constraints:
  // * A user has access to notes that refer to Bibles the user has access to.
  // * A note can be a general one, not referring to any specific Bible.
  //   Select such notes also.
  query.append (" AND (bible = '' ");
  for (string bible : bibles) {
    bible = database_sqlite_no_sql_injection (bible);
    query.append (" OR bible = '");
    query.append (bible);
    query.append ("' ");
  }
  query.append (" ) ");

  // Notes get ordered on relevance of search hits.
  query.append (notes_order_by_relevance_statement ());

  // Complete query.
  query.append (";");
  
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, query) ["identifier"];
  database_sqlite_disconnect (db);
  for (auto & id : result) {
    identifiers.push_back (convert_to_int (id));
  }

  return identifiers;
}


void Database_Notes::mark_for_deletion_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    mark_for_deletion_v1 (identifier);
  } else {
    mark_for_deletion_v2 (identifier);
  }
}


void Database_Notes::mark_for_deletion_v1 (int identifier)
{
  string file = expiry_file_v1 (identifier);
  // Delete after 7 days.
  filter_url_file_put_contents (file, "7");
}


void Database_Notes::mark_for_deletion_v2 (int identifier)
{
  // Delete after 7 days.
  set_field_v2 (identifier, expiry_key_v2 (), "7");
}


void Database_Notes::unmark_for_deletion_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    unmark_for_deletion_v1 (identifier);
  } else {
    unmark_for_deletion_v2 (identifier);
  }
}


void Database_Notes::unmark_for_deletion_v1 (int identifier)
{
  string file = expiry_file_v1 (identifier);
  filter_url_unlink (file);
}


void Database_Notes::unmark_for_deletion_v2 (int identifier)
{
  set_field_v2 (identifier, expiry_key_v2 (), "");
}


bool Database_Notes::is_marked_for_deletion_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    return is_marked_for_deletion_v1 (identifier);
  } else {
    return is_marked_for_deletion_v2 (identifier);
  }
}


bool Database_Notes::is_marked_for_deletion_v1 (int identifier)
{
  string file = expiry_file_v1 (identifier);
  return file_or_dir_exists (file);
}


bool Database_Notes::is_marked_for_deletion_v2 (int identifier)
{
  string expiry = get_field_v2 (identifier, expiry_key_v2 ());
  return !expiry.empty ();
}


void Database_Notes::touch_marked_for_deletion_v12 ()
{
  vector <int> identifiers = get_identifiers_v12 ();
  for (auto & identifier : identifiers) {
    if (is_v1 (identifier)) {
      if (is_marked_for_deletion_v1 (identifier)) {
        string file = expiry_file_v1 (identifier);
        int days = convert_to_int (filter_url_file_get_contents (file));
        days--;
        filter_url_file_put_contents (file, convert_to_string (days));
      }
    } else {
      if (is_marked_for_deletion_v2 (identifier)) {
        string expiry = get_field_v2 (identifier, expiry_key_v2 ());
        int days = convert_to_int (expiry);
        days--;
        set_field_v2 (identifier, expiry_key_v2 (), convert_to_string (days));
      }
    }
  }
}


vector <int> Database_Notes::get_due_for_deletion_v1 ()
{
  vector <int> deletes;
  vector <int> identifiers = get_identifiers_v12 ();
  for (auto & identifier : identifiers) {
    if (is_marked_for_deletion_v1 (identifier)) {
      string file = expiry_file_v1 (identifier);
      string sdays = filter_url_file_get_contents (file);
      int idays = convert_to_int (sdays);
      if ((sdays == "0") || (idays < 0)) {
        deletes.push_back (identifier);
      }
    }
  }
  return deletes;
}


vector <int> Database_Notes::get_due_for_deletion_v2 ()
{
  vector <int> deletes;
  vector <int> identifiers = get_identifiers_v12 ();
  for (auto & identifier : identifiers) {
    if (is_marked_for_deletion_v2 (identifier)) {
      string sdays = get_field_v2 (identifier, expiry_key_v2 ());
      int idays = convert_to_int (sdays);
      if ((sdays == "0") || (idays < 0)) {
        deletes.push_back (identifier);
      }
    }
  }
  return deletes;
}


// Writes the checksum for note identifier in the database.
void Database_Notes::set_checksum_v12 (int identifier, const string & checksum)
{
  // Do not write the checksum if it is already up to date.
  if (checksum == get_checksum_v12 (identifier)) return;
  // Write the checksum to the database.
  delete_checksum_v12 (identifier);
  SqliteSQL sql;
  sql.add ("INSERT INTO checksums VALUES (");
  sql.add (identifier);
  sql.add (",");
  sql.add (checksum);
  sql.add (");");
  sqlite3 * db = connect_checksums ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Reads the checksum for note identifier from the database.
string Database_Notes::get_checksum_v12 (int identifier)
{
  SqliteSQL sql;
  sql.add ("SELECT checksum FROM checksums WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect_checksums ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["checksum"];
  database_sqlite_disconnect (db);
  string value;
  for (auto & row : result) {
    value = row;
  }
  return value;
}


// Deletes the checksum for note identifier from the database.
void Database_Notes::delete_checksum_v12 (int identifier)
{
  SqliteSQL sql;
  sql.add ("DELETE FROM checksums WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect_checksums ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  // Delete from range-based checksums.
  Database_State::eraseNoteChecksum (identifier);
}


void Database_Notes::update_checksum_v12 (int identifier)
{
  if (is_v1 (identifier)) {
    update_checksum_v1 (identifier);
  }
  else {
    update_checksum_v2 (identifier); 
  }
}

// The function calculates the checksum of the note signature,
// and writes it to the filesystem.
void Database_Notes::update_checksum_v1 (int identifier)
{
  // Read the raw data from disk to speed up checksumming.
  string checksum;
  checksum.append ("modified");
  checksum.append (filter_url_file_get_contents (modified_file_v1 (identifier)));
  checksum.append ("assignees");
  checksum.append (filter_url_file_get_contents (assigned_file_v1 (identifier)));
  checksum.append ("subscribers");
  checksum.append (filter_url_file_get_contents (subscriptions_file_v1 (identifier)));
  checksum.append ("bible");
  checksum.append (filter_url_file_get_contents (bible_file_v1 (identifier)));
  checksum.append ("passages");
  checksum.append (filter_url_file_get_contents (passage_file_v1 (identifier)));
  checksum.append ("status");
  checksum.append (filter_url_file_get_contents (status_file_v1 (identifier)));
  checksum.append ("severity");
  checksum.append (filter_url_file_get_contents (severity_file_v1 (identifier)));
  checksum.append ("summary");
  checksum.append (filter_url_file_get_contents (summary_file_v1 (identifier)));
  checksum.append ("contents");
  checksum.append (filter_url_file_get_contents (contents_file_v1 (identifier)));
  checksum = md5 (checksum);
  set_checksum_v12 (identifier, checksum);
}


// The function calculates the checksum of the note signature,
// and writes it to the filesystem.
void Database_Notes::update_checksum_v2 (int identifier)
{
  // Read the raw data from disk to speed up checksumming.
  string checksum;
  checksum.append ("modified");
  checksum.append (get_field_v2 (identifier, modified_key_v2 ()));
  checksum.append ("assignees");
  checksum.append (get_field_v2 (identifier, assigned_key_v2 ()));
  checksum.append ("subscribers");
  checksum.append (get_field_v2 (identifier, subscriptions_key_v2 ()));
  checksum.append ("bible");
  checksum.append (get_field_v2 (identifier, bible_key_v2 ()));
  checksum.append ("passages");
  checksum.append (get_field_v2 (identifier, passage_key_v2 ()));
  checksum.append ("status");
  checksum.append (get_field_v2 (identifier, status_key_v2 ()));
  checksum.append ("severity");
  checksum.append (get_field_v2 (identifier, severity_key_v2 ()));
  checksum.append ("summary");
  checksum.append (get_field_v2 (identifier, summary_key_v2 ()));
  checksum.append ("contents");
  checksum.append (get_field_v2 (identifier, contents_key_v2 ()));
  checksum = md5 (checksum);
  set_checksum_v12 (identifier, checksum);
}


// Queries the database for the checksum for the notes given in the list of $identifiers.
string Database_Notes::get_multiple_checksum_v12 (const vector <int> & identifiers)
{
  sqlite3 * db = connect_checksums ();
  string checksum;
  for (auto & identifier : identifiers) {
    SqliteSQL sql;
    sql.add ("SELECT checksum FROM checksums WHERE identifier =");
    sql.add (identifier);
    sql.add (";");
    vector <string> result = database_sqlite_query (db, sql.sql) ["checksum"];
    string value = "";
    for (auto & row : result) {
      value = row;
    }
    checksum.append (value);
  }
  database_sqlite_disconnect (db);
  checksum = md5 (checksum);
  return checksum;
}


// This function gets the identifiers for notes
// within the note identifier range of lowId to highId
// which refer to any Bible in the array of bibles
// or refer to no Bible.
vector <int> Database_Notes::get_notes_in_range_for_bibles_v12 (int lowId, int highId, vector <string> bibles, bool anybible)
{
  vector <int> identifiers;
  
  string query = "SELECT identifier FROM notes WHERE identifier >= ";
  query.append (convert_to_string (lowId));
  query.append (" AND identifier <= ");
  query.append (convert_to_string (highId));
  query.append (" ");
  if (!anybible) {
    bibles.push_back (""); // Select general note also
    string bibleSelector = " AND (";
    for (unsigned int i = 0; i < bibles.size(); i++) {
      bibles[i] = database_sqlite_no_sql_injection (bibles[i]);
      if (i > 0) bibleSelector.append (" OR ");
      bibleSelector.append (" bible = '");
      bibleSelector.append (bibles[i]);
      bibleSelector.append ("' ");
    }
    bibleSelector.append (")");
    query.append (bibleSelector);
  }
  query.append (" ORDER BY identifier;");

  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, query) ["identifier"];
  database_sqlite_disconnect (db);
  for (auto & row : result) {
    identifiers.push_back (convert_to_int (row));
  }
  
  return identifiers;
}


string Database_Notes::availability_flag ()
{
  return filter_url_create_root_path ("databases", "notes.busy");
}


// Sets whether the notes databases are available, as a boolean.
void Database_Notes::set_availability_v12 (bool available)
{
  if (available) {
    filter_url_unlink (availability_flag ());
  } else {
   filter_url_file_put_contents (availability_flag (), "");
  }
}


// Returns whether the notes databases are available, as a boolean.
bool Database_Notes::available_v12 ()
{
  return !file_or_dir_exists (availability_flag ());
}


string Database_Notes::notes_select_identifier ()
{
  return " SELECT identifier ";
}


string Database_Notes::notes_optional_fulltext_search_relevance_statement (string search)
{
  if (search == "") return "";
  search = filter_string_str_replace (",", "", search);
  search = database_sqlite_no_sql_injection (search);
  string query = "";
  return query;
}


string Database_Notes::notes_from_where_statement ()
{
  return " FROM notes WHERE 1 ";
}


string Database_Notes::notes_optional_fulltext_search_statement (string search)
{
  if (search == "") return "";
  search = filter_string_str_replace (",", "", search);
  search = database_sqlite_no_sql_injection (search);
  string query = " AND cleantext LIKE '%" + search + "%' ";
  return query;
}


string Database_Notes::notes_order_by_relevance_statement ()
{
  return "";
}


// This returns JSON that contains the notes indicated by $identifiers.
string Database_Notes::get_bulk_v12 (vector <int> identifiers)
{
  // JSON container for the bulk notes.
  Array bulk;
  // Go through all the notes.
  for (auto identifier : identifiers) {
    // JSON object for the note.
    Object note;
    // Determine the note's storage mechanism: Version 1 (old) or version 2 (new, JSON).
    bool v1 = is_v1 (identifier);
    // Add all the fields of the note.
    string assigned;
    if (v1) assigned = filter_url_file_get_contents (assigned_file_v1 (identifier));
    else assigned = get_field_v2 (identifier, assigned_key_v2 ());
    note << "a" << assigned;
    string bible;
    if (v1) bible = get_bible_v1 (identifier);
    else bible = get_bible_v2 (identifier);;
    note << "b" << bible;
    string contents;
    if (v1) contents = get_contents_v1 (identifier);
    else contents = get_contents_v2 (identifier);
    note << "c" << contents;
    note << "i" << identifier;
    int modified;
    if (v1) modified = get_modified_v1 (identifier);
    else modified = get_modified_v2 (identifier);
    note << "m" << modified;
    string passage;
    if (v1) passage = get_raw_passage_v1 (identifier);
    else passage = get_raw_passage_v2 (identifier);
    note << "p" << passage;
    string subscriptions;
    if (v1) subscriptions = filter_url_file_get_contents (subscriptions_file_v1 (identifier));
    else subscriptions = get_field_v2 (identifier, subscriptions_key_v2 ());
    note << "sb" << subscriptions;
    string summary;
    if (v1) summary = get_summary_v1 (identifier);
    else summary = get_summary_v2 (identifier);
    note << "sm" << summary;
    string status;
    if (v1) status = get_raw_status_v1 (identifier);
    else status = get_raw_status_v2 (identifier);
    note << "st" << status;
    int severity;
    if (v1) severity = get_raw_severity_v1 (identifier);
    else severity = get_raw_severity_v2 (identifier);
    note << "sv" << severity;
    // Add the note to the bulk container.
    bulk << note;
  }
  // Resulting JSON string.
  return bulk.json ();
}


// This takes $json and stores all the notes it contains in the filesystem.
vector <string> Database_Notes::set_bulk_v1 (string json)
{
  // Container for the summaries that were stored.
  vector <string> summaries;

  // Parse the incoming JSON.
  Array bulk;
  bulk.parse (json);
  
  // Go through the notes the JSON contains.
  for (size_t i = 0; i < bulk.size (); i++) {

    // Get all the different fields for this note.
    Object note = bulk.get<Object>(i);
    string assigned = note.get<String> ("a");
    string bible = note.get<String> ("b");
    string contents = note.get<String> ("c");
    int identifier = note.get<Number> ("i");
    int modified = note.get<Number> ("m");
    string passage = note.get<String> ("p");
    string subscriptions = note.get<String> ("sb");
    string summary = note.get<String> ("sm");
    string status = note.get<String> ("st");
    int severity = note.get<Number> ("sv");

    // Store the note in the filesystem.
    filter_url_mkdir (note_folder_v1 (identifier));
    filter_url_file_put_contents (assigned_file_v1 (identifier), assigned);
    filter_url_file_put_contents (bible_file_v1 (identifier), bible);
    set_raw_contents_v1 (identifier, contents);
    filter_url_file_put_contents (modified_file_v1 (identifier), convert_to_string (modified));
    filter_url_file_put_contents (passage_file_v1 (identifier), passage);
    filter_url_file_put_contents (severity_file_v1 (identifier), convert_to_string (severity));
    filter_url_file_put_contents (status_file_v1 (identifier), status);
    filter_url_file_put_contents (subscriptions_file_v1 (identifier), subscriptions);
    filter_url_file_put_contents (summary_file_v1 (identifier), summary);

    // Update the indexes.
    update_database_v1 (identifier);
    update_search_fields_v12 (identifier);
    update_checksum_v1 (identifier);
  }
  
  // Container with all the summaries of the notes that were stored.
  return summaries;
}


// This takes $json and stores all the notes it contains in the filesystem.
vector <string> Database_Notes::set_bulk_v2 (string json)
{
  // Container for the summaries that were stored.
  vector <string> summaries;
  
  // Parse the incoming JSON.
  Array bulk;
  bulk.parse (json);
  
  // Go through the notes the JSON contains.
  for (size_t i = 0; i < bulk.size (); i++) {
    
    // Get all the different fields for this note.
    Object note = bulk.get<Object>(i);
    string assigned = note.get<String> ("a");
    string bible = note.get<String> ("b");
    string contents = note.get<String> ("c");
    int identifier = note.get<Number> ("i");
    int modified = note.get<Number> ("m");
    string passage = note.get<String> ("p");
    string subscriptions = note.get<String> ("sb");
    string summary = note.get<String> ("sm");
    string status = note.get<String> ("st");
    int severity = note.get<Number> ("sv");
    
    // Feedback about which note it received in bulk.
    summaries.push_back (summary);
    
    // If the note exists in the old format, delete it.
    string oldfolder = note_folder_v1 (identifier);
    if (file_or_dir_exists (oldfolder)) {
      filter_url_rmdir (oldfolder);
    }
    
    // Store the note in the filesystem.
    string path = note_file_v2 (identifier);
    string folder = filter_url_dirname (path);
    filter_url_mkdir (folder);
    Object note2;
    note2 << assigned_key_v2 () << assigned;
    note2 << bible_key_v2 () << bible;
    note2 << contents_key_v2 () << contents;
    note2 << modified_key_v2 () << convert_to_string (modified);
    note2 << passage_key_v2 () << passage;
    note2 << subscriptions_key_v2 () << subscriptions;
    note2 << summary_key_v2 () << summary;
    note2 << status_key_v2 () << status;
    note2 << severity_key_v2 () << convert_to_string (severity);
    string json = note2.json ();
    filter_url_file_put_contents (path, json);
    
    // Update the indexes.
    update_database_v2 (identifier);
    update_search_fields_v12 (identifier);
    update_checksum_v2 (identifier);
  }
  
  // Container with all the summaries of the notes that were stored.
  return summaries;
}


// Gets a field from a note in JSON format.
string Database_Notes::get_field_v2 (int identifier, string key)
{
  string file = note_file_v2 (identifier);
  string json = filter_url_file_get_contents (file);
  Object note;
  note.parse (json);
  string value;
  if (note.has<String> (key)) value = note.get<String> (key);
  return value;
}


// Sets a field in a note in JSON format.
void Database_Notes::set_field_v2 (int identifier, string key, string value)
{
  string file = note_file_v2 (identifier);
  string json = filter_url_file_get_contents (file);
  Object note;
  note.parse (json);
  note << key << value;
  json = note.json ();
  filter_url_file_put_contents (file, json);
}


string Database_Notes::bible_key_v2 ()
{
  return "bible";
}


string Database_Notes::passage_key_v2 ()
{
  return "passage";
}


string Database_Notes::status_key_v2 ()
{
  return "status";
}


string Database_Notes::severity_key_v2 ()
{
  return "severity";
}


string Database_Notes::modified_key_v2 ()
{
  return "modified";
}


string Database_Notes::summary_key_v2 ()
{
  return "summary";
}


string Database_Notes::contents_key_v2 ()
{
  return "contents";
}


string Database_Notes::subscriptions_key_v2 ()
{
  return "subscriptions";
}


string Database_Notes::assigned_key_v2 ()
{
  return "assigned";
}


string Database_Notes::expiry_key_v2 ()
{
  return "expiry";
}


string Database_Notes::public_key_v2 ()
{
  return "public";
}


// Returns true if the note $identifier is stored in format version 1.
// If the note is stored in the new format, version 2, it returns false.
// That is the JSON format.
bool Database_Notes::is_v1 (int identifier)
{
  string file_v2 = note_file_v2 (identifier);
  return !file_or_dir_exists (file_v2);
}


// Converts the storage model of note $identifier
// from version 1 to version 2 - JSON.
void Database_Notes::convert_v1_to_v2 (int identifier)
{
  // Read the note in version 1 format.
  string assigned = filter_url_file_get_contents (assigned_file_v1 (identifier));
  string bible = filter_url_file_get_contents (bible_file_v1 (identifier));
  string contents = filter_url_file_get_contents (contents_file_v1 (identifier));
  string expiry = filter_url_file_get_contents (expiry_file_v1 (identifier));
  string modified = filter_url_file_get_contents (modified_file_v1 (identifier));
  string passage = filter_url_file_get_contents (passage_file_v1 (identifier));
  string publicc = filter_url_file_get_contents (public_file_v1 (identifier));
  string severity = filter_url_file_get_contents (severity_file_v1 (identifier));
  string status = filter_url_file_get_contents (status_file_v1 (identifier));
  string subscriptions = filter_url_file_get_contents (subscriptions_file_v1 (identifier));
  string summary = filter_url_file_get_contents (summary_file_v1 (identifier));

  // Remove the note's folder.
  string folder = note_folder_v1 (identifier);
  filter_url_rmdir (folder);
 
  // Get the note in version 2 - JSON - format.
  Object note;
  note << assigned_key_v2 () << assigned;
  note << bible_key_v2 () << bible;
  note << contents_key_v2 () << contents;
  note << expiry_key_v2 () << expiry;
  note << modified_key_v2 () << convert_to_string (modified);
  note << passage_key_v2 () << passage;
  note << public_key_v2 () << publicc;
  note << severity_key_v2 () << convert_to_string (severity);
  note << status_key_v2 () << status;
  note << subscriptions_key_v2 () << subscriptions;
  note << summary_key_v2 () << summary;
  string json = note.json ();

  // Store the JSON note to file.
  string path = note_file_v2 (identifier);
  folder = filter_url_dirname (path);
  filter_url_mkdir (folder);
  filter_url_file_put_contents (path, json);

  // No need to update any database, as there's no change there.
}
