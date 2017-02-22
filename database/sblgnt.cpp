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


#include <database/sblgnt.h>
#include <filter/string.h>
#include <database/sqlite.h>


// This is the database for the Greek New Testament.
// Resilience: It is never written to. 
// Chances of corruption are nearly zero.


Database_Sblgnt::Database_Sblgnt ()
{
}


Database_Sblgnt::~Database_Sblgnt ()
{
}


sqlite3 * Database_Sblgnt::connect ()
{
  return database_sqlite_connect ("sblgnt");
}


// Get Greek words for $book $chapter $verse.
vector <string> Database_Sblgnt::getVerse (int book, int chapter, int verse)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT greek FROM sblgnt WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> words = database_sqlite_query (db, sql.sql) ["greek"];
  database_sqlite_disconnect (db);
  return words;
}


// Get the passages that contain a $greek word.
vector <Passage> Database_Sblgnt::searchGreek (string greek)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT book, chapter, verse FROM sblgnt WHERE greek =");
  sql.add (greek);
  sql.add (";");
  vector <Passage> hits;
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
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


