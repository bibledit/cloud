/*
Copyright (©) 2003-2020 Teus Benschop.

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


#ifdef HAVE_CLOUD


sqlite3 * Database_Sprint::connect ()
{
  return database_sqlite_connect ("sprint");
}


void Database_Sprint::create ()
{
  sqlite3 * db = connect ();

  string sql = 
    "CREATE TABLE IF NOT EXISTS sprint ("
    "  year integer,"
    "  month integer,"
    "  title text,"
    "  body text,"
    "  complete integer"
    ");";
  database_sqlite_exec (db, sql);

  sql = "CREATE INDEX IF NOT EXISTS yearmonth ON sprint (year, month)";
  database_sqlite_exec (db, sql);

  sql = "CREATE INDEX IF NOT EXISTS complete ON sprint (complete)";
  database_sqlite_exec (db, sql);

  sql =
    "CREATE TABLE IF NOT EXISTS sprinthistory ("
    "  year integer,"
    "  month integer,"
    "  day integer,"
    "  tasks integer,"
    "  complete integer"
    ");";
  database_sqlite_exec (db, sql);

  // Upgrade the two tables: Add a column for the Bible.
  sql = "PRAGMA table_info (sprint);";
  vector <string> columns = database_sqlite_query (db, sql) ["name"];
  if (find (columns.begin(), columns.end(), "bible") == columns.end()) {
    sql = "ALTER TABLE sprint ADD COLUMN bible text;";
    database_sqlite_exec (db, sql);
  }
  sql = "PRAGMA table_info (sprinthistory);";
  columns = database_sqlite_query (db, sql) ["name"];
  if (find (columns.begin(), columns.end(), "bible") == columns.end()) {
    sql = "ALTER TABLE sprinthistory ADD COLUMN bible text;";
    database_sqlite_exec (db, sql);
  }

  database_sqlite_disconnect (db);
}


void Database_Sprint::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "REINDEX sprint;");
  database_sqlite_exec (db, "VACUUM;");
  database_sqlite_exec (db, "VACUUM;");
  database_sqlite_disconnect (db);
}


void Database_Sprint::storeTask (const string& bible, int year, int month, const string& title)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO sprint VALUES (");
  sql.add (year);
  sql.add (",");
  sql.add (month);
  sql.add (",");
  sql.add (title);
  sql.add (", '', 0,");
  sql.add (bible);
  sql.add (");");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


void Database_Sprint::deleteTask (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM sprint WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


vector <int> Database_Sprint::getTasks (const string& bible, int year, int month)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT rowid FROM sprint WHERE bible =");
  sql.add (bible);
  sql.add ("AND year =");
  sql.add (year);
  sql.add ("AND month =");
  sql.add (month);
  sql.add ("ORDER BY rowid ASC;");
  sqlite3 * db = connect ();
  vector <string> rowids = database_sqlite_query (db, sql.sql) ["rowid"];
  database_sqlite_disconnect (db);
  vector <int> ids;
  for (auto & id : rowids) {
    ids.push_back (convert_to_int (id));
  }
  return ids;
}


string Database_Sprint::getTitle (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT title FROM sprint WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> title = database_sqlite_query (db, sql.sql) ["title"];
  database_sqlite_disconnect (db);
  if (!title.empty ()) return title [0];
  return "";
}


void Database_Sprint::updateComplete (int id, int percentage)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE sprint SET complete =");
  sql.add (percentage);
  sql.add ("WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


int Database_Sprint::getComplete (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT complete FROM sprint WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> complete = database_sqlite_query (db, sql.sql) ["complete"];
  database_sqlite_disconnect (db);
  if (!complete.empty ()) return convert_to_int (complete [0]);
  return 0;
}


void Database_Sprint::updateMonthYear (int id, int month, int year)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE sprint SET month =");
  sql.add (month);
  sql.add (", year =");
  sql.add (year);
  sql.add ("WHERE rowid =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


void Database_Sprint::logHistory (const string& bible, int year, int month, int day, int tasks, int complete)
{
  SqliteSQL sql1 = SqliteSQL ();
  sql1.add ("DELETE FROM sprinthistory WHERE bible =");
  sql1.add (bible);
  sql1.add ("AND year =");
  sql1.add (year);
  sql1.add ("AND month =");
  sql1.add (month);
  sql1.add ("AND day =");
  sql1.add (day);
  sql1.add (";");

  SqliteSQL sql2 = SqliteSQL ();
  sql2.add ("INSERT INTO sprinthistory VALUES (");
  sql2.add (year);
  sql2.add (",");
  sql2.add (month);
  sql2.add (",");
  sql2.add (day);
  sql2.add (",");
  sql2.add (tasks);
  sql2.add (",");
  sql2.add (complete);
  sql2.add (",");
  sql2.add (bible);
  sql2.add (");");

  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql1.sql);
  database_sqlite_exec (db, sql2.sql);
  database_sqlite_disconnect (db);
}


vector <Database_Sprint_Item> Database_Sprint::getHistory (const string& bible, int year, int month)
{
  vector <Database_Sprint_Item> history;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT day, tasks, complete FROM sprinthistory WHERE bible =");
  sql.add (bible);
  sql.add ("AND year =");
  sql.add (year);
  sql.add ("AND month =");
  sql.add (month);
  sql.add ("ORDER BY day ASC;");
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> days = result ["day"];
  vector <string> tasks = result ["tasks"];
  vector <string> completes = result ["complete"];
  for (unsigned int i = 0; i < days.size(); i++) {
    Database_Sprint_Item item = Database_Sprint_Item ();
    item.day = convert_to_int (days [i]);
    item.tasks = convert_to_int (tasks [i]);
    item.complete = convert_to_int (completes [i]);
    history.push_back (item);
  }
  return history;
}


void Database_Sprint::clearHistory (const string& bible, int year, int month)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM sprinthistory WHERE bible =");
  sql.add (bible);
  sql.add ("AND year =");
  sql.add (year);
  sql.add ("AND month =");
  sql.add (month);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


#endif
