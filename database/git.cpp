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


#include <database/git.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <database/sqlite.h>


// Database resilience: It contains statistical and non-essential data.
// It is checked and optionally recreated at least once a day.


#ifdef HAVE_CLOUD


constexpr const char * database_name {"git"};


namespace database::git {


void create ()
{
  SqliteDatabase sql (database_name);
  sql.add ("CREATE TABLE IF NOT EXISTS changes ("
           " timestamp integer,"
           " user text,"
           " bible text,"
           " book integer,"
           " chapter integer,"
           " oldusfm text,"
           " newusfm text"
           ");");
  sql.execute ();
}


void optimize ()
{
  const bool healthy_database = database::sqlite::healthy (database_name);
  if (!healthy_database) {
    filter_url_unlink (database::sqlite::get_file (database_name));
    create ();
  }
  
  SqliteDatabase sql (database_name);
  
  // On Android, this pragma prevents the following error: VACUUM; Unable to open database file.
  sql.add ("PRAGMA temp_store = MEMORY;");
  sql.execute ();
  
  sql.clear ();
  
  // Delete entries older than 10 days.
  const int timestamp = filter::date::seconds_since_epoch () - 432000;
  sql.add ("DELETE FROM changes WHERE timestamp <");
  sql.add (timestamp);
  sql.add (";");
  sql.execute ();
  
  sql.clear ();
  
  sql.add ("VACUUM;");
  sql.execute ();
}


void store_chapter (const std::string& user, const std::string& bible, int book, int chapter,
                                  const std::string& oldusfm, const std::string& newusfm)
{
  SqliteDatabase sql (database_name);
  sql.add ("INSERT INTO changes VALUES (");
  sql.add (filter::date::seconds_since_epoch ());
  sql.add (",");
  sql.add (user);
  sql.add (",");
  sql.add (bible);
  sql.add (",");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (oldusfm);
  sql.add (",");
  sql.add (newusfm);
  sql.add (");");
  sql.execute ();
}


// Fetches the distinct users from the database for $bible.
std::vector <std::string> get_users (const std::string& bible)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT DISTINCT user FROM changes WHERE bible =");
  sql.add (bible);
  sql.add (";");
  std::vector <std::string> users = sql.query () ["user"];
  return users;
}


// Fetches the rowids from the database for $user and $bible.
std::vector <int> get_rowids (const std::string& user, const std::string& bible)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT rowid FROM changes WHERE user =");
  sql.add (user);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add ("ORDER BY rowid;");
  const std::vector <std::string> values = sql.query () ["rowid"];
  std::vector <int> rowids;
  for (const auto& value : values) {
    rowids.push_back (filter::strings::convert_to_int (value));
  }
  return rowids;
}


bool get_chapter (int rowid,
                  std::string & user, std::string & bible, int & book, int & chapter,
                  std::string & oldusfm, std::string & newusfm)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT * FROM changes WHERE rowid =");
  sql.add (rowid);
  sql.add (";");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> users = result ["user"];
  const std::vector <std::string> bibles = result ["bible"];
  const std::vector <std::string> books = result ["book"];
  const std::vector <std::string> chapters = result ["chapter"];
  const std::vector <std::string> oldusfms = result ["oldusfm"];
  const std::vector <std::string> newusfms = result ["newusfm"];
  if (bibles.empty ()) 
    return false;
  if (!users.empty ())
    user = users.at(0);
  if (!bibles.empty ())
    bible = bibles.at(0);
  if (!books.empty ())
    book = filter::strings::convert_to_int (books.at(0));
  if (!chapters.empty ())
    chapter = filter::strings::convert_to_int (chapters.at(0));
  if (oldusfm.empty ())
    oldusfm = oldusfms.at(0);
  if (newusfm.empty ())
    newusfm = newusfms.at(0);
  return true;
}


void erase_rowid (int rowid)
{
  SqliteDatabase sql (database_name);
  sql.add ("DELETE FROM changes WHERE rowid =");
  sql.add (rowid);
  sql.add (";");
  sql.execute ();
}


void touch_timestamps (int timestamp)
{
  SqliteDatabase sql (database_name);
  sql.add ("UPDATE changes SET timestamp =");
  sql.add (timestamp);
  sql.add (";");
  sql.execute ();
}


} // Namespace.


#endif
