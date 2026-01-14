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


#include <database/sprint.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// Database robustness:
// 1. It is infrequently written to.
// 2. Chances of corruption are low.
// 3. In case of corruption, data for the sprints is lost. 
//    This is acceptable, because translation work can still continue.


constexpr const auto database_name {"sprint"};


#ifdef HAVE_CLOUD


void Database_Sprint::create ()
{
  SqliteDatabase sql (database_name);

  sql.set_sql ("CREATE TABLE IF NOT EXISTS sprint ("
               " year integer,"
               " month integer,"
               " title text,"
               " body text,"
               " complete integer"
               ");");
  sql.execute ();

  sql.set_sql ("CREATE INDEX IF NOT EXISTS yearmonth ON sprint (year, month)");
  sql.execute ();

  sql.set_sql ("CREATE INDEX IF NOT EXISTS complete ON sprint (complete)");
  sql.execute ();

  sql.set_sql ("CREATE TABLE IF NOT EXISTS sprinthistory ("
               " year integer,"
               " month integer,"
               " day integer,"
               " tasks integer,"
               " complete integer"
               ");");
  sql.execute ();

  // Upgrade the two tables: Add a column for the Bible.
  sql.set_sql ("PRAGMA table_info (sprint);");
  std::vector <std::string> columns = sql.query () ["name"];
  if (std::find (columns.begin(), columns.end(), "bible") == columns.end()) {
    sql.set_sql ("ALTER TABLE sprint ADD COLUMN bible text;");
    sql.execute ();
  }
  sql.set_sql ("PRAGMA table_info (sprinthistory);");
  columns = sql.query () ["name"];
  if (std::find (columns.begin(), columns.end(), "bible") == columns.end()) {
    sql.set_sql ("ALTER TABLE sprinthistory ADD COLUMN bible text;");
    sql.execute ();
  }
}


void Database_Sprint::optimize ()
{
  SqliteDatabase sql (database_name);
  sql.set_sql ("REINDEX sprint;");
  sql.execute();
  sql.set_sql ("VACUUM;");
  sql.execute();
  sql.set_sql ("VACUUM;");
  sql.execute();
}


void Database_Sprint::storeTask (const std::string& bible, int year, int month, const std::string& title)
{
  SqliteDatabase sql (database_name);
  sql.add ("INSERT INTO sprint VALUES (");
  sql.add (year);
  sql.add (",");
  sql.add (month);
  sql.add (",");
  sql.add (title);
  sql.add (", '', 0,");
  sql.add (bible);
  sql.add (");");
  sql.execute ();
}


void Database_Sprint::deleteTask (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("DELETE FROM sprint WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


std::vector <int> Database_Sprint::getTasks (const std::string& bible, int year, int month)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT rowid FROM sprint WHERE bible =");
  sql.add (bible);
  sql.add ("AND year =");
  sql.add (year);
  sql.add ("AND month =");
  sql.add (month);
  sql.add ("ORDER BY rowid ASC;");
  const std::vector <std::string> rowids = sql.query () ["rowid"];
  std::vector <int> ids;
  for (const auto& id : rowids) {
    ids.push_back (filter::string::convert_to_int (id));
  }
  return ids;
}


std::string Database_Sprint::getTitle (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT title FROM sprint WHERE rowid =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> title = sql.query () ["title"];
  if (!title.empty ())
    return title.at(0);
  return std::string();
}


void Database_Sprint::updateComplete (int id, int percentage)
{
  SqliteDatabase sql (database_name);
  sql.add ("UPDATE sprint SET complete =");
  sql.add (percentage);
  sql.add ("WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


int Database_Sprint::getComplete (int id)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT complete FROM sprint WHERE rowid =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> complete = sql.query () ["complete"];
  if (!complete.empty ())
    return filter::string::convert_to_int (complete.at(0));
  return 0;
}


void Database_Sprint::updateMonthYear (int id, int month, int year)
{
  SqliteDatabase sql (database_name);
  sql.add ("UPDATE sprint SET month =");
  sql.add (month);
  sql.add (", year =");
  sql.add (year);
  sql.add ("WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


void Database_Sprint::logHistory (const std::string& bible, int year, int month, int day, int tasks, int complete)
{
  SqliteDatabase sql (database_name);
  sql.add ("DELETE FROM sprinthistory WHERE bible =");
  sql.add (bible);
  sql.add ("AND year =");
  sql.add (year);
  sql.add ("AND month =");
  sql.add (month);
  sql.add ("AND day =");
  sql.add (day);
  sql.add (";");
  sql.execute ();

  sql.clear();
  sql.add ("INSERT INTO sprinthistory VALUES (");
  sql.add (year);
  sql.add (",");
  sql.add (month);
  sql.add (",");
  sql.add (day);
  sql.add (",");
  sql.add (tasks);
  sql.add (",");
  sql.add (complete);
  sql.add (",");
  sql.add (bible);
  sql.add (");");
  sql.execute ();
}


std::vector <Database_Sprint_Item> Database_Sprint::getHistory (const std::string& bible, int year, int month)
{
  std::vector <Database_Sprint_Item> history;
  SqliteDatabase sql (database_name);
  sql.add ("SELECT day, tasks, complete FROM sprinthistory WHERE bible =");
  sql.add (bible);
  sql.add ("AND year =");
  sql.add (year);
  sql.add ("AND month =");
  sql.add (month);
  sql.add ("ORDER BY day ASC;");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  const std::vector <std::string> days = result ["day"];
  const std::vector <std::string> tasks = result ["tasks"];
  const std::vector <std::string> completes = result ["complete"];
  for (unsigned int i = 0; i < days.size(); i++) {
    Database_Sprint_Item item = Database_Sprint_Item ();
    item.day = filter::string::convert_to_int (days [i]);
    item.tasks = filter::string::convert_to_int (tasks [i]);
    item.complete = filter::string::convert_to_int (completes [i]);
    history.push_back (item);
  }
  return history;
}


void Database_Sprint::clearHistory (const std::string& bible, int year, int month)
{
  SqliteDatabase sql (database_name);
  sql.add ("DELETE FROM sprinthistory WHERE bible =");
  sql.add (bible);
  sql.add ("AND year =");
  sql.add (year);
  sql.add ("AND month =");
  sql.add (month);
  sql.add (";");
  sql.execute ();
}


#endif
