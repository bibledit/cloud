/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <database/sample.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// Database resilience: It is only read from.


void Database_Sample::create ()
{
  string path = database_sqlite_file (name ());
  filter_url_unlink (path);
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("CREATE TABLE IF NOT EXISTS sample (file text, data text);");
  sql.execute ();
}


// Store a file and its data the sample.
void Database_Sample::store (string file, string data)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("INSERT INTO sample VALUES (");
  sql.add (file);
  sql.add (",");
  sql.add (data);
  sql.add (");");
  sql.execute ();
}


// Get the row identifiers in the database.
vector <int> Database_Sample::get ()
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("SELECT rowid FROM sample;");
  vector <string> rowids = sql.query () ["rowid"];
  vector <int> ids;
  for (auto rowid : rowids) ids.push_back (convert_to_int (rowid));
  return ids;
}


// Get a a file and its contents.
void Database_Sample::get (int rowid, string & file, string & data)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("SELECT file, data FROM sample WHERE rowid =");
  sql.add (rowid);
  sql.add (";");
  map <string, vector <string> > sample = sql.query ();
  vector <string> files = sample ["file"];
  if (files.empty ()) file.clear ();
  else file = files [0];
  vector <string> datas = sample ["data"];
  if (datas.empty ()) data.clear ();
  else data = datas [0];
}


const char * Database_Sample::name ()
{
  return "sample";
}
