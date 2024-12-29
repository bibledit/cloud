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


#include <database/versifications.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <database/books.h>
#include <database/logs.h>
#include <versification/logic.h>
#include <locale/logic.h>


// This is a database for the versification systems.
// Resilience: It is normally not written to, so corruption is unlikely.


// Versification information is also available from the Bible Organisational System.
// (This system contains versification information, no mapping data.)
// See http://freely-given.org/Software/BibleOrganisationalSystem/
// See https://github.com/openscriptures/BibleOrgSys/


constexpr const auto versifications {"versifications"};


void Database_Versifications::create ()
{
  SqliteDatabase sql (versifications);
  sql.set_sql ("CREATE TABLE IF NOT EXISTS names ("
               " system integer,"
               " name text"
               ");");
  sql.execute ();
  sql.set_sql ("CREATE TABLE IF NOT EXISTS data ("
               " system integer,"
               " book integer,"
               " chapter integer,"
               " verse integer"
               ");");
  sql.execute ();
}


void Database_Versifications::optimize ()
{
  SqliteDatabase sql (versifications);
  sql.set_sql ("VACUUM;");
  sql.execute ();
}


// Import data.
void Database_Versifications::input (const std::string& contents, const std::string& name)
{
  // Delete old system if it is there, and create new one.
  erase (name);
  int id = createSystem (name);

  SqliteDatabase sql (versifications);
  sql.set_sql ("PRAGMA temp_store = MEMORY;");
  sql.execute();
  sql.set_sql ("PRAGMA synchronous = OFF;");
  sql.execute();
  sql.set_sql ("PRAGMA journal_mode = OFF;");
  sql.execute();
  sql.set_sql ("BEGIN;");
  sql.execute();

  std::vector <std::string> lines = filter::strings::explode (contents, '\n');
  for (auto line : lines) {
    line = filter::strings::trim (line);
    if (line.empty ()) continue;
    // The line will be something similar to this: 1 Corinthians 1:31
    // Split the passage entry on the colon (:) to get the verse.
    std::vector <std::string> bits = filter::strings::explode(line, ':');
    if (bits.size() != 2) continue;
    int verse = filter::strings::convert_to_int(bits[1]);
    // Split the first bit on the spaces and get the last item as the chapter.
    bits = filter::strings::explode(bits[0], ' ');
    if (bits.size() < 2) continue;
    int chapter = filter::strings::convert_to_int(bits[bits.size()-1]);
    // Remove the last bit so it remains with the book, and get that book.
    bits.pop_back();
    std::string passage_book_string = filter::strings::implode(bits, " ");
    int book = static_cast<int>(database::books::get_id_from_english(passage_book_string));
    // Check result.
    if ((book == 0) || (chapter == 0)) {
      Database_Logs::log ("Malformed versification entry: " + line);
      continue;
    }
    // Store result.
    sql.clear();
    sql.add ("INSERT INTO data (system, book, chapter, verse) VALUES (");
    sql.add (id);
    sql.add (",");
    sql.add (book);
    sql.add (",");
    sql.add (chapter);
    sql.add (",");
    sql.add (verse);
    sql.add (");");
    sql.execute ();
  }
  
  sql.set_sql ("COMMIT;");
  sql.execute();
}


// Export data.
std::string Database_Versifications::output (const std::string& name)
{
  std::vector <std::string> lines;
  std::vector <Passage> versification_data = getBooksChaptersVerses (name);
  for (Passage & passage : versification_data) {
    std::string line = database::books::get_english_from_id (static_cast<book_id>(passage.m_book));
    line.append (" ");
    line.append (std::to_string (passage.m_chapter));
    line.append (":");
    line.append (passage.m_verse);
    lines.push_back (line);
  }
  return filter::strings::implode (lines, "\n");
}


// Delete a versification system.
void Database_Versifications::erase (const std::string& name)
{
  int id = getID (name);

  SqliteDatabase sql (versifications);
  sql.add ("DELETE FROM names WHERE system =");
  sql.add (id);
  sql.add (";");
  sql.execute ();

  sql.clear();
  sql.add ("DELETE FROM data WHERE system =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


// Returns the ID for a named versification system.
int Database_Versifications::getID (const std::string& name)
{
  SqliteDatabase sql (versifications);
  sql.add ("SELECT system FROM names WHERE name =");
  sql.add (name);
  sql.add (";");
  const std::vector <std::string> systems = sql.query () ["system"];
  if (!systems.empty()) {
    const auto id = systems.at(0);
    return filter::strings::convert_to_int (id);
  }
  return 0;
}


// Creates a new empty versification system.
// Returns the new ID.
int Database_Versifications::createSystem (const std::string& name)
{
  // If the versification system already exists, return its ID.
  int id = getID (name);
  if (id > 0) {
    return id;
  }
  // Get the first free ID starting from 1000 (except when creating the default systems).
  id = 0;
  SqliteDatabase sql (versifications);
  sql.set_sql ("SELECT system FROM names ORDER BY system DESC LIMIT 1;");
  const std::vector <std::string> systems = sql.query () ["system"];
  for (const auto& system : systems) {
    id = filter::strings::convert_to_int (system);
  }
  id++;
  if (!creating_defaults) if (id < 1000) id = 1000;
  // Create the empty system.
  sql.clear();
  sql.add ("INSERT INTO names VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (name);
  sql.add (");");
  sql.execute ();
  // Return new ID.
  return id;
}


// Returns an array of the available versification systems.
std::vector <std::string> Database_Versifications::getSystems ()
{
  SqliteDatabase sql (versifications);
  sql.set_sql ("SELECT name FROM names ORDER BY name ASC;");
  const std::vector <std::string> systems = sql.query () ["name"];
  return systems;
}


// Returns the books, chapters, verses for the given versification system.
std::vector <Passage> Database_Versifications::getBooksChaptersVerses (const std::string& name)
{
  std::vector <Passage> data;
  const int id = getID (name);
  SqliteDatabase sql (versifications);
  sql.add ("SELECT book, chapter, verse FROM data WHERE system =");
  sql.add (id);
  sql.add ("ORDER BY book, chapter, verse ASC;");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> books = result ["book"];
  const std::vector <std::string> chapters = result ["chapter"];
  const std::vector <std::string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size (); i++) {
    Passage passage;
    passage.m_book = filter::strings::convert_to_int (books [i]);
    passage.m_chapter = filter::strings::convert_to_int (chapters [i]);
    passage.m_verse = verses [i];
    data.push_back (passage);
  }
  return data;
}


std::vector <int> Database_Versifications::getBooks (const std::string& name)
{
  std::vector <int> books;
  const int id = getID (name);
  SqliteDatabase sql (versifications);
  sql.add ("SELECT DISTINCT book FROM data WHERE system =");
  sql.add (id);
  sql.add ("ORDER BY book ASC;");
  std::vector <std::string> sbooks = sql.query () ["book"];
  for (const auto& book : sbooks) {
    books.push_back (filter::strings::convert_to_int (book));
  }
  return books;
}


// This returns all the chapters in book of versification system name.
// include0: Includes chapter 0 also.
std::vector <int> Database_Versifications::getChapters (const std::string& name, int book, bool include0)
{
  std::vector <int> chapters;
  if (include0) chapters.push_back (0);
  int id = getID (name);
  SqliteDatabase sql (versifications);
  sql.add ("SELECT DISTINCT chapter FROM data WHERE system =");
  sql.add (id);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("ORDER BY chapter ASC;");
  std::vector <std::string> schapters = sql.query () ["chapter"];
  for (const auto& chapter : schapters) {
    chapters.push_back (filter::strings::convert_to_int (chapter));
  }
  return chapters;
}


std::vector <int> Database_Versifications::getVerses (const std::string& name, int book, int chapter)
{
  std::vector <int> verses;
  int id = getID (name);
  SqliteDatabase sql (versifications);
  sql.add ("SELECT DISTINCT verse FROM data WHERE system =");
  sql.add (id);
  sql.add ("AND book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("ORDER BY verse ASC;");
  std::vector <std::string> sverses = sql.query () ["verse"];
  for (const auto& verse : sverses) {
    const int maxverse = filter::strings::convert_to_int (verse);
    for (int i = 0; i <= maxverse; i++) {
      verses.push_back (i);
    }
  }
  // Put verse 0 in chapter 0.
  if (chapter == 0) verses.push_back (0);
  return verses;
}


void Database_Versifications::defaults ()
{
  SqliteDatabase sql (versifications);
  sql.set_sql ("DELETE FROM names WHERE system < 1000;");
  sql.execute();
  sql.set_sql ("DELETE FROM data WHERE system < 1000;");
  sql.execute();

  creating_defaults = true;
  std::vector <std::string> names = versification_logic_names ();
  for (auto name : names) {
    // Read the file contents.
    std::string contents = versification_logic_data (name);
    // Due to a need of obfuscating "Bible" and similar,
    // If a versification system is called "Staten Bible",
    // it will be stored in the file system as "Staten Bb",
    // so here deobfuscate the word "Bb",
    // and give the full word "Bible" instead.
    name = locale_logic_deobfuscate (name);
    // Parse it and store it in the database.
    input (contents, name);
  }
  creating_defaults = false;
}


// This returns all possible books in any versification system.
std::vector <int> Database_Versifications::getMaximumBooks ()
{
  std::vector <int> books;
  SqliteDatabase sql (versifications);
  sql.add ("SELECT DISTINCT book FROM data ORDER BY book ASC;");
  const std::vector <std::string> sbooks = sql.query () ["book"];
  for (const auto& book : sbooks) {
    books.push_back (filter::strings::convert_to_int (book));
  }
  return books;
}


// This returns all possible chapters in a book of any versification system.
std::vector <int> Database_Versifications::getMaximumChapters (int book)
{
  std::vector <int> chapters;
  chapters.push_back (0);
  SqliteDatabase sql (versifications);
  sql.add ("SELECT DISTINCT chapter FROM data WHERE book =");
  sql.add (book);
  sql.add ("ORDER BY chapter ASC;");
  const std::vector <std::string> schapters = sql.query () ["chapter"];
  for (const auto& chapter : schapters) {
    chapters.push_back (filter::strings::convert_to_int (chapter));
  }
  return chapters;
}


// This returns all possible verses in a book / chapter of any versification system.
std::vector <int> Database_Versifications::getMaximumVerses (int book, int chapter)
{
  std::vector <int> verses;
  SqliteDatabase sql (versifications);
  sql.add ("SELECT DISTINCT verse FROM data WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("ORDER BY verse ASC;");
  const std::vector <std::string> sverses = sql.query () ["verse"];
  for (const auto& verse : sverses) {
    const int maxverse = filter::strings::convert_to_int (verse);
    for (int i = 0; i <= maxverse; i++) {
      verses.push_back (i);
    }
  }
  // Put verse 0 in chapter 0.
  if (chapter == 0) verses.push_back (0);
  return verses;
}
