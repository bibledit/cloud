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


#include <database/check.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>


// Database resilience.
// Write operations to table "suppress" are very infrequent.
// Write operations to table "output" happen every night.
// In case of database corruption, running the Bibledit setup re-creates
// this table. The table does not contain important data.
// In cases of extreme corruption, the database file should be manually removed
// before running setup.


const char * Database_Check::filename ()
{
  return "check";
}


void Database_Check::create ()
{
  SqliteDatabase sql (filename ());

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


void Database_Check::optimize ()
{
  SqliteDatabase sql (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_Check::truncateOutput (string bible)
{
  SqliteDatabase sql (filename ());
  if (bible == "") {
    sql.add ("DELETE FROM output2;");
  } else {
    sql.add ("DELETE FROM output2 WHERE bible =");
    sql.add (bible);
    sql.add (";");
  }
  sql.execute ();
}


void Database_Check::recordOutput (string bible, int book, int chapter, int verse, string data)
{
  SqliteDatabase sql (filename ());
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
  vector <string> result = sql.query () ["count(*)"];
  if (!result.empty ()) {
    count = convert_to_int (result [0]);
  }
  if (count == 0) {
    // Check how often $data has been recorded already.
    sql.clear ();
    sql.add ("SELECT count(*) FROM output2 WHERE bible =");
    sql.add (bible);
    sql.add ("AND data = ");
    sql.add (data);
    sql.add (";");
    vector <string> result = sql.query () ["count(*)"];
    if (!result.empty ()) {
      count = convert_to_int (result [0]);
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
      data.append (" (displaying no more of these)");
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


vector <Database_Check_Hit> Database_Check::getHits ()
{
  vector <Database_Check_Hit> hits;
  SqliteDatabase sql (filename ());
  sql.add ("SELECT rowid, bible, book, chapter, verse, data FROM output2;");
  map <string, vector <string> > result = sql.query ();
  vector <string> rowids = result ["rowid"];
  vector <string> bibles = result ["bible"];
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  vector <string> data = result ["data"];
  for (unsigned int i = 0; i < rowids.size(); i++) {
    Database_Check_Hit hit = Database_Check_Hit ();
    hit.rowid = convert_to_int (rowids [i]);
    hit.bible = bibles [i];
    hit.book = convert_to_int (books [i]);
    hit.chapter = convert_to_int (chapters [i]);
    hit.verse = convert_to_int (verses [i]);
    hit.data = data [i];
    hits.push_back (hit);
  }
  return hits;
}


void Database_Check::approve (int id)
{
  // The query moves all values, apart from the auto_increment id.
  SqliteDatabase sql (filename ());
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


void Database_Check::erase (int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("DELETE FROM output2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


Passage Database_Check::getPassage (int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT book, chapter, verse FROM output2 WHERE rowid =");
  sql.add (id);
  sql.add (";");
  map <string, vector <string> > result = sql.query ();
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size(); i++) {
    Passage passage = Passage ("", convert_to_int (books[i]), convert_to_int (chapters[i]), verses[i]);
    return passage;
  }
  return Passage ("", 0, 0, "");
}


vector <Database_Check_Hit> Database_Check::getSuppressions ()
{
  SqliteDatabase sql (filename ());
  vector <Database_Check_Hit> hits;
  sql.add ("SELECT rowid, bible, book, chapter, verse, data FROM suppress2;");
  map <string, vector <string> > result = sql.query ();
  vector <string> rowids = result ["rowid"];
  vector <string> bibles = result ["bible"];
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  vector <string> data = result ["data"];
  for (unsigned int i = 0; i < rowids.size(); i++) {
    Database_Check_Hit hit = Database_Check_Hit ();
    hit.rowid = convert_to_int (rowids [i]);
    hit.bible = bibles [i];
    hit.book = convert_to_int (books [i]);
    hit.chapter = convert_to_int (chapters [i]);
    hit.verse = convert_to_int (verses [i]);
    hit.data = data [i];
    hits.push_back (hit);
  }
  return hits;
}


void Database_Check::release (int id)
{
  SqliteDatabase sql (filename ());
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

