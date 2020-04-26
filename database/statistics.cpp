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


#include <database/statistics.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <database/sqlite.h>


// Database resilience: It contains statistical and non-essential data.
// It is checked and optionally recreated at least once a day.


#ifdef HAVE_CLOUD


void Database_Statistics::create ()
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("CREATE TABLE IF NOT EXISTS changes (timestamp integer, user text, count integer);");
  sql.execute ();
}


void Database_Statistics::optimize ()
{
  bool healthy_database = database_sqlite_healthy (name ());
  if (!healthy_database) {
    filter_url_unlink (database_sqlite_file (name ()));
    create ();
  }
}


void Database_Statistics::store_changes (int timestamp, string user, int count)
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("INSERT INTO changes VALUES (");
  sql.add (timestamp);
  sql.add (",");
  sql.add (user);
  sql.add (",");
  sql.add (count);
  sql.add (");");
  sql.execute ();
}


// Fetches the distinct users who have been active within the last 365 days.
vector <string> Database_Statistics::get_users ()
{
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("SELECT DISTINCT user FROM changes WHERE timestamp >=");
  sql.add (year_ago ());
  sql.add ("ORDER BY user;");
  vector <string> users = sql.query () ["user"];
  return users;
}


// Fetches the change statistics from the database for $user for no more than a year ago.
vector <pair <int, int>> Database_Statistics::get_changes (string user)
{
  vector <pair <int, int>> changes;
  SqliteDatabase sql = SqliteDatabase (name ());
  sql.add ("SELECT timestamp, count FROM changes WHERE timestamp >=");
  sql.add (year_ago ());
  if (!user.empty ()) {
    // Empty user: Get all changes for all users.
    sql.add ("AND user =");
    sql.add (user);
  }
  sql.add ("ORDER BY timestamp DESC;");
  vector <string> timestamps = sql.query () ["timestamp"];
  vector <string> counts = sql.query () ["count"];
  for (size_t i = 0; i < timestamps.size (); i++) {
    int timestamp = convert_to_int (timestamps[i]);
    int count = convert_to_int (counts[i]);
    changes.push_back (make_pair (timestamp, count));
  }
  return changes;
}


const char * Database_Statistics::name ()
{
  return "statistics";
}


int Database_Statistics::year_ago ()
{
  int timestamp = filter_date_seconds_since_epoch ();
  timestamp -= (3600 * 24 * 365);
  return timestamp;
}


#endif
