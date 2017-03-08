/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <database/morphgnt.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// This is the database for the Greek Bible text morphology.
// Resilience: It is not written to.
// Chances of corruption are nearly zero.


const char * Database_MorphGnt::filename ()
{
  return "morphgnt";
}


void Database_MorphGnt::create ()
{
  filter_url_unlink (database_sqlite_file (filename ()));

  SqliteDatabase sql = SqliteDatabase (filename ());
  
  sql.clear ();
  sql.add ("CREATE TABLE morphgnt (book int, chapter int, verse int, pos int, parsing int, word int, lemma int);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS pos (pos text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS parsing (parsing text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS word (word text);");
  sql.execute ();

  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS lemma (lemma text);");
  sql.execute ();
}


void Database_MorphGnt::optimize ()
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_MorphGnt::store (int book, int chapter, int verse,
                               string pos, string parsing, string word, string lemma)
{
  int pos_id = get_id ("pos", pos);
  int parsing_id = get_id ("parsing", parsing);
  int word_id = get_id ("word", word);
  int lemma_id = get_id ("lemma", lemma);
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
  sql.add ("INSERT INTO morphgnt VALUES (");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (pos_id);
  sql.add (",");
  sql.add (parsing_id);
  sql.add (",");
  sql.add (word_id);
  sql.add (",");
  sql.add (lemma_id);
  sql.add (");");
  sql.execute ();
}


vector <int> Database_MorphGnt::rowids (int book, int chapter, int verse)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT rowid FROM morphgnt WHERE book =");
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


string Database_MorphGnt::pos (int rowid)
{
  return get_item ("pos", rowid);
}


string Database_MorphGnt::parsing (int rowid)
{
  return get_item ("parsing", rowid);
}


string Database_MorphGnt::word (int rowid)
{
  return get_item ("word", rowid);
}


string Database_MorphGnt::lemma (int rowid)
{
  return get_item ("lemma", rowid);
}


int Database_MorphGnt::get_id (const char * table_row, string item)
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


string Database_MorphGnt::get_item (const char * item, int rowid)
{
  // The $rowid refers to the main table.
  // Update it so it refers to the sub table.
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT");
  sql.add (item);
  sql.add ("FROM morphgnt WHERE rowid =");
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
