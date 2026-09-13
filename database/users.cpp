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


#include <database/sqlite.h>
#include <database/users.h>
#include <filter/md5.h>
#include <filter/roles.h>
#include <filter/string.h>


// This database is resilient.
// The data is stored in a SQLite database.
// This part is read often, and infrequently written to.
// Due to the infrequent write operations, there is a low and acceptable chance of corruption.


namespace database::users {


// The filename of the database.
static const char * filename()
{
    return "users";
}


void create()
{
    SqliteDatabase sql(filename());
    sql.add("CREATE TABLE IF NOT EXISTS users (username text, password text, level integer, email text);");
    sql.execute();
}


void upgrade()
{
    // Several extra columns are available in older databases.
    // They are not in use.
    // They cannot be dropped easily in SQLite.
    // Leave them for just now.

    // Add columns for LDAP authentication and for disabling an account,
    // if the columns are not yet there.
    SqliteDatabase sql(filename());
    sql.add("PRAGMA table_info (users);");
    const std::vector<std::string> columns = sql.query()["name"];
    if (not filter::string::in_array(static_cast<std::string>("ldap"), columns))
    {
        sql.clear();
        sql.add("ALTER TABLE users ADD COLUMN ldap boolean;");
        sql.execute();
    }
    if (not filter::string::in_array(static_cast<std::string>("disabled"), columns))
    {
        sql.clear();
        sql.add("ALTER TABLE users ADD COLUMN disabled boolean;");
        sql.execute();
    }
}


void trim()
{
}


void optimize()
{
    SqliteDatabase sql(filename());
    sql.add("VACUUM;");
    sql.execute();
}


// Add the user details to the database.
void add_user(const std::string& user, const std::string& password, const int level,
              const std::string& email)
{
    {
        SqliteDatabase sql(database::users::filename());
        sql.add("INSERT INTO users (username, level, email) VALUES (");
        sql.add(user);
        sql.add(",");
        sql.add(level);
        sql.add(",");
        sql.add(email);
        sql.add(");");
        sql.execute();
    }
    set_password(user, password);
}


// Updates the password for user.
void set_password(const std::string& user, const std::string& password)
{
    SqliteDatabase sql(database::users::filename());
    sql.add("UPDATE users SET password =");
    sql.add(md5(password));
    sql.add("WHERE username =");
    sql.add(user);
    sql.add(";");
    sql.execute();
}


// Returns true if the user and password match.
bool match_user_password(const std::string& user, const std::string& password)
{
    SqliteDatabase sql(filename());
    sql.add("SELECT username FROM users WHERE username =");
    sql.add(user);
    sql.add("AND password =");
    sql.add(md5(password));
    sql.add("AND (disabled IS NULL OR disabled = 0);");
    const std::vector<std::string> result = sql.query()["username"];
    return (not result.empty());
}


// Returns true if the email and password match.
bool match_email_password (const std::string& email, const std::string& password)
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT username FROM users WHERE email =");
    sql.add (email);
    sql.add ("AND password =");
    sql.add (md5 (password));
    sql.add ("AND (disabled IS NULL OR disabled = 0);");
    const std::vector <std::string> result = sql.query () ["username"];
    return (not result.empty());
}


// Returns the query to execute to add a new user.
std::string add_user_query (std::string user, std::string password, const int level, std::string email)
{
    user = sqlite::no_sql_injection (user);
    password = md5 (password);
    email = sqlite::no_sql_injection (email);
    const std::string query = "INSERT INTO users (username, password, level, email) VALUES ('" + user + "', '" + password + "', " + std::to_string (level) + ", '" + email + "');";
    return query;
}


// Returns the username that belongs to the email.
std::string get_email_to_user (const std::string& email)
{
    SqliteDatabase sql (database::users::filename ());
    sql.add ("SELECT username FROM users WHERE email =");
    sql.add (email);
    sql.add (";");
    if (const auto result = sql.query()["username"]; not result.empty())
        return result[0];
    return {};
}


// Returns the email address that belongs to user.
std::string get_email (const std::string& user)
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT email FROM users WHERE username = ");
    sql.add (user);
    sql.add (";");
    if (const auto result = sql.query()["email"]; not result.empty())
        return result[0];
    return {};
}


// Returns true if the username exists in the database.
bool username_exists (const std::string& user)
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT username FROM users WHERE username =");
    sql.add (user);
    sql.add (";");
    const std::vector <std::string> result = sql.query () ["username"];
    return not result.empty ();
}


// Returns true if the email address exists in the database.
bool email_exists (const std::string& email)
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT username FROM users WHERE email = ");
    sql.add (email);
    sql.add (";");
    const auto result = sql.query()["username"];
    return not result.empty ();
}


// Returns the level that belongs to the user.
int get_level (const std::string& user)
{
    SqliteDatabase sql (database::users::filename ());
    sql.add ("SELECT level FROM users WHERE username = ");
    sql.add (user);
    sql.add (";");
    if (const auto result = sql.query () ["level"]; not result.empty())
        return filter::string::convert_to_int(result[0]);
    return roles::guest;
}


// Updates the level of a given user.
void set_level (const std::string& user, const int level)
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
void remove_user (const std::string& user)
{
    SqliteDatabase sql (filename ());
    sql.add ("DELETE FROM users WHERE username =");
    sql.add (user);
    sql.add (";");
    sql.execute ();
}


// Returns an array with the usernames of the site administrators.
std::vector <std::string> get_administrators ()
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT username FROM users WHERE level =");
    sql.add (roles::admin);
    sql.add ("AND (disabled IS NULL OR disabled = 0);");
    const auto result = sql.query () ["username"];
    return result;
}


// Returns the query to update a user's email address.
std::string update_email_query (const std::string& user, const std::string& email)
{
    SqliteDatabase sql (filename ());
    sql.add ("UPDATE users SET email =");
    sql.add (email);
    sql.add ("WHERE username =");
    sql.add (user);
    sql.add (";");
    return sql.get_sql();
}


// Executes the SQL fragment.
void execute (const std::string& sql_fragment)
{
    SqliteDatabase sql (filename ());
    sql.set_sql(sql_fragment);
    sql.execute ();
}


// Updates the "email" for "user".
void update_user_email (const std::string& user, const std::string& email)
{
    execute (update_email_query (user, email));
}


// Return an array with the available users.
std::vector <std::string> get_users ()
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT username FROM users;");
    const auto result = sql.query () ["username"];
    return result;
}


// Returns the md5 hash for the $user's password.
std::string get_md5 (const std::string& user)
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT password FROM users WHERE username =");
    sql.add (user);
    sql.add (";");
    if (const auto result = sql.query () ["password"]; not result.empty())
        return result [0];
    return {};
}


// Set the LDAP state for the $user account $on or off.
void set_ldap (const std::string& user, const bool on)
{
    SqliteDatabase sql (database::users::filename ());
    sql.add ("UPDATE users SET ldap =");
    sql.add (filter::string::convert_to_int (on));
    sql.add ("WHERE username =");
    sql.add (user);
    sql.add (";");
    sql.execute ();
}


// Get whether the $user account comes from a LDAP server.
bool get_ldap (const std::string& user)
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT ldap FROM users WHERE username =");
    sql.add (user);
    sql.add (";");
    if (const auto result = sql.query () ["ldap"]; not result.empty()) {
        const bool ldap_is_on = filter::string::convert_to_bool (result [0]);
        return ldap_is_on;
    }
    return false;
}


// Enable the $user account.
void set_enabled (const std::string& user, const bool on)
{
    SqliteDatabase sql (filename ());
    sql.add ("UPDATE users SET disabled =");
    sql.add (filter::string::convert_to_int (!on));
    sql.add ("WHERE username =");
    sql.add (user);
    sql.add (";");
    sql.execute ();
}


// Disable the $user account.
bool get_enabled (const std::string& user)
{
    SqliteDatabase sql (filename ());
    sql.add ("SELECT disabled FROM users WHERE username =");
    sql.add (user);
    sql.add (";");
    std::vector <std::string> result = sql.query () ["disabled"];
    if (!result.empty()) return !filter::string::convert_to_bool (result [0]);
    return false;
}


} // namespace
