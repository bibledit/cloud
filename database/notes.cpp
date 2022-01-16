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
#include <database/logic.h>


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
 In older versions the notes were stored as a bundle of separate files.
 In newer versions each note is stored as one JSON file.
 This uses less space on disk.
 In older versions, on a Linux server, one notes took 32 kbytes.
 A lot of that space is wasted.
 In newer versions one notes takes only 4 kbytes.
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


void Database_Notes::create ()
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


string Database_Notes::database_path ()
{
  return filter_url_create_root_path_cpp17_Todo ({database_logic_databases (), "notes.sqlite"});
}


string Database_Notes::checksums_database_path ()
{
  return filter_url_create_root_path_cpp17_Todo ({database_logic_databases (), "notes_checksums.sqlite"});
}


// Returns whether the notes database is healthy, as a boolean.
bool Database_Notes::healthy ()
{
  return database_sqlite_healthy (database_path ());
}


// Returns whether the notes checksums database is healthy, as a boolean.
bool Database_Notes::checksums_healthy ()
{
  return database_sqlite_healthy (checksums_database_path ());
}


// Does a checkup on the health of the main database.
// Optionally recreates it.
// Returns true if to be synced, else false.
bool Database_Notes::checkup ()
{
  if (healthy ()) return false;
  filter_url_unlink_cpp17 (database_path ());
  create ();
  return true;
}


// Does a checkup on the health of the checksums database.
// Optionally recreates it.
// Returns true if to synced, else false.
bool Database_Notes::checkup_checksums ()
{
  if (checksums_healthy ()) return false;
  filter_url_unlink_cpp17 (checksums_database_path ());
  create ();
  return true;
}


void Database_Notes::trim ()
{
  // Clean empty directories.
  string message = "Deleting empty notes folder ";
  string main_folder = main_folder_path ();
  vector <string> bits1 = filter_url_scandir (main_folder);
  for (auto bit1 : bits1) {
    if (bit1.length () == 3) {
      string folder = filter_url_create_path_cpp17 ({main_folder, bit1});
      vector <string> bits2 = filter_url_scandir (folder);
      if (bits2.empty ()) {
        Database_Logs::log (message + folder);
        remove (folder.c_str ());
      }
      for (auto bit2 : bits2) {
        if (bit2.length () == 3) {
          string folder = filter_url_create_path_cpp17 ({main_folder, bit1, bit2});
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


void Database_Notes::trim_server ()
{
  // Notes expiry.
  touch_marked_for_deletion ();
  /// Storage for notes to be deleted.
  vector <int> identifiers;
  // Deal with new notes storage in JSON.
  identifiers = get_due_for_deletion ();
  for (auto & identifier : identifiers) {
    trash_consultation_note (webserver_request, identifier);
    erase (identifier);
  }
}


void Database_Notes::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM;");
  database_sqlite_disconnect (db);
}


void Database_Notes::sync ()
{
  string main_folder = main_folder_path ();

  // List of notes in the filesystem.
  vector <int> identifiers;

  vector <string> bits1 = filter_url_scandir (main_folder);
  for (auto & bit1 : bits1) {
    // Bit 1 / 2 / 3 may start with a 0, so conversion to int cannot be used, rather use a length of 3.
    // It used conversion to int before to determine it was a real note,
    // with the result that it missed 10% of the notes, which subsequently got deleted, oops!
    if (bit1.length () == 3) {
      vector <string> bits2 = filter_url_scandir (filter_url_create_path_cpp17 ({main_folder, bit1}));
      for (auto & bit2 : bits2) {
        // Old storage mechanism, e.g. folder "425".
        if (bit2.length () == 3) {
          vector <string> bits3 = filter_url_scandir (filter_url_create_path_cpp17 ({main_folder, bit1, bit2}));
          for (auto & bit3 : bits3) {
            if (bit3.length () == 3) {
              int identifier = convert_to_int (bit1 + bit2 + bit3);
              identifiers.push_back (identifier);
              update_search_fields (identifier);
            }
          }
        }
        // New JSON storage mechanism, e.g. file "894093.json".
        if ((bit2.length () == 11) && bit2.find (".json") != string::npos) {
          int identifier = convert_to_int (bit1 + bit2.substr (0,6));
          identifiers.push_back (identifier);
          update_database (identifier);
          update_search_fields (identifier);
          update_checksum (identifier);
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
      erase (id);
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
      delete_checksum (id);
    }
  }
  
}


void Database_Notes::update_database (int identifier)
{
  // Read the relevant values from the filesystem.
  int modified = get_modified (identifier);
  string assigned = get_field (identifier, assigned_key ());
  string subscriptions = get_field (identifier, subscriptions_key ());
  string bible = get_bible (identifier);
  string passage = get_raw_passage (identifier);
  string status = get_raw_status (identifier);
  int severity = get_raw_severity (identifier);
  string summary = get_summary (identifier);
  string contents = get_contents (identifier);
  
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


string Database_Notes::main_folder_path ()
{
  return filter_url_create_root_path_cpp17_Todo ({"consultations"});
}


string Database_Notes::note_file (int identifier)
{
  // The maximum number of folders a folder may contain is constrained by the filesystem.
  // To overcome this, the notes will be stored in a folder structure.
  string sidentifier = convert_to_string (identifier);
  string folder = sidentifier.substr (0, 3);
  string file = sidentifier.substr (3, 6) + ".json";
  return filter_url_create_path_cpp17 ({main_folder_path (), folder, file});
}


// This checks whether the note identifier exists.
// It works for the old way of storing notes in many files,
// and for the new way of storing notes in JSON.
bool Database_Notes::identifier_exists (int identifier)
{
  if (file_or_dir_exists (note_file (identifier))) return true;
  return false;
}


// Update a note's identifier.
// new_identifier is the value given to the note identified by $identifier.
void Database_Notes::set_identifier (int identifier, int new_identifier)
{
  // Move data on the filesystem.
  erase (new_identifier);
  string path = note_file (identifier);
  string json = filter_url_file_get_contents (path);
  path = note_file (new_identifier);
  string folder = filter_url_dirname_cpp17 (path);
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
  erase (identifier);
}


// Gets new unique note identifier.
// Works for the old and new storage system.
int Database_Notes::get_new_unique_identifier ()
{
  int identifier = 0;
  do {
    identifier = filter_string_rand (Notes_Logic::lowNoteIdentifier, Notes_Logic::highNoteIdentifier);
  } while (identifier_exists (identifier));
  return identifier;
}


vector <int> Database_Notes::get_identifiers ()
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


string Database_Notes::assemble_contents (int identifier, string contents)
{
  string new_contents = get_contents (identifier);
  int time = filter_date_seconds_since_epoch ();
  string datetime = convert_to_string (filter_date_numerical_month_day (time)) + "/" + convert_to_string (filter_date_numerical_month (time)) + "/" + convert_to_string (filter_date_numerical_year (time));
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic ()->currentUser ();

  // To make the notes more readable, add whitespace between the comments.
  bool is_initial_comment = new_contents.empty ();
  if (!is_initial_comment) {
    new_contents.append ("\n");
    new_contents.append ("<br>\n");
  }
  // Put the user and date in bold, for extra clarity.
  new_contents.append ("<p><b>");
  new_contents.append (user);
  new_contents.append (" (");
  new_contents.append (datetime);
  new_contents.append ("):</b></p>\n");
  // Add the note body.
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


// Store a new consultation note into the database and in JSON.
// bible: The notes's Bible.
// book, chapter, verse: The note's passage.
// summary: The note's summary.
// contents: The note's contents.
// raw: Import contents as it is.
// It returns the identifier of this new note.
int Database_Notes::store_new_note (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw)
{
  // Create a new identifier.
  int identifier = get_new_unique_identifier ();
  
  // Passage.
  string passage = encode_passage (book, chapter, verse);
  
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
  if (!raw) contents = assemble_contents (identifier, contents);
  if ((contents.empty()) && (summary.empty())) return 0;
  
  // Store the JSON representation of the note in the file system.
  string path = note_file (identifier);
  string folder = filter_url_dirname_cpp17 (path);
  filter_url_mkdir (folder);
  Object note;
  note << bible_key () << bible;
  note << passage_key () << passage;
  note << status_key () << status;
  note << severity_key () << convert_to_string (severity);
  note << summary_key () << summary;
  note << contents_key () << contents;
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
  update_search_fields (identifier);
  note_modified_actions (identifier);
  
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
vector <int> Database_Notes::select_notes (vector <string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const string& status_selector, string bible_selector, string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const string& search_text, int limit)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string username = request->session_logic ()->currentUser ();
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
      passage = encode_passage (book, chapter, verse);
      query.append (" AND passage LIKE '%" + passage + "%' ");
      break;
    case 1:
      // Select notes that refer to the current chapter.
      // It means that the book and the chapter should match.
      passage = encode_passage (book, chapter, -1);
      query.append (" AND passage LIKE '%" + passage + "%' ");
      break;
    case 2:
      // Select notes that refer to the current book.
      // It means that the book should match.
      passage = encode_passage (book, -1, -1);
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


string Database_Notes::get_summary (int identifier)
{
  return get_field (identifier, summary_key ());
}


void Database_Notes::set_summary (int identifier, const string& summary)
{
  // Store authoritative copy in the filesystem.
  set_field (identifier, summary_key (), summary);
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
  update_search_fields (identifier);
  // Update checksum.
  update_checksum (identifier);
}


string Database_Notes::get_contents (int identifier)
{
  return get_field (identifier, contents_key ());
}


void Database_Notes::set_raw_contents (int identifier, const string& contents)
{
  set_field (identifier, contents_key (), contents);
}


void Database_Notes::set_contents (int identifier, const string& contents)
{
  // Store in file system.
  set_raw_contents (identifier, contents);
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
  update_search_fields (identifier);
  // Update checksum.
  update_checksum (identifier);
}


// Erases a note stored in the old and in the new format.
void Database_Notes::erase (int identifier)
{
  // Delete new storage from filesystem.
  string path = note_file (identifier);
  filter_url_unlink_cpp17 (path);
  // Update databases as well.
  delete_checksum (identifier);
  SqliteSQL sql;
  sql.add ("DELETE FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Add a comment to an exiting note identified by $identifier.
void Database_Notes::add_comment (int identifier, const string& comment)
{
  // Assemble the new content and store it.
  // This updates the search database also.
  string contents = assemble_contents (identifier, comment);
  set_contents (identifier, contents);
  
  // Some triggers.
  note_modified_actions (identifier);
  unmark_for_deletion (identifier);
  
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
void Database_Notes::subscribe (int identifier)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic ()->currentUser ();
  subscribe_user (identifier, user);
}


// Subscribe the user to the note identified by identifier.
void Database_Notes::subscribe_user (int identifier, const string& user)
{
  // If the user already is subscribed to the note, bail out.
  vector <string> subscribers = get_subscribers (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) != subscribers.end()) return;
  // Subscribe user.
  subscribers.push_back (user);
  set_subscribers (identifier, subscribers);
}


// Returns an array with the subscribers to the note identified by identifier.
vector <string> Database_Notes::get_subscribers (int identifier)
{
  string contents = get_raw_subscriptions (identifier);
  if (contents.empty()) return {};
  vector <string> subscribers = filter_string_explode (contents, '\n');
  for (auto & subscriber : subscribers) {
    subscriber = filter_string_trim (subscriber);
  }
  return subscribers;
}


string Database_Notes::get_raw_subscriptions (int identifier)
{
  return get_field (identifier, subscriptions_key ());
}


void Database_Notes::set_raw_subscriptions (int identifier, const string& subscriptions)
{
  // Store them in the filesystem.
  set_field (identifier, subscriptions_key (), subscriptions);
  
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


void Database_Notes::set_subscribers (int identifier, vector <string> subscribers)
{
  // Add a space at both sides of the subscriber to allow for easier note selection based on note assignment.
  for (auto & subscriber : subscribers) {
    subscriber.insert (0, " ");
    subscriber.append (" ");
  }
  string subscriberstring = filter_string_implode (subscribers, "\n");
  
  // Store them to file and in the database.
  set_raw_subscriptions (identifier, subscriberstring);
  
  // Checksum.
  update_checksum (identifier);
}


// Returns true if user is subscribed to the note identified by identifier.
bool Database_Notes::is_subscribed (int identifier, const string& user)
{
  vector <string> subscribers = get_subscribers (identifier);
  return find (subscribers.begin(), subscribers.end(), user) != subscribers.end();
}


// Unsubscribes the currently logged in user from the note identified by identifier.
void Database_Notes::unsubscribe (int identifier)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic ()->currentUser ();
  unsubscribe_user (identifier, user);
}


// Unsubscribes user from the note identified by identifier.
void Database_Notes::unsubscribe_user (int identifier, const string& user)
{
  // If the user is not subscribed to the note, bail out.
  vector <string> subscribers = get_subscribers (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) == subscribers.end()) return;
  // Unsubscribe user.
  subscribers.erase (remove (subscribers.begin(), subscribers.end(), user), subscribers.end());
  set_subscribers (identifier, subscribers);
}


string Database_Notes::get_raw_assigned (int identifier)
{
  // Get the asssignees from the filesystem.
  return get_field (identifier, assigned_key ());
}


void Database_Notes::set_raw_assigned (int identifier, const string& assigned)
{
  // Store the assignees in the filesystem.
  set_field (identifier, assigned_key (), assigned);
  
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
vector <string> Database_Notes::get_all_assignees (const vector <string>& bibles)
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
vector <string> Database_Notes::get_assignees (int identifier)
{
  // Get the asssignees from the filesystem.
  string assignees = get_raw_assigned (identifier);
  return get_assignees_internal (assignees);
}


vector <string> Database_Notes::get_assignees_internal (string assignees)
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
void Database_Notes::set_assignees (int identifier, vector <string> assignees)
{
  // Add a space at both sides of the assignee to allow for easier note selection based on note assignment.
  for (auto & assignee : assignees) {
    assignee.insert (0, " ");
    assignee.append (" ");
  }
  string assignees_string = filter_string_implode (assignees, "\n");
  set_raw_assigned (identifier, assignees_string);
  note_modified_actions (identifier);
}


// Assign the note identified by identifier to user.
void Database_Notes::assign_user (int identifier, const string& user)
{
  // If the note already is assigned to the user, bail out.
  vector <string> assignees = get_assignees (identifier);
  if (find (assignees.begin (), assignees.end(), user) != assignees.end()) return;
  // Assign the note to the user.
  assignees.push_back (user);
  // Store the whole lot.
  set_assignees (identifier, assignees);
}


// Returns true if the note identified by identifier has been assigned to user.
bool Database_Notes::is_assigned (int identifier, const string& user)
{
  vector <string> assignees = get_assignees (identifier);
  return find (assignees.begin(), assignees.end(), user) != assignees.end();
}


// Unassigns user from the note identified by identifier.
void Database_Notes::unassign_user (int identifier, const string& user)
{
  // If the note is not assigned to the user, bail out.
  vector <string> assignees = get_assignees (identifier);
  if (find (assignees.begin(), assignees.end(), user) == assignees.end()) return;
  // Remove assigned user.
  assignees.erase (remove (assignees.begin(), assignees.end(), user), assignees.end());
  set_assignees (identifier, assignees);
}


string Database_Notes::get_bible (int identifier)
{
  return get_field (identifier, bible_key ());
}


void Database_Notes::set_bible (int identifier, const string& bible)
{
  // Write the bible to the filesystem.
  set_field (identifier, bible_key (), bible);
  
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
  
  note_modified_actions (identifier);
}


vector <string> Database_Notes::get_all_bibles ()
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
string Database_Notes::encode_passage (int book, int chapter, int verse)
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
Passage Database_Notes::decode_passage (string passage)
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
string Database_Notes::decode_passage (int identifier)
{
  return get_raw_passage (identifier);
}


// Returns the raw passage text of the note identified by identifier.
string Database_Notes::get_raw_passage (int identifier)
{
  return get_field (identifier, passage_key ());
}


// Returns an array with the passages that the note identified by identifier refers to.
// Each passages is an array (book, chapter, verse).
vector <Passage> Database_Notes::get_passages (int identifier)
{
  string contents = get_raw_passage (identifier);
  if (contents.empty()) return {};
  vector <string> lines = filter_string_explode (contents, '\n');
  vector <Passage> passages;
  for (auto & line : lines) {
    if (line.empty()) continue;
    Passage passage = decode_passage (line);
    passages.push_back (passage);
  }
  return passages;
}


// Set the passages for note identifier.
// passages is an array of an array (book, chapter, verse) passages.
// import: If true, just write passages, no further actions.
void Database_Notes::set_passages (int identifier, const vector <Passage>& passages, bool import)
{
  // Format the passages.
  string line;
  for (auto & passage : passages) {
    if (!line.empty ()) line.append ("\n");
    line.append (encode_passage (passage.book, passage.chapter, convert_to_int (passage.verse)));
  }
  // Store it.
  set_raw_passage (identifier, line);
  
  // Update index.
  index_raw_passage (identifier, line);

  if (!import) note_modified_actions (identifier);
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
void Database_Notes::set_raw_passage (int identifier, const string& passage)
{
  // Store the authoritative copy in the filesystem.
  set_field (identifier, passage_key (), passage);
}


void Database_Notes::index_raw_passage (int identifier, const string& passage)
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
string Database_Notes::get_raw_status (int identifier)
{
  return get_field (identifier, status_key ());
}


// Gets the localized status of a note.
// Returns it as a string.
string Database_Notes::get_status (int identifier)
{
  string status = get_raw_status (identifier);
  // Localize status if possible.
  status = translate (status.c_str());
  // Return status.
  return status;
}


// Sets the status of the note identified by identifier.
// status is a string.
// import: Just write the status, and skip any logic.
void Database_Notes::set_status (int identifier, const string& status, bool import)
{
  // Store the authoritative copy in the filesystem.
  set_field (identifier, status_key (), status);
  
  if (!import) note_modified_actions (identifier);
  
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
vector <Database_Notes_Text> Database_Notes::get_possible_statuses ()
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


vector <string> Database_Notes::standard_severities ()
{
  return {"Wish", "Minor", "Normal", "Important", "Major", "Critical"};
}


// Returns the severity of a note as a number.
int Database_Notes::get_raw_severity (int identifier)
{
  string severity = get_field (identifier, severity_key ());
  if (severity.empty ()) return 2;
  return convert_to_int (severity);
}


// Returns the severity of a note as a localized string.
string Database_Notes::get_severity (int identifier)
{
  int severity = get_raw_severity (identifier);
  vector <string> standard = standard_severities ();
  string severitystring;
  if ((severity >= 0) && (severity < (int)standard.size())) severitystring = standard [severity];
  if (severitystring.empty()) severitystring = "Normal";
  severitystring = translate (severitystring.c_str());
  return severitystring;
}


// Sets the severity of the note identified by identifier.
// severity is a number.
void Database_Notes::set_raw_severity (int identifier, int severity)
{
  // Update the file system.
  set_field (identifier, severity_key (), convert_to_string (severity));
  
  note_modified_actions (identifier);
  
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
vector <Database_Notes_Text> Database_Notes::get_possible_severities ()
{
  vector <string> standard = standard_severities ();
  vector <Database_Notes_Text> severities;
  for (size_t i = 0; i < standard.size(); i++) {
    Database_Notes_Text severity;
    severity.raw = convert_to_string (i);
    severity.localized = translate (standard[i].c_str());
    severities.push_back (severity);
  }
  return severities;
}


int Database_Notes::get_modified (int identifier)
{
  string modified = get_field (identifier, modified_key ());
  if (modified.empty ()) return 0;
  return convert_to_int (modified);
}


void Database_Notes::set_modified (int identifier, int time)
{
  // Update the filesystem.
  set_field (identifier, modified_key (), convert_to_string (time));
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
  update_checksum (identifier);
}


bool Database_Notes::get_public (int identifier)
{
  string value = get_field (identifier, public_key ());
  return convert_to_bool (value);
}


void Database_Notes::set_public (int identifier, bool value)
{
  set_field (identifier, public_key (), convert_to_string (value));
}


// Takes actions when a note has been edited.
void Database_Notes::note_modified_actions (int identifier)
{
  // Update 'modified' field.
  set_modified (identifier, filter_date_seconds_since_epoch());
}


void Database_Notes::update_search_fields (int identifier)
{
  // The search field is a combination of the summary and content converted to clean text.
  // It enables us to search with wildcards before and after the search query.
  string noteSummary = get_summary (identifier);
  string noteContents = get_contents (identifier);
  string cleanText = noteSummary + "\n" + filter_string_html2text (noteContents);
  // Bail out if the search field is already up to date.
  if (cleanText == get_search_field (identifier)) return;
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


string Database_Notes::get_search_field (int identifier)
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
vector <int> Database_Notes::search_notes (string search, const vector <string> & bibles)
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


void Database_Notes::mark_for_deletion (int identifier)
{
  // Delete after 7 days.
  set_field (identifier, expiry_key (), "7");
}


void Database_Notes::unmark_for_deletion (int identifier)
{
  set_field (identifier, expiry_key (), "");
}


bool Database_Notes::is_marked_for_deletion (int identifier)
{
  string expiry = get_field (identifier, expiry_key ());
  return !expiry.empty ();
}


void Database_Notes::touch_marked_for_deletion ()
{
  vector <int> identifiers = get_identifiers ();
  for (auto & identifier : identifiers) {
    if (is_marked_for_deletion (identifier)) {
      string expiry = get_field (identifier, expiry_key ());
      int days = convert_to_int (expiry);
      days--;
      set_field (identifier, expiry_key (), convert_to_string (days));
    }
  }
}


vector <int> Database_Notes::get_due_for_deletion ()
{
  vector <int> deletes;
  vector <int> identifiers = get_identifiers ();
  for (auto & identifier : identifiers) {
    if (is_marked_for_deletion (identifier)) {
      string sdays = get_field (identifier, expiry_key ());
      int idays = convert_to_int (sdays);
      if ((sdays == "0") || (idays < 0)) {
        deletes.push_back (identifier);
      }
    }
  }
  return deletes;
}


// Writes the checksum for note identifier in the database.
void Database_Notes::set_checksum (int identifier, const string & checksum)
{
  // Do not write the checksum if it is already up to date.
  if (checksum == get_checksum (identifier)) return;
  // Write the checksum to the database.
  delete_checksum (identifier);
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
string Database_Notes::get_checksum (int identifier)
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
void Database_Notes::delete_checksum (int identifier)
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


// The function calculates the checksum of the note signature,
// and writes it to the filesystem.
void Database_Notes::update_checksum (int identifier)
{
  // Read the raw data from disk to speed up checksumming.
  string checksum;
  checksum.append ("modified");
  checksum.append (get_field (identifier, modified_key ()));
  checksum.append ("assignees");
  checksum.append (get_field (identifier, assigned_key ()));
  checksum.append ("subscribers");
  checksum.append (get_field (identifier, subscriptions_key ()));
  checksum.append ("bible");
  checksum.append (get_field (identifier, bible_key ()));
  checksum.append ("passages");
  checksum.append (get_field (identifier, passage_key ()));
  checksum.append ("status");
  checksum.append (get_field (identifier, status_key ()));
  checksum.append ("severity");
  checksum.append (get_field (identifier, severity_key ()));
  checksum.append ("summary");
  checksum.append (get_field (identifier, summary_key ()));
  checksum.append ("contents");
  checksum.append (get_field (identifier, contents_key ()));
  checksum = md5 (checksum);
  set_checksum (identifier, checksum);
}


// Queries the database for the checksum for the notes given in the list of $identifiers.
string Database_Notes::get_multiple_checksum (const vector <int> & identifiers)
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
vector <int> Database_Notes::get_notes_in_range_for_bibles (int lowId, int highId, vector <string> bibles, bool anybible)
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
  return filter_url_create_root_path_cpp17_Todo ({database_logic_databases (), "notes.busy"});
}


// Sets whether the notes databases are available, as a boolean.
void Database_Notes::set_availability (bool available)
{
  if (available) {
    filter_url_unlink_cpp17 (availability_flag ());
  } else {
   filter_url_file_put_contents (availability_flag (), "");
  }
}


// Returns whether the notes databases are available, as a boolean.
bool Database_Notes::available ()
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
string Database_Notes::get_bulk (vector <int> identifiers)
{
  // JSON container for the bulk notes.
  Array bulk;
  // Go through all the notes.
  for (auto identifier : identifiers) {
    // JSON object for the note.
    Object note;
    // Add all the fields of the note.
    string assigned = get_field (identifier, assigned_key ());
    note << "a" << assigned;
    string bible = get_bible (identifier);;
    note << "b" << bible;
    string contents = get_contents (identifier);
    note << "c" << contents;
    note << "i" << identifier;
    int modified = get_modified (identifier);
    note << "m" << modified;
    string passage = get_raw_passage (identifier);
    note << "p" << passage;
    string subscriptions = get_field (identifier, subscriptions_key ());
    note << "sb" << subscriptions;
    string summary;
    summary = get_summary (identifier);
    note << "sm" << summary;
    string status;
    status = get_raw_status (identifier);
    note << "st" << status;
    int severity = get_raw_severity (identifier);
    note << "sv" << severity;
    // Add the note to the bulk container.
    bulk << note;
  }
  // Resulting JSON string.
  return bulk.json ();
}


// This takes $json and stores all the notes it contains in the filesystem.
vector <string> Database_Notes::set_bulk (string json)
{
  // Container for the summaries that were stored.
  vector <string> summaries;
  
  // Parse the incoming JSON.
  Array bulk;
  bulk.parse (json);
  
  // Go through the notes the JSON contains.
  for (size_t i = 0; i < bulk.size (); i++) {
    
    // Get all the different fields for this note.
    Object note = bulk.get<Object>((int)i);
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
    
    // Store the note in the filesystem.
    string path = note_file (identifier);
    string folder = filter_url_dirname_cpp17 (path);
    filter_url_mkdir (folder);
    Object note2;
    note2 << assigned_key () << assigned;
    note2 << bible_key () << bible;
    note2 << contents_key () << contents;
    note2 << modified_key () << convert_to_string (modified);
    note2 << passage_key () << passage;
    note2 << subscriptions_key () << subscriptions;
    note2 << summary_key () << summary;
    note2 << status_key () << status;
    note2 << severity_key () << convert_to_string (severity);
    string json = note2.json ();
    filter_url_file_put_contents (path, json);
    
    // Update the indexes.
    update_database (identifier);
    update_search_fields (identifier);
    update_checksum (identifier);
  }
  
  // Container with all the summaries of the notes that were stored.
  return summaries;
}


// Gets a field from a note in JSON format.
string Database_Notes::get_field (int identifier, string key)
{
  string file = note_file (identifier);
  string json = filter_url_file_get_contents (file);
  Object note;
  note.parse (json);
  string value;
  if (note.has<String> (key)) value = note.get<String> (key);
  return value;
}


// Sets a field in a note in JSON format.
void Database_Notes::set_field (int identifier, string key, string value)
{
  string file = note_file (identifier);
  string json = filter_url_file_get_contents (file);
  Object note;
  note.parse (json);
  note << key << value;
  json = note.json ();
  filter_url_file_put_contents (file, json);
}


string Database_Notes::bible_key ()
{
  return "bible";
}


string Database_Notes::passage_key ()
{
  return "passage";
}


string Database_Notes::status_key ()
{
  return "status";
}


string Database_Notes::severity_key ()
{
  return "severity";
}


string Database_Notes::modified_key ()
{
  return "modified";
}


string Database_Notes::summary_key ()
{
  return "summary";
}


string Database_Notes::contents_key ()
{
  return "contents";
}


string Database_Notes::subscriptions_key ()
{
  return "subscriptions";
}


string Database_Notes::assigned_key ()
{
  return "assigned";
}


string Database_Notes::expiry_key ()
{
  return "expiry";
}


string Database_Notes::public_key ()
{
  return "public";
}
