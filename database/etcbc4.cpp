/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <database/etcbc4.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <config/globals.h>
#include <database/sqlite.h>


// Database resilience: 
// The database is normally only read from.


sqlite3 * Database_Etcbc4::connect ()
{
  return database_sqlite_connect ("etcb4");
}


void Database_Etcbc4::create ()
{
  sqlite3 * db = connect ();
  string sql;

  sql =
  "CREATE TABLE IF NOT EXISTS rawdata (book int, chapter int, verse int, data text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS data;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS data (book int, chapter int, verse int, "
  "word int, vocalized_lexeme int, consonantal_lexeme int, "
  "gloss int, pos int, subpos int, "
  "gender int, number int, person int, "
  "state int, tense int, stem int, "
  "phrase_function int, phrase_type int, phrase_relation int, "
  "phrase_a_relation int, clause_text_type int, clause_type int, clause_relation int"
  ");";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS word;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS word (word text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS vocalized_lexeme;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS vocalized_lexeme (vocalized_lexeme text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS consonantal_lexeme;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS consonantal_lexeme (consonantal_lexeme text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS gloss;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS gloss (gloss text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS pos;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS pos (pos text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS subpos;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS subpos (subpos text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS gender;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS gender (gender text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS number;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS number (number text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS person;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS person (person text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS state;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS state (state text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS tense;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS tense (tense text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS stem;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS stem (stem text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS phrase_function;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS phrase_function (phrase_function text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS phrase_type;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS phrase_type (phrase_type text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS phrase_relation;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS phrase_relation (phrase_relation text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS phrase_a_relation;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS phrase_a_relation (phrase_a_relation text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS clause_text_type;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS clause_text_type (clause_text_type text);";
  database_sqlite_exec (db, sql);
  
  sql =
  "DROP TABLE IF EXISTS clause_type;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS clause_type (clause_type text);";
  database_sqlite_exec (db, sql);

  sql =
  "DROP TABLE IF EXISTS clause_relation;";
  database_sqlite_exec (db, sql);
  sql =
  "CREATE TABLE IF NOT EXISTS clause_relation (clause_relation text);";
  database_sqlite_exec (db, sql);
  
  database_sqlite_disconnect (db);
}


string Database_Etcbc4::raw (int book, int chapter, int verse)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT data FROM rawdata WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["data"];
  database_sqlite_disconnect (db);
  if (!result.empty ()) return result [0];
  return "";
}


void Database_Etcbc4::store (int book, int chapter, int verse, string data)
{
  sqlite3 * db = connect ();
  {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("DELETE FROM rawdata WHERE book =");
    sql.add (book);
    sql.add ("AND chapter =");
    sql.add (chapter);
    sql.add ("AND verse =");
    sql.add (verse);
    sql.add (";");
    database_sqlite_exec (db, sql.sql);
  }
  {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("INSERT INTO rawdata VALUES (");
    sql.add (book);
    sql.add (",");
    sql.add (chapter);
    sql.add (",");
    sql.add (verse);
    sql.add (",");
    sql.add (data);
    sql.add (");");
    database_sqlite_exec (db, sql.sql);
  }
  database_sqlite_disconnect (db);
}


void Database_Etcbc4::store (int book, int chapter, int verse,
                            string word, string vocalized_lexeme, string consonantal_lexeme,
                            string gloss, string pos, string subpos,
                            string gender, string number, string person,
                            string state, string tense, string stem,
                            string phrase_function, string phrase_type, string phrase_relation,
                            string phrase_a_relation, string clause_text_type, string clause_type, string clause_relation)
{
  sqlite3 * db = connect ();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO data VALUES (");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (get_id (db, "word", word));
  sql.add (",");
  sql.add (get_id (db, "vocalized_lexeme", vocalized_lexeme));
  sql.add (",");
  sql.add (get_id (db, "consonantal_lexeme", consonantal_lexeme));
  sql.add (",");
  sql.add (get_id (db, "gloss", gloss));
  sql.add (",");
  sql.add (get_id (db, "pos", pos));
  sql.add (",");
  sql.add (get_id (db, "subpos", subpos));
  sql.add (",");
  sql.add (get_id (db, "gender", gender));
  sql.add (",");
  sql.add (get_id (db, "number", number));
  sql.add (",");
  sql.add (get_id (db, "person", person));
  sql.add (",");
  sql.add (get_id (db, "state", state));
  sql.add (",");
  sql.add (get_id (db, "tense", tense));
  sql.add (",");
  sql.add (get_id (db, "stem", stem));
  sql.add (",");
  sql.add (get_id (db, "phrase_function", phrase_function));
  sql.add (",");
  sql.add (get_id (db, "phrase_type", phrase_type));
  sql.add (",");
  sql.add (get_id (db, "phrase_relation", phrase_relation));
  sql.add (",");
  sql.add (get_id (db, "phrase_a_relation", phrase_a_relation));
  sql.add (",");
  sql.add (get_id (db, "clause_text_type", clause_text_type));
  sql.add (",");
  sql.add (get_id (db, "clause_type", clause_type));
  sql.add (",");
  sql.add (get_id (db, "clause_relation", clause_relation));
  sql.add (");");
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


vector <int> Database_Etcbc4::books ()
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT book FROM rawdata ORDER BY book;");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["book"];
  database_sqlite_disconnect (db);
  vector <int> books;
  for (auto b : result) books.push_back (convert_to_int (b));
  return books;
}


vector <int> Database_Etcbc4::chapters (int book)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT chapter FROM rawdata WHERE book =");
  sql.add (book);
  sql.add ("ORDER BY chapter;");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["chapter"];
  database_sqlite_disconnect (db);
  vector <int> chapters;
  for (auto c : result) chapters.push_back (convert_to_int (c));
  return chapters;
}


vector <int> Database_Etcbc4::verses (int book, int chapter)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT verse FROM rawdata WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("ORDER BY verse;");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["verse"];
  database_sqlite_disconnect (db);
  vector <int> verses;
  for (auto v : result) verses.push_back (convert_to_int (v));
  return verses;
}


vector <int> Database_Etcbc4::rowids (int book, int chapter, int verse)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid FROM data WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["rowid"];
  database_sqlite_disconnect (db);
  vector <int> rowids;
  for (auto rowid : result) rowids.push_back (convert_to_int (rowid));
  return rowids;
}


string Database_Etcbc4::word (int rowid)
{
  return get_item ("word", rowid);
}


string Database_Etcbc4::vocalized_lexeme (int rowid)
{
  return get_item ("vocalized_lexeme", rowid);
}


string Database_Etcbc4::consonantal_lexeme (int rowid)
{
  return get_item ("consonantal_lexeme", rowid);
}


string Database_Etcbc4::gloss (int rowid)
{
  return get_item ("gloss", rowid);
}


string Database_Etcbc4::pos (int rowid)
{
  return get_item ("pos", rowid);
}


string Database_Etcbc4::subpos (int rowid)
{
  return get_item ("subpos", rowid);
}


string Database_Etcbc4::gender (int rowid)
{
  return get_item ("gender", rowid);
}


string Database_Etcbc4::number (int rowid)
{
  return get_item ("number", rowid);
}


string Database_Etcbc4::person (int rowid)
{
  return get_item ("person", rowid);
}


string Database_Etcbc4::state (int rowid)
{
  return get_item ("state", rowid);
}


string Database_Etcbc4::tense (int rowid)
{
  return get_item ("tense", rowid);
}


string Database_Etcbc4::stem (int rowid)
{
  return get_item ("stem", rowid);
}


string Database_Etcbc4::phrase_function (int rowid)
{
  return get_item ("phrase_function", rowid);
}


string Database_Etcbc4::phrase_type (int rowid)
{
  return get_item ("phrase_type", rowid);
}


string Database_Etcbc4::phrase_relation (int rowid)
{
  return get_item ("phrase_relation", rowid);
}


string Database_Etcbc4::phrase_a_relation (int rowid)
{
  return get_item ("phrase_a_relation", rowid);
}


string Database_Etcbc4::clause_text_type (int rowid)
{
  return get_item ("clause_text_type", rowid);
}


string Database_Etcbc4::clause_type (int rowid)
{
  return get_item ("clause_type", rowid);
}


string Database_Etcbc4::clause_relation (int rowid)
{
  return get_item ("clause_relation", rowid);
}


int Database_Etcbc4::get_id (sqlite3 * db, const char * table_row, string item)
{
  // Two iterations to be sure a rowid can be returned.
  for (unsigned int i = 0; i < 2; i++) {
    {
      // Check on the rowid and return it if it's there.
      SqliteSQL sql = SqliteSQL ();
      sql.add ("SELECT rowid FROM");
      sql.add (table_row);
      sql.add ("WHERE");
      sql.add (table_row);
      sql.add ("=");
      sql.add (item);
      sql.add (";");
      vector <string> result = database_sqlite_query (db, sql.sql) ["rowid"];
      if (!result.empty ()) return convert_to_int (result [0]);
    }
    {
      // The rowid was not found: Insert the word into the table.
      // The rowid will now be found during the second iteration.
      SqliteSQL sql = SqliteSQL ();
      sql.add ("INSERT INTO");
      sql.add (table_row);
      sql.add ("VALUES (");
      sql.add (item);
      sql.add (");");
      database_sqlite_exec (db, sql.sql);
    }
  }
  return 0;
}


string Database_Etcbc4::get_item (const char * item, int rowid)
{
  // The $rowid refers to the main table.
  // Update it so it refers to the sub table.
  sqlite3 * db = connect ();
  {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT");
    sql.add (item);
    sql.add ("FROM data WHERE rowid =");
    sql.add (rowid);
    sql.add (";");
    vector <string> result = database_sqlite_query (db, sql.sql) [item];
    rowid = 0;
    if (!result.empty ()) rowid = convert_to_int (result [0]);
  }
  // Retrieve the requested value from the sub table.
  string value;
  {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("SELECT");
    sql.add (item);
    sql.add ("FROM");
    sql.add (item);
    sql.add ("WHERE rowid =");
    sql.add (rowid);
    sql.add (";");
    vector <string> result = database_sqlite_query (db, sql.sql) [item];
    if (!result.empty ()) value = result [0];
  }
  database_sqlite_disconnect (db);
  // Done.
  return value;
}
