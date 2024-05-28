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


#include <database/confirm.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <filter/date.h>
#include <config/globals.h>


// Handles email and web page confirmations.
// Database resilience: It is used infrequently.
// It does not contain essential information.


#ifdef HAVE_CLOUD


constexpr const char * filename ("confirm");


namespace database::confirm {


void create ()
{
  SqliteDatabase sql (filename);
  sql.add ("CREATE TABLE IF NOT EXISTS confirm ("
           " id integer,"
           " query text,"
           " timestamp integer,"
           " mailto text,"
           " subject text,"
           " body text"
           ");");
  sql.execute ();
}


void upgrade ()
{
  // Get the existing columns in the database.
  SqliteDatabase sql (filename);
  sql.add ("PRAGMA table_info (confirm);");
  const std::vector <std::string> columns = sql.query () ["name"];
  
  // Add the column for the username if it's not yet there.
  if (!in_array (static_cast<std::string> ("username"), columns)) {
    sql.clear ();
    sql.add ("ALTER TABLE confirm ADD COLUMN username text;");
    sql.execute ();
  }
}


void optimize ()
{
  SqliteDatabase sql (filename);
  sql.add ("VACUUM;");
  sql.execute ();
}


// Returns a new unique confirmation ID as an integer
unsigned int get_new_id ()
{
  unsigned int id = 0;
  do {
    id = static_cast<unsigned int>(config_globals_int_distribution (config_globals_random_engine));
  } while (id_exists (id));
  return id;
}


// Returns true if the $id exists
bool id_exists (unsigned int id)
{
  SqliteDatabase sql (filename);
  sql.add ("SELECT id FROM confirm WHERE id =");
  sql.add (static_cast<int>(id));
  sql.add (";");
  const std::vector <std::string> ids = sql.query () ["id"];
  return !ids.empty ();
}


// Store a confirmation cycle
void store (unsigned int id, const std::string& query, 
            const std::string& to, const std::string& subject, const std::string& body,
            const std::string& username)
{
  SqliteDatabase sql (filename);
  sql.add ("INSERT INTO confirm VALUES (");
  sql.add (static_cast<int>(id));
  sql.add (",");
  sql.add (query);
  sql.add (",");
  sql.add (filter::date::seconds_since_epoch ());
  sql.add (",");
  sql.add (to);
  sql.add (",");
  sql.add (subject);
  sql.add (",");
  sql.add (body);
  sql.add (",");
  sql.add (username);
  sql.add (");");
  sql.execute ();
}


// Search the database for an existing ID in $subject.
// If it exists, it returns the ID number, else it returns 0.
unsigned int search_id (const std::string& subject)
{
  SqliteDatabase sql (filename);
  sql.add ("SELECT id FROM confirm;");
  const std::vector <std::string> ids = sql.query () ["id"];
  for (const auto& id : ids) {
    const size_t pos = subject.find (id);
    if (pos != std::string::npos) {
      return static_cast<unsigned>(filter::strings::convert_to_int (id));
    }
  }
  return 0;
}


// Returns the query for $id.
std::string get_query (unsigned int id)
{
  SqliteDatabase sql (filename);
  sql.add ("SELECT query FROM confirm WHERE id =");
  sql.add (static_cast<int>(id));
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["query"];
  if (!result.empty ())
    return result.front();
  return std::string();
}


// Returns the To: address for $id.
std::string get_mail_to (unsigned int id)
{
  SqliteDatabase sql (filename);
  sql.add ("SELECT mailto FROM confirm WHERE id =");
  sql.add (static_cast<int>(id));
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["mailto"];
  if (!result.empty ())
    return result.front();
  return std::string();
}


// Returns the Subject: for $id.
std::string get_subject (unsigned int id)
{
  SqliteDatabase sql (filename);
  sql.add ("SELECT subject FROM confirm WHERE id =");
  sql.add (static_cast<int>(id));
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["subject"];
  if (!result.empty ())
    return result.front();
  return std::string();
}


// Returns the email's body for $id.
std::string get_body (unsigned int id)
{
  SqliteDatabase sql (filename);
  sql.add ("SELECT body FROM confirm WHERE id =");
  sql.add (static_cast<int>(id));
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["body"];
  if (!result.empty ())
    return result.front();
  return std::string();
}


// Returns the username for $id.
std::string get_username (unsigned int id) // Test return valid and invalid username.
{
  SqliteDatabase sql (filename);
  sql.add ("SELECT username FROM confirm WHERE id =");
  sql.add (static_cast<int>(id));
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["username"];
  if (!result.empty ())
    return result.front();
  return std::string();
}


// Deletes $id from the table.
void erase (unsigned int id)
{
  SqliteDatabase sql (filename);
  sql.add ("DELETE FROM confirm WHERE id =");
  sql.add (static_cast<int>(id));
  sql.add (";");
  sql.execute ();
}


void trim ()
{
  // Leave entries for no more than 30 days.
  const int time = filter::date::seconds_since_epoch () - 2592000;
  SqliteDatabase sql (filename);
  sql.add ("DELETE FROM confirm WHERE timestamp <");
  sql.add (time);
  sql.add (";");
  sql.execute ();
}


}


#endif
