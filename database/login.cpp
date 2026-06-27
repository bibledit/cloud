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


#include <database/login.h>
#include <database/sqlite.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/date.h>


// This database is resilient.
// The data is stored in a SQLite database.
// This part is read often, and infrequently written to.
// Due to the infrequent write operations, there is a low and acceptable change of corruption.


namespace database::login {
// Gets the current number of days since the Unix epoch.
static int timestamp()
{
    return filter::date::seconds_since_epoch() / 86400;
}


// The name of the database.
const char* database()
{
    return "login";
}


void create()
{
    SqliteDatabase sql(database());
    sql.add("CREATE TABLE IF NOT EXISTS logins ("
        " username text,"
        " address text,"
        " agent text,"
        " fingerprint text,"
        " cookie text,"
        " touch boolean,"
        " timestamp integer"
        ");");
    sql.execute();
}


void trim()
{
    // Remove persistent logins after 365 days of inactivity.
    SqliteDatabase sql(database());
    sql.add("DELETE FROM logins WHERE timestamp < ");
    sql.add(timestamp() - 365);
    sql.add(";");
    sql.execute();
}


void optimize()
{
    if (!healthy())
    {
        // (Re)create damaged or non-existing database.
        filter_url_unlink(sqlite::get_file(database()));
        create();
    }
    // Vacuum it.
    SqliteDatabase sql(database());
    // On Android, this pragma prevents the following error: VACUUM; Unable to open database file.
    sql.add("PRAGMA temp_store = MEMORY;");
    sql.execute();
    sql.clear();
    sql.add("VACUUM;");
    sql.execute();
}


bool healthy()
{
    return sqlite::healthy(database());
}


// Sets the login security tokens for a user.
// Also store whether the device is touch-enabled.
// It only writes to the table if the combination of username and tokens differs from what the table already contains.
void set_tokens(std::string username, std::string address, std::string agent, std::string fingerprint,
                std::string cookie, const bool touch)
{
    if (bool daily; username == get_username(cookie, daily)) return;
    address = md5(address);
    agent = md5(agent);
    fingerprint = md5(fingerprint);
    SqliteDatabase sql(database());
    sql.add("INSERT INTO logins VALUES (");
    sql.add(username);
    sql.add(",");
    sql.add(address);
    sql.add(",");
    sql.add(agent);
    sql.add(",");
    sql.add(fingerprint);
    sql.add(",");
    sql.add(cookie);
    sql.add(",");
    sql.add(touch);
    sql.add(",");
    sql.add(timestamp());
    sql.add(");");
    sql.execute();
}


// Remove the login security tokens for a user.
void remove_tokens(const std::string& username)
{
    SqliteDatabase sql(database());
    sql.add("DELETE FROM logins WHERE username =");
    sql.add(username);
    sql.add(";");
    sql.execute();
}


// Remove the login security tokens for a user based on the cookie.
void remove_tokens(const std::string& username, const std::string& cookie)
{
    //address = md5 (address);
    //agent = md5 (agent);
    //fingerprint = md5 (fingerprint);
    SqliteDatabase sql(database());
    sql.add("DELETE FROM logins WHERE username =");
    sql.add(username);
    sql.add("AND cookie =");
    sql.add(cookie);
    sql.add(";");
    sql.execute();
}


void rename_tokens(const std::string& username_existing, const std::string& username_new, const std::string& cookie)
{
    SqliteDatabase sql(database());
    sql.add("UPDATE logins SET username =");
    sql.add(username_new);
    sql.add("WHERE username =");
    sql.add(username_existing);
    sql.add("AND cookie =");
    sql.add(cookie);
    sql.add(";");
    sql.execute();
}


// Returns the username that matches the cookie sent by the browser.
// Once a day, $daily will be set true.
std::string get_username(const std::string& cookie, bool& daily)
{
    SqliteDatabase sql(database());
    sql.add("SELECT rowid, timestamp, username FROM logins WHERE cookie =");
    sql.add(cookie);
    sql.add(";");
    std::map<std::string, std::vector<std::string>> result = sql.query();
    if (result.empty()) return std::string();
    std::string username = result["username"][0];
    if (const int stamp = filter::string::convert_to_int(result["timestamp"][0]);
        stamp != timestamp())
    {
        // Touch the timestamp. This occurs once a day.
        const int row_id = filter::string::convert_to_int(result["rowid"][0]);
        sql.clear();
        sql.add("UPDATE logins SET timestamp =");
        sql.add(timestamp());
        sql.add("WHERE rowid =");
        sql.add(row_id);
        sql.execute();
        daily = true;
    }
    else
    {
        daily = false;
    }
    return username;
}


// Returns whether the device, that matches the cookie it sent, is touch-enabled.
bool get_touch_enabled(const std::string& cookie)
{
    SqliteDatabase sql(database());
    sql.add("SELECT touch FROM logins WHERE cookie =");
    sql.add(cookie);
    sql.add(";");
    const std::vector<std::string> result = sql.query()["touch"];
    if (not result.empty())
        return filter::string::convert_to_bool(result.at(0));
    return false;
}


void test_timestamp()
{
    SqliteDatabase sql(database());
    sql.add("UPDATE logins SET timestamp = timestamp - 370;");
    sql.execute();
}
}
