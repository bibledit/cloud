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


#include <database/etcbc4.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <config/globals.h>
#include <database/sqlite.h>


// Database resilience: 
// The database is normally only read from.


namespace database::etcbc4 {


constexpr const auto etcb4 {"etcb4"};


static int get_id (SqliteDatabase& sql, const char* table_row, const std::string& item)
{
  int id {0};
  
  // Two iterations to be sure a rowid can be returned.
  for (unsigned int i = 0; i < 2; i++) {

    // Save the existing SQL just now because the code below puts new SQL into the object.
    sql.push_sql();

    {
      // Check on the rowid and fetch it if it's there.
      sql.clear();
      sql.add ("SELECT rowid FROM");
      sql.add (table_row);
      sql.add ("WHERE");
      sql.add (table_row);
      sql.add ("=");
      sql.add (item);
      sql.add (";");
      const std::vector <std::string> result = sql.query () ["rowid"];
      if (!result.empty ())
        id = filter::strings::convert_to_int (result [0]);
    }
    if (!id) {
      // The rowid was not found: Insert the word into the table.
      // The rowid will now be found during the second iteration.
      sql.clear();
      sql.add ("INSERT INTO");
      sql.add (table_row);
      sql.add ("VALUES (");
      sql.add (item);
      sql.add (");");
      sql.execute ();
    }

    // Restore the previously saved SQL so the caller can again use it.
    sql.pop_sql();
    
    if (id)
      return id;
  }
  
  return id;
}


static std::string get_item (const char * item, int rowid)
{
  SqliteDatabase sql (etcb4);

  // The $rowid refers to the main table.
  // Update it so it refers to the sub table.
  {
    sql.clear();
    sql.add ("SELECT");
    sql.add (item);
    sql.add ("FROM data WHERE rowid =");
    sql.add (rowid);
    sql.add (";");
    const std::vector <std::string> result = sql.query () [item];
    rowid = 0;
    if (!result.empty ()) 
      rowid = filter::strings::convert_to_int (result.at(0));
  }

  // Retrieve the requested value from the sub table.
  std::string value {};
  {
    sql.clear();
    sql.add ("SELECT");
    sql.add (item);
    sql.add ("FROM");
    sql.add (item);
    sql.add ("WHERE rowid =");
    sql.add (rowid);
    sql.add (";");
    const std::vector <std::string> result = sql.query () [item];
    if (!result.empty ())
      value = result.at(0);
  }

  // Done.
  return value;
}


void create ()
{
  SqliteDatabase sql (etcb4);

  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS rawdata (book int, chapter int, verse int, data text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS data;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS data (book int, chapter int, verse int, "
            "word int, vocalized_lexeme int, consonantal_lexeme int, "
            "gloss int, pos int, subpos int, "
            "gender int, number int, person int, "
            "state int, tense int, stem int, "
            "phrase_function int, phrase_type int, phrase_relation int, "
            "phrase_a_relation int, clause_text_type int, clause_type int, clause_relation int"
            ");");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS word;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS word (word text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS vocalized_lexeme;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS vocalized_lexeme (vocalized_lexeme text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS consonantal_lexeme;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS consonantal_lexeme (consonantal_lexeme text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS gloss;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS gloss (gloss text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS pos;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS pos (pos text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS subpos;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS subpos (subpos text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS gender;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS gender (gender text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS number;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS number (number text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS person;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS person (person text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS state;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS state (state text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS tense;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS tense (tense text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS stem;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS stem (stem text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS phrase_function;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS phrase_function (phrase_function text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS phrase_type;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS phrase_type (phrase_type text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS phrase_relation;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS phrase_relation (phrase_relation text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS phrase_a_relation;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS phrase_a_relation (phrase_a_relation text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS clause_text_type;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS clause_text_type (clause_text_type text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS clause_type;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS clause_type (clause_type text);");
  sql.execute();
  
  sql.clear();
  sql.add ("DROP TABLE IF EXISTS clause_relation;");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS clause_relation (clause_relation text);");
  sql.execute();
}


std::string raw (int book, int chapter, int verse)
{
  SqliteDatabase sql (etcb4);
  sql.add ("SELECT data FROM rawdata WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["data"];
  if (!result.empty ())
    return result.at(0);
  return std::string();
}


void store (int book, int chapter, int verse, const std::string& data)
{
  SqliteDatabase sql (etcb4);

  sql.clear();
  sql.add ("DELETE FROM rawdata WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  sql.execute();

  sql.clear();
  sql.add ("INSERT INTO rawdata VALUES (");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (data);
  sql.add (");");
  sql.execute();
}


void store (int book, int chapter, int verse,
            std::string word, std::string vocalized_lexeme, std::string consonantal_lexeme,
            std::string gloss, std::string pos, std::string subpos,
            std::string gender, std::string number, std::string person,
            std::string state, std::string tense, std::string stem,
            std::string phrase_function, std::string phrase_type, std::string phrase_relation,
            std::string phrase_a_relation, std::string clause_text_type, std::string clause_type, std::string clause_relation)
{
  SqliteDatabase sql (etcb4);
  sql.add ("INSERT INTO data VALUES (");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (get_id (sql, "word", word));
  sql.add (",");
  sql.add (get_id (sql, "vocalized_lexeme", vocalized_lexeme));
  sql.add (",");
  sql.add (get_id (sql, "consonantal_lexeme", consonantal_lexeme));
  sql.add (",");
  sql.add (get_id (sql, "gloss", gloss));
  sql.add (",");
  sql.add (get_id (sql, "pos", pos));
  sql.add (",");
  sql.add (get_id (sql, "subpos", subpos));
  sql.add (",");
  sql.add (get_id (sql, "gender", gender));
  sql.add (",");
  sql.add (get_id (sql, "number", number));
  sql.add (",");
  sql.add (get_id (sql, "person", person));
  sql.add (",");
  sql.add (get_id (sql, "state", state));
  sql.add (",");
  sql.add (get_id (sql, "tense", tense));
  sql.add (",");
  sql.add (get_id (sql, "stem", stem));
  sql.add (",");
  sql.add (get_id (sql, "phrase_function", phrase_function));
  sql.add (",");
  sql.add (get_id (sql, "phrase_type", phrase_type));
  sql.add (",");
  sql.add (get_id (sql, "phrase_relation", phrase_relation));
  sql.add (",");
  sql.add (get_id (sql, "phrase_a_relation", phrase_a_relation));
  sql.add (",");
  sql.add (get_id (sql, "clause_text_type", clause_text_type));
  sql.add (",");
  sql.add (get_id (sql, "clause_type", clause_type));
  sql.add (",");
  sql.add (get_id (sql, "clause_relation", clause_relation));
  sql.add (");");
  sql.execute();
}


std::vector <int> books ()
{
  SqliteDatabase sql (etcb4);
  sql.add ("SELECT DISTINCT book FROM rawdata ORDER BY book;");
  const std::vector <std::string> result = sql.query () ["book"];
  std::vector <int> books;
  for (const auto& b : result)
    books.push_back (filter::strings::convert_to_int (b));
  return books;
}


std::vector <int> chapters (const int book)
{
  SqliteDatabase sql (etcb4);
  sql.add ("SELECT DISTINCT chapter FROM rawdata WHERE book =");
  sql.add (book);
  sql.add ("ORDER BY chapter;");
  const std::vector <std::string> result = sql.query () ["chapter"];
  std::vector <int> chapters;
  for (const auto& c : result)
    chapters.push_back (filter::strings::convert_to_int (c));
  return chapters;
}


std::vector <int> verses (const int book, const int chapter)
{
  SqliteDatabase sql (etcb4);
  sql.add ("SELECT DISTINCT verse FROM rawdata WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("ORDER BY verse;");
  const std::vector <std::string> result = sql.query () ["verse"];
  std::vector <int> verses;
  for (const auto& v : result)
    verses.push_back (filter::strings::convert_to_int (v));
  return verses;
}


std::vector <int> rowids (const int book, const int chapter, const int verse)
{
  SqliteDatabase sql (etcb4);
  sql.add ("SELECT rowid FROM data WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["rowid"];
  std::vector <int> rowids;
  for (const auto& rowid : result) 
    rowids.push_back (filter::strings::convert_to_int (rowid));
  return rowids;
}


std::string word (const int rowid)
{
  return get_item ("word", rowid);
}


std::string vocalized_lexeme (const int rowid)
{
  return get_item ("vocalized_lexeme", rowid);
}


std::string consonantal_lexeme (const int rowid)
{
  return get_item ("consonantal_lexeme", rowid);
}


std::string gloss (const int rowid)
{
  return get_item ("gloss", rowid);
}


std::string pos (const int rowid)
{
  return get_item ("pos", rowid);
}


std::string subpos (const int rowid)
{
  return get_item ("subpos", rowid);
}


std::string gender (const int rowid)
{
  return get_item ("gender", rowid);
}


std::string number (const int rowid)
{
  return get_item ("number", rowid);
}


std::string person (const int rowid)
{
  return get_item ("person", rowid);
}


std::string state (const int rowid)
{
  return get_item ("state", rowid);
}


std::string tense (const int rowid)
{
  return get_item ("tense", rowid);
}


std::string stem (const int rowid)
{
  return get_item ("stem", rowid);
}


std::string phrase_function (const int rowid)
{
  return get_item ("phrase_function", rowid);
}


std::string phrase_type (const int rowid)
{
  return get_item ("phrase_type", rowid);
}


std::string phrase_relation (const int rowid)
{
  return get_item ("phrase_relation", rowid);
}


std::string phrase_a_relation (const int rowid)
{
  return get_item ("phrase_a_relation", rowid);
}


std::string clause_text_type (const int rowid)
{
  return get_item ("clause_text_type", rowid);
}


std::string clause_type (const int rowid)
{
  return get_item ("clause_type", rowid);
}


std::string clause_relation (const int rowid)
{
  return get_item ("clause_relation", rowid);
}


} // Namespace.
