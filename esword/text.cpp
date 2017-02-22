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


#include <esword/text.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/url.h>
#include <database/sqlite.h>


// Class for creating e-Sword documents.


Esword_Text::Esword_Text (string bible)
{
  currentBook = 0;
  currentChapter = 0;
  currentVerse = 0;
  currentText = "";
  bible = database_sqlite_no_sql_injection (bible);
  sql.push_back ("PRAGMA foreign_keys=OFF;");
  sql.push_back ("PRAGMA synchronous=OFF;");
  sql.push_back ("CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), Unicode BOOL, RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL);");
  sql.push_back ("INSERT INTO Details VALUES ('" + bible + "', '" + bible + "', '" + bible + "', 1, 'UNICODE', 1, 0, 1, 1, 0, 0);");
  sql.push_back ("CREATE TABLE Bible (Book INT, Chapter INT, Verse INT, Scripture TEXT);");
}


void Esword_Text::flushCache ()
{
  string text = filter_string_trim (currentText);
  if (!text.empty ()) {
    string unicode;
    size_t length = unicode_string_length (text);
    for (size_t pos = 0; pos < length; pos++) {
      string s = unicode_string_substr (text, pos, 1);
      int codepoint = unicode_string_convert_to_codepoint (s);
      unicode.append ("\\u" + convert_to_string (codepoint) + "?");
    }
    int book = currentBook;
    int chapter = currentChapter;
    int verse = currentVerse;
    string statement = "INSERT INTO Bible VALUES (" + convert_to_string (book) + ", " + convert_to_string (chapter) + ", " + convert_to_string (verse) + ", '" + unicode + "');";
    sql.push_back (statement);
  }
  currentText.clear ();
}


void Esword_Text::newBook (int book)
{
  flushCache ();
  currentBook = book;
  newChapter (0);
}


void Esword_Text::newChapter (int chapter)
{
  flushCache ();
  currentChapter = chapter;
  currentVerse = 0;
}


void Esword_Text::newVerse (int verse)
{
  flushCache ();
  currentVerse = verse;
}


void Esword_Text::addText (string text)
{
  if (text != "") currentText += text;
}


// This finalizes the SQL script.
void Esword_Text::finalize ()
{
  flushCache ();
  // Add the final SQL statements.
  sql.push_back ("CREATE INDEX BookChapterVerseIndex ON Bible (Book, Chapter, Verse);");
}


// This creates the eSword module.
// $filename: the name of the file to create.
void Esword_Text::createModule (string filename)
{
  flushCache ();
  sqlite3 * db = database_sqlite_connect_file (filename);
  for (string statement : sql) {
    database_sqlite_exec (db, statement);
  }
  database_sqlite_disconnect (db);
}


vector <string> Esword_Text::get_sql ()
{
  return sql;
}

