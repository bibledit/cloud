/*
Copyright (©) 2003-2022 Teus Benschop.

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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <database/users.h>
#include <database/sqlite.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/roles.h>
#include <filter/date.h>


// This database is resilient.
// The data is stored in a SQLite database.
// This part is read often, and infrequently written to.
// Due to the infrequent write operations, there is a low and acceptable chance of corruption.


void Database_Users::create ()
{
  SqliteDatabase sql (filename ());
  sql.add ("CREATE TABLE IF NOT EXISTS users (username text, password text, level integer, email text);");
  sql.execute ();
}


void Database_Users::upgrade ()
{
  // Several extra columns are available in older databases.
  // They are not in use.
  // They cannot be dropped easily in SQLite.
  // Leave them for just now.

  // Add columns for LDAP authentication and for disabling an account,
  // if the columns are not yet there.
  SqliteDatabase sql (filename ());
  sql.add ("PRAGMA table_info (users);");
  vector <string> columns = sql.query () ["name"];
  if (!in_array ((string)"ldap", columns)) {
    sql.clear ();
    sql.add ("ALTER TABLE users ADD COLUMN ldap boolean;");
    sql.execute ();
  }
  if (!in_array ((string)"disabled", columns)) {
    sql.clear ();
    sql.add ("ALTER TABLE users ADD COLUMN disabled boolean;");
    sql.execute ();
  }
}


void Database_Users::trim ()
{
}


void Database_Users::optimize ()
{
  SqliteDatabase sql (filename ());
  sql.add ("VACUUM;");
  sql.execute ();
}


// Add the user details to the database.
void Database_Users::add_user (string user, string password, int level, string email)
{
  {
    SqliteDatabase sql (filename ());
    sql.add ("INSERT INTO users (username, level, email) VALUES (");
    sql.add (user);
    sql.add (",");
    sql.add (level);
    sql.add (",");
    sql.add (email);
    sql.add (");");
    sql.execute ();
  }
  set_password (user, password);
}


// Updates the password for user.
void Database_Users::set_password (string user, string password)
{
  SqliteDatabase sql (filename ());
  sql.add ("UPDATE users SET password =");
  sql.add (md5 (password));
  sql.add ("WHERE username =");
  sql.add (user);
  sql.add (";");
  sql.execute ();
}


// Returns true if the user and password match.
bool Database_Users::matchUserPassword (string user, string password)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT username FROM users WHERE username =");
  sql.add (user);
  sql.add ("AND password =");
  sql.add (md5 (password));
  sql.add ("AND (disabled IS NULL OR disabled = 0);");
  vector <string> result = sql.query () ["username"];
  return (!result.empty());
}


// Returns true if the email and password match.
bool Database_Users::matchEmailPassword (string email, string password)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT username FROM users WHERE email =");
  sql.add (email);
  sql.add ("AND password =");
  sql.add (md5 (password));
  sql.add ("AND (disabled IS NULL OR disabled = 0);");
  vector <string> result = sql.query () ["username"];
  return (!result.empty());
}


// Returns the query to execute to add a new user.
string Database_Users::add_userQuery (string user, string password, int level, string email)
{
  user = database_sqlite_no_sql_injection (user);
  password = md5 (password);
  email = database_sqlite_no_sql_injection (email);
  string query = "INSERT INTO users (username, password, level, email) VALUES ('" + user + "', '" + password + "', " + convert_to_string (level) + ", '" + email + "');";
  return query;
}


// Returns the username that belongs to the email.
string Database_Users::getEmailToUser (string email)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT username FROM users WHERE email =");
  sql.add (email);
  sql.add (";");
  vector <string> result = sql.query () ["username"];
  if (!result.empty()) return result [0];
  return "";
}


// Returns the email address that belongs to user.
string Database_Users::get_email (string user)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT email FROM users WHERE username = ");
  sql.add (user);
  sql.add (";");
  vector <string> result = sql.query () ["email"];
  if (!result.empty()) return result [0];
  return "";
}


// Returns true if the username exists in the database.
bool Database_Users::usernameExists (string user)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT username FROM users WHERE username =");
  sql.add (user);
  sql.add (";");
  vector <string> result = sql.query () ["username"];
  return !result.empty ();
}


// Returns true if the email address exists in the database.
bool Database_Users::emailExists (string email)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT username FROM users WHERE email = ");
  sql.add (email);
  sql.add (";");
  vector <string> result = sql.query () ["username"];
  return !result.empty ();
}


// Returns the level that belongs to the user.
int Database_Users::get_level (string user)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT level FROM users WHERE username = ");
  sql.add (user);
  sql.add (";");
  vector <string> result = sql.query () ["level"];
  if (!result.empty()) return convert_to_int (result [0]);
  return Filter_Roles::guest ();
}


// Updates the level of a given user.
void Database_Users::set_level (string user, int level)
{
  SqliteDatabase sql (filename ());
  sql.add ("UPDATE users SET level =");
  sql.add (level);
  sql.add ("WHERE username =");
  sql.add (user);
  sql.add (";");
  sql.execute ();
}


// Remove a user from the database.
void Database_Users::removeUser (string user)
{
  SqliteDatabase sql (filename ());
  sql.add ("DELETE FROM users WHERE username =");
  sql.add (user);
  sql.add (";");
  sql.execute ();
}


// Returns an array with the usernames of the site administrators.
vector <string> Database_Users::getAdministrators ()
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT username FROM users WHERE level =");
  sql.add (Filter_Roles::admin ());
  sql.add ("AND (disabled IS NULL OR disabled = 0);");
  vector <string> result = sql.query () ["username"];
  return result;
}


// Returns the query to update a user's email address.
string Database_Users::updateEmailQuery (string user, string email)
{
  SqliteDatabase sql (filename ());
  sql.add ("UPDATE users SET email =");
  sql.add (email);
  sql.add ("WHERE username =");
  sql.add (user);
  sql.add (";");
  return sql.sql;
}


// Updates the "email" for "user".
void Database_Users::updateUserEmail (string user, string email)
{
  execute (updateEmailQuery (user, email));
}


// Return an array with the available users.
vector <string> Database_Users::get_users ()
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT username FROM users;");
  vector <string> result = sql.query () ["username"];
  return result;
}


// Returns the md5 hash for the $user's password.
string Database_Users::get_md5 (string user)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT password FROM users WHERE username =");
  sql.add (user);
  sql.add (";");
  vector <string> result = sql.query () ["password"];
  if (!result.empty()) return result [0];
  return "";
}


// Executes the SQL fragment.
void Database_Users::execute (string sqlfragment)
{
  SqliteDatabase sql (filename ());
  sql.sql = sqlfragment;
  sql.execute ();
}


// Set the LDAP state for the $user account $on or off.
void Database_Users::set_ldap (string user, bool on)
{
  SqliteDatabase sql (filename ());
  sql.add ("UPDATE users SET ldap =");
  sql.add (convert_to_int (on));
  sql.add ("WHERE username =");
  sql.add (user);
  sql.add (";");
  sql.execute ();
}


// Get whether the $user account comes from a LDAP server.
bool Database_Users::get_ldap (string user)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT ldap FROM users WHERE username =");
  sql.add (user);
  sql.add (";");
  vector <string> result = sql.query () ["ldap"];
  if (!result.empty()) {
    bool ldap_is_on = convert_to_bool (result [0]);
    return ldap_is_on;
  }
  return false;
}


// Enable the $user account.
void Database_Users::set_enabled (string user, bool on)
{
  SqliteDatabase sql (filename ());
  sql.add ("UPDATE users SET disabled =");
  sql.add (convert_to_int (!on));
  sql.add ("WHERE username =");
  sql.add (user);
  sql.add (";");
  sql.execute ();
}


// Disable the $user account.
bool Database_Users::get_enabled (string user)
{
  SqliteDatabase sql (filename ());
  sql.add ("SELECT disabled FROM users WHERE username =");
  sql.add (user);
  sql.add (";");
  vector <string> result = sql.query () ["disabled"];
  if (!result.empty()) return !convert_to_bool (result [0]);
  return false;
}


// The filename of the database.
const char * Database_Users::filename ()
{
  return "users";
}
