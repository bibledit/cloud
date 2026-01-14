/*
Copyright (©) 2003-2026 Teus Benschop.

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


#include <database/navigation.h>
#include <filter/string.h>
#include <filter/date.h>
#include <database/sqlite.h>
#include <webserver/request.h>
#include <filter/passage.h>


// Database resilience: It is re-created every night 


constexpr const auto navigation {"navigation"};


void Database_Navigation::create ()
{
  SqliteDatabase sql (navigation);
  sql.set_sql ("CREATE TABLE IF NOT EXISTS navigation ("
               " timestamp integer,"
               " username text,"
               " book integer,"
               " chapter integer,"
               " verse integer,"
               " active boolean,"
               " focusgroup integer"
               ");");
  sql.execute ();
}


void Database_Navigation::upgrade ()
{
  SqliteDatabase sql (navigation);
  // If there's no column for focus group yet, add it, and if so, delete all data for ease of use.
  sql.add ("PRAGMA table_info (navigation);");
  const std::vector <std::string> columns = sql.query () ["name"];
  if (!filter::string::in_array (static_cast<std::string> ("focusgroup"), columns)) {
    sql.clear ();
    sql.add ("ALTER TABLE navigation ADD COLUMN focusgroup integer;");
    sql.execute ();
    sql.clear ();
    sql.add ("DELETE FROM navigation;");
    sql.execute ();
  }
}


void Database_Navigation::downgrade ()
{
  SqliteDatabase sql (navigation);
  sql.add ("ALTER TABLE navigation DROP COLUMN focusgroup;");
  sql.execute ();
}


void Database_Navigation::trim ()
{
  // Delete items older than, say, several weeks.
  int time = filter::date::seconds_since_epoch ();
  time -= (3600 * 24 * 14);
  SqliteDatabase sql (navigation);
  sql.add ("DELETE FROM navigation WHERE timestamp <=");
  sql.add (time);
  sql.add (";");
  sql.execute ();
}


void Database_Navigation::record (const int time, const std::string& user,
                                  const int book, const int chapter, const int verse,
                                  const int focus_group)
{
  SqliteDatabase sql (navigation);

  // Clear any 'active' flags.
  sql.add ("UPDATE navigation SET active = 0 WHERE username =");
  sql.add (user);
  sql.add ("AND focusgroup =");
  sql.add (focus_group);
  sql.add (";");
  sql.execute();

  // Remove entries recorded less than several seconds ago.
  const int recent = time - 5;
  sql.clear();
  sql.add ("DELETE FROM navigation WHERE timestamp >=");
  sql.add (recent);
  sql.add ("AND username =");
  sql.add (user);
  sql.add ("AND focusgroup =");
  sql.add (focus_group);
  sql.add (";");
  sql.execute();

  // Record entry.
  sql.clear();
  sql.add ("INSERT INTO navigation VALUES (");
  sql.add (time);
  sql.add (",");
  sql.add (user);
  sql.add (",");
  sql.add (book);
  sql.add (",");
  sql.add (chapter);
  sql.add (",");
  sql.add (verse);
  sql.add (",");
  sql.add (true);
  sql.add (",");
  sql.add (focus_group);
  sql.add (");");
  sql.execute();
}


bool Database_Navigation::previous_exists (const std::string& user, const int focus_group)
{
  return (get_previous_id (user, focus_group) != 0);
}


bool Database_Navigation::next_exists (const std::string& user, const int focus_group)
{
  return (get_next_id (user, focus_group) != 0);
}


Passage Database_Navigation::get_previous (const std::string& user, const int focus_group)
{
  int id = get_previous_id (user, focus_group);
  if (id == 0) return Passage ();

  // Update the 'active' flag.
  SqliteDatabase sql1 (navigation);
  sql1.add ("UPDATE navigation SET active = 0 WHERE username =");
  sql1.add (user);
  sql1.add ("AND focusgroup =");
  sql1.add (focus_group);
  sql1.add (";");
  SqliteDatabase sql2 (navigation);
  sql2.add ("UPDATE navigation SET active = 1 WHERE rowid =");
  sql2.add (id);
  sql2.add (";");

  // Read the passage.
  SqliteDatabase sql3 (navigation);
  sql3.add ("SELECT book, chapter, verse FROM navigation WHERE rowid =");
  sql3.add (id);
  sql3.add (";");

  // Run all of the SQL at once, to minimize the database connection time.
  sql1.execute ();
  sql1.disconnect();
  sql2.execute ();
  sql2.disconnect();
  std::map <std::string, std::vector <std::string> > result = sql3.query ();
  sql3.disconnect();
  
  const std::vector <std::string> books = result ["book"];
  const std::vector <std::string> chapters = result ["chapter"];
  const std::vector <std::string> verses = result ["verse"];
  if (!books.empty()) {
    Passage passage;
    passage.m_book = filter::string::convert_to_int (books [0]);
    passage.m_chapter = filter::string::convert_to_int (chapters [0]);
    passage.m_verse = verses [0];
    return passage;
  }
  return Passage ();
}


Passage Database_Navigation::get_next (const std::string& user, const int focus_group)
{
  int id = get_next_id (user, focus_group);
  if (id == 0) return Passage ();

  // Update the 'active' flag.
  SqliteDatabase sql1 (navigation);
  sql1.add ("UPDATE navigation SET active = 0 WHERE username =");
  sql1.add (user);
  sql1.add ("AND focusgroup =");
  sql1.add (focus_group);
  sql1.add (";");
  SqliteDatabase sql2 (navigation);
  sql2.add ("UPDATE navigation SET active = 1 WHERE rowid =");
  sql2.add (id);
  sql2.add (";");

  // Read the passage.
  SqliteDatabase sql3 (navigation);
  sql3.add ("SELECT book, chapter, verse FROM navigation WHERE rowid =");
  sql3.add (id);
  sql3.add (";");

  // Run all of the SQL at once, to minimize the database connection time.
  sql1.execute ();
  sql1.disconnect();
  sql2.execute ();
  sql2.disconnect();
  std::map <std::string, std::vector <std::string> > result = sql3.query ();
  sql3.disconnect();
  
  const std::vector <std::string> books = result ["book"];
  const std::vector <std::string> chapters = result ["chapter"];
  const std::vector <std::string> verses = result ["verse"];
  if (!books.empty()) {
    Passage passage;
    passage.m_book = filter::string::convert_to_int (books [0]);
    passage.m_chapter = filter::string::convert_to_int (chapters [0]);
    passage.m_verse = verses [0];
    return passage;
  }
  return Passage ();
}


int Database_Navigation::get_previous_id (const std::string& user, const int focus_group)
{
  // Get the database row identifier of the active entry for the user.
  int id = 0;
  {
    SqliteDatabase sql (navigation);
    sql.add ("SELECT rowid FROM navigation WHERE username =");
    sql.add (user);
    sql.add ("AND active = true AND focusgroup =");
    sql.add (focus_group);
    sql.add (";");
    const std::vector <std::string> ids = sql.query () ["rowid"];
    for (const auto& s : ids) {
      id = filter::string::convert_to_int (s);
    }
  }
  // If no active row identifier was found, return zero.
  if (id == 0) return 0;

  // Get the database row identifier of the entry just before the active entry.
  SqliteDatabase sql (navigation);
  sql.add ("SELECT rowid FROM navigation WHERE rowid <");
  sql.add (id);
  sql.add ("AND username =");
  sql.add (user);
  sql.add ("AND focusgroup =");
  sql.add (focus_group);
  sql.add ("ORDER BY rowid DESC LIMIT 1;");
  const std::vector <std::string> ids = sql.query () ["rowid"];
  if (!ids.empty()) {
    return filter::string::convert_to_int (ids.at(0));
  }

  // Nothing found.
  return 0;
}


int Database_Navigation::get_next_id (const std::string& user, const int focus_group)
{
  // Get the database row identifier of the active entry for the user.
  int id = 0;
  {
    SqliteDatabase sql (navigation);
    sql.add ("SELECT rowid FROM navigation WHERE username =");
    sql.add (user);
    sql.add ("AND active = true AND focusgroup =");
    sql.add (focus_group);
    sql.add (";");
    const std::vector <std::string> ids = sql.query () ["rowid"];
    for (const auto& s : ids) {
      id = filter::string::convert_to_int (s);
    }
  }
  // If no active row identifier was found, return zero.
  if (id == 0) return 0;

  // Get the database row identifier of the entry just after the active entry.
  SqliteDatabase sql (navigation);
  sql.add ("SELECT rowid FROM navigation WHERE rowid >");
  sql.add (id);
  sql.add ("AND username =");
  sql.add (user);
  sql.add ("AND focusgroup =");
  sql.add (focus_group);
  sql.add ("ORDER BY rowid ASC LIMIT 1;");
  const std::vector <std::string> ids = sql.query () ["rowid"];
  if (!ids.empty()) {
    return filter::string::convert_to_int (ids.at(0));
  }

  // Nothing found.
  return 0;
}


// The $user for whom to get the history.
// The $direction into which to get the history:
// * negative: Get the past history as if going back.
// * positive: Get the future history as if going forward.
std::vector <Passage> Database_Navigation::get_history (const std::string& user, const int direction, const int focus_group)
{
  std::vector <Passage> passages;
  
  int id = 0;
  if (direction > 0) id = get_next_id(user, focus_group);
  if (direction < 0) id = get_previous_id (user, focus_group);
  if (id) {

    // Read the passages history for this user.
    SqliteDatabase sql (navigation);
    sql.add ("SELECT book, chapter, verse FROM navigation WHERE rowid");
    if (direction > 0) sql.add (">=");
    if (direction < 0) sql.add ("<=");
    sql.add (id);
    sql.add ("AND username =");
    sql.add (user);
    sql.add ("AND focusgroup =");
    sql.add (focus_group);

    // Order the results depending on getting the history forward or backward.
    sql.add ("ORDER BY rowid");
    if (direction > 0) sql.add ("ASC");
    if (direction < 0) sql.add ("DESC");
    sql.add (";");

    // Run the query on the database.
    std::map <std::string, std::vector <std::string> > result = sql.query ();

    // Assemble the results.
    const std::vector <std::string> books = result ["book"];
    const std::vector <std::string> chapters = result ["chapter"];
    const std::vector <std::string> verses = result ["verse"];
    for (unsigned int i = 0; i < books.size(); i++) {
      Passage passage;
      passage.m_book = filter::string::convert_to_int (books [i]);
      passage.m_chapter = filter::string::convert_to_int (chapters [i]);
      passage.m_verse = verses [i];
      passages.push_back(passage);
    }
  }
  
  // Done.
  return passages;
}
