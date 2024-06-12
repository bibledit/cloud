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


#include <database/hebrewlexicon.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// This is the database that contains Open Scriptures's Hebrew Lexicon.
// Resilience: It is not written to.
// Chances of corruption are nearly zero.


const char * Database_HebrewLexicon::filename ()
{
  return "hebrewlexicon";
}


void Database_HebrewLexicon::create ()
{
  filter_url_unlink (database::sqlite::get_file (filename ()));
  
  SqliteDatabase sql = SqliteDatabase (filename ());

  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS aug (aug text, target text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS bdb (id text, definition text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS map (id text, bdb text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS pos (code text, name text);");
  sql.execute ();
  
  sql.clear ();
  sql.add ("CREATE TABLE IF NOT EXISTS strong (strong text, definition text);");
  sql.execute ();
}


void Database_HebrewLexicon::optimize ()
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_HebrewLexicon::setaug (std::string aug, std::string target)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO aug VALUES (");
  sql.add (aug);
  sql.add (",");
  sql.add (target);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setbdb (std::string id, std::string definition)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO bdb VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (definition);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setmap (std::string id, std::string bdb)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO map VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (bdb);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setpos (std::string code, std::string name)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO pos VALUES (");
  sql.add (code);
  sql.add (",");
  sql.add (name);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setstrong (std::string strong, std::string definition)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO strong VALUES (");
  sql.add (strong);
  sql.add (",");
  sql.add (definition);
  sql.add (");");
  sql.execute ();
}


std::string Database_HebrewLexicon::getaug (std::string aug)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT target FROM aug WHERE aug =");
  sql.add (aug);
  sql.add (";");
  std::vector <std::string> result = sql.query () ["target"];
  if (!result.empty ()) return result [0];
  return std::string();
}


std::string Database_HebrewLexicon::getbdb (std::string id)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT definition FROM bdb WHERE id =");
  sql.add (id);
  sql.add (";");
  std::vector <std::string> result = sql.query () ["definition"];
  if (!result.empty ()) return result [0];
  return std::string();
}


std::string Database_HebrewLexicon::getmap (std::string id)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT bdb FROM map WHERE id =");
  sql.add (id);
  sql.add (";");
  std::vector <std::string> result = sql.query () ["bdb"];
  if (!result.empty ()) return result [0];
  return std::string();
}


std::string Database_HebrewLexicon::getpos (std::string code)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT name FROM pos WHERE code =");
  sql.add (code);
  sql.add (";");
  std::vector <std::string> result = sql.query () ["name"];
  if (!result.empty ()) return result [0];
  return std::string();
}


std::string Database_HebrewLexicon::getstrong (std::string strong)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT definition FROM strong WHERE strong =");
  sql.add (strong);
  sql.add (";");
  std::vector <std::string> result = sql.query () ["definition"];
  if (!result.empty ()) return result [0];
  return std::string();
}
