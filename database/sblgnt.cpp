/*
Copyright (©) 2003-2025 Teus Benschop.

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


constexpr const auto sblgnt {"sblgnt"};


Database_Sblgnt::Database_Sblgnt ()
{
}


Database_Sblgnt::~Database_Sblgnt ()
{
}


// Get Greek words for $book $chapter $verse.
std::vector <std::string> Database_Sblgnt::getVerse (int book, int chapter, int verse)
{
  SqliteDatabase sql {sblgnt};
  sql.add ("SELECT greek FROM sblgnt WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  const std::vector <std::string> words = sql.query () ["greek"];
  return words;
}


// Get the passages that contain a $greek word.
std::vector <Passage> Database_Sblgnt::searchGreek (std::string greek)
{
  SqliteDatabase sql {sblgnt};
  sql.add ("SELECT DISTINCT book, chapter, verse FROM sblgnt WHERE greek =");
  sql.add (greek);
  sql.add (";");
  std::vector <Passage> hits;
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> books = result ["book"];
  const std::vector <std::string> chapters = result ["chapter"];
  const std::vector <std::string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size (); i++) {
    Passage passage;
    passage.m_book = filter::strings::convert_to_int (books [i]);
    passage.m_chapter = filter::strings::convert_to_int (chapters [i]);
    passage.m_verse = verses [i];
    hits.push_back (passage);
  }
  return hits;
}
