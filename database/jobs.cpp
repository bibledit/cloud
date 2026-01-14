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


#include <database/jobs.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <config/globals.h>
#include <database/sqlite.h>


// Database resilience: 
// The database does not contain important information.
// Re-create it on corruption.


constexpr const auto jobs {"jobs"};


void Database_Jobs::create ()
{
  SqliteDatabase sql (jobs);
  
  sql.add ("DROP TABLE IF EXISTS jobs");
  sql.execute();
  
  sql.clear();
  sql.add ("CREATE TABLE IF NOT EXISTS jobs ("
           " id integer,"
           " timestamp integer,"
           " level integer,"
           " start text,"
           " percentage integer,"
           " progress text,"
           " result text"
           ");");
  sql.execute ();
}


void Database_Jobs::optimize ()
{
  SqliteDatabase sql (jobs);
  sql.add ("VACUUM;");
  sql.execute();
}


void Database_Jobs::trim ()
{
  // Delete jobs older than 30 days.
  const int timestamp = filter::date::seconds_since_epoch () - (30 * 24 * 3600);
  SqliteDatabase sql (jobs);
  sql.add ("DELETE FROM jobs WHERE timestamp <");
  sql.add (timestamp);
  sql.add (";");
  sql.execute ();
}


bool Database_Jobs::id_exists (int id)
{
  SqliteDatabase sql (jobs);
  sql.add ("SELECT id FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["id"];
  return !result.empty ();
}


// Gets a new unique identifier.
int Database_Jobs::get_new_id ()
{
  // Iterate till a non-existing identifier is found.
  int id;
  do {
    id = filter::string::rand (100'000'000, 999'999'999);
  } while (id_exists (id));
  // Store the new id so it can't be given out again just now.
  // Also store the timestamp. used for entry expiry.
  const int timestamp = filter::date::seconds_since_epoch ();
  SqliteDatabase sql (jobs);
  sql.add ("INSERT INTO jobs (id, timestamp) VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (timestamp);
  sql.add (");");
  sql.execute ();
  return id;
}


void Database_Jobs::set_level (int id, int level)
{
  SqliteDatabase sql (jobs);
  sql.add ("UPDATE jobs SET level =");
  sql.add (level);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


int Database_Jobs::get_level (int id)
{
  SqliteDatabase sql (jobs);
  sql.add ("SELECT level FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> levels = sql.query () ["level"];
  if (!levels.empty()) {
    return filter::string::convert_to_int (levels.at(0));
  }
  return 0;
}


void Database_Jobs::set_start (int id, std::string start)
{
  SqliteDatabase sql (jobs);
  sql.add ("UPDATE jobs SET start =");
  sql.add (start);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


std::string Database_Jobs::get_start (int id)
{
  SqliteDatabase sql (jobs);
  sql.add ("SELECT start FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["start"];
  if (!result.empty()) {
    const auto start = result.at(0);
    return start;
  }
  return std::string();
}


void Database_Jobs::set_percentage (int id, int percentage)
{
  SqliteDatabase sql (jobs);
  sql.add ("UPDATE jobs SET percentage =");
  sql.add (percentage);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


std::string Database_Jobs::get_percentage (int id)
{
  SqliteDatabase sql (jobs);
  sql.add ("SELECT percentage FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> percentages = sql.query () ["percentage"];
  if (!percentages.empty()) {
    return percentages.at(0);
  }
  return std::string();
}


void Database_Jobs::set_progress (int id, std::string progress)
{
  SqliteDatabase sql (jobs);
  sql.add ("UPDATE jobs SET progress =");
  sql.add (progress);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


std::string Database_Jobs::get_progress (int id)
{
  SqliteDatabase sql (jobs);
  sql.add ("SELECT progress FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> progress = sql.query () ["progress"];
  if (!progress.empty()) {
    return progress.at(0);
  }
  return std::string();
}


void Database_Jobs::set_result (int id, std::string result)
{
  SqliteDatabase sql (jobs);
  sql.add ("UPDATE jobs SET result =");
  sql.add (result);
  sql.add ("WHERE id =");
  sql.add (id);
  sql.add (";");
  sql.execute();
}


std::string Database_Jobs::get_result (int id)
{
  SqliteDatabase sql (jobs);
  sql.add ("SELECT result FROM jobs WHERE id =");
  sql.add (id);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["result"];
  if (!result.empty()) {
    return result.at(0);
  }
  return std::string();
}
