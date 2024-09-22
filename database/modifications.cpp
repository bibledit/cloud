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


#include <database/modifications.h>
#include <database/bibles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <database/sqlite.h>
#include <database/logic.h>


// Database resilience:
// All information is stored in the filesystem as files or as very small databases.
// The large database is only used as an index.
// It is re-indexed every night.


constexpr const char * database_name {"modifications"};


static std::string teamFolder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "modifications", "team"});
}


static std::string teamFile (const std::string& bible, int book, int chapter)
{
  return filter_url_create_path ({teamFolder (), bible + "." + std::to_string (book) + "." + std::to_string (chapter)});
}


static std::string userMainFolder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "modifications", "users"});
}


static std::string userUserFolder (const std::string& username)
{
  return filter_url_create_path ({userMainFolder (), username});
}


static std::string userBibleFolder (const std::string& username, const std::string& bible)
{
  return filter_url_create_path ({userUserFolder (username), bible});
}


static std::string userBookFolder (const std::string& username, const std::string& bible, int book)
{
  return filter_url_create_path ({userBibleFolder (username, bible), std::to_string (book)});
}


static std::string userChapterFolder (const std::string& username, const std::string& bible, int book, int chapter)
{
  return filter_url_create_path ({userBookFolder (username, bible, book), std::to_string (chapter)});
}


static std::string userNewIDFolder (const std::string& username, const std::string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path ({userChapterFolder (username, bible, book, chapter), std::to_string (newID)});
}


static std::string userTimeFile (const std::string& username, const std::string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path ({userNewIDFolder (username, bible, book, chapter, newID), "time"});
}


static std::string userOldIDFile (const std::string& username, const std::string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path ({userNewIDFolder (username, bible, book, chapter, newID), "oldid"});
}


static std::string userOldTextFile (const std::string& username, const std::string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path ({userNewIDFolder (username, bible, book, chapter, newID), "oldtext"});
}


static std::string userNewTextFile (const std::string& username, const std::string& bible, int book, int chapter, int newID)
{
  return filter_url_create_path ({userNewIDFolder (username, bible, book, chapter, newID), "newtext"});
}


static std::string notificationsMainFolder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "modifications", "notifications"});
}


static std::string notificationIdentifierDatabase (int identifier)
{
  return filter_url_create_path ({notificationsMainFolder (), std::to_string (identifier)});
}


static const char * createNotificationsDbSql ()
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


static void deleteNotificationFile (int identifier)
{
  std::string path = notificationIdentifierDatabase (identifier);
  // Delete the old folder from the file system (used till Februari 2016).
  if (filter_url_is_dir (path)) filter_url_rmdir (path);
  // Delete the new database file from the file system.
  if (file_or_dir_exists (path)) filter_url_unlink (path);
}


static void deleteNotification (SqliteDatabase& sql, int identifier)
{
  // Delete from file.
  deleteNotificationFile (identifier);
  // Delete from the database.
  sql.push_sql ();
  sql.add ("DELETE FROM notifications WHERE identifier =");
  sql.add (identifier);
  sql.add (";");
  sql.execute ();
  sql.pop_sql ();
}


namespace database::modifications {


// Delete the entire database
void erase ()
{
  std::string file = database::sqlite::get_file ("modifications");
  filter_url_unlink (file);
}


void create ()
{
  SqliteDatabase sql (database_name);
  sql.set_sql ("CREATE TABLE IF NOT EXISTS notifications ("
               " identifier integer,"
               " timestamp integer,"
               " username text,"
               " category text,"
               " bible text,"
               " book integer,"
               " chapter integer,"
               " verse integer,"
               " modification text"
               ");");
  sql.execute ();
}


bool healthy ()
{
  return database::sqlite::healthy ("modifications");
}


void vacuum ()
{
  SqliteDatabase sql (database_name);
  sql.set_sql ("VACUUM;");
  sql.execute ();
}


// Code dealing with the "teams" data.


// Returns true if diff data exists for the chapter.
// Else it returns false.
bool teamDiffExists (const std::string& bible, int book, int chapter)
{
  std::string file = teamFile (bible, book, chapter);
  return file_or_dir_exists (file);
}


// Stores diff data for a "bible" (string) and book (int) and chapter (int).
void storeTeamDiff (const std::string& bible, int book, int chapter)
{
  // Return when the diff exists.
  if (teamDiffExists (bible, book, chapter)) return;
  
  // Retrieve current chapter USFM data.
  std::string data = database::bibles::get_chapter (bible, book, chapter);
  
  // Store.
  std::string file = teamFile (bible, book, chapter);
  filter_url_file_put_contents (file, data);
}


// Gets the diff data as a string.
std::string getTeamDiff (const std::string& bible, int book, int chapter)
{
  std::string file = teamFile (bible, book, chapter);
  return filter_url_file_get_contents (file);
}


// Stores diff data for all chapters in a "bible" (string) and book (int).
void storeTeamDiffBook (const std::string& bible, int book)
{
  std::vector <int> chapters = database::bibles::get_chapters (bible, book);
  for (auto & chapter : chapters) {
    storeTeamDiff (bible, book, chapter);
  }
}


// Stores diff data for all books in a "bible" (string).
void storeTeamDiffBible (const std::string& bible)
{
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto & book : books) {
    storeTeamDiffBook (bible, book);
  }
}


// Deletes the diffs for a whole Bible.
void deleteTeamDiffBible (const std::string& bible)
{
  std::string pattern = bible + ".";
  size_t length = pattern.length ();
  std::vector <std::string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    filter_url_unlink (filter_url_create_path ({teamFolder (), file}));
  }
}


// Deletes the diffs for one chapter of a Bible.
void deleteTeamDiffChapter (const std::string& bible, int book, int chapter)
{
  std::string file = teamFile (bible, book, chapter);
  filter_url_unlink (file);
}


// Returns an array with the available chapters that have diff data in a book in a Bible.
std::vector <int> getTeamDiffChapters (const std::string& bible, int book)
{
  std::string pattern = bible + "." + std::to_string (book) + ".";
  size_t length = pattern.length ();
  std::vector <int> chapters;
  std::vector <std::string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    std::vector <std::string> bits = filter::strings::explode (file, '.');
    if (bits.size() != 3) continue;
    std::string path = filter_url_create_path ({teamFolder (), file});
    const int time = filter_url_file_modification_time (path);
    const int days = (filter::date::seconds_since_epoch () - time) / 86400;
    if (days > 5) {
      // Unprocessed team changes older than so many days usually indicate a problem.
      // Perhaps the server crashed so it never could process them.
      // Cases like this have been seen on servers with limited memory.
      // Therefore just remove this change, without processing it.
      filter_url_unlink (path);
    } else {
      chapters.push_back (filter::strings::convert_to_int (bits [2]));
    }
  }
  std::sort (chapters.begin(), chapters.end());
  return chapters;
}


// Returns the number of items in bible that have diff data.
// The bible can be a name or an identifier. This is because the bible identifier may no longer exist.
int getTeamDiffCount (const std::string& bible)
{
  std::string pattern = bible + ".";
  size_t length = pattern.length ();
  int count = 0;
  std::vector <std::string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    count++;
  }
  return count;
}


// Returns an array with the available books that have diff data in a Bible.
// The bible can be a name, or an identifier. This is because the bible identifier may no longer exist.
std::vector <int> getTeamDiffBooks (const std::string& bible)
{
  std::vector <int> books;
  std::string pattern = bible + ".";
  size_t length = pattern.length ();
  std::vector <std::string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    if (file.substr (0, length) != pattern) continue;
    std::vector <std::string> bits = filter::strings::explode (file, '.');
    if (bits.size() != 3) continue;
    books.push_back (filter::strings::convert_to_int (bits [1]));
  }
  std::set <int> bookset (books.begin (), books.end());
  books.assign (bookset.begin(), bookset.end());
  std::sort (books.begin(), books.end());
  return books;
}


// Returns an array with the available Bibles that have diff data.
std::vector <std::string> getTeamDiffBibles ()
{
  std::vector <std::string> bibles;
  std::vector <std::string> files = filter_url_scandir (teamFolder ());
  for (auto & file : files) {
    std::vector <std::string> bits = filter::strings::explode (file, '.');
    if (bits.size() != 3) continue;
    bibles.push_back (bits [0]);
  }
  std::set <std::string> bibleset (bibles.begin (), bibles.end());
  bibles.assign (bibleset.begin(), bibleset.end());
  sort (bibles.begin(), bibles.end());
  return bibles;
}


// Truncates all team modifications.
void truncateTeams ()
{
  std::vector <std::string> files = filter_url_scandir (teamFolder ());
  for (auto file : files) {
    filter_url_unlink (filter_url_create_path ({teamFolder (), file}));
  }
}


// Code dealing with the "users" data.


void recordUserSave (const std::string& username, const std::string& bible, int book, int chapter, int oldID, const std::string& oldText, int newID, const std::string& newText)
{
  // This entry is saved in a deep folder structure with the new ID in it.
  std::string folder = userNewIDFolder (username, bible, book, chapter, newID);
  if (!file_or_dir_exists (folder)) filter_url_mkdir (folder);
  // The other data is stored in separate files in the newID folder.
  std::string timeFile = userTimeFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (timeFile, std::to_string (filter::date::seconds_since_epoch ()));
  std::string oldIDFile = userOldIDFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (oldIDFile, std::to_string (oldID));
  std::string oldTextFile = userOldTextFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (oldTextFile, oldText);
  std::string newTextFile = userNewTextFile (username, bible, book, chapter, newID);
  filter_url_file_put_contents (newTextFile, newText);
}


void clearUserUser (const std::string& username)
{
  std::string folder = userUserFolder (username);
  filter_url_rmdir (folder);
}


std::vector <std::string> getUserUsernames ()
{
  std::string folder = userMainFolder ();
  std::vector <std::string> usernames = filter_url_scandir (folder);
  return usernames;
}


std::vector <std::string> getUserBibles (const std::string& username)
{
  std::string folder = userUserFolder (username);
  return filter_url_scandir (folder);
}


std::vector <int> getUserBooks (const std::string& username, const std::string& bible)
{
  std::string folder = userBibleFolder (username, bible);
  std::vector <std::string> files = filter_url_scandir (folder);
  std::vector <int> books;
  for (auto & file : files) {
    books.push_back (filter::strings::convert_to_int (file));
  }
  sort (books.begin(), books.end());
  return books;
}


std::vector <int> getUserChapters (const std::string& username, const std::string& bible, int book)
{
  const std::string folder = userBookFolder (username, bible, book);
  const std::vector <std::string> files = filter_url_scandir (folder);
  std::vector <int> chapters;
  for (const auto& file : files) {
    const std::string path = filter_url_create_path ({folder, file});
    const int time = filter_url_file_modification_time (path);
    const int days = (filter::date::seconds_since_epoch () - time) / 86400;
    if (days > 5) {
      // Unprocessed user changes older than so many days usually indicate a problem.
      // Perhaps the server crashed so it never could process them.
      // Cases like this have been seen on servers with limited memory.
      // Therefore just remove this change, without processing it.
      filter_url_rmdir (path);
    } else {
      chapters.push_back (filter::strings::convert_to_int (file));
    }
  }
  std::sort (chapters.begin(), chapters.end());
  return chapters;
}


std::vector <id_bundle> getUserIdentifiers (const std::string& username, const std::string& bible, int book, int chapter)
{
  std::string folder = userChapterFolder (username, bible, book, chapter);
  std::vector <id_bundle> ids;
  std::vector <std::string> newids = filter_url_scandir (folder);
  for (auto & newid : newids) {
    std::string file = userOldIDFile (username, bible, book, chapter, filter::strings::convert_to_int (newid));
    std::string oldid = filter_url_file_get_contents (file);
    id_bundle id;
    id.oldid = filter::strings::convert_to_int (oldid);
    id.newid = filter::strings::convert_to_int (newid);
    ids.push_back (id);
  }
  return ids;
}


text_bundle getUserChapter (const std::string& username, const std::string& bible, int book, int chapter, int newID)
{
  std::string oldfile = userOldTextFile (username, bible, book, chapter, newID);
  std::string newfile = userNewTextFile (username, bible, book, chapter, newID);
  std::string oldtext = filter_url_file_get_contents (oldfile);
  std::string newtext = filter_url_file_get_contents (newfile);
  text_bundle data;
  data.oldtext = oldtext;
  data.newtext = newtext;
  return data;
}


int getUserTimestamp (const std::string& username, const std::string& bible, int book, int chapter, int newID)
{
  std::string file = userTimeFile (username, bible, book, chapter, newID);
  std::string contents = filter_url_file_get_contents (file);
  int time = filter::strings::convert_to_int (contents);
  if (time > 0) return time;
  return filter::date::seconds_since_epoch ();
}


// Code dealing with the notifications.


// This function is for the unit tests.
void notificationUpdateTime (int identifier, int timestamp)
{
  SqliteDatabase sql (notificationIdentifierDatabase (identifier));
  sql.add ("UPDATE notification SET timestamp =");
  sql.add (timestamp);
  sql.add (";");
  sql.execute ();
}


int getNextAvailableNotificationIdentifier ()
{
  // Read the existing identifiers in the folder.
  std::vector <std::string> files = filter_url_scandir (notificationsMainFolder ());
  // Sort from low to high.
  std::vector <int> identifiers;
  for (auto file : files) {
    identifiers.push_back (filter::strings::convert_to_int (file));
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


void recordNotification (const std::vector <std::string> & users, const std::string& category, const std::string& bible, int book, int chapter, int verse, const std::string& oldtext, const std::string& modification, const std::string& newtext)
{
  // Normally this function is called just after midnight.
  // It would then put the current time on changes made the day before.
  // Make a correction for that by subtracting 6 hours.
  int timestamp = filter::date::seconds_since_epoch () - 21600;
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


void indexTrimAllNotifications ()
{
  // When the index is not healthy, delete it.
  if (!healthy ()) erase ();
  // Create a new index if it does not exist.
  create ();
  
  // Get the notification identifiers on disk.
  std::vector <std::string> sidentifiers = filter_url_scandir (notificationsMainFolder ());
  
  // Change notifications expire after 30 days.
  // But the more there are, the sooner they expire.
  int expiry_time = filter::date::seconds_since_epoch () - (30 * 3600 * 24);
  if (sidentifiers.size () > 10000) expiry_time = filter::date::seconds_since_epoch () - (14 * 3600 * 24);
  if (sidentifiers.size () > 20000) expiry_time = filter::date::seconds_since_epoch () - (7 * 3600 * 14);
  if (sidentifiers.size () > 30000) expiry_time = filter::date::seconds_since_epoch () - (4 * 3600 * 14);
  
  // Database: Connect and speed it up.
  SqliteDatabase sql (database_name);
  sql.set_sql ("PRAGMA synchronous = OFF;");
  sql.execute ();
  
  // Go through the notifications on disk.
  std::vector <int> identifiers;
  for (auto s : sidentifiers) {
    identifiers.push_back (filter::strings::convert_to_int (s));
  }
  sort (identifiers.begin(), identifiers.end());
  for (auto & identifier : identifiers) {
    
    // Fetch the data from the database and validate it.
    
    std::string path = notificationIdentifierDatabase (identifier);
    bool valid = !filter_url_is_dir (path);
    
    std::map <std::string, std::vector <std::string> > result;
    if (valid) {
      SqliteDatabase sql2 (path);
      sql2.add ("SELECT * FROM notification;");
      result = sql2.query ();
    }
    if (result.empty ()) valid = false;
    
    int timestamp = 0;
    if (valid) {
      std::vector <std::string> timestamps = result ["timestamp"];
      if (timestamps.empty ()) valid = false;
      else timestamp = filter::strings::convert_to_int (timestamps [0]);
    }
    if (timestamp < expiry_time) valid = false;
    
    bool exists = false;
    if (valid) {
      sql.clear();
      sql.add ("SELECT count(*) FROM notifications WHERE identifier = ");
      sql.add (identifier);
      sql.add (";");
      const std::vector <std::string> count_result = sql.query () ["count(*)"];
      if (!count_result.empty ()) {
        const int count = filter::strings::convert_to_int (count_result.at(0));
        exists = (count > 0);
      }
    }
    
    std::string user;
    if (!exists && valid) {
      std::vector <std::string> usernames = result ["username"];
      if (usernames.empty ()) valid = false;
      else user = usernames [0];
      if (user.empty ()) valid = false;
    }
    
    std::string category;
    if (!exists && valid) {
      std::vector <std::string> categories = result ["category"];
      if (categories.empty ()) valid = false;
      else category = categories [0];
    }
    
    std::string bible;
    if (!exists && valid) {
      std::vector <std::string> bibles = result ["bible"];
      if (bibles.empty ()) valid = false;
      else bible = bibles [0];
      if (bible.empty ()) valid = false;
    }
    
    int book = 0;
    if (!exists && valid) {
      std::vector <std::string> books = result ["book"];
      if (books.empty ()) valid = false;
      else book = filter::strings::convert_to_int (books [0]);
      if (book == 0) valid = false;
    }
    
    int chapter = 0;
    if (!exists && valid) {
      std::vector <std::string> chapters = result ["chapter"];
      if (chapters.empty ()) valid = false;
      else chapter = filter::strings::convert_to_int (chapters [0]);
      if (chapters [0].empty ()) valid = false;
    }
    
    int verse = 0;
    if (!exists && valid) {
      std::vector <std::string> verses = result ["verse"];
      if (verses.empty ()) valid = false;
      else verse = filter::strings::convert_to_int (verses [0]);
      if (verses [0].empty ()) valid = false;
    }
    
    std::string modification;
    if (!exists && valid) {
      std::vector <std::string> modifications = result ["modification"];
      if (modifications.empty ()) valid = false;
      else modification = modifications [0];
      if (modification.empty()) valid = false;
    }
    
    if (valid) {
      // Store valid data in the database if it does not yet exist.
      if (!exists) {
        sql.clear ();
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
        sql.execute ();
      }
    } else {
      // Delete invalid or expired data.
      deleteNotification (sql, identifier);
    }
  }
}


std::vector <int> getNotificationIdentifiers (std::string username, std::string bible, bool sort_on_category)
{
  std::vector <int> ids;
  
  SqliteDatabase sql (database_name);
  sql.add ("SELECT identifier FROM notifications WHERE 1");
  if (username != "") {
    sql.add ("AND username =");
    sql.add (username);
  }
  if (bible != "") {
    sql.add ("AND bible =");
    sql.add (bible);
  }
  // Sort on reference, so that related change notifications are in context.
  // Or sort on user, so it's easy to view a user's changes together.
  // https://github.com/bibledit/cloud/issues/267
  sql.add ("ORDER BY");
  if (sort_on_category) sql.add ("category ASC,");
  sql.add ("book ASC, chapter ASC, verse ASC, identifier ASC;");
  
  const std::vector <std::string> sidentifiers = sql.query () ["identifier"];
  for (auto & identifier : sidentifiers) {
    ids.push_back (filter::strings::convert_to_int (identifier));
  }
  
  return ids;
}


// This gets the identifiers of the team's changes.
std::vector <int> getNotificationTeamIdentifiers (const std::string& username, const std::string& category, std::string bible)
{
  std::vector <int> ids;
  SqliteDatabase sql (database_name);
  sql.add ("SELECT identifier FROM notifications WHERE username =");
  sql.add (username);
  sql.add ("AND category =");
  sql.add (category);
  if (bible != "") {
    sql.add ("AND bible =");
    sql.add (bible);
  }
  sql.add ("ORDER BY book ASC, chapter ASC, verse ASC, identifier ASC;");
  const std::vector <std::string> sidentifiers = sql.query () ["identifier"];
  for (const auto& sid : sidentifiers) {
    ids.push_back (filter::strings::convert_to_int (sid));
  }
  return ids;
}


// This gets the distinct Bibles in the user's notifications.
std::vector <std::string> getNotificationDistinctBibles (std::string username)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT DISTINCT bible FROM notifications WHERE 1");
  if (username != "") {
    sql.add ("AND username =");
    sql.add (username);
  }
  sql.add (";");
  const std::vector <std::string> bibles = sql.query () ["bible"];
  return bibles;
}


void deleteNotification (int identifier)
{
  SqliteDatabase sql (database_name);
  deleteNotification (sql, identifier);
}


int getNotificationTimeStamp (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT timestamp FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> timestamps = sql.query () ["timestamp"];
  int time = filter::date::seconds_since_epoch ();
  for (const auto& stamp : timestamps) {
    time = filter::strings::convert_to_int (stamp);
  }
  return time;
}


std::string getNotificationCategory (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT category FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> categories = sql.query () ["category"];
  std::string category {};
  for (const auto & row : categories) {
    category = row;
  }
  return category;
}


std::string getNotificationBible (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT bible FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> bibles = sql.query () ["bible"];
  std::string bible {};
  for (const auto& item : bibles) {
    bible = item;
  }
  return bible;
}


Passage getNotificationPassage (int id)
{
  Passage passage;
  SqliteDatabase sql (database_name);
  sql.add ("SELECT book, chapter, verse FROM notifications WHERE identifier =");
  sql.add (id);
  sql.add (";");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> books = result ["book"];
  const std::vector <std::string> chapters = result ["chapter"];
  const std::vector <std::string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size (); i++) {
    passage.m_book = filter::strings::convert_to_int (books [i]);
    passage.m_chapter = filter::strings::convert_to_int (chapters [i]);
    passage.m_verse = verses [i];
  }
  return passage;
}


std::string getNotificationOldText (int id)
{
  std::string path = notificationIdentifierDatabase (id);
  if (!file_or_dir_exists (path)) return std::string();
  SqliteDatabase sql (path);
  sql.add ("SELECT oldtext FROM notification;");
  std::vector <std::string> result = sql.query () ["oldtext"];
  if (result.empty ()) return std::string();
  return result [0];
}


std::string getNotificationModification (int id)
{
  std::string path = notificationIdentifierDatabase (id);
  if (!file_or_dir_exists (path)) return std::string();
  SqliteDatabase sql (path);
  sql.add ("SELECT modification FROM notification;");
  std::vector <std::string> result = sql.query () ["modification"];
  if (result.empty ()) return std::string();
  return result [0];
}


std::string getNotificationNewText (int id)
{
  std::string path = notificationIdentifierDatabase (id);
  if (!file_or_dir_exists (path)) return std::string();
  SqliteDatabase sql (path);
  sql.add ("SELECT newtext FROM notification;");
  std::vector <std::string> result = sql.query () ["newtext"];
  if (result.empty ()) return std::string();
  return result [0];
}


// Clears change notifications for $username.
// It does not clear all of the change notifications in all cases.
// It clears a limited number of them.
// It returns how many change notifications it cleared.
int clearNotificationsUser (const std::string& username)
{
  int cleared_counter = 0;
  std::string any_bible = "";
  std::vector <int> identifiers = getNotificationIdentifiers (username, any_bible);
  SqliteDatabase sql (database_name);
  // A transaction speeds up the operation.
  sql.set_sql ("BEGIN;");
  sql.execute ();
  for (auto& identifier : identifiers) {
    if (cleared_counter >= 100) continue;
    deleteNotification (sql, identifier);
    cleared_counter++;
  }
  sql.set_sql ("COMMIT;");
  sql.execute ();
  // How many change notifications it cleared.
  return cleared_counter;
}


// This function deletes personal changes and their matching change notifications.
// It returns the deleted identifiers.
std::vector <int> clearNotificationMatches (std::string username, std::string personal, std::string team, std::string bible)
{
  // Select all identifiers of the personal changes.
  SqliteDatabase sql (database_name);
  sql.add ("SELECT identifier FROM notifications WHERE username =");
  sql.add (username);
  sql.add ("AND category =");
  sql.add (personal);
  if (!bible.empty ()) {
    sql.add ("AND bible =");
    sql.add (bible);
  }
  sql.add (";");
  
  std::vector <int> personals;
  const std::vector <std::string> result = sql.query () ["identifier"];
  for (const auto& item : result) {
    personals.push_back (filter::strings::convert_to_int (item));
  }
  
  // Matches to be deleted.
  std::vector <int> deletes;
  
  // Go through each of the personal changes.
  for (auto & personalID : personals) {
    std::string bible2 = getNotificationBible (personalID);
    Passage passage = getNotificationPassage (personalID);
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    int verse = filter::strings::convert_to_int (passage.m_verse);
    std::string modification = getNotificationModification (personalID);
    // Get all matching identifiers from the team's change notifications.
    sql.clear();
    sql.add ("SELECT identifier FROM notifications WHERE username =");
    sql.add (username);
    sql.add ("AND category =");
    sql.add (team);
    sql.add ("AND bible =");
    sql.add (bible2);
    sql.add ("AND book =");
    sql.add (book);
    sql.add ("AND chapter =");
    sql.add (chapter);
    sql.add ("AND verse =");
    sql.add (verse);
    sql.add ("AND modification =");
    sql.add (modification);
    sql.add (";");
    std::vector <int> teamMatches;
    const std::vector <std::string> result2 = sql.query () ["identifier"];
    for (const auto& item : result2) {
      teamMatches.push_back (filter::strings::convert_to_int (item));
    }
    // There should be exactly one candidate for the matches to be removed.
    // If there are none, it means that the personal change didn't make it to the team's text.
    // If there are two or more matching changes, then one could have undone the other, so should not be automatically removed.
    if (teamMatches.size () == 1) {
      // Check there are only two change notifications for this user / Bible / book / chapter / verse.
      // If there are more, we can't be sure that the personal change was not overwritten somehow.
      std::vector <int> passageMatches;
      sql.clear();
      sql.add ("SELECT identifier FROM notifications WHERE username =");
      sql.add (username);
      sql.add ("AND bible =");
      sql.add (bible2);
      sql.add ("AND book =");
      sql.add (book);
      sql.add ("AND chapter =");
      sql.add (chapter);
      sql.add ("AND verse =");
      sql.add (verse);
      sql.add (";");
      const std::vector <std::string> result3 = sql.query () ["identifier"];
      for (const auto& item : result3) {
        passageMatches.push_back (filter::strings::convert_to_int (item));
      }
      if (passageMatches.size () == 2) {
        // Store the personal change to be deleted.
        // Store the matching change notification to be deleted also.
        for (auto & passageMatch : passageMatches) {
          deletes.push_back (passageMatch);
        }
      }
    }
  }
  
  // Delete all stored identifiers to be deleted.
  for (const auto& id : deletes) {
    deleteNotification (sql, id);
  }
  
  // Return deleted identifiers.
  return deletes;
}


// Store a change notification on the client, as received from the server.
void storeClientNotification (int id, std::string username, std::string category, std::string bible, int book, int chapter, int verse, std::string oldtext, std::string modification, std::string newtext)
{
  // Erase any existing database.
  deleteNotificationFile (id);
  // Timestamp is not used: Just put the current time.
  int timestamp = filter::date::seconds_since_epoch ();
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
    SqliteDatabase sql (database_name);
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
    sql.execute ();
  }
}


std::vector <std::string> getCategories ()
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT DISTINCT category FROM notifications ORDER BY category;");
  std::vector <std::string> categories = sql.query ()["category"];
  return categories;
}


}
