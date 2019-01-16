/*
Copyright (©) 2003-2018 Teus Benschop.

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


void Database_Git::create ()
{
  SqliteDatabase sql = SqliteDatabase (name ());
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


void Database_Git::optimize ()
{
  bool healthy_database = database_sqlite_healthy (name ());
  if (!healthy_database) {
    filter_url_unlink (database_sqlite_file (name ()));
    create ();
  }
  
  SqliteDatabase sql = SqliteDatabase (name ());
  
  // On Android, this pragma prevents the following error: VACUUM; Unable to open database file.
  sql.add ("PRAGMA temp_store = MEMORY;");
  sql.execute ();
  
  sql.clear ();

  // Delete entries older than 10 days.
  int timestamp = filter_date_seconds_since_epoch () - 432000;
  sql.add ("DELETE FROM changes WHERE timestamp <");
  sql.add (timestamp);
  sql.add (";");
  sql.execute ();
  
  sql.clear ();
  
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_Git::store_chapter (string user, string bible, int book, int chapter,
                                  string oldusfm, string newusfm)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("INSERT INTO changes VALUES (");
  sql.add (filter_date_seconds_since_epoch ());
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
vector <string> Database_Git::get_users (string bible)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("SELECT DISTINCT user FROM changes WHERE bible =");
  sql.add (bible);
  sql.add (";");
  vector <string> users = sql.query () ["user"];
  return users;
}


// Fetches the rowids from the database for $user and $bible.
vector <int> Database_Git::get_rowids (string user, string bible)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("SELECT rowid FROM changes WHERE user =");
  sql.add (user);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add ("ORDER BY rowid;");
  vector <string> values = sql.query () ["rowid"];
  vector <int> rowids;
  for (auto value : values) {
    rowids.push_back (convert_to_int (value));
  }
  return rowids;
}


bool Database_Git::get_chapter (int rowid,
                                string & user, string & bible, int & book, int & chapter,
                                string & oldusfm, string & newusfm)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("SELECT * FROM changes WHERE rowid =");
  sql.add (rowid);
  sql.add (";");
  map <string, vector <string> > result = sql.query ();
  vector <string> users    = result ["user"];
  vector <string> bibles   = result ["bible"];
  vector <string> books    = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> oldusfms = result ["oldusfm"];
  vector <string> newusfms = result ["newusfm"];
  if (bibles.empty ()) return false;
  if (!users.empty ())    user    = users [0];
  if (!bibles.empty ())   bible   = bibles [0];
  if (!books.empty ())    book    = convert_to_int (books [0]);
  if (!chapters.empty ()) chapter = convert_to_int (chapters [0]);
  if (oldusfm.empty ())   oldusfm = oldusfms [0];
  if (newusfm.empty ())   newusfm = newusfms [0];
  return true;
}


// Flag export of $bible $book to $format.
void Database_Git::erase_rowid (int rowid)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("DELETE FROM changes WHERE rowid =");
  sql.add (rowid);
  sql.add (";");
  sql.execute ();
}


void Database_Git::touch_timestamps (int timestamp)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("UPDATE changes SET timestamp =");
  sql.add (timestamp);
  sql.add (";");
  sql.execute ();
}


const char * Database_Git::name ()
{
  return "git";
}


#endif
