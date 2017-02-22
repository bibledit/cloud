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
 Storing notes, now being done as separate files, each file taking up a default space,
 will become more efficient, that is, take up less space,
 when each note is stored as a separate SQLite database.
 On a Linux server, one notes takes up 32kbytes, and a lot of that space is wasted.
 If one notes was stored in one file, it would take up 4kbytes.
 That is a difference of 8 times.
 This would be a possible improvement.
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
  if (false) {
    translate("New");
    translate("Pending");
    translate("In progress");
    translate("Done");
    translate("Reopened");
  }

  // Enter the standard severities in the list of translatable strings.
  if (false) {
    translate("Wish");
    translate("Minor");
    translate("Normal");
    translate("Important");
    translate("Major");
    translate("Critical");
  }
}


string Database_Notes::database_path ()
{
  return filter_url_create_root_path ("databases", "notes.sqlite");
}


string Database_Notes::checksums_database_path ()
{
  return filter_url_create_root_path ("databases", "notes_checksums.sqlite");
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
  filter_url_unlink (database_path ());
  create ();
  return true;
}


// Does a checkup on the health of the checksums database.
// Optionally recreates it.
// Returns true if to synced, else false.
bool Database_Notes::checkup_checksums ()
{
  if (checksums_healthy ()) return false;
  filter_url_unlink (checksums_database_path ());
  create ();
  return true;
}


void Database_Notes::trim ()
{
  // Clean empty directories.
  string message = "Deleting empty notes folder ";
  string mainfolder = mainFolder ();
  vector <string> bits1 = filter_url_scandir (mainfolder);
  for (auto bit1 : bits1) {
    if (bit1.length () == 3) {
      string folder = filter_url_create_path (mainfolder, bit1);
      vector <string> bits2 = filter_url_scandir (folder);
      if (bits2.empty ()) {
        Database_Logs::log (message + folder);
        remove (folder.c_str ());
      }
      for (auto bit2 : bits2) {
        if (bit2.length () == 3) {
          string folder = filter_url_create_path (mainfolder, bit1, bit2);
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
  touchMarkedForDeletion ();
  vector <int> identifiers = getDueForDeletion ();
  for (auto & identifier : identifiers) {
    trash_consultation_note (webserver_request, identifier);
    erase (identifier);
  }
}


void Database_Notes::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM notes;");
  database_sqlite_disconnect (db);
}


void Database_Notes::sync ()
{
  string mainfolder = mainFolder ();

  // List of notes in the filesystem.
  vector <int> identifiers;

  vector <string> bits1 = filter_url_scandir (mainfolder);
  for (auto & bit1 : bits1) {
    // Bit1/2/3 may start with a 0, so conversion to int cannot be used, rather use a length of 3.
    // It used conversion to int before to determine it was a real note,
    // with the result that it missed 10% of the notes, which subsequently got deleted, oops!
    if (bit1.length () == 3) {
      vector <string> bits2 = filter_url_scandir (filter_url_create_path (mainfolder, bit1));
      for (auto & bit2 : bits2) {
        if (bit2.length () == 3) {
          vector <string> bits3 = filter_url_scandir (filter_url_create_path (mainfolder, bit1, bit2));
          for (auto & bit3 : bits3) {
            if (bit3.length () == 3) {
              int identifier = convert_to_int (bit1 + bit2 + bit3);
              identifiers.push_back (identifier);
              updateDatabase (identifier);
              updateSearchFields (identifier);
              updateChecksum (identifier);
            }
          }
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
      deleteChecksum (id);
    }
  }
  
}


void Database_Notes::updateDatabase (int identifier)
{
  // Read the relevant values from the filesystem.
  int modified = getModified (identifier);

  string file = assignedFile (identifier);
  string assigned = filter_url_file_get_contents (file);

  file = subscriptionsFile (identifier);
  string subscriptions = filter_url_file_get_contents (file);

  string bible = getBible (identifier);

  string passage = getRawPassage (identifier);

  string status = getRawStatus (identifier);

  int severity = getRawSeverity (identifier);

  string summary = getSummary (identifier);

  string contents = getContents (identifier);

  // Read the relevant values from the database.
  // If all the values in the database are the same as the values on the filesystem,
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


string Database_Notes::mainFolder ()
{
  return filter_url_create_root_path ("consultations");
}


string Database_Notes::noteFolder (int identifier)
{
  // The maximum number of folders a folder may contain is constrained by the filesystem.
  // To overcome this, the notes will be stored in a deep folder structure.
  string sidentifier = convert_to_string (identifier);
  string bit1 = sidentifier.substr (0, 3);
  string bit2 = sidentifier.substr (3, 3);
  string bit3 = sidentifier.substr (6, 3);
  return filter_url_create_path (mainFolder (), bit1, bit2, bit3);
}


string Database_Notes::bibleFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "bible");
}


string Database_Notes::passageFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "passage");
}


string Database_Notes::statusFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "status");
}


string Database_Notes::severityFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "severity");
}


string Database_Notes::modifiedFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "modified");
}


string Database_Notes::summaryFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "summary");
}


string Database_Notes::contentsFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "contents");
}


string Database_Notes::subscriptionsFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "subscriptions");
}


string Database_Notes::assignedFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "assigned");
}


string Database_Notes::publicFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "public");
}


string Database_Notes::expiryFile (int identifier)
{
  return filter_url_create_path (noteFolder (identifier), "expiry");
}


// This checks whether the note identifier exists.
bool Database_Notes::identifierExists (int identifier)
{
  return file_or_dir_exists (noteFolder (identifier));
}


// Update a note's identifier.
// new_identifier is the value given to the note identifier by identifier.
void Database_Notes::setIdentifier (int identifier, int new_identifier)
{
  // Move data on the filesystem.
  erase (new_identifier);
  string file = noteFolder (identifier);
  string newfile = noteFolder (new_identifier);
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


int Database_Notes::getNewUniqueIdentifier ()
{
  int identifier = 0;
  do {
    identifier = filter_string_rand (Notes_Logic::lowNoteIdentifier, Notes_Logic::highNoteIdentifier);
  } while (identifierExists (identifier));
  return identifier;
}


vector <int> Database_Notes::getIdentifiers ()
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


string Database_Notes::assembleContents (int identifier, string contents)
{
  string new_contents;
  new_contents = getContents (identifier);
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
// raw: Import contents as it is. Useful for import from Bibledit-Gtk.
// It returns the identifier of this new note.
int Database_Notes::storeNewNote (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw)
{
  // Create a new identifier.
  int identifier = getNewUniqueIdentifier ();

  // Passage.
  string passage = encodePassage (book, chapter, verse);
  
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
  if (!raw) contents = assembleContents (identifier, contents);
  if ((contents.empty()) && (summary.empty())) return 0;
  
  // Store the note in the file system.
  filter_url_mkdir (noteFolder (identifier));
  filter_url_file_put_contents (bibleFile (identifier), bible);
  filter_url_file_put_contents (passageFile (identifier), passage);
  filter_url_file_put_contents (statusFile (identifier), status);
  filter_url_file_put_contents (severityFile (identifier), convert_to_string (severity));
  filter_url_file_put_contents (summaryFile (identifier), summary);
  filter_url_file_put_contents (contentsFile (identifier), contents);
  
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
  updateSearchFields (identifier);
  noteEditedActions (identifier);

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
vector <int> Database_Notes::selectNotes (vector <string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const string& status_selector, string bible_selector, string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const string& search_text, int limit)
{
  string username = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  vector <int> identifiers;
  // SQL SELECT statement.
  string query = notesSelectIdentifier ();
  // SQL optional fulltext search statement sorted on relevance.
  if (text_selector == 1) {
    query.append (notesOptionalFulltextSearchRelevanceStatement (search_text));
  }
  // SQL FROM ... WHERE statement.
  query.append (notesFromWhereStatement ());
  // Consider passage selector.
  string passage;
  switch (passage_selector) {
    case 0:
      // Select notes that refer to the current verse.
      // It means that the book, the chapter, and the verse, should match.
      passage = encodePassage (book, chapter, verse);
      query.append (" AND passage LIKE '%" + passage + "%' ");
      break;
    case 1:
      // Select notes that refer to the current chapter.
      // It means that the book and the chapter should match.
      passage = encodePassage (book, chapter, -1);
      query.append (" AND passage LIKE '%" + passage + "%' ");
      break;
    case 2:
      // Select notes that refer to the current book.
      // It means that the book should match.
      passage = encodePassage (book, -1, -1);
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
    query.append (notesOptionalFulltextSearchStatement (search_text));
  }
  if (text_selector == 1) {
    // If searching in fulltext mode, notes get ordered on relevance of search hits.
    query.append (notesOrderByRelevanceStatement ());
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


string Database_Notes::getSummary (int identifier)
{
  string file = summaryFile (identifier);
  return filter_url_file_get_contents (file);
}


void Database_Notes::setSummary (int identifier, const string& summary)
{
  // Store authoritative copy in the filesystem.
  string file = summaryFile (identifier);
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
  updateSearchFields (identifier);
  // Update checksum.
  updateChecksum (identifier);
}


string Database_Notes::getContents (int identifier)
{
  string file = contentsFile (identifier);
  return filter_url_file_get_contents (file);
}


void Database_Notes::setContents (int identifier, const string& contents)
{
  // Store in file system.
  string file = contentsFile (identifier);
  filter_url_file_put_contents (file, contents);
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
  updateSearchFields (identifier);
  // Update checksum.
  updateChecksum (identifier);
}


void Database_Notes::erase (int identifier)
{
  // Delete from filesystem.
  string folder = noteFolder (identifier);
  filter_url_rmdir (folder);
  // Update database as well.
  deleteChecksum (identifier);
  SqliteSQL sql;
  sql.add ("DELETE FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


// Add a comment to an exiting note identified by identifier.
void Database_Notes::addComment (int identifier, const string& comment)
{
  // Assemble the new content and store it.
  // This updates the search database also.
  string contents = assembleContents (identifier, comment);
  setContents (identifier, contents);

  // Some triggers.
  noteEditedActions (identifier);
  unmarkForDeletion (identifier);

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
  string user = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  subscribeUser (identifier, user);
}


// Subscribe the user to the note identified by identifier.
void Database_Notes::subscribeUser (int identifier, const string& user)
{
  // If the user already is subscribed to the note, bail out.
  vector <string> subscribers = getSubscribers (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) != subscribers.end()) return;
  // Subscribe user.
  subscribers.push_back (user);
  setSubscribers (identifier, subscribers);
}


// Returns an array with the subscribers to the note identified by identifier.
vector <string> Database_Notes::getSubscribers (int identifier)
{
  string file = subscriptionsFile (identifier);
  string contents = filter_url_file_get_contents (file);
  if (contents.empty()) return {};
  vector <string> subscribers = filter_string_explode (contents, '\n');
  for (auto & subscriber : subscribers) {
    subscriber = filter_string_trim (subscriber);
  }
  return subscribers;
}


void Database_Notes::setSubscribers (int identifier, vector <string> subscribers)
{
  // Add a space at both sides of the subscriber to allow for easier note selection based on note assignment.
  for (auto & subscriber : subscribers) {
    subscriber.insert (0, " ");
    subscriber.append (" ");
  }
  string subscriberstring = filter_string_implode (subscribers, "\n");
  
  // Store them in the filesystem, and remove the file if there's no data to store.
  string file = subscriptionsFile (identifier);
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
  updateChecksum (identifier);
}


// Returns true if user is subscribed to the note identified by identifier.
bool Database_Notes::isSubscribed (int identifier, const string& user)
{
  vector <string> subscribers = getSubscribers (identifier);
  return find (subscribers.begin(), subscribers.end(), user) != subscribers.end();
}


// Unsubscribes the currently logged in user from the note identified by identifier.
void Database_Notes::unsubscribe (int identifier)
{
  string user = ((Webserver_Request *) webserver_request)->session_logic ()->currentUser ();
  unsubscribeUser (identifier, user);
}


// Unsubscribes user from the note identified by identifier.
void Database_Notes::unsubscribeUser (int identifier, const string& user)
{
  // If the user is not subscribed to the note, bail out.
  vector <string> subscribers = getSubscribers (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) == subscribers.end()) return;
  // Unsubscribe user.
  subscribers.erase (remove (subscribers.begin(), subscribers.end(), user), subscribers.end());
  setSubscribers (identifier, subscribers);
}


// Returns an array with all assignees to the notes selection.
// These are the usernames to which one or more notes have been assigned.
// This means that if no notes have been assigned to anybody, it will return an empty array.
// Normally the authoritative copy of the notes is stored in the file system.
// But as retrieving the assignees from the file system would be slow, 
// this function retrieves them from the database.
// Normally the database is in sync with the filesystem.
vector <string> Database_Notes::getAllAssignees (const vector <string>& bibles)
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
vector <string> Database_Notes::getAssignees (int identifier)
{
  // Get the asssignees from the filesystem.
  string file = assignedFile (identifier);
  string assignees = filter_url_file_get_contents (file);
  return getAssigneesInternal (assignees);
}


vector <string> Database_Notes::getAssigneesInternal (string assignees)
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
void Database_Notes::setAssignees (int identifier, vector <string> assignees)
{
  // Add a space at both sides of the assignee to allow for easier note selection based on note assignment.
  for (auto & assignee : assignees) {
    assignee.insert (0, " ");
    assignee.append (" ");
  }
  string assignees_string = filter_string_implode (assignees, "\n");

  // Store the assignees in the filesystem, or remove the file if there's no data to store.
  string file = assignedFile (identifier);
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
  
  noteEditedActions (identifier);
}


// Assign the note identified by identifier to user.
void Database_Notes::assignUser (int identifier, const string& user)
{
  // If the note already is assigned to the user, bail out.
  vector <string> assignees = getAssignees (identifier);
  if (find (assignees.begin (), assignees.end(), user) != assignees.end()) return;
  // Assign the note to the user.
  assignees.push_back (user);
  // Store the whole lot.
  setAssignees (identifier, assignees);
}


// Returns true if the note identified by identifier has been assigned to user.
bool Database_Notes::isAssigned (int identifier, const string& user)
{
  vector <string> assignees = getAssignees (identifier);
  return find (assignees.begin(), assignees.end(), user) != assignees.end();
}


// Unassigns user from the note identified by identifier.
void Database_Notes::unassignUser (int identifier, const string& user)
{
  // If the note is not assigned to the user, bail out.
  vector <string> assignees = getAssignees (identifier);
  if (find (assignees.begin(), assignees.end(), user) == assignees.end()) return;
  // Remove assigned user.
  assignees.erase (remove (assignees.begin(), assignees.end(), user), assignees.end());
  setAssignees (identifier, assignees);
}


string Database_Notes::getBible (int identifier)
{
  string file = bibleFile (identifier);
  return filter_url_file_get_contents (file);
}


void Database_Notes::setBible (int identifier, const string& bible)
{
  // Write the bible to the filesystem, or remove the bible in case there's no data to store.
  string file = bibleFile (identifier);
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

  noteEditedActions (identifier);
}


vector <string> Database_Notes::getAllBibles ()
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
string Database_Notes::encodePassage (int book, int chapter, int verse)
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
Passage Database_Notes::decodePassage (string passage)
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
string Database_Notes::getRawPassage (int identifier)
{
  string file = passageFile (identifier);
  return filter_url_file_get_contents (file);
}


// Returns an array with the passages that the note identified by identifier refers to.
// Each passages is an array (book, chapter, verse).
vector <Passage> Database_Notes::getPassages (int identifier)
{
  string contents = getRawPassage (identifier);
  if (contents.empty()) return {};
  vector <string> lines = filter_string_explode (contents, '\n');
  vector <Passage> passages;
  for (auto & line : lines) {
    if (line.empty()) continue;
    Passage passage = decodePassage (line);
    passages.push_back (passage);
  }
  return passages;
}


// Set the passages for note identifier.
// passages is an array of an array (book, chapter, verse) passages.
// import: If true, just write passages, no further actions.
void Database_Notes::setPassages (int identifier, const vector <Passage>& passages, bool import)
{
  // Format the passages.
  string line;
  for (auto & passage : passages) {
    if (!line.empty ()) line.append ("\n");
    line.append (encodePassage (passage.book, passage.chapter, convert_to_int (passage.verse)));
  }
  // Store it.
  setRawPassage (identifier, line);

  if (!import) noteEditedActions (identifier);
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
void Database_Notes::setRawPassage (int identifier, const string& passage)
{
  // Store the authoritative copy in the filesystem.
  string file = passageFile (identifier);
  filter_url_file_put_contents (file, passage);
  
  // Update the shadow database also.
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
string Database_Notes::getRawStatus (int identifier)
{
  string file = statusFile (identifier);
  return filter_url_file_get_contents (file);
}


// Gets the localized status of a note.
// Returns it as a string.
string Database_Notes::getStatus (int identifier)
{
  string status = getRawStatus (identifier);
  // Localize status if possible.
  status = translate (status.c_str());
  // Return status.
  return status;
}


// Sets the status of the note identified by identifier.
// status is a string.
// import: Just write the status, and skip any logic.
void Database_Notes::setStatus (int identifier, const string& status, bool import)
{
  // Store the authoritative copy in the filesystem.
  string file = statusFile (identifier);
  filter_url_file_put_contents (file, status);

  if (!import) noteEditedActions (identifier);

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
vector <Database_Notes_Text> Database_Notes::getPossibleStatuses ()
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
int Database_Notes::getRawSeverity (int identifier)
{
  string file = severityFile (identifier);
  string severity = filter_url_file_get_contents (file);
  if (severity.empty ()) return 2;
  return convert_to_int (severity);
}


// Returns the severity of a note as a localized string.
string Database_Notes::getSeverity (int identifier)
{
  int severity = getRawSeverity (identifier);
  vector <string> standard = standard_severities ();
  string severitystring;
  if ((severity >= 0) && (severity < (int)standard.size())) severitystring = standard [severity];
  if (severitystring.empty()) severitystring = "Normal";
  severitystring = translate (severitystring.c_str());
  return severitystring;
}


// Sets the severity of the note identified by identifier.
// severity is a number.
void Database_Notes::setRawSeverity (int identifier, int severity)
{
  // Update the file system.
  string file = severityFile (identifier);
  filter_url_file_put_contents (file, convert_to_string (severity));
  
  noteEditedActions (identifier);
  
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
vector <Database_Notes_Text> Database_Notes::getPossibleSeverities ()
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


int Database_Notes::getModified (int identifier)
{
  string file = modifiedFile (identifier);
  string modified = filter_url_file_get_contents (file);
  if (modified.empty ()) return 0;
  return convert_to_int (modified);
}


void Database_Notes::setModified (int identifier, int time)
{
  // Update the filesystem.
  string file = modifiedFile (identifier);
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
  updateChecksum (identifier);
}


bool Database_Notes::getPublic (int identifier)
{
  string file = publicFile (identifier);
  return file_or_dir_exists (file);
}


void Database_Notes::setPublic (int identifier, bool value)
{
  string file = publicFile (identifier);
  if (value) {
    filter_url_file_put_contents (file, "");
  } else {
    filter_url_unlink (file);
  }
}


// Takes actions when a note has been edited.
void Database_Notes::noteEditedActions (int identifier)
{
  // Update 'modified' field.
  setModified (identifier, filter_date_seconds_since_epoch());
}


// Returns an array of duplicate note identifiers selected.
vector <int> Database_Notes::selectDuplicateNotes (const string& rawpassage, const string& summary, const string& contents)
{
  vector <int> identifiers;
  SqliteSQL sql;
  sql.add ("SELECT identifier FROM notes WHERE passage =");
  sql.add (rawpassage);
  sql.add ("AND summary =");
  sql.add (summary);
  sql.add ("AND contents =");
  sql.add (contents);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["identifier"];
  database_sqlite_disconnect (db);
  for (auto & id : result) {
    identifiers.push_back (convert_to_int (id));
  }
  return identifiers;
}


void Database_Notes::updateSearchFields (int identifier)
{
  // The search field is a combination of the summary and content converted to clean text.
  // It enables us to search with wildcards before and after the search query.
  string noteSummary = getSummary (identifier);
  string noteContents = getContents (identifier);
  string cleanText = noteSummary + "\n" + filter_string_html2text (noteContents);
  // Bail out if the search field is already up to date.
  if (cleanText == getSearchField (identifier)) return;
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


string Database_Notes::getSearchField (int identifier)
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
vector <int> Database_Notes::searchNotes (string search, const vector <string> & bibles)
{
  vector <int> identifiers;

  search = filter_string_trim (search);
  if (search == "") return identifiers;

  // SQL SELECT statement.
  string query = notesSelectIdentifier ();

  // SQL fulltext search statement sorted on relevance.
  query.append (notesOptionalFulltextSearchRelevanceStatement (search));

  // SQL FROM ... WHERE statement.
  query.append (notesFromWhereStatement ());

  // Consider text contained in notes.
  query.append (notesOptionalFulltextSearchStatement (search));

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
  query.append (notesOrderByRelevanceStatement ());

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


void Database_Notes::markForDeletion (int identifier)
{
  string file = expiryFile (identifier);
  // Delete after 7 days.
  filter_url_file_put_contents (file, "7");
}


void Database_Notes::unmarkForDeletion (int identifier)
{
  string file = expiryFile (identifier);
  filter_url_unlink (file);
}


bool Database_Notes::isMarkedForDeletion (int identifier)
{
  string file = expiryFile (identifier);
  return file_or_dir_exists (file);
}


void Database_Notes::touchMarkedForDeletion ()
{
  vector <int> identifiers = getIdentifiers ();
  for (auto & identifier : identifiers) {
    if (isMarkedForDeletion (identifier)) {
      string file = expiryFile (identifier);
      int days = convert_to_int (filter_url_file_get_contents (file));
      days--;
      filter_url_file_put_contents (file, convert_to_string (days));
    }
  }
}


vector <int> Database_Notes::getDueForDeletion ()
{
  vector <int> deletes;
  vector <int> identifiers = getIdentifiers ();
  for (auto & identifier : identifiers) {
    if (isMarkedForDeletion (identifier)) {
      string file = expiryFile (identifier);
      string sdays = filter_url_file_get_contents (file);
      int idays = convert_to_int (sdays);
      if ((sdays == "0") || (idays < 0)) {
        deletes.push_back (identifier);
      }
    }
  }
  return deletes;
}


// Writes the checksum for note identifier in the database.
void Database_Notes::setChecksum (int identifier, const string & checksum)
{
  // Do not write the checksum if it is already up to date.
  if (checksum == getChecksum (identifier)) return;
  // Write the checksum to the database.
  deleteChecksum (identifier);
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
string Database_Notes::getChecksum (int identifier)
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
void Database_Notes::deleteChecksum (int identifier)
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
void Database_Notes::updateChecksum (int identifier)
{
  // Read the raw data from disk to speed up checksumming.
  string checksum;
  checksum.append ("modified");
  checksum.append (filter_url_file_get_contents (modifiedFile (identifier)));
  checksum.append ("assignees");
  checksum.append (filter_url_file_get_contents (assignedFile (identifier)));
  checksum.append ("subscribers");
  checksum.append (filter_url_file_get_contents (subscriptionsFile (identifier)));
  checksum.append ("bible");
  checksum.append (filter_url_file_get_contents (bibleFile (identifier)));
  checksum.append ("passages");
  checksum.append (filter_url_file_get_contents (passageFile (identifier)));
  checksum.append ("status");
  checksum.append (filter_url_file_get_contents (statusFile (identifier)));
  checksum.append ("severity");
  checksum.append (filter_url_file_get_contents (severityFile (identifier)));
  checksum.append ("summary");
  checksum.append (filter_url_file_get_contents (summaryFile (identifier)));
  checksum.append ("contents");
  checksum.append (filter_url_file_get_contents (contentsFile (identifier)));
  checksum = md5 (checksum);
  setChecksum (identifier, checksum);
}


// Queries the database for the checksum for the notes given in the list of $identifiers.
string Database_Notes::getMultipleChecksum (const vector <int> & identifiers)
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


// Internal function.
string Database_Notes::getBibleSelector (vector <string> bibles)
{
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
  return bibleSelector;
}


// This function gets the identifiers for notes
// within the note identifier range of lowId to highId
// which refer to any Bible in the array of bibles
// or refer to no Bible.
vector <int> Database_Notes::getNotesInRangeForBibles (int lowId, int highId, const vector <string> & bibles, bool anybible)
{
  vector <int> identifiers;
  string bibleSelector = getBibleSelector (bibles);
  string query = "SELECT identifier FROM notes WHERE identifier >= ";
  query.append (convert_to_string (lowId));
  query.append (" AND identifier <= ");
  query.append (convert_to_string (highId));
  query.append (" ");
  if (!anybible) query.append (bibleSelector);
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
void Database_Notes::set_availability (bool available)
{
  if (available) {
    filter_url_unlink (availability_flag ());
  } else {
   filter_url_file_put_contents (availability_flag (), "");
  }
}


// Returns whether the notes databases are available, as a boolean.
bool Database_Notes::available ()
{
  return !file_or_dir_exists (availability_flag ());
}


string Database_Notes::notesSelectIdentifier ()
{
  return " SELECT identifier ";
}


string Database_Notes::notesOptionalFulltextSearchRelevanceStatement (string search)
{
  if (search == "") return "";
  search = filter_string_str_replace (",", "", search);
  search = database_sqlite_no_sql_injection (search);
  string query = "";
  return query;
}


string Database_Notes::notesFromWhereStatement ()
{
  return " FROM notes WHERE 1 ";
}


string Database_Notes::notesOptionalFulltextSearchStatement (string search)
{
  if (search == "") return "";
  search = filter_string_str_replace (",", "", search);
  search = database_sqlite_no_sql_injection (search);
  string query = " AND cleantext LIKE '%" + search + "%' ";
  return query;
}


string Database_Notes::notesOrderByRelevanceStatement ()
{
  return "";
}


// This returns JSON that contains the notes indicated by $identifiers.
string Database_Notes::getBulk (vector <int> identifiers)
{
  // JSON container for the bulk notes.
  Array bulk;
  // Go through all the notes.
  for (auto identifier : identifiers) {
    // JSON object for the note.
    Object note;
    // Add all the fields of the note.
    string assigned = filter_url_file_get_contents (assignedFile (identifier));
    note << "a" << assigned;
    string bible = getBible (identifier);
    note << "b" << bible;
    string contents = getContents (identifier);
    note << "c" << contents;
    note << "i" << identifier;
    int modified = getModified (identifier);
    note << "m" << modified;
    string passage = getRawPassage (identifier);
    note << "p" << passage;
    string subscriptions = filter_url_file_get_contents (subscriptionsFile (identifier));
    note << "sb" << subscriptions;
    string summary = getSummary (identifier);
    note << "sm" << summary;
    string status = getRawStatus (identifier);
    note << "st" << status;
    int severity = getRawSeverity (identifier);
    note << "sv" << severity;
    // Add the note to the bulk container.
    bulk << note;
  }
  // Resulting JSON string.
  return bulk.json ();
}


// This takes $json and stores all the notes it contains in the filesystem.
vector <string> Database_Notes::setBulk (string json)
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
    filter_url_mkdir (noteFolder (identifier));
    filter_url_file_put_contents (assignedFile (identifier), assigned);
    filter_url_file_put_contents (bibleFile (identifier), bible);
    filter_url_file_put_contents (contentsFile (identifier), contents);
    filter_url_file_put_contents (modifiedFile (identifier), convert_to_string (modified));
    filter_url_file_put_contents (passageFile (identifier), passage);
    filter_url_file_put_contents (severityFile (identifier), convert_to_string (severity));
    filter_url_file_put_contents (statusFile (identifier), status);
    filter_url_file_put_contents (subscriptionsFile (identifier), subscriptions);
    filter_url_file_put_contents (summaryFile (identifier), summary);

    // Update the indexes.
    updateDatabase (identifier);
    updateSearchFields (identifier);
    updateChecksum (identifier);
  }
  
  // Container with all the summaries of the notes that were stored.
  return summaries;
}
