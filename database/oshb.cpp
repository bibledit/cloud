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


#include <database/oshb.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>


// This is the database for the Hebrew Bible text plus lemmas and morphology.
// Resilience: It is never written to.
// Chances of corruption are nearly zero.


const char * Database_OsHb::filename ()
{
  return "oshb";
}


void Database_OsHb::create ()
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("DROP TABLE IF EXISTS oshb;");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE oshb (book int, chapter int, verse int, lemma int, word int, morph int);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("DROP TABLE IF EXISTS lemma;");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS lemma (lemma text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("DROP TABLE IF EXISTS word;");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS word (word text);");
  sql.execute ();

  sql.clear ();
  sql.add ("DROP TABLE IF EXISTS morph;");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS morph (morph text);");
  sql.execute ();
}


void Database_OsHb::optimize ()
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


// Get Hebrew words for $book $chapter $verse.
vector <string> Database_OsHb::getVerse (int book, int chapter, int verse)
{
  vector <string> words;
  vector <int> rows = rowids (book, chapter, verse);
  for (auto row : rows) {
    words.push_back (word (row));
  }
  return words;
}


// Get array of book / chapter / verse of all passages that contain a $hebrew word.
vector <Passage> Database_OsHb::searchHebrew (string hebrew)
{
  int word_id = get_id ("word", hebrew);
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT DISTINCT book, chapter, verse FROM oshb WHERE word =");
  sql.add (word_id);
  sql.add ("ORDER BY book, chapter, verse ASC;");
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


void Database_OsHb::store (int book, int chapter, int verse, string lemma, string word, string morph)
{
  int lemma_id = get_id ("lemma", lemma);
  int word_id = get_id ("word", word);
  int morph_id = get_id ("morph", morph);
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
  sql.add ("INSERT INTO oshb VALUES (");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (lemma_id);
  sql.add (",");
  sql.add (word_id);
  sql.add (",");
  sql.add (morph_id);
  sql.add (");");
  sql.execute ();
}


vector <int> Database_OsHb::rowids (int book, int chapter, int verse)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT rowid FROM oshb WHERE book =");
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


string Database_OsHb::lemma (int rowid)
{
  return get_item ("lemma", rowid);
}


string Database_OsHb::word (int rowid)
{
  return get_item ("word", rowid);
}


string Database_OsHb::morph (int rowid)
{
  return get_item ("morph", rowid);
}


int Database_OsHb::get_id (const char * table_row, string item)
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


string Database_OsHb::get_item (const char * item, int rowid)
{
  // The $rowid refers to the main table.
  // Update it so it refers to the sub table.
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT");
  sql.add (item);
  sql.add ("FROM oshb WHERE rowid =");
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
