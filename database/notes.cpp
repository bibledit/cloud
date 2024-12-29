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
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <jsonxx/jsonxx.h>
#pragma GCC diagnostic pop
#include <database/logic.h>
#include <time.h>


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
 In older versions, on a Linux server, one note took 32 kbytes.
 A lot of that space is wasted.
 In newer versions one notes takes only 4 kbytes.
 That is a difference of 8 times.
*/


constexpr const auto database_notes {"notes"};
constexpr const auto database_notes_checksums {"notes_checksums"};


Database_Notes::Database_Notes (Webserver_Request& webserver_request):
m_webserver_request (webserver_request)
{
}


void Database_Notes::create ()
{
  // Create the main database and table.
  {
    SqliteDatabase sql (database_notes);
    sql.set_sql ("CREATE TABLE IF NOT EXISTS notes ("
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
                 ");");
    sql.execute ();
  }

  // Create the database and table for the checksums.
  // A general reason for having this separate is robustness.
  // A specific reason for this is that when the main notes database is being repaired,
  // and several clients keep reading it, it may disrupt the repair.
  {
    SqliteDatabase sql (database_notes_checksums);
    sql.set_sql ("CREATE TABLE IF NOT EXISTS checksums ("
                 " identifier integer,"
                 " checksum checksum"
                 ");");
    sql.execute ();
  }

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


std::string Database_Notes::database_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "notes.sqlite"});
}


std::string Database_Notes::checksums_database_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "notes_checksums.sqlite"});
}


// Returns whether the notes database is healthy, as a boolean.
bool Database_Notes::healthy ()
{
  return database::sqlite::healthy (database_path ());
}


// Returns whether the notes checksums database is healthy, as a boolean.
bool Database_Notes::checksums_healthy ()
{
  return database::sqlite::healthy (checksums_database_path ());
}


// Does a checkup on the health of the main database.
// Optionally recreates it.
// Returns true if to be synced, else false.
bool Database_Notes::checkup ()
{
  if (healthy ()) 
    return false;
  filter_url_unlink (database_path ());
  create ();
  return true;
}


// Does a checkup on the health of the checksums database.
// Optionally recreates it.
// Returns true if to synced, else false.
bool Database_Notes::checkup_checksums ()
{
  if (checksums_healthy ()) 
    return false;
  filter_url_unlink (checksums_database_path ());
  create ();
  return true;
}


void Database_Notes::trim ()
{
  // Clean empty directories.
  const std::string message = "Deleting empty notes folder ";
  const std::string main_folder = main_folder_path ();
  const std::vector <std::string> bits1 = filter_url_scandir (main_folder);
  for (const auto& bit1 : bits1) {
    if (bit1.length () == 3) {
      const std::string folder1 = filter_url_create_path ({main_folder, bit1});
      const std::vector <std::string> bits2 = filter_url_scandir (folder1);
      if (bits2.empty ()) {
        Database_Logs::log (message + folder1);
        remove (folder1.c_str ());
      }
      for (const auto& bit2 : bits2) {
        if (bit2.length () == 3) {
          const std::string folder2 = filter_url_create_path ({main_folder, bit1, bit2});
          const std::vector <std::string> bits3 = filter_url_scandir (folder2);
          if (bits3.empty ()) {
            Database_Logs::log (message + folder2);
            remove (folder2.c_str());
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
  // Storage for notes to be deleted.
  const std::vector <int> identifiers = get_due_for_deletion ();
  for (const auto identifier : identifiers) {
    trash_consultation_note (m_webserver_request, identifier);
    erase (identifier);
  }
}


void Database_Notes::optimize ()
{
  SqliteDatabase sql (database_notes);
  sql.set_sql ("VACUUM;");
  sql.execute();
}


void Database_Notes::sync ()
{
  const std::string main_folder = main_folder_path ();

  // The good notes in the filesystem.
  std::vector <int> good_note_ids;

  // Gather the notes from the filesystem and update indices.
  const std::vector <std::string> bits1 = filter_url_scandir (main_folder);
  for (const auto& bit1 : bits1) {
    // Bit 1 / 2 / 3 may start with a 0, so conversion to int cannot be used, rather use a length of 3.
    // It used conversion to int before to determine it was a real note,
    // with the result that it missed 10% of the notes, which subsequently got deleted, oops!
    if (bit1.length () == 3) {
      const std::vector <std::string> bits2 = filter_url_scandir (filter_url_create_path ({main_folder, bit1}));
      for (const auto& bit2 : bits2) {
        // Old storage mechanism, e.g. folder "425".
        if (bit2.length () == 3) {
          const std::vector <std::string> bits3 = filter_url_scandir (filter_url_create_path ({main_folder, bit1, bit2}));
          for (const auto& bit3 : bits3) {
            if (bit3.length () == 3) {
              const int identifier = filter::strings::convert_to_int (bit1 + bit2 + bit3);
              good_note_ids.push_back (identifier);
              update_search_fields (identifier);
            }
          }
        }
        // New JSON storage mechanism, e.g. file "894093.json".
        if ((bit2.length () == 11) && bit2.find (".json") != std::string::npos) {
          const int identifier = filter::strings::convert_to_int (bit1 + bit2.substr (0,6));
          if (get_raw_passage (identifier).empty()) {
            Database_Logs::log ("Damaged consultation note found");
            continue;
          }
          good_note_ids.push_back (identifier);
          update_database (identifier);
          update_search_fields (identifier);
          update_checksum (identifier);
        }
      }
    }
  }

  // Get all identifiers in the main notes index.
  SqliteDatabase sql_notes (database_notes);
  std::vector <int> database_identifiers;
  sql_notes.set_sql ("SELECT identifier FROM notes;");
  std::vector <std::string> result = sql_notes.query () ["identifier"];
  for (auto & id : result) {
    database_identifiers.push_back (filter::strings::convert_to_int (id));
  }

  // Any note identifiers in the main index, and not in the filesystem, remove them.
  for (const auto id : database_identifiers) {
    if (std::find (good_note_ids.cbegin(), good_note_ids.cend(), id) == good_note_ids.cend()) {
      trash_consultation_note (m_webserver_request, id);
      erase (id);
    }
  }
  
  // Get all identifiers in the checksums database.
  SqliteDatabase sql_checksums (database_notes_checksums);
  database_identifiers.clear ();
  sql_checksums.set_sql ("SELECT identifier FROM checksums;");
  result = sql_checksums.query () ["identifier"];
  for (const auto& id : result) {
    database_identifiers.push_back (filter::strings::convert_to_int (id));
  }

  // Any note identifiers in the checksums database, and not in the filesystem, remove them.
  for (const auto id : database_identifiers) {
    if (std::find (good_note_ids.cbegin(), good_note_ids.cend(), id) == good_note_ids.end()) {
      delete_checksum (id);
    }
  }
}


void Database_Notes::update_database (int identifier)
{
  // Read the relevant values from the filesystem.
  const int modified = get_modified (identifier);
  const std::string assigned = get_field (identifier, assigned_key ());
  const std::string subscriptions = get_field (identifier, subscriptions_key ());
  const std::string bible = get_bible (identifier);
  const std::string passage = get_raw_passage (identifier);
  const std::string status = get_raw_status (identifier);
  const int severity = get_raw_severity (identifier);
  const std::string summary = get_summary (identifier);
  const std::string contents = get_contents (identifier);
  
  // Sync the values to the database.
  update_database_internal (identifier, modified, assigned, subscriptions, bible, passage, status, severity, summary, contents);
}


void Database_Notes::update_database_internal (int identifier, int modified, std::string assigned, std::string subscriptions, std::string bible, std::string passage, std::string status, int severity, std::string summary, std::string contents)
{
  // Read the relevant values from the database.
  // If all the values in the database are the same as the values in the filesystem,
  // it means that the database is already in sync with the filesystem.
  // Bail out in that case.
  SqliteDatabase sql (database_notes);
  bool database_in_sync = true;
  bool record_in_database = false;
  sql.add ("SELECT modified, assigned, subscriptions, bible, passage, status, severity, summary, contents FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> vmodified = result ["modified"];
  const std::vector <std::string> vassigned = result ["assigned"];
  const std::vector <std::string> vsubscriptions = result ["subscriptions"];
  const std::vector <std::string> vbible = result ["bible"];
  const std::vector <std::string> vpassage = result ["passage"];
  const std::vector <std::string> vstatus = result ["status"];
  const std::vector <std::string> vseverity = result ["severity"];
  const std::vector <std::string> vsummary = result ["summary"];
  const std::vector <std::string> vcontents = result ["contents"];
  for (unsigned int i = 0; i < vmodified.size(); i++) {
    record_in_database = true;
    if (modified != filter::strings::convert_to_int (vmodified[i])) database_in_sync = false;
    if (assigned != vassigned[i]) database_in_sync = false;
    if (subscriptions != vsubscriptions[i]) database_in_sync = false;
    if (bible != vbible [i]) database_in_sync = false;
    if (passage != vpassage [i]) database_in_sync = false;
    if (status != vstatus [i]) database_in_sync = false;
    if (severity != filter::strings::convert_to_int (vseverity [i])) database_in_sync = false;
    if (summary != vsummary [i]) database_in_sync = false;
    if (contents != vcontents [i]) database_in_sync = false;
  }
  if (database_in_sync && record_in_database) return;
  
  // At this stage, the index needs to be brought in sync with the filesystem.
  
  sql.clear ();
  sql.add ("DELETE FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
  
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
  sql.execute ();
}


std::string Database_Notes::main_folder_path ()
{
  return filter_url_create_root_path ({"consultations"});
}


std::string Database_Notes::note_file (int identifier)
{
  // The maximum number of folders a folder may contain is constrained by the filesystem.
  // To overcome this, the notes will be stored in a folder structure.
  const std::string id_path = std::to_string (identifier);
  const std::string folder = id_path.substr (0, 3);
  const std::string file = id_path.substr (3, 6) + ".json";
  return filter_url_create_path ({main_folder_path (), folder, file});
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
  std::string path = note_file (identifier);
  std::string json = filter_url_file_get_contents (path);
  path = note_file (new_identifier);
  std::string folder = filter_url_dirname (path);
  filter_url_mkdir (folder);
  filter_url_file_put_contents (path, json);
  
  // Update main notes database.
  {
    SqliteDatabase sql (database_notes);
    sql.add ("UPDATE notes SET identifier =");
    sql.add (new_identifier);
    sql.add ("WHERE identifier =");
    sql.add (identifier);
    sql.add (";");
    sql.execute ();
  }
  
  // Update checksums database.
  {
    SqliteDatabase sql (database_notes_checksums);
    sql.add ("UPDATE checksums SET identifier =");
    sql.add (new_identifier);
    sql.add ("WHERE identifier =");
    sql.add (identifier);
    sql.add (";");
    sql.execute ();
  }
  
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
    identifier = filter::strings::rand (Notes_Logic::lowNoteIdentifier, Notes_Logic::highNoteIdentifier);
  } while (identifier_exists (identifier));
  return identifier;
}


std::vector <int> Database_Notes::get_identifiers ()
{
  SqliteDatabase sql (database_notes);
  sql.set_sql ("SELECT identifier FROM notes;");
  std::vector <int> identifiers;
  const std::vector <std::string> result = sql.query () ["identifier"];
  for (const auto& id : result) {
    identifiers.push_back (filter::strings::convert_to_int (id));
  }
  return identifiers;
}


std::string Database_Notes::assemble_contents (int identifier, std::string contents)
{
  std::string new_contents = get_contents (identifier);
  std::string datetime = filter::date::localized_date_format (m_webserver_request);
  const std::string& user = m_webserver_request.session_logic ()->get_username ();
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
  std::vector <std::string> lines = filter::strings::explode (contents, '\n');
  for (auto line : lines) {
    line = filter::strings::trim (line);
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
int Database_Notes::store_new_note (const std::string& bible, int book, int chapter, int verse, std::string summary, std::string contents, bool raw)
{
  // Create a new identifier.
  int identifier = get_new_unique_identifier ();
  
  // Passage.
  std::string passage = encode_passage (book, chapter, verse);
  
  std::string status = "New";
  int severity = 2;
  
  // If the summary is not given, take the first line of the contents as the summary.
  if (summary == "") {
    // The notes editor does not put new lines at each line, but instead <div>s. Handle these also.
    summary = filter::strings::replace ("<", "\n", contents);
    std::vector <std::string> bits = filter::strings::explode (summary, '\n');
    if (!bits.empty ()) summary = bits [0];
  }
  
  // Assemble contents.
  if (!raw) contents = assemble_contents (identifier, contents);
  if ((contents.empty()) && (summary.empty())) return 0;
  
  // Store the JSON representation of the note in the file system.
  std::string path = note_file (identifier);
  std::string folder = filter_url_dirname (path);
  filter_url_mkdir (folder);
  jsonxx::Object note;
  note << bible_key () << bible;
  note << passage_key () << passage;
  note << status_key () << status;
  note << severity_key () << std::to_string (severity);
  note << summary_key () << summary;
  note << contents_key () << contents;
  std::string json = note.json ();
  filter_url_file_put_contents (path, json);
  
  // Store new default note into the database.
  {
    SqliteDatabase sql (database_notes);
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
    sql.execute ();
  }
  
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
std::vector <int> Database_Notes::select_notes (std::vector <std::string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const std::string& status_selector, std::string bible_selector, std::string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const std::string& search_text, int limit)
{
  const std::string& username = m_webserver_request.session_logic ()->get_username ();
  std::vector <int> identifiers;
  // SQL SELECT statement.
  std::string query = notes_select_identifier ();
  // SQL optional fulltext search statement sorted on relevance.
  if (text_selector == 1) {
    query.append (notes_optional_fulltext_search_relevance_statement (search_text));
  }
  // SQL FROM ... WHERE statement.
  query.append (notes_from_where_statement ());
  // Consider passage selector.
  std::string passage;
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
    default: break;
  }
  // Consider edit selector.
  int time { 0 };
  switch (edit_selector) {
    case 0:
      // Select notes that have been edited at any time. Apply no constraint.
      time = 0;
      break;
    case 1:
      // Select notes that have been edited during the last 30 days.
      time = filter::date::seconds_since_epoch () - 30 * 24 * 3600;
      break;
    case 2:
      // Select notes that have been edited during the last 7 days.
      time = filter::date::seconds_since_epoch () - 7 * 24 * 3600;
      break;
    case 3:
      // Select notes that have been edited since yesterday.
      time = filter::date::seconds_since_epoch () - 1 * 24 * 3600 - filter::date::numerical_hour (filter::date::seconds_since_epoch ()) * 3600;
      break;
    case 4:
      // Select notes that have been edited today.
      time = filter::date::seconds_since_epoch () - filter::date::numerical_hour (filter::date::seconds_since_epoch ()) * 3600;
      break;
    default: break;
  }
  if (time != 0) {
    query.append (" AND modified >= ");
    query.append (std::to_string (time));
    query.append (" ");
  }
  // Consider non-edit selector.
  int nonedit { 0 };
  switch (non_edit_selector) {
    case 0:
      // Select notes that have not been edited at any time. Apply no constraint.
      nonedit = 0;
      break;
    case 1:
      // Select notes that have not been edited for a day.
      nonedit = filter::date::seconds_since_epoch () - 1 * 24 * 3600;
      break;
    case 2:
      // Select notes that have not been edited for two days.
      nonedit = filter::date::seconds_since_epoch () - 2 * 24 * 3600;
      break;
    case 3:
      // Select notes that have not been edited for a week.
      nonedit = filter::date::seconds_since_epoch () - 7 * 24 * 3600;
      break;
    case 4:
      // Select notes that have not been edited for a month.
      nonedit = filter::date::seconds_since_epoch () - 30 * 24 * 3600;
      break;
    case 5:
      // Select notes that have not been edited for a year.
      nonedit = filter::date::seconds_since_epoch () - 365 * 24 * 3600;
      break;
    default: break;
  }
  if (nonedit != 0) {
    query.append (" AND modified <= ");
    query.append (std::to_string (nonedit));
    query.append (" ");
  }
  // Consider status constraint.
  if (status_selector != "") {
    query.append (" AND status = '");
    query.append (database::sqlite::no_sql_injection (status_selector));
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
      bible = database::sqlite::no_sql_injection (bible);
      query.append (" OR bible = '");
      query.append (bible);
      query.append ("' ");
    }
    query.append (" ) ");
  }
  // Consider note assignment constraints.
  if (assignment_selector != "") {
    assignment_selector = database::sqlite::no_sql_injection (assignment_selector);
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
    query.append (std::to_string (severity_selector));
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
    query.append (std::to_string (limit));
    query.append (", 50 ");
  }
  query.append (";");

  SqliteDatabase sql (database_notes);
  sql.set_sql (query);
  const std::vector <std::string> result = sql.query () ["identifier"];
  for (const auto& id : result) {
    identifiers.push_back (filter::strings::convert_to_int (id));
  }
  return identifiers;
}


std::string Database_Notes::get_summary (int identifier)
{
  return get_field (identifier, summary_key ());
}


void Database_Notes::set_summary (int identifier, const std::string& summary)
{
  // Store authoritative copy in the filesystem.
  set_field (identifier, summary_key (), summary);
  // Update the shadow database.
  {
    SqliteDatabase sql (database_notes);
    sql.add ("UPDATE notes SET summary =");
    sql.add (summary);
    sql.add ("WHERE identifier =");
    sql.add (identifier);
    sql.add (";");
    sql.execute ();
  }
  // Update the search data in the database.
  update_search_fields (identifier);
  // Update checksum.
  update_checksum (identifier);
}


std::string Database_Notes::get_contents (int identifier)
{
  return get_field (identifier, contents_key ());
}


void Database_Notes::set_raw_contents (int identifier, const std::string& contents)
{
  set_field (identifier, contents_key (), contents);
}


void Database_Notes::set_contents (int identifier, const std::string& contents)
{
  // Store in file system.
  set_raw_contents (identifier, contents);
  {
    // Update database.
    SqliteDatabase sql (database_notes);
    sql.add ("UPDATE notes SET contents =");
    sql.add (contents);
    sql.add ("WHERE identifier =");
    sql.add (identifier);
    sql.add (";");
    sql.execute ();
  }
  // Update search system.
  update_search_fields (identifier);
  // Update checksum.
  update_checksum (identifier);
}


// Erases a note stored in the old and in the new format.
void Database_Notes::erase (int identifier)
{
  // Delete new storage from filesystem.
  const std::string path = note_file (identifier);
  filter_url_unlink (path);
  // Update databases as well.
  delete_checksum (identifier);
  SqliteDatabase sql (database_notes);
  sql.add ("DELETE FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


// Add a comment to an exiting note identified by $identifier.
void Database_Notes::add_comment (int identifier, const std::string& comment)
{
  // Assemble the new content and store it.
  // This updates the search database also.
  const std::string contents = assemble_contents (identifier, comment);
  set_contents (identifier, contents);
  
  // Some triggers.
  note_modified_actions (identifier);
  unmark_for_deletion (identifier);
  
  // Update shadow database.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET contents =");
  sql.add (contents);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


// Subscribe the current user to the note identified by identifier.
void Database_Notes::subscribe (int identifier)
{
  const std::string& user = m_webserver_request.session_logic ()->get_username ();
  subscribe_user (identifier, user);
}


// Subscribe the user to the note identified by identifier.
void Database_Notes::subscribe_user (int identifier, const std::string& user)
{
  // If the user already is subscribed to the note, bail out.
  std::vector <std::string> subscribers = get_subscribers (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) != subscribers.end()) return;
  // Subscribe user.
  subscribers.push_back (user);
  set_subscribers (identifier, subscribers);
}


// Returns an array with the subscribers to the note identified by identifier.
std::vector <std::string> Database_Notes::get_subscribers (int identifier)
{
  const std::string contents = get_raw_subscriptions (identifier);
  if (contents.empty()) return {};
  std::vector <std::string> subscribers = filter::strings::explode (contents, '\n');
  for (auto& subscriber : subscribers) {
    subscriber = filter::strings::trim (subscriber);
  }
  return subscribers;
}


std::string Database_Notes::get_raw_subscriptions (int identifier)
{
  return get_field (identifier, subscriptions_key ());
}


void Database_Notes::set_raw_subscriptions (int identifier, const std::string& subscriptions)
{
  // Store them in the filesystem.
  set_field (identifier, subscriptions_key (), subscriptions);
  
  // Store them in the database as well.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET subscriptions =");
  sql.add (subscriptions);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


void Database_Notes::set_subscribers (int identifier, std::vector <std::string> subscribers)
{
  // Add a space at both sides of the subscriber to allow for easier note selection based on note assignment.
  for (auto & subscriber : subscribers) {
    subscriber.insert (0, " ");
    subscriber.append (" ");
  }
  std::string subscriberstring = filter::strings::implode (subscribers, "\n");
  
  // Store them to file and in the database.
  set_raw_subscriptions (identifier, subscriberstring);
  
  // Checksum.
  update_checksum (identifier);
}


// Returns true if user is subscribed to the note identified by identifier.
bool Database_Notes::is_subscribed (int identifier, const std::string& user)
{
  std::vector <std::string> subscribers = get_subscribers (identifier);
  return find (subscribers.begin(), subscribers.end(), user) != subscribers.end();
}


// Unsubscribes the currently logged in user from the note identified by identifier.
void Database_Notes::unsubscribe (int identifier)
{
  const std::string& user = m_webserver_request.session_logic ()->get_username ();
  unsubscribe_user (identifier, user);
}


// Unsubscribes user from the note identified by identifier.
void Database_Notes::unsubscribe_user (int identifier, const std::string& user)
{
  // If the user is not subscribed to the note, bail out.
  std::vector <std::string> subscribers = get_subscribers (identifier);
  if (find (subscribers.begin(), subscribers.end(), user) == subscribers.end()) return;
  // Unsubscribe user.
  subscribers.erase (remove (subscribers.begin(), subscribers.end(), user), subscribers.end());
  set_subscribers (identifier, subscribers);
}


std::string Database_Notes::get_raw_assigned (int identifier)
{
  // Get the asssignees from the filesystem.
  return get_field (identifier, assigned_key ());
}


void Database_Notes::set_raw_assigned (int identifier, const std::string& assigned)
{
  // Store the assignees in the filesystem.
  set_field (identifier, assigned_key (), assigned);
  
  // Store the assignees in the database also.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET assigned =");
  sql.add (assigned);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


// Returns an array with all assignees to the notes selection.
// These are the usernames to which one or more notes have been assigned.
// This means that if no notes have been assigned to anybody, it will return an empty array.
// Normally the authoritative copy of the notes is stored in the file system.
// But as retrieving the assignees from the file system would be slow, 
// this function retrieves them from the database.
// Normally the database is in sync with the filesystem.
std::vector <std::string> Database_Notes::get_all_assignees (const std::vector <std::string>& bibles)
{
  std::set <std::string> unique_assignees;
  SqliteDatabase sql (database_notes);
  sql.add ("SELECT DISTINCT assigned FROM notes WHERE bible = ''");
  for (const auto& bible : bibles) {
    sql.add ("OR bible =");
    sql.add (bible);
  }
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["assigned"];
  for (const auto& item : result) {
    if (item.empty ()) continue;
    std::vector <std::string> names = filter::strings::explode (item, '\n');
    for (const auto& name : names) unique_assignees.insert (name);
  }
  
  std::vector <std::string> assignees (unique_assignees.begin(), unique_assignees.end());
  for (auto& assignee : assignees) {
    assignee = filter::strings::trim (assignee);
  }
  return assignees;
}


// Returns an array with the assignees to the note identified by identifier.
std::vector <std::string> Database_Notes::get_assignees (int identifier)
{
  // Get the asssignees from the filesystem.
  std::string assignees = get_raw_assigned (identifier);
  return get_assignees_internal (assignees);
}


std::vector <std::string> Database_Notes::get_assignees_internal (std::string assignees)
{
  if (assignees.empty ()) return {};
  std::vector <std::string> assignees_vector = filter::strings::explode (assignees, '\n');
  // Remove the padding space at both sides of the assignee.
  for (auto & assignee : assignees_vector) {
    assignee = filter::strings::trim (assignee);
  }
  return assignees_vector;
}


// Sets the note's assignees.
// identifier : note identifier.
// assignees : array of user names.
void Database_Notes::set_assignees (int identifier, std::vector <std::string> assignees)
{
  // Add a space at both sides of the assignee to allow for easier note selection based on note assignment.
  for (auto & assignee : assignees) {
    assignee.insert (0, " ");
    assignee.append (" ");
  }
  std::string assignees_string = filter::strings::implode (assignees, "\n");
  set_raw_assigned (identifier, assignees_string);
  note_modified_actions (identifier);
}


// Assign the note identified by identifier to user.
void Database_Notes::assign_user (int identifier, const std::string& user)
{
  // If the note already is assigned to the user, bail out.
  std::vector <std::string> assignees = get_assignees (identifier);
  if (find (assignees.begin (), assignees.end(), user) != assignees.end()) return;
  // Assign the note to the user.
  assignees.push_back (user);
  // Store the whole lot.
  set_assignees (identifier, assignees);
}


// Returns true if the note identified by identifier has been assigned to user.
bool Database_Notes::is_assigned (int identifier, const std::string& user)
{
  std::vector <std::string> assignees = get_assignees (identifier);
  return find (assignees.begin(), assignees.end(), user) != assignees.end();
}


// Unassigns user from the note identified by identifier.
void Database_Notes::unassign_user (int identifier, const std::string& user)
{
  // If the note is not assigned to the user, bail out.
  std::vector <std::string> assignees = get_assignees (identifier);
  if (find (assignees.begin(), assignees.end(), user) == assignees.end()) return;
  // Remove assigned user.
  assignees.erase (remove (assignees.begin(), assignees.end(), user), assignees.end());
  set_assignees (identifier, assignees);
}


std::string Database_Notes::get_bible (int identifier)
{
  return get_field (identifier, bible_key ());
}


void Database_Notes::set_bible (int identifier, const std::string& bible)
{
  // Write the bible to the filesystem.
  set_field (identifier, bible_key (), bible);
  
  // Update the database also.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET bible =");
  sql.add (bible);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
  
  note_modified_actions (identifier);
}


std::vector <std::string> Database_Notes::get_all_bibles ()
{
  std::vector <std::string> bibles;
  SqliteDatabase sql (database_notes);
  sql.set_sql ("SELECT DISTINCT bible FROM notes;");
  std::vector <int> identifiers;
  const std::vector <std::string> result = sql.query () ["bible"];
  for (auto & bible : result) {
    if (bible.empty ()) continue;
    bibles.push_back (bible);
  }
  return bibles;
}


// Encodes the book, chapter and verse, like to, e.g.: "40.5.13",
// and returns this as a string.
// The chapter and the verse can be negative, in which case they won't be included.
std::string Database_Notes::encode_passage (int book, int chapter, int verse)
{
  // Space before and after the passage enables notes selection on passage.
  // Special way of encoding, as done below, is to enable note selection on book / chapter / verse.
  std::string passage;
  passage.append (" ");
  passage.append (std::to_string (book));
  passage.append (".");
  // Whether to include the chapter number.
  if (chapter >= 0) {
    passage.append (std::to_string (chapter));
    passage.append (".");
    // Inclusion of verse, also depends on chapter inclusion.
    if (verse >= 0) {
      passage.append (std::to_string (verse));
      passage.append (" ");
    }
  }
  return passage;
}


// Takes the passage as a string, and returns an object with book, chapter, and verse.
Passage Database_Notes::decode_passage (std::string passage)
{
  passage = filter::strings::trim (passage);
  Passage decodedpassage = Passage ();
  std::vector <std::string> lines = filter::strings::explode (passage, '.');
  if (lines.size() > 0) decodedpassage.m_book = filter::strings::convert_to_int (lines[0]);
  if (lines.size() > 1) decodedpassage.m_chapter = filter::strings::convert_to_int (lines[1]);
  if (lines.size() > 2) decodedpassage.m_verse = lines[2];
  return decodedpassage;
}


// Returns the raw passage text of the note identified by identifier.
std::string Database_Notes::decode_passage (int identifier)
{
  return get_raw_passage (identifier);
}


// Returns the raw passage text of the note identified by identifier.
std::string Database_Notes::get_raw_passage (int identifier)
{
  return get_field (identifier, passage_key ());
}


// Returns an array with the passages that the note identified by identifier refers to.
// Each passages is an array (book, chapter, verse).
std::vector <Passage> Database_Notes::get_passages (int identifier)
{
  std::string contents = get_raw_passage (identifier);
  if (contents.empty()) return {};
  std::vector <std::string> lines = filter::strings::explode (contents, '\n');
  std::vector <Passage> passages;
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
void Database_Notes::set_passages (int identifier, const std::vector <Passage>& passages, bool import)
{
  // Format the passages.
  std::string line;
  for (auto & passage : passages) {
    if (!line.empty ()) line.append ("\n");
    line.append (encode_passage (passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse)));
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
void Database_Notes::set_raw_passage (int identifier, const std::string& passage)
{
  // Store the authoritative copy in the filesystem.
  set_field (identifier, passage_key (), passage);
}


void Database_Notes::index_raw_passage (int identifier, const std::string& passage)
{
  // Update the search index database.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET passage =");
  sql.add (passage);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


// Gets the raw status of a note.
// Returns it as a string.
std::string Database_Notes::get_raw_status (int identifier)
{
  return get_field (identifier, status_key ());
}


// Gets the localized status of a note.
// Returns it as a string.
std::string Database_Notes::get_status (int identifier)
{
  std::string status = get_raw_status (identifier);
  // Localize status if possible.
  status = translate (status.c_str());
  // Return status.
  return status;
}


// Sets the status of the note identified by identifier.
// status is a string.
// import: Just write the status, and skip any logic.
void Database_Notes::set_status (int identifier, const std::string& status, bool import)
{
  // Store the authoritative copy in the filesystem.
  set_field (identifier, status_key (), status);
  
  if (!import) note_modified_actions (identifier);
  
  // Store a copy in the database also.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET status =");
  sql.add (status);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


// Gets an array of array with the possible statuses of consultation notes,
// both raw and localized versions.
std::vector <Database_Notes_Text> Database_Notes::get_possible_statuses ()
{
  // Get an array with the statuses used in the database, ordered by occurrence, most often used ones first.
  SqliteDatabase sql (database_notes);
  sql.set_sql ("SELECT status, COUNT(status) AS occurrences FROM notes GROUP BY status ORDER BY occurrences DESC;");
  std::vector <std::string> statuses = sql.query () ["status"];
  sql.disconnect ();
  // Ensure the standard statuses are there too.
  std::vector <std::string> standard_statuses = {"New", "Pending", "In progress", "Done", "Reopened"};
  for (auto & standard_status : standard_statuses) {
    if (find (statuses.begin(), statuses.end(), standard_status) == statuses.end()) {
      statuses.push_back (standard_status);
    }
  }
  // Localize the results.
  std::vector <Database_Notes_Text> localized_statuses;
  for (auto & status : statuses) {
    std::string localization = translate (status.c_str());
    Database_Notes_Text localized_status;
    localized_status.raw = status;
    localized_status.localized = localization;
    localized_statuses.push_back (localized_status);
  }
  // Return result.
  return localized_statuses;
}


std::vector <std::string> Database_Notes::standard_severities ()
{
  return {"Wish", "Minor", "Normal", "Important", "Major", "Critical"};
}


// Returns the severity of a note as a number.
int Database_Notes::get_raw_severity (int identifier)
{
  const std::string severity = get_field (identifier, severity_key ());
  if (severity.empty ()) return 2;
  return filter::strings::convert_to_int (severity);
}


// Returns the severity of a note as a localized string.
std::string Database_Notes::get_severity (int identifier)
{
  int severity = get_raw_severity (identifier);
  std::vector <std::string> standard = standard_severities ();
  std::string severitystring;
  if ((severity >= 0) && (severity < static_cast<int>(standard.size()))) severitystring = standard [static_cast<size_t> (severity)];
  if (severitystring.empty()) severitystring = "Normal";
  severitystring = translate (severitystring.c_str());
  return severitystring;
}


// Sets the severity of the note identified by identifier.
// severity is a number.
void Database_Notes::set_raw_severity (int identifier, int severity)
{
  // Update the file system.
  set_field (identifier, severity_key (), std::to_string (severity));
  
  note_modified_actions (identifier);
  
  // Update the database also.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET severity =");
  sql.add (severity);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


// Gets an array with the possible severities.
std::vector <Database_Notes_Text> Database_Notes::get_possible_severities ()
{
  std::vector <std::string> standard = standard_severities ();
  std::vector <Database_Notes_Text> severities;
  for (size_t i = 0; i < standard.size(); i++) {
    Database_Notes_Text severity;
    severity.raw = std::to_string (i);
    severity.localized = translate (standard[i].c_str());
    severities.push_back (severity);
  }
  return severities;
}


int Database_Notes::get_modified (int identifier)
{
  std::string modified = get_field (identifier, modified_key ());
  if (modified.empty ()) return 0;
  return filter::strings::convert_to_int (modified);
}


void Database_Notes::set_modified (int identifier, int time)
{
  // Update the filesystem.
  set_field (identifier, modified_key (), std::to_string (time));
  // Update the database.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET modified =");
  sql.add (time);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
  sql.disconnect();
  // Update checksum.
  update_checksum (identifier);
}


bool Database_Notes::get_public (int identifier)
{
  const std::string value = get_field (identifier, public_key ());
  return filter::strings::convert_to_bool (value);
}


void Database_Notes::set_public (int identifier, bool value)
{
  set_field (identifier, public_key (), filter::strings::convert_to_string (value));
}


// Takes actions when a note has been edited.
void Database_Notes::note_modified_actions (int identifier)
{
  // Update 'modified' field.
  set_modified (identifier, filter::date::seconds_since_epoch());
}


void Database_Notes::update_search_fields (int identifier)
{
  // The search field is a combination of the summary and content converted to clean text.
  // It enables us to search with wildcards before and after the search query.
  std::string noteSummary = get_summary (identifier);
  std::string noteContents = get_contents (identifier);
  std::string cleanText = noteSummary + "\n" + filter::strings::html2text (noteContents);
  // Bail out if the search field is already up to date.
  if (cleanText == get_search_field (identifier)) return;
  // Update the field.
  SqliteDatabase sql (database_notes);
  sql.add ("UPDATE notes SET cleantext =");
  sql.add (cleanText);
  sql.add ("WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
}


std::string Database_Notes::get_search_field (int identifier)
{
  SqliteDatabase sql (database_notes);
  sql.add ("SELECT cleantext FROM notes WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["cleantext"];
  std::string value;
  for (auto & cleantext : result) {
    value = cleantext;
  }
  return value;
}


// Searches the notes.
// Returns an array of note identifiers.
// search: Contains the text to search for.
// bibles: Array of Bibles the notes should refer to.
std::vector <int> Database_Notes::search_notes (std::string search, const std::vector <std::string> & bibles)
{
  std::vector <int> identifiers;

  search = filter::strings::trim (search);
  if (search == "") return identifiers;

  // SQL SELECT statement.
  std::string query = notes_select_identifier ();

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
  for (std::string bible : bibles) {
    bible = database::sqlite::no_sql_injection (bible);
    query.append (" OR bible = '");
    query.append (bible);
    query.append ("' ");
  }
  query.append (" ) ");

  // Notes get ordered on relevance of search hits.
  query.append (notes_order_by_relevance_statement ());

  // Complete query.
  query.append (";");
  
  SqliteDatabase sql (database_notes);
  sql.set_sql (query);
  const std::vector <std::string> result = sql.query () ["identifier"];
  for (const auto & id : result) {
    identifiers.push_back (filter::strings::convert_to_int (id));
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
  std::string expiry = get_field (identifier, expiry_key ());
  return !expiry.empty ();
}


void Database_Notes::touch_marked_for_deletion ()
{
  std::vector <int> identifiers = get_identifiers ();
  for (auto & identifier : identifiers) {
    if (is_marked_for_deletion (identifier)) {
      std::string expiry = get_field (identifier, expiry_key ());
      int days = filter::strings::convert_to_int (expiry);
      days--;
      set_field (identifier, expiry_key (), std::to_string (days));
    }
  }
}


std::vector <int> Database_Notes::get_due_for_deletion ()
{
  std::vector <int> deletes;
  std::vector <int> identifiers = get_identifiers ();
  for (auto & identifier : identifiers) {
    if (is_marked_for_deletion (identifier)) {
      std::string sdays = get_field (identifier, expiry_key ());
      int idays = filter::strings::convert_to_int (sdays);
      if ((sdays == "0") || (idays < 0)) {
        deletes.push_back (identifier);
      }
    }
  }
  return deletes;
}


// Writes the checksum for note identifier in the database.
void Database_Notes::set_checksum (int identifier, const std::string& checksum)
{
  // Do not write the checksum if it is already up to date.
  if (checksum == get_checksum (identifier)) return;
  // Write the checksum to the database.
  delete_checksum (identifier);
  SqliteDatabase sql (database_notes_checksums);
  sql.add ("INSERT INTO checksums VALUES (");
  sql.add (identifier);
  sql.add (",");
  sql.add (checksum);
  sql.add (");");
  sql.execute ();
}


// Reads the checksum for note identifier from the database.
std::string Database_Notes::get_checksum (int identifier)
{
  SqliteDatabase sql (database_notes_checksums);
  sql.add ("SELECT checksum FROM checksums WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["checksum"];
  std::string value;
  for (const auto& row : result) {
    value = row;
  }
  return value;
}


// Deletes the checksum for note identifier from the database.
void Database_Notes::delete_checksum (int identifier)
{
  {
    SqliteDatabase sql (database_notes_checksums);
    sql.add ("DELETE FROM checksums WHERE identifier =");
    sql.add (identifier);
    sql.add (";");
    sql.execute ();
  }
  // Delete from range-based checksums.
  Database_State::eraseNoteChecksum (identifier);
}


// The function calculates the checksum of the note signature,
// and writes it to the filesystem.
void Database_Notes::update_checksum (int identifier)
{
  // Read the raw data from disk to speed up checksumming.
  std::string checksum;
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
std::string Database_Notes::get_multiple_checksum (const std::vector <int> & identifiers)
{
  SqliteDatabase sql (database_notes_checksums);
  std::string checksum;
  for (auto & identifier : identifiers) {
    sql.clear();
    sql.add ("SELECT checksum FROM checksums WHERE identifier =");
    sql.add (identifier);
    sql.add (";");
    const std::vector <std::string> result = sql.query () ["checksum"];
    std::string value = "";
    for (const auto& row : result) {
      value = row;
    }
    checksum.append (value);
  }
  checksum = md5 (checksum);
  return checksum;
}


// This function gets the identifiers for notes
// within the note identifier range of lowId to highId
// which refer to any Bible in the array of bibles
// or refer to no Bible.
std::vector <int> Database_Notes::get_notes_in_range_for_bibles (int lowId, int highId, std::vector <std::string> bibles, bool anybible)
{
  std::vector <int> identifiers;
  
  std::string query = "SELECT identifier FROM notes WHERE identifier >= ";
  query.append (std::to_string (lowId));
  query.append (" AND identifier <= ");
  query.append (std::to_string (highId));
  query.append (" ");
  if (!anybible) {
    bibles.push_back (""); // Select general note also
    std::string bibleSelector = " AND (";
    for (unsigned int i = 0; i < bibles.size(); i++) {
      bibles[i] = database::sqlite::no_sql_injection (bibles[i]);
      if (i > 0) bibleSelector.append (" OR ");
      bibleSelector.append (" bible = '");
      bibleSelector.append (bibles[i]);
      bibleSelector.append ("' ");
    }
    bibleSelector.append (")");
    query.append (bibleSelector);
  }
  query.append (" ORDER BY identifier;");

  SqliteDatabase sql (database_notes);
  sql.set_sql (query);
  const std::vector <std::string> result = sql.query () ["identifier"];
  for (const auto& row : result) {
    identifiers.push_back (filter::strings::convert_to_int (row));
  }
  
  return identifiers;
}


std::string Database_Notes::availability_flag ()
{
  return filter_url_create_root_path ({database_logic_databases (), "notes.busy"});
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


std::string Database_Notes::notes_select_identifier ()
{
  return " SELECT identifier ";
}


std::string Database_Notes::notes_optional_fulltext_search_relevance_statement (std::string search)
{
  if (search == "") return std::string();
  search = filter::strings::replace (",", "", search);
  search = database::sqlite::no_sql_injection (search);
  std::string query = "";
  return query;
}


std::string Database_Notes::notes_from_where_statement ()
{
  return " FROM notes WHERE 1 ";
}


std::string Database_Notes::notes_optional_fulltext_search_statement (std::string search)
{
  if (search == "") return std::string();
  search = filter::strings::replace (",", "", search);
  search = database::sqlite::no_sql_injection (search);
  std::string query = " AND cleantext LIKE '%" + search + "%' ";
  return query;
}


std::string Database_Notes::notes_order_by_relevance_statement ()
{
  return std::string();
}


// This returns JSON that contains the notes indicated by $identifiers.
std::string Database_Notes::get_bulk (std::vector <int> identifiers)
{
  // JSON container for the bulk notes.
  jsonxx::Array bulk;
  // Go through all the notes.
  for (auto identifier : identifiers) {
    // JSON object for the note.
    jsonxx::Object note;
    // Add all the fields of the note.
    std::string assigned = get_field (identifier, assigned_key ());
    note << "a" << assigned;
    std::string bible = get_bible (identifier);;
    note << "b" << bible;
    std::string contents = get_contents (identifier);
    note << "c" << contents;
    note << "i" << identifier;
    int modified = get_modified (identifier);
    note << "m" << modified;
    std::string passage = get_raw_passage (identifier);
    note << "p" << passage;
    std::string subscriptions = get_field (identifier, subscriptions_key ());
    note << "sb" << subscriptions;
    std::string summary;
    summary = get_summary (identifier);
    note << "sm" << summary;
    std::string status;
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
std::vector <std::string> Database_Notes::set_bulk (std::string json)
{
  // Container for the summaries that were stored.
  std::vector <std::string> summaries;
  
  // Parse the incoming JSON.
  jsonxx::Array bulk;
  bulk.parse (json);
  
  // Go through the notes the JSON contains.
  for (size_t i = 0; i < bulk.size (); i++) {
    
    // Get all the different fields for this note.
    jsonxx::Object note = bulk.get<jsonxx::Object>(static_cast<unsigned>(i));
    std::string assigned = note.get<jsonxx::String> ("a");
    std::string bible = note.get<jsonxx::String> ("b");
    std::string contents = note.get<jsonxx::String> ("c");
    int identifier = static_cast<int>(note.get<jsonxx::Number> ("i"));
    int modified = static_cast<int>(note.get<jsonxx::Number> ("m"));
    std::string passage = note.get<jsonxx::String> ("p");
    std::string subscriptions = note.get<jsonxx::String> ("sb");
    std::string summary = note.get<jsonxx::String> ("sm");
    std::string status = note.get<jsonxx::String> ("st");
    int severity = static_cast<int>(note.get<jsonxx::Number> ("sv"));
    
    // Feedback about which note it received in bulk.
    summaries.push_back (summary);
    
    // Store the note in the filesystem.
    std::string path = note_file (identifier);
    std::string folder = filter_url_dirname (path);
    filter_url_mkdir (folder);
    jsonxx::Object note2;
    note2 << assigned_key () << assigned;
    note2 << bible_key () << bible;
    note2 << contents_key () << contents;
    note2 << modified_key () << std::to_string (modified);
    note2 << passage_key () << passage;
    note2 << subscriptions_key () << subscriptions;
    note2 << summary_key () << summary;
    note2 << status_key () << status;
    note2 << severity_key () << std::to_string (severity);
    std::string json2 = note2.json ();
    filter_url_file_put_contents (path, json2);
    
    // Update the indexes.
    update_database (identifier);
    update_search_fields (identifier);
    update_checksum (identifier);
  }
  
  // Container with all the summaries of the notes that were stored.
  return summaries;
}


// Gets a field from a note in JSON format.
std::string Database_Notes::get_field (int identifier, const std::string& key)
{
  const std::string file = note_file (identifier);
  const std::string json = filter_url_file_get_contents (file);
  jsonxx::Object note;
  note.parse (json);
  if (note.has<jsonxx::String> (key))
    return note.get<jsonxx::String> (key);
  return std::string();
}


// Sets a field in a note in JSON format.
void Database_Notes::set_field (int identifier, std::string key, std::string value)
{
  std::string file = note_file (identifier);
  std::string json = filter_url_file_get_contents (file);
  jsonxx::Object note;
  note.parse (json);
  note << key << value;
  json = note.json ();
  filter_url_file_put_contents (file, json);
}


std::string Database_Notes::bible_key ()
{
  return "bible";
}


std::string Database_Notes::passage_key ()
{
  return "passage";
}


std::string Database_Notes::status_key ()
{
  return "status";
}


std::string Database_Notes::severity_key ()
{
  return "severity";
}


std::string Database_Notes::modified_key ()
{
  return "modified";
}


std::string Database_Notes::summary_key ()
{
  return "summary";
}


std::string Database_Notes::contents_key ()
{
  return "contents";
}


std::string Database_Notes::subscriptions_key ()
{
  return "subscriptions";
}


std::string Database_Notes::assigned_key ()
{
  return "assigned";
}


std::string Database_Notes::expiry_key ()
{
  return "expiry";
}


std::string Database_Notes::public_key ()
{
  return "public";
}
