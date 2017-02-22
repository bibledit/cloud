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


#include <database/modifications.h>
#include <database/bibles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <database/sqlite.h>


// Database resilience:
// All information is stored in the filesystem as files or as very small databases.
// The large database is only used as an index.
// It is re-indexed every night.


const char * Database_Modifications::filename ()
{
  return "modifications";
}


sqlite3 * Database_Modifications::connect ()
{
  return database_sqlite_connect (filename ());
}


// Delete the entire database
void Database_Modifications::erase ()
{
  string file = database_sqlite_file ("modifications");
  filter_url_unlink (file);
}


void Database_Modifications::create ()
{
  sqlite3 * db = connect ();
  string sql =
    "CREATE TABLE IF NOT EXISTS notifications ("
    " identifier integer,"
    " timestamp integer,"
    " username text,"
    " category text,"
    " bible text,"
    " book integer,"
    " chapter integer,"
    " verse integer,"
    " modification text"
    ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


bool Database_Modifications::healthy ()
{
  return database_sqlite_healthy ("modifications");
}


void Database_Modifications::vacuum ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM;");
  database_sqlite_disconnect (db);
}


// Code dealing with the "teams" data.


string Database_Modifications::teamFolder ()
{
  return filter_url_create_root_path ("databases", "modifications", "team");
}


string Database_Modifications::teamFile (const string& bible, int book, int chapter)
{
  return filter_url_create_path (teamFolder (), bible + "." + convert_to_string (book) + "." + convert_to_string (chapter));
}


// Returns true if diff data exists for the chapter.
// Else it returns false.
bool Database_Modifications::teamDiffExists (const string& bible, int book, int chapter)
{
  string file = teamFile (bible, book, chapter);
  return file_or_dir_exists (file);
}


// Stores diff data for a "bible" (string) and book (int) and chapter (int).
void Database_Modifications::storeTeamDiff (const string& bible, int book, int chapter)
{
  // Return when the diff exists.
  if (teamDiffExists (bible, book, chapter)) return;

  // Retrieve current chapter USFM data.
  Database_Bibles database_bibles;
  string data = database_bibles.getChapter (bible, book, chapter);

  // Store.
  string file = teamFile (bible, book, chapter);
  filter_url_file_put_contents (file, data);
}


// Gets the diff data as a string.
string Database_Modifications::getTeamDiff (const string& bible, int book, int chapter)
{
  string file = teamFile (bible, book, chapter);
  return filter_url_file_get_contents (file);
}


// Stores diff data for all chapters in a "bible" (string) and book (int).
void Database_Modifications::storeTeamDiffBook (const string& bible, int book)
{
  Database_Bibles database_bibles;
  vector <int> chapters = database_bibles.getChapters (bible, book);
  for (auto & chapter : chapters) {
    storeTeamDiff (bible, book, chapter);
  }
}


// Stores diff data for all books in a "bible" (string).
void Database_Modifications::storeTeamDiffBible (const string& bible)
{
  Database_Bibles database_bibles;
  vector <int> books = database_bibles.getBooks (bible);
  for (auto & book : books) {
    storeTeamDiffBook (bible, book);
  }
}


// Deletes the diffs for a whole Bible.
void Database_Modifications::deleteTeamDiffBible (const string& bible)
{
  string pattern = bible + ".";
  int length = pattern.length ();
  vector <string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    filter_url_unlink (filter_url_create_path (teamFolder (), file));
  }
}


// Deletes the diffs for one chapter of a Bible.
void Database_Modifications::deleteTeamDiffChapter (const string& bible, int book, int chapter)
{
  string file = teamFile (bible, book, chapter);
  filter_url_unlink (file);
}


// Returns an array with the available chapters that have diff data in a book in a Bible.
vector <int> Database_Modifications::getTeamDiffChapters (const string& bible, int book)
{
  string pattern = bible + "." + convert_to_string (book) + ".";
  int length = pattern.length ();
  vector <int> chapters;
  vector <string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    vector <string> bits = filter_string_explode (file, '.');
    if (bits.size() != 3) continue;
    string path = filter_url_create_path (teamFolder (), file);
    int time = filter_url_file_modification_time (path);
    int days = (filter_date_seconds_since_epoch () - time) / 86400;
    if (days > 5) {
      // Unprocessed team changes older than so many days usually indicate a problem.
      // Perhaps the server crashed so it never could process them.
      // Cases like this have been seen on servers with limited memory.
      // Therefore just remove this change, without processing it.
      filter_url_unlink (path);
    } else {
      chapters.push_back (convert_to_int (bits [2]));
    }
  }
  sort (chapters.begin(), chapters.end());
  return chapters;
}


// Returns the number of items in bible that have diff data.
// The bible can be a name or an identifier. This is because the bible identifier may no longer exist.
int Database_Modifications::getTeamDiffCount (const string& bible)
{
  string pattern = bible + ".";
  int length = pattern.length ();
  int count = 0;
  vector <string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    count++;
  }
  return count;
}


// Returns an array with the available books that have diff data in a Bible.
// The bible can be a name, or an identifier. This is because the bible identifier may no longer exist.
vector <int> Database_Modifications::getTeamDiffBooks (const string& bible)
{
  vector <int> books;
  string pattern = bible + ".";
  int length = pattern.length ();
  vector <string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    vector <string> bits = filter_string_explode (file, '.');
    if (bits.size() != 3) continue;
    books.push_back (convert_to_int (bits [1]));
  }
  set <int> bookset (books.begin (), books.end());
  books.assign (bookset.begin(), bookset.end());
  sort (books.begin(), books.end());
  return books;
}


// Returns an array with the available Bibles that have diff data.
vector <string> Database_Modifications::getTeamDiffBibles ()
{
  vector <string> bibles;
  vector <string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    vector <string> bits = filter_string_explode (file, '.');
    if (bits.size() != 3) continue;
    bibles.push_back (bits [0]);
  }
  set <string> bibleset (bibles.begin (), bibles.end());
  bibles.assign (bibleset.begin(), bibleset.end());
  sort (bibles.begin(), bibles.end());
  return bibles;
}


// Truncates all team modifications.
void Database_Modifications::truncateTeams ()
{
  vector <string> files = filter_url_scandir (teamFolder ());
  for (auto file : files) {
    filter_url_unlink (filter_url_create_path (teamFolder (), file));
  }
}


// Code dealing with the "users" data.


string Database_Modifications::userMainFolder ()
{
  return filter_url_create_root_path ("databases", "modifications", "users");
}


string Database_Modifications::userUserFolder (const string& username)
{
  return filter_url_create_path (userMainFolder (), username);
}


string Database_Modifications::userBibleFolder (const string& username, const string& bible)
{
  return filter_url_create_path (userUserFolder (username), bible);
}


string Database_Modifications::userBookFolder (const string& username, const string& bible, int book)
{
  return filter_url_create_path (userBibleFolder (username, bible), convert_to_string (book));
}


string Database_Modifications::userChapterFolder (const string& username, const string& bible, int book, int chapter)
{
  return filter_url_create_path (userBookFolder (username, bible, book), convert_to_string (chapter));
}


string Database_Modifications::userNewIDFolder (const string& username, const string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path (userChapterFolder (username, bible, book, chapter), convert_to_string (newID));
}


string Database_Modifications::userTimeFile (const string& username, const string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path (userNewIDFolder (username, bible, book, chapter, newID), "time");
}


string Database_Modifications::userOldIDFile (const string& username, const string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path (userNewIDFolder (username, bible, book, chapter, newID), "oldid");
}


string Database_Modifications::userOldTextFile (const string& username, const string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path (userNewIDFolder (username, bible, book, chapter, newID), "oldtext");
}


string Database_Modifications::userNewTextFile (const string& username, const string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path (userNewIDFolder (username, bible, book, chapter, newID), "newtext");
}


void Database_Modifications::recordUserSave (const string& username, const string& bible, int book, int chapter, int oldID, const string& oldText, int newID, const string& newText)
{
  // This entry is saved in a deep folder structure with the new ID in it.
  string folder = userNewIDFolder (username, bible, book, chapter, newID);
  if (!file_or_dir_exists (folder)) filter_url_mkdir (folder);
  // The other data is stored in separate files in the newID folder.
  string timeFile = userTimeFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (timeFile, convert_to_string (filter_date_seconds_since_epoch ()));
  string oldIDFile = userOldIDFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (oldIDFile, convert_to_string (oldID));
  string oldTextFile = userOldTextFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (oldTextFile, oldText);
  string newTextFile = userNewTextFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (newTextFile, newText);
}


void Database_Modifications::clearUserUser (const string& username)
{
  string folder = userUserFolder (username);
  filter_url_rmdir (folder);
}


vector <string> Database_Modifications::getUserUsernames ()
{
  string folder = userMainFolder ();
  vector <string> usernames = filter_url_scandir (folder);
  return usernames;
}


vector <string> Database_Modifications::getUserBibles (const string& username)
{
  string folder = userUserFolder (username);
  return filter_url_scandir (folder);
}


vector <int> Database_Modifications::getUserBooks (const string& username, const string& bible)
{
  string folder = userBibleFolder (username, bible);
  vector <string> files = filter_url_scandir (folder);
  vector <int> books;
  for (auto & file : files) {
    books.push_back (convert_to_int (file));
  }
  sort (books.begin(), books.end());
  return books;
}


vector <int> Database_Modifications::getUserChapters (const string& username, const string& bible, int book)
{
  string folder = userBookFolder (username, bible, book);
  vector <string> files = filter_url_scandir (folder);
  vector <int> chapters;
  for (auto & file : files) {
    string path = filter_url_create_path (folder, file);
    int time = filter_url_file_modification_time (path);
    int days = (filter_date_seconds_since_epoch () - time) / 86400;
    if (days > 5) {
      // Unprocessed user changes older than so many days usually indicate a problem.
      // Perhaps the server crashed so it never could process them.
      // Cases like this have been seen on servers with limited memory.
      // Therefore just remove this change, without processing it.
      filter_url_rmdir (path);
    } else {
      chapters.push_back (convert_to_int (file));
    }
  }
  sort (chapters.begin(), chapters.end());
  return chapters;
}


vector <Database_Modifications_Id> Database_Modifications::getUserIdentifiers (const string& username, const string& bible, int book, int chapter)
{
  string folder = userChapterFolder (username, bible, book, chapter);
  vector <Database_Modifications_Id> ids;
  vector <string> newids = filter_url_scandir (folder);
  for (auto & newid : newids) {
    string file = userOldIDFile (username, bible, book, chapter, convert_to_int (newid));
    string oldid = filter_url_file_get_contents (file);
    Database_Modifications_Id id;
    id.oldid = convert_to_int (oldid);
    id.newid = convert_to_int (newid);
    ids.push_back (id);
  }
  return ids;
}


Database_Modifications_Text Database_Modifications::getUserChapter (const string& username, const string& bible, int book, int chapter, int newID)
{
  string oldfile = userOldTextFile (username, bible, book, chapter, newID);
  string newfile = userNewTextFile (username, bible, book, chapter, newID);
  string oldtext = filter_url_file_get_contents (oldfile);
  string newtext = filter_url_file_get_contents (newfile);
  Database_Modifications_Text data;
  data.oldtext = oldtext;
  data.newtext = newtext;
  return data;
}


int Database_Modifications::getUserTimestamp (const string& username, const string& bible, int book, int chapter, int newID)
{
  string file = userTimeFile (username, bible, book, chapter, newID);
  string contents = filter_url_file_get_contents (file);
  int time = convert_to_int (contents);
  if (time > 0) return time;
  return filter_date_seconds_since_epoch ();
}


// Code dealing with the notifications.


string Database_Modifications::notificationsMainFolder ()
{
  return filter_url_create_root_path ("databases", "modifications", "notifications");
}


string Database_Modifications::notificationIdentifierDatabase (int identifier)
{
  return filter_url_create_path (notificationsMainFolder (), convert_to_string (identifier));
}


// This function is for the unit tests.
void Database_Modifications::notificationUpdateTime (int identifier, int timestamp)
{
  SqliteDatabase sql (notificationIdentifierDatabase (identifier));
  sql.add ("UPDATE notification SET timestamp =");
  sql.add (timestamp);
  sql.add (";");
  sql.execute ();
}


int Database_Modifications::getNextAvailableNotificationIdentifier ()
{
  // Read the existing identifiers in the folder.
  vector <string> files = filter_url_scandir (notificationsMainFolder ());
  // Sort from low to high.
  vector <int> identifiers;
  for (auto file : files) {
    identifiers.push_back (convert_to_int (file));
  }
  sort (identifiers.begin(), identifiers.end());
  // Fetch the last and highest identifier.
  int identifier = 0;
  size_t length = identifiers.size();
  if (length) identifier = identifiers [length - 1];
  // Take next one.
  identifier++;
  // Done.
  return identifier;
}


void Database_Modifications::recordNotification (const vector <string> & users, const string& category, const string& bible, int book, int chapter, int verse, const string& oldtext, const string& modification, const string& newtext)
{
  // Normally this function is called just after midnight.
  // It would then put the current time on changes made the day before.
  // Make a correction for that by subtracting 6 hours.
  int timestamp = filter_date_seconds_since_epoch () - 21600;
  for (auto & user : users) {
    int identifier = getNextAvailableNotificationIdentifier ();
    SqliteDatabase sql (notificationIdentifierDatabase (identifier));
    sql.add (createNotificationsDbSql ());
    sql.execute ();
    sql.clear ();
    sql.add ("INSERT INTO notification VALUES (");
    sql.add (timestamp);
    sql.add (",");
    sql.add (user);
    sql.add (",");
    sql.add (category);
    sql.add (",");
    sql.add (bible);
    sql.add (",");
    sql.add (book);
    sql.add (",");
    sql.add (chapter);
    sql.add (",");
    sql.add (verse);
    sql.add (",");
    sql.add (oldtext);
    sql.add (",");
    sql.add (modification);
    sql.add (",");
    sql.add (newtext);
    sql.add (");");
    sql.execute ();
  }
}


void Database_Modifications::indexTrimAllNotifications ()
{
  // When the index is not healthy, delete it.
  if (!healthy ()) erase ();
  // Create a new index if it does not exist.
  create ();

  // Get the notification identifiers on disk.
  vector <string> sidentifiers = filter_url_scandir (notificationsMainFolder ());

  // Change notifications expire after 30 days.
  // But the more there are, the sooner they expire.
  int expiry_time = filter_date_seconds_since_epoch () - (30 * 3600 * 24);
  if (sidentifiers.size () > 10000) expiry_time = filter_date_seconds_since_epoch () - (14 * 3600 * 24);
  if (sidentifiers.size () > 20000) expiry_time = filter_date_seconds_since_epoch () - (7 * 3600 * 14);
  if (sidentifiers.size () > 30000) expiry_time = filter_date_seconds_since_epoch () - (4 * 3600 * 14);

  // Database: Connect and speed it up.
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "PRAGMA synchronous = OFF;");

  // Go through the notifications on disk.
  vector <int> identifiers;
  for (auto s : sidentifiers) {
    identifiers.push_back (convert_to_int (s));
  }
  sort (identifiers.begin(), identifiers.end());
  for (auto & identifier : identifiers) {

    // Fetch the data from the database and validate it.

    string path = notificationIdentifierDatabase (identifier);
    bool valid = !filter_url_is_dir (path);

    map <string, vector <string> > result;
    if (valid) {
      SqliteDatabase sql (path);
      sql.add ("SELECT * FROM notification;");
      result = sql.query ();
    }
    if (result.empty ()) valid = false;

    int timestamp = 0;
    if (valid) {
      vector <string> timestamps = result ["timestamp"];
      if (timestamps.empty ()) valid = false;
      else timestamp = convert_to_int (timestamps [0]);
    }
    if (timestamp < expiry_time) valid = false;

    bool exists = false;
    if (valid) {
      SqliteSQL sql = SqliteSQL ();
      sql.add ("SELECT count(*) FROM notifications WHERE identifier = ");
      sql.add (identifier);
      sql.add (";");
      vector <string> result = database_sqlite_query (db, sql.sql) ["count(*)"];
      if (!result.empty ()) {
        int count = convert_to_int (result [0]);
        exists = (count > 0);
      }
    }
    
    string user;
    if (!exists && valid) {
      vector <string> usernames = result ["username"];
      if (usernames.empty ()) valid = false;
      else user = usernames [0];
      if (user.empty ()) valid = false;
    }

    string category;
    if (!exists && valid) {
      vector <string> categories = result ["category"];
      if (categories.empty ()) valid = false;
      else category = categories [0];
    }

    string bible;
    if (!exists && valid) {
      vector <string> bibles = result ["bible"];
      if (bibles.empty ()) valid = false;
      else bible = bibles [0];
      if (bible.empty ()) valid = false;
    }

    int book = 0;
    if (!exists && valid) {
      vector <string> books = result ["book"];
      if (books.empty ()) valid = false;
      else book = convert_to_int (books [0]);
      if (book == 0) valid = false;
    }

    int chapter = 0;
    if (!exists && valid) {
      vector <string> chapters = result ["chapter"];
      if (chapters.empty ()) valid = false;
      else chapter = convert_to_int (chapters [0]);
      if (chapters [0].empty ()) valid = false;
    }
    
    int verse = 0;
    if (!exists && valid) {
      vector <string> verses = result ["verse"];
      if (verses.empty ()) valid = false;
      else verse = convert_to_int (verses [0]);
      if (verses [0].empty ()) valid = false;
    }

    string modification;
    if (!exists && valid) {
      vector <string> modifications = result ["modification"];
      if (modifications.empty ()) valid = false;
      else modification = modifications [0];
      if (modification.empty()) valid = false;
    }
   
    if (valid) {
      // Store valid data in the database if it does not yet exist.
      if (!exists) {
        SqliteSQL sql = SqliteSQL ();
        sql.add ("INSERT INTO notifications VALUES (");
        sql.add (identifier);
        sql.add (",");
        sql.add (timestamp);
        sql.add (",");
        sql.add (user);
        sql.add (",");
        sql.add (category);
        sql.add (",");
        sql.add (bible);
        sql.add (",");
        sql.add (book);
        sql.add (",");
        sql.add (chapter);
        sql.add (",");
        sql.add (verse);
        sql.add (",");
        sql.add (modification);
        sql.add (");");
        database_sqlite_exec (db, sql.sql);
      }
    } else {
      // Delete invalid or expired data.
      deleteNotification (identifier, db);
    }
  }
  
  database_sqlite_disconnect (db);
}


vector <int> Database_Modifications::getNotificationIdentifiers (const string& username, bool limit)
{
  vector <int> ids;

  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT identifier FROM notifications");
  if (username != "") {
    sql.add ("WHERE username =");
    sql.add (username);
  }
  // Sort on reference, so that related change notifications are near each other.
  sql.add ("ORDER BY book ASC, chapter ASC, verse ASC, identifier ASC");
  // Optionally limit the hits.
  if (limit) sql.add ("LIMIT 1000");
  sql.add (";");

  sqlite3 * db = connect ();
  vector <string> sidentifiers = database_sqlite_query (db, sql.sql) ["identifier"];
  database_sqlite_disconnect (db);
  for (auto & identifier : sidentifiers) {
    ids.push_back (convert_to_int (identifier));
  }

  return ids;
}


// This gets the identifiers of the personal change proposals.
// For easier comparison, it also gets the identifiers of the changes
// in the verses that have changes entered by a person.
vector <int> Database_Modifications::getNotificationPersonalIdentifiers (const string& username, const string& category, bool limit)
{
  sqlite3 * db = connect ();

  // Get all personal change proposals.
  vector <int> personalIDs;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT identifier FROM notifications WHERE username =");
  sql.add (username);
  sql.add ("AND category =");
  sql.add (category);
  sql.add ("ORDER BY book ASC, chapter ASC, verse ASC, identifier ASC");
  // Optionally limit the hits.
  if (limit) sql.add ("LIMIT 1000");
  sql.add (";");

  vector <string> sidentifiers = database_sqlite_query (db, sql.sql) ["identifier"];
  for (auto & identifier : sidentifiers) {
    personalIDs.push_back (convert_to_int (identifier));
  }

  vector <int> allIDs;

  // Go through each of the personal change proposals.
  for (int & personalID : personalIDs) {
    // Add the personal change proposal to the results.
    allIDs.push_back (personalID);
    // Get the Bible and passage for this change proposal.
    string bible = getNotificationBible (personalID);
    Passage passage = getNotificationPassage (personalID);
    int book = passage.book;
    int chapter = passage.chapter;
    int verse = convert_to_int (passage.verse);
    // Look for change proposals for this Bible and passage.
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT identifier FROM notifications WHERE username =");
    sql.add (username);
    sql.add ("AND bible =");
    sql.add (bible);
    sql.add ("AND book =");
    sql.add (book);
    sql.add ("AND chapter =");
    sql.add (chapter);
    sql.add ("AND verse =");
    sql.add (verse);
    sql.add ("ORDER BY identifier ASC;");
    vector <string> sidentifiers = database_sqlite_query (db, sql.sql) ["identifier"];
    for (auto & identifier : sidentifiers) {
      int id = convert_to_int (identifier);
      // Add the identifier if it's not yet in.
      if (find (allIDs.begin(), allIDs.end(), id) == allIDs.end()) {
        allIDs.push_back (id);
      }
    }
  }
  
  database_sqlite_disconnect (db);

  return allIDs;
}


// This gets the identifiers of the team's changes.
vector <int> Database_Modifications::getNotificationTeamIdentifiers (const string& username, const string& category, bool limit)
{
  vector <int> ids;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT identifier FROM notifications WHERE username =");
  sql.add (username);
  sql.add ("AND category =");
  sql.add (category);
  sql.add ("ORDER BY book ASC, chapter ASC, verse ASC, identifier ASC");
  if (limit) sql.add ("LIMIT 1000");
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> sidentifiers = database_sqlite_query (db, sql.sql) ["identifier"];
  database_sqlite_disconnect (db);
  for (auto & sid : sidentifiers) {
    ids.push_back (convert_to_int (sid));
  }
  return ids;
}


void Database_Modifications::deleteNotification (int identifier, sqlite3 * db)
{
  // Delete from file.
  deleteNotificationFile (identifier);
  // Delete from the database.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM notifications WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  // Make a very short connection to the database, 
  // to prevent corruption when a user deletes lots of changes notifications 
  // by keeping the delete key pressed.
  bool local_connection = (db == NULL);
  if (local_connection) db = connect ();
  database_sqlite_exec (db, sql.sql);
  if (local_connection) database_sqlite_disconnect (db);
}


int Database_Modifications::getNotificationTimeStamp (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT timestamp FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> timestamps = database_sqlite_query (db, sql.sql) ["timestamp"];
  database_sqlite_disconnect (db);
  int time = filter_date_seconds_since_epoch ();
  for (auto & stamp : timestamps) {
    time = convert_to_int (stamp);
  }
  return time;
}


string Database_Modifications::getNotificationCategory (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT category FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> categories = database_sqlite_query (db, sql.sql) ["category"];
  database_sqlite_disconnect (db);
  string category = "";
  for (auto & row : categories) {
    category = row;
  }
  return category;
}


string Database_Modifications::getNotificationBible (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT bible FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> bibles = database_sqlite_query (db, sql.sql) ["bible"];
  database_sqlite_disconnect (db);
  string bible = "";
  for (auto & item : bibles) {
    bible = item;
  }
  return bible;
}


Passage Database_Modifications::getNotificationPassage (int id)
{
  Passage passage;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT book, chapter, verse FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size (); i++) {
    passage.book = convert_to_int (books [i]);
    passage.chapter = convert_to_int (chapters [i]);
    passage.verse = verses [i];
  }
  return passage;
}


string Database_Modifications::getNotificationOldText (int id)
{
  string path = notificationIdentifierDatabase (id);
  if (!file_or_dir_exists (path)) return "";
  SqliteDatabase sql (path);
  sql.add ("SELECT oldtext FROM notification;");
  vector <string> result = sql.query () ["oldtext"];
  if (result.empty ()) return "";
  return result [0];
}


string Database_Modifications::getNotificationModification (int id)
{
  string path = notificationIdentifierDatabase (id);
  if (!file_or_dir_exists (path)) return "";
  SqliteDatabase sql (path);
  sql.add ("SELECT modification FROM notification;");
  vector <string> result = sql.query () ["modification"];
  if (result.empty ()) return "";
  return result [0];
}


string Database_Modifications::getNotificationNewText (int id)
{
  string path = notificationIdentifierDatabase (id);
  if (!file_or_dir_exists (path)) return "";
  SqliteDatabase sql (path);
  sql.add ("SELECT newtext FROM notification;");
  vector <string> result = sql.query () ["newtext"];
  if (result.empty ()) return "";
  return result [0];
}


void Database_Modifications::clearNotificationsUser (const string& username)
{
  vector <int> identifiers = getNotificationIdentifiers (username);
  sqlite3 * db = connect ();
  // Transaction speeds up the operation.
  database_sqlite_exec (db, "BEGIN;");
  for (auto& identifier : identifiers) {
    deleteNotification (identifier, db);
  }
  database_sqlite_exec (db, "COMMIT;");
  database_sqlite_disconnect (db);
}


// This function deletes personal change proposals and their matching change notifications.
// It returns the deleted identifiers.
vector <int> Database_Modifications::clearNotificationMatches (const string& username, const string& personal, const string& team)
{
  sqlite3 * db = connect ();
  
  // Select all identifiers of the personal change proposals.
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT identifier FROM notifications WHERE username =");
  sql.add (username);
  sql.add ("AND category =");
  sql.add (personal);
  sql.add (";");

  vector <int> personals;
  vector <string> result = database_sqlite_query (db, sql.sql) ["identifier"];
  for (auto & item : result) {
    personals.push_back (convert_to_int (item));
  }
  
  // Matches to be deleted.
  vector <int> deletes;

  // Go through each of the personal change proposals.
  for (auto & personalID : personals) {
    string bible = getNotificationBible (personalID);
    Passage passage = getNotificationPassage (personalID);
    int book = passage.book;
    int chapter = passage.chapter;
    int verse = convert_to_int (passage.verse);
    string modification = getNotificationModification (personalID);
    // Get all matching identifiers from the team's change notifications.
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT identifier FROM notifications WHERE username =");
    sql.add (username);
    sql.add ("AND category =");
    sql.add (team);
    sql.add ("AND bible =");
    sql.add (bible);
    sql.add ("AND book =");
    sql.add (book);
    sql.add ("AND chapter =");
    sql.add (chapter);
    sql.add ("AND verse =");
    sql.add (verse);
    sql.add ("AND modification =");
    sql.add (modification);
    sql.add (";");
    vector <int> teamMatches;
    vector <string> result = database_sqlite_query (db, sql.sql) ["identifier"];
    for (auto & item : result) {
      teamMatches.push_back (convert_to_int (item));
    }
    // There should be exactly one candidate for the matches to be removed.
    // If there are none, it means that the personal change didn't make it to the team's text.
    // If there are two or more matching changes, then one could have undone the other, so should not be automatically removed.
    if (teamMatches.size () == 1) {
      // Check there are only two change notifications for this user / Bible / book / chapter / verse.
      // If there are more, we can't be sure that the personal change proposal was not overwritten somehow.
      vector <int> passageMatches;
      SqliteSQL sql = SqliteSQL ();
      sql.add ("SELECT identifier FROM notifications WHERE username =");
      sql.add (username);
      sql.add ("AND bible =");
      sql.add (bible);
      sql.add ("AND book =");
      sql.add (book);
      sql.add ("AND chapter =");
      sql.add (chapter);
      sql.add ("AND verse =");
      sql.add (verse);
      sql.add (";");
      vector <string> result = database_sqlite_query (db, sql.sql) ["identifier"];
      for (auto & item : result) {
        passageMatches.push_back (convert_to_int (item));
      }
      if (passageMatches.size () == 2) {
        // Store the personal change proposal to be deleted.
        // Store the matching change notification to be deleted also.
        for (auto & passageMatch : passageMatches) {
          deletes.push_back (passageMatch);
        }
      }
    }
  }

  // Delete all stored identifiers to be deleted.
  for (auto & id : deletes) {
    deleteNotification (id, db);
  }
  
  database_sqlite_disconnect (db);
  
  // Return deleted identifiers.
  return deletes;
}


// Store a change notification on the client, as received from the server.
void Database_Modifications::storeClientNotification (int id, string username, string category, string bible, int book, int chapter, int verse, string oldtext, string modification, string newtext)
{
  // Erase any existing database.
  deleteNotificationFile (id);
  // Timestamp is not used: Just put the current time.
  int timestamp = filter_date_seconds_since_epoch ();
  {
    SqliteDatabase sql (notificationIdentifierDatabase (id));
    sql.add (createNotificationsDbSql ());
    sql.execute ();
    sql.clear ();
    sql.add ("INSERT INTO notification VALUES (");
    sql.add (timestamp);
    sql.add (",");
    sql.add (username);
    sql.add (",");
    sql.add (category);
    sql.add (",");
    sql.add (bible);
    sql.add (",");
    sql.add (book);
    sql.add (",");
    sql.add (chapter);
    sql.add (",");
    sql.add (verse);
    sql.add (",");
    sql.add (oldtext);
    sql.add (",");
    sql.add (modification);
    sql.add (",");
    sql.add (newtext);
    sql.add (");");
    sql.execute ();
  }
  {
    sqlite3 * db = connect ();
    SqliteSQL sql = SqliteSQL ();
    sql.add ("INSERT INTO notifications VALUES (");
    sql.add (id);
    sql.add (",");
    sql.add (timestamp);
    sql.add (",");
    sql.add (username);
    sql.add (",");
    sql.add (category);
    sql.add (",");
    sql.add (bible);
    sql.add (",");
    sql.add (book);
    sql.add (",");
    sql.add (chapter);
    sql.add (",");
    sql.add (verse);
    sql.add (",");
    sql.add (modification);
    sql.add (");");
    database_sqlite_exec (db, sql.sql);
    database_sqlite_disconnect (db);
  }
}


const char * Database_Modifications::createNotificationsDbSql ()
{
  return
  "CREATE TABLE notification ("
  " timestamp integer,"
  " username text,"
  " category text,"
  " bible text,"
  " book integer,"
  " chapter integer,"
  " verse integer,"
  " oldtext text,"
  " modification text,"
  " newtext text"
  ");";
}


void Database_Modifications::deleteNotificationFile (int identifier)
{
  string path = notificationIdentifierDatabase (identifier);
  // Delete the old folder from the file system (used till Februari 2016).
  if (filter_url_is_dir (path)) filter_url_rmdir (path);
  // Delete the new database file from the file system.
  if (file_or_dir_exists (path)) filter_url_unlink (path);
}


vector <string> Database_Modifications::getCategories ()
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT DISTINCT category FROM notifications ORDER BY category;");
  vector <string> categories = sql.query ()["category"];
  return categories;
}
