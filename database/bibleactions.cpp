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


#include <database/bibleactions.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>


const char * Database_BibleActions::filename ()
{
  return "bibleactions";
}


void Database_BibleActions::create ()
{
  SqliteDatabase sql (filename ());
  sql.add ("CREATE TABLE IF NOT EXISTS bibleactions ("
           " bible text,"
           " book integer,"
           " chapter integer,"
           " usfm text"
           ");");
  sql.execute ();
}


void Database_BibleActions::clear ()
{
  SqliteDatabase sql (filename ());
  sql.add ("DROP TABLE IF EXISTS bibleactions;");
  sql.execute ();
}


void Database_BibleActions::optimize ()
{
  SqliteDatabase sql (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


void Database_BibleActions::record (std::string bible, int book, int chapter, std::string usfm)
{
  if (getUsfm (bible, book, chapter).empty ()) {
    SqliteDatabase sql (filename ());
    sql.add ("INSERT INTO bibleactions VALUES (");
    sql.add (bible);
    sql.add (",");
    sql.add (book);
    sql.add (",");
    sql.add (chapter);
    sql.add (",");
    sql.add (usfm);
    sql.add (");");
    sql.execute ();
  }
}


std::vector <std::string> Database_BibleActions::getBibles ()
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT DISTINCT bible FROM bibleactions ORDER BY bible;");
  std::vector <std::string> notes = sql.query ()["bible"];
  return notes;
}


std::vector <int> Database_BibleActions::getBooks (std::string bible)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT DISTINCT book FROM bibleactions WHERE bible =");
  sql.add (bible);
  sql.add ("ORDER BY book;");
  std::vector <std::string> result = sql.query ()["book"];
  std::vector <int> books;
  for (auto book : result) books.push_back (filter::strings::convert_to_int (book));
  return books;
}


std::vector <int> Database_BibleActions::getChapters (std::string bible, int book)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT DISTINCT chapter FROM bibleactions WHERE bible =");
  sql.add (bible);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("ORDER BY chapter;");
  std::vector <std::string> result = sql.query ()["chapter"];
  std::vector <int> chapters;
  for (auto chapter : result) chapters.push_back (filter::strings::convert_to_int (chapter));
  return chapters;
}


std::string Database_BibleActions::getUsfm (std::string bible, int book, int chapter)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT usfm FROM bibleactions WHERE bible =");
  sql.add (bible);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add (";");
  std::vector <std::string> result = sql.query ()["usfm"];
  if (!result.empty()) {
    std::string usfm = result[0];
    return usfm;
  };
  return std::string();
}


void Database_BibleActions::erase (std::string bible, int book, int chapter)
{
  SqliteDatabase sql (filename ());
  sql.add ("DELETE FROM bibleactions WHERE bible =");
  sql.add (bible);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add (";");
  sql.execute ();
}

