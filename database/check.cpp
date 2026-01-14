/*
Copyright (©) 2003-2026 Teus Benschop.

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


#include <database/check.h>
#include <filter/url.h>
#include <filter/passage.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>
#include <database/check.h>
#include <locale/translate.h>


// Database resilience.
// Write operations to table "suppress" are very infrequent.
// Write operations to table "output" happen every night.
// In case of database corruption, running the Bibledit setup re-creates
// this table. The table does not contain important data.
// In cases of extreme corruption, the database file should be manually removed
// before running setup.


constexpr const auto database_name {"check"};


namespace database::check {


void create ()
{
  SqliteDatabase sql (database_name);
  
  sql.add ("DROP TABLE IF EXISTS output;");
  sql.execute ();
  
  sql.clear ();
  
  sql.add ("CREATE TABLE IF NOT EXISTS output2 ("
           " bible text,"
           " book integer,"
           " chapter integer,"
           " verse integer,"
           " data text"
           ");");
  sql.execute ();
  
  sql.clear ();
  
  sql.add ("DROP TABLE IF EXISTS suppress;");
  sql.execute ();
  
  sql.clear ();
  
  sql.add ("CREATE TABLE IF NOT EXISTS suppress2 ("
           " bible text,"
           " book integer,"
           " chapter integer,"
           " verse integer,"
           " data text"
           ");");
  sql.execute ();
}


void optimize ()
{
  SqliteDatabase sql (database_name);
  sql.add ("VACUUM;");
  sql.execute ();
}


void record_output (const std::string& bible, int book, int chapter, int verse, std::string data)
{
  SqliteDatabase sql (database_name);
  int count = 0;
  // Check whether this is a suppressed item.
  // If it was suppressed, do not record it.
  sql.clear ();
  sql.add ("SELECT count(*) FROM suppress2 WHERE bible =");
  sql.add (bible);
  sql.add ("AND book = ");
  sql.add (book);
  sql.add ("AND chapter = ");
  sql.add (chapter);
  sql.add ("AND verse = ");
  sql.add (verse);
  sql.add ("AND data = ");
  sql.add (data);
  sql.add (";");
  std::vector <std::string> result = sql.query () ["count(*)"];
  if (!result.empty ()) {
    count = filter::string::convert_to_int (result [0]);
  }
  if (count == 0) {
    // Check how often $data has been recorded already.
    sql.clear ();
    sql.add ("SELECT count(*) FROM output2 WHERE bible =");
    sql.add (bible);
    sql.add ("AND data = ");
    sql.add (data);
    sql.add (";");
    std::vector <std::string> count_result = sql.query () ["count(*)"];
    if (!count_result.empty ()) {
      count = filter::string::convert_to_int (count_result [0]);
    }
    // Record the data no more than so often.
    if (count < 10) {
      sql.clear ();
      sql.add ("INSERT INTO output2 VALUES (");
      sql.add (bible);
      sql.add (",");
      sql.add (book);
      sql.add (",");
      sql.add (chapter);
      sql.add (",");
      sql.add (verse);
      sql.add (",");
      sql.add (data);
      sql.add (");");
      sql.execute ();
    }
    // Store message saying that no more of this messages will be stored.
    // This is for situations where the checks produce thousands and thousands of results.
    // That would choke the entire server.
    // It has been seen on a service provider that its system shut Bibledit's server down
    // due to excessive CPU usage during a long time.
    if (count == 9) {
      data.append (" (" + translate ("displaying no more of these") + ")");
      sql.clear ();
      sql.add ("INSERT INTO output2 VALUES (");
      sql.add (bible);
      sql.add (",");
      sql.add (book);
      sql.add (",");
      sql.add (chapter);
      sql.add (",");
      sql.add (verse);
      sql.add (",");
      sql.add (data);
      sql.add (");");
      sql.execute ();
    }
  }
}


std::vector <database::check::Hit> get_hits ()
{
  std::vector <database::check::Hit> hits;
  SqliteDatabase sql (database_name);
  sql.add ("SELECT rowid, bible, book, chapter, verse, data FROM output2;");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  std::vector <std::string> rowids = result ["rowid"];
  std::vector <std::string> bibles = result ["bible"];
  std::vector <std::string> books = result ["book"];
  std::vector <std::string> chapters = result ["chapter"];
  std::vector <std::string> verses = result ["verse"];
  std::vector <std::string> data = result ["data"];
  for (unsigned int i = 0; i < rowids.size(); i++) {
    database::check::Hit hit = database::check::Hit ();
    hit.rowid = filter::string::convert_to_int (rowids [i]);
    hit.bible = bibles [i];
    hit.book = filter::string::convert_to_int (books [i]);
    hit.chapter = filter::string::convert_to_int (chapters [i]);
    hit.verse = filter::string::convert_to_int (verses [i]);
    hit.data = data [i];
    hits.push_back (hit);
  }
  return hits;
}


void approve (int id)
{
  // The query moves all values, apart from the auto_increment id.
  SqliteDatabase sql (database_name);
  sql.add ("INSERT INTO suppress2 (bible, book, chapter, verse, data) SELECT bible, book, chapter, verse, data FROM output2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
  sql.clear ();
  sql.add ("DELETE FROM output2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


void delete_id (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("DELETE FROM output2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


// This deletes all entries for the given Bible.
// If the Bible is left empty, then it deletes all entries.
void delete_output (const std::string& bible)
{
  SqliteDatabase sql (database_name);
  if (bible.empty()) {
    sql.add ("DELETE FROM output2;");
  } else {
    sql.add ("DELETE FROM output2 WHERE bible =");
    sql.add (bible);
    sql.add (";");
  }
  sql.execute ();
}


Passage get_passage (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT book, chapter, verse FROM output2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  std::vector <std::string> books = result ["book"];
  std::vector <std::string> chapters = result ["chapter"];
  std::vector <std::string> verses = result ["verse"];
  if (!books.empty()) {
    Passage passage = Passage ("", filter::string::convert_to_int (books[0]), filter::string::convert_to_int (chapters[0]), verses[0]);
    return passage;
  }
  return Passage ("", 0, 0, "");
}


std::vector <database::check::Hit> get_suppressions ()
{
  SqliteDatabase sql (database_name);
  std::vector <database::check::Hit> hits;
  sql.add ("SELECT rowid, bible, book, chapter, verse, data FROM suppress2;");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  std::vector <std::string> rowids = result ["rowid"];
  std::vector <std::string> bibles = result ["bible"];
  std::vector <std::string> books = result ["book"];
  std::vector <std::string> chapters = result ["chapter"];
  std::vector <std::string> verses = result ["verse"];
  std::vector <std::string> data = result ["data"];
  for (unsigned int i = 0; i < rowids.size(); i++) {
    database::check::Hit hit = database::check::Hit ();
    hit.rowid = filter::string::convert_to_int (rowids [i]);
    hit.bible = bibles [i];
    hit.book = filter::string::convert_to_int (books [i]);
    hit.chapter = filter::string::convert_to_int (chapters [i]);
    hit.verse = filter::string::convert_to_int (verses [i]);
    hit.data = data [i];
    hits.push_back (hit);
  }
  return hits;
}


void release (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("INSERT INTO output2 (bible, book, chapter, verse, data) SELECT bible, book, chapter, verse, data FROM suppress2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
  sql.clear ();
  sql.add ("DELETE FROM suppress2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


} // Namespace.
