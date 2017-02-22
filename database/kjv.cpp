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


#include <database/kjv.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>


// This is the database for the Strong's numbers and English glosses.
// Resilience: It is not written to.
// Chances of corruption are nearly zero.


void Database_Kjv::create ()
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("DROP TABLE IF EXISTS kjv2;");
  sql.execute ();

  sql.clear ();
  sql.add ("CREATE TABLE kjv2 (book int, chapter int, verse int, strong int, english int);");
  sql.execute ();

  sql.clear ();
  sql.add ("DROP TABLE IF EXISTS strong;");
  sql.execute ();

  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS strong (strong text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("DROP TABLE IF EXISTS english;");
  sql.execute ();

  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS english (english text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_Kjv::optimize ()
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


const char * Database_Kjv::filename ()
{
  return "kjv";
}


// Get Strong's numbers and English snippets for book / chapter / verse.
vector <Database_Kjv_Item> Database_Kjv::getVerse (int book, int chapter, int verse)
{
  vector <Database_Kjv_Item> hits;
  vector <int> rows = rowids (book, chapter, verse);
  for (auto row : rows) {
    Database_Kjv_Item item;
    item.strong = strong (row);
    item.english = english (row);
    hits.push_back (item);
  }
  return hits;
}


// Get all passages that contain a strong's number.
vector <Passage> Database_Kjv::searchStrong (string strong)
{
  int strongid = get_id ("strong", strong);
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT DISTINCT book, chapter, verse FROM kjv2 WHERE strong =");
  sql.add (strongid);
  sql.add ("ORDER BY rowid;");
  vector <Passage> hits;
  map <string, vector <string> > result = sql.query ();
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size (); i++) {
    Passage passage;
    passage.book = convert_to_int (books [i]);
    passage.chapter = convert_to_int (chapters [i]);
    passage.verse = verses [i];
    hits.push_back (passage);
  }
  return hits;
}


void Database_Kjv::store (int book, int chapter, int verse, string strong, string english)
{
  int strongid = get_id ("strong", strong);
  int englishid = get_id ("english", english);
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("PRAGMA temp_store = MEMORY;");
  sql.execute ();
  sql.clear ();
  sql.add ("PRAGMA synchronous = OFF;");
  sql.execute ();
  sql.clear ();
  sql.add ("PRAGMA journal_mode = OFF;");
  sql.execute ();
  sql.clear ();
  sql.add ("INSERT INTO kjv2 VALUES (");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (strongid);
  sql.add (",");
  sql.add (englishid);
  sql.add (");");
  sql.execute ();
}


vector <int> Database_Kjv::rowids (int book, int chapter, int verse)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT rowid FROM kjv2 WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add ("ORDER BY rowid;");
  vector <string> result = sql.query () ["rowid"];
  vector <int> rowids;
  for (auto rowid : result) rowids.push_back (convert_to_int (rowid));
  return rowids;
}


string Database_Kjv::strong (int rowid)
{
  return get_item ("strong", rowid);
}


string Database_Kjv::english (int rowid)
{
  return get_item ("english", rowid);
}


int Database_Kjv::get_id (const char * table_row, string item)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  // Two iterations to be sure a rowid can be returned.
  for (unsigned int i = 0; i < 2; i++) {
    // Check on the rowid and return it if it's there.
    sql.clear ();
    sql.add ("SELECT rowid FROM");
    sql.add (table_row);
    sql.add ("WHERE");
    sql.add (table_row);
    sql.add ("=");
    sql.add (item);
    sql.add (";");
    vector <string> result = sql.query () ["rowid"];
    if (!result.empty ()) return convert_to_int (result [0]);
    // The rowid was not found: Insert the word into the table.
    // The rowid will now be found during the second iteration.
    sql.clear ();
    sql.add ("INSERT INTO");
    sql.add (table_row);
    sql.add ("VALUES (");
    sql.add (item);
    sql.add (");");
    sql.execute ();
  }
  return 0;
}


string Database_Kjv::get_item (const char * item, int rowid)
{
  // The $rowid refers to the main table.
  // Update it so it refers to the sub table.
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT");
  sql.add (item);
  sql.add ("FROM kjv2 WHERE rowid =");
  sql.add (rowid);
  sql.add (";");
  vector <string> result = sql.query () [item];
  rowid = 0;
  if (!result.empty ()) rowid = convert_to_int (result [0]);
  // Retrieve the requested value from the sub table.
  sql.clear ();
  sql.add ("SELECT");
  sql.add (item);
  sql.add ("FROM");
  sql.add (item);
  sql.add ("WHERE rowid =");
  sql.add (rowid);
  sql.add (";");
  result = sql.query () [item];
  if (!result.empty ()) return result [0];
  // Not found.
  return "";
}
