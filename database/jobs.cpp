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


#include <database/jobs.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <config/globals.h>
#include <database/sqlite.h>


// Database resilience: 
// The database does not contain important information.
// Re-create it on corruption.


sqlite3 * Database_Jobs::connect ()
{
  return database_sqlite_connect ("jobs");
}


void Database_Jobs::create ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "DROP TABLE IF EXISTS jobs");
  string sql = "CREATE TABLE IF NOT EXISTS jobs ("
               " id integer,"
               " timestamp integer,"
               " level integer,"
               " start text,"
               " percentage integer,"
               " progress text,"
               " result text"
               ");";
  database_sqlite_exec (db, sql);
  database_sqlite_disconnect (db);
}


void Database_Jobs::optimize ()
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "VACUUM jobs;");
  database_sqlite_disconnect (db);
}


void Database_Jobs::trim ()
{
  // Delete jobs older than 30 days.
  int timestamp = filter_date_seconds_since_epoch () - (30 * 24 * 3600);
  SqliteSQL sql = SqliteSQL ();
  sql.add ("DELETE FROM jobs WHERE timestamp <");
  sql.add (timestamp);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


bool Database_Jobs::idExists (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT id FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["id"];
  database_sqlite_disconnect (db);
  return !result.empty ();
}


// Gets a new unique identifier.
int Database_Jobs::getNewId ()
{
  // Iterate till a non-existing identifier is found.
  int id;
  do {
    id = filter_string_rand (100000000, 999999999);
  } while (idExists (id));
  // Store the new id so it can't be given out again just now.
  // Also store the timestamp. used for entry expiry.
  int timestamp = filter_date_seconds_since_epoch ();
  SqliteSQL sql = SqliteSQL ();
  sql.add ("INSERT INTO jobs (id, timestamp) VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (timestamp);
  sql.add (");");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
  return id;
}


void Database_Jobs::setLevel (int id, int level)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE jobs SET level =");
  sql.add (level);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


int Database_Jobs::getLevel (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT level FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["level"];
  database_sqlite_disconnect (db);
  for (auto & level : result) {
    return convert_to_int (level);
  }
  return 0;
}


void Database_Jobs::setStart (int id, string start)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE jobs SET start =");
  sql.add (start);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


string Database_Jobs::getStart (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT start FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["start"];
  database_sqlite_disconnect (db);
  for (auto & start : result) {
    return start;
  }
  return "";
}


void Database_Jobs::setPercentage (int id, int percentage)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE jobs SET percentage =");
  sql.add (percentage);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


string Database_Jobs::getPercentage (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT percentage FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["percentage"];
  database_sqlite_disconnect (db);
  for (auto & percentage : result) {
    return percentage;
  }
  return "";
}


void Database_Jobs::setProgress (int id, string progress)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE jobs SET progress =");
  sql.add (progress);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


string Database_Jobs::getProgress (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT progress FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["progress"];
  database_sqlite_disconnect (db);
  for (auto & progress : result) {
    return progress;
  }
  return "";
}


void Database_Jobs::setResult (int id, string result)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("UPDATE jobs SET result =");
  sql.add (result);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  database_sqlite_exec (db, sql.sql);
  database_sqlite_disconnect (db);
}


string Database_Jobs::getResult (int id)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT result FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> result = database_sqlite_query (db, sql.sql) ["result"];
  database_sqlite_disconnect (db);
  for (auto & s : result) {
    return s;
  }
  return "";
}


