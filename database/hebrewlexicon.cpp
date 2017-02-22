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
  filter_url_unlink (database_sqlite_file (filename ()));
  
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


void Database_HebrewLexicon::setaug (string aug, string target)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO aug VALUES (");
  sql.add (aug);
  sql.add (",");
  sql.add (target);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setbdb (string id, string definition)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO bdb VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (definition);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setmap (string id, string bdb)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO map VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (bdb);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setpos (string code, string name)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO pos VALUES (");
  sql.add (code);
  sql.add (",");
  sql.add (name);
  sql.add (");");
  sql.execute ();
}


void Database_HebrewLexicon::setstrong (string strong, string definition)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("INSERT INTO strong VALUES (");
  sql.add (strong);
  sql.add (",");
  sql.add (definition);
  sql.add (");");
  sql.execute ();
}


string Database_HebrewLexicon::getaug (string aug)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT target FROM aug WHERE aug =");
  sql.add (aug);
  sql.add (";");
  vector <string> result = sql.query () ["target"];
  if (!result.empty ()) return result [0];
  return "";
}


string Database_HebrewLexicon::getbdb (string id)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT definition FROM bdb WHERE id =");
  sql.add (id);
  sql.add (";");
  vector <string> result = sql.query () ["definition"];
  if (!result.empty ()) return result [0];
  return "";
}


string Database_HebrewLexicon::getmap (string id)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT bdb FROM map WHERE id =");
  sql.add (id);
  sql.add (";");
  vector <string> result = sql.query () ["bdb"];
  if (!result.empty ()) return result [0];
  return "";
}


string Database_HebrewLexicon::getpos (string code)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT name FROM pos WHERE code =");
  sql.add (code);
  sql.add (";");
  vector <string> result = sql.query () ["name"];
  if (!result.empty ()) return result [0];
  return "";
}


string Database_HebrewLexicon::getstrong (string strong)
{
  SqliteDatabase sql = SqliteDatabase (filename ());
  sql.add ("SELECT definition FROM strong WHERE strong =");
  sql.add (strong);
  sql.add (";");
  vector <string> result = sql.query () ["definition"];
  if (!result.empty ()) return result [0];
  return "";
}
