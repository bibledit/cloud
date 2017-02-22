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


#include <database/confirm.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <filter/date.h>


// Handles email and web page confirmations.
// Database resilience: It is used infrequently.
// It does not contain essential information.


#ifdef HAVE_CLOUD


const char * Database_Confirm::filename ()
{
  return "confirm";
}


void Database_Confirm::create ()
{
  SqliteDatabase sql (filename ());
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


void Database_Confirm::optimize ()
{
  SqliteDatabase sql (filename ());
  sql.add ("VACUUM confirm;");
  sql.execute ();
}


// getNewID - returns a new unique confirmation ID as an integer
unsigned int Database_Confirm::getNewID ()
{
  unsigned int id = 0;
  do {
    id = filter_string_rand (100000000, 999999999);
  } while (IDExists (id));
  return id;
}


// Returns true if the $id exists
bool Database_Confirm::IDExists (unsigned int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT id FROM confirm WHERE id =");
  sql.add (id);
  sql.add (";");
  vector <string> ids = sql.query () ["id"];
  return !ids.empty ();
}


// stores a confirmation cycle
void Database_Confirm::store (unsigned int id, string query, string to, string subject, string body)
{
  SqliteDatabase sql (filename ());
  sql.add ("INSERT INTO confirm VALUES (");
  sql.add (id);
  sql.add (",");
  sql.add (query);
  sql.add (",");
  sql.add (filter_date_seconds_since_epoch ());
  sql.add (",");
  sql.add (to);
  sql.add (",");
  sql.add (subject);
  sql.add (",");
  sql.add (body);
  sql.add (");");
  sql.execute ();
}


// Search the database for an existing ID in $subject.
// If it exists, it returns the ID number, else it returns 0.
unsigned int Database_Confirm::searchID (string subject)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT id FROM confirm;");
  vector <string> ids = sql.query () ["id"];
  for (string id : ids) {
    size_t pos = subject.find (id);
    if (pos != string::npos) {
      return convert_to_int (id);
    }
  }
  return 0;
}


// Returns the query for $id.
string Database_Confirm::getQuery (unsigned int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT query FROM confirm WHERE id =");
  sql.add (id);
  sql.add (";");
  vector <string> result = sql.query () ["query"];
  if (!result.empty ()) return result [0];
  return "";
}


// Returns the To: address for $id.
string Database_Confirm::getMailTo (unsigned int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT mailto FROM confirm WHERE id =");
  sql.add (id);
  sql.add (";");
  vector <string> result = sql.query () ["mailto"];
  if (!result.empty ()) return result [0];
  return "";
}


// Returns the Subject: for $id.
string Database_Confirm::getSubject (unsigned int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT subject FROM confirm WHERE id =");
  sql.add (id);
  sql.add (";");
  vector <string> result = sql.query () ["subject"];
  if (!result.empty ()) return result [0];
  return "";
}


// Returns the email's body for $id.
string Database_Confirm::getBody (unsigned int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT body FROM confirm WHERE id =");
  sql.add (id);
  sql.add (";");
  vector <string> result = sql.query () ["body"];
  if (!result.empty ()) return result [0];
  return "";
}


// Deletes $id from the table.
void Database_Confirm::erase (unsigned int id)
{
  SqliteDatabase sql (filename ());
  sql.add ("DELETE FROM confirm WHERE id =");
  sql.add (id);
  sql.add (";");
  sql.execute ();
}


void Database_Confirm::trim ()
{
  // Leave entries for no more than 30 days.
  unsigned int time = filter_date_seconds_since_epoch () - 2592000; 
  SqliteDatabase sql (filename ());
  sql.add ("DELETE FROM confirm WHERE timestamp <");
  sql.add (time);
  sql.add (";");
  sql.execute ();
}


#endif
