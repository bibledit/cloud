/*
Copyright (©) 2003-2025 Teus Benschop.

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
#include <filter/url.h>
#include <filter/string.h>
#include <database/logs.h>
#include <database/logic.h>


/*
It has been seen on a shared hosting platform that the MySQL server did not have
sufficient available concurrent connections at times.
Other processes were using many connections, so that none remained for Bibledit.
That is a reason against using MySQL on shared hosting.

A reason for using SQLite is that it is easier to set up.
No users, no privileges, no server.

Another reason is that a backup of the web space also back ups the SQLite databases,
provided they are stored in the web space.
With MySQL this is hard to do.

Usually with shared hosting, the web space is big, but the MySQL database space is small.
With a few Bibles, and several notes, plus resources, the MySQL database is full.
SQLite uses the huge web space that usually come with shared hosting.
Therefore it can store much more data with the same shared hosting package.

While MySQL is a faster database than SQLite in isolated experiments, 
on shared hosts it may be different.
The reasons is that on a shared host, SQLite gets the data in the same process 
straight from disk. This works differently for MySQL. In most cases, the shared
host uses a separate database server. Thus the web server fetches its data
from another host through the network. This introduces some delays.
For smaller data sets SQLite is much faster in this scenario.

Some often-used databases at times display database errors:
  disk I/O error
  unable to open database file
It was tried wiether the errors go away when "PRAGMA busy_timeout = 100;"
is executed after opening the database connection. The errors did not go away.

It was tried whether the above errors go away on shared hosting with this command:
PRAGMA journal_mode = TRUNCATE;
The errors did not go away.

It was tried whether the above errors go away on shared hosting with this command:
PRAGMA temp_store = MEMORY;
The errors did not go away.

It was tried whether the above errors go away on shared hosting with this command:
PRAGMA journal_mode = MEMORY;
The errors went away, but the database behaved in an inconsistent way.
It did not keeps its data properly, and did not update it properly.
The same behaviour was found with:
PRAGMA journal_mode = OFF;

It was tried whether setting the environment variable TMPDIR to a directory
in our own web space would improve SQLite, but this did not improve SQLite.
However, in other areas it looked as if this did give improvement.

What made a big difference is this:
1. Changing the database for the logbook from SQLite to the filesystem.
2. Setting the TMPDIR to a folder inside Bibledit's own webspace.
3. Letting the tasks runner not use a SQLite database, but the file system and memory.
The database errors went away.

*/


// Despite compiling the SQLite library in thread-safe mode,
// there's still 'database locked' and "SQL error' errors.
// Sample errors:
// INSERT INTO cache VALUES ( 136 , 0 , '' ); - database is locked - database is locked
// INSERT INTO cache VALUES ( 25 , 21 , '' ); - unrecognized token: "'" - SQL logic error or missing database
// Therefore here's an extra mutex for our own logic, and that should fix those errors.
std::mutex sqlite_execute_mutex;


// Stores values collected during a reading session of sqlite3.
class SqliteReader
{
public:
  SqliteReader (int dummy);
  ~SqliteReader ();
  std::map <std::string, std::vector <std::string> > result {};
  static int callback (void *userdata, int argc, char **argv, char **column_names);
private:
};


namespace database::sqlite {


sqlite3 * connect_file (const std::string& filename)
{
  sqlite3 *db {nullptr};
  const int rc = sqlite3_open (filename.c_str(), &db);
  if (rc) {
    const char * error = sqlite3_errmsg (db);
    database::sqlite::error (db, "Database " + filename, const_cast<char*>(error));
    return nullptr;
  }
  sqlite3_busy_timeout (db, 1000);
  return db;
}


// The function provides the path to the "database" in the default database folder.
// It does this in case "database" contains no path.
// If it has a path, then it returns the path as given.
std::string get_file (const std::string& database)
{
  if (filter_url_dirname (database) == ".") {
    return filter_url_create_root_path ({database_logic_databases (), database + suffix ()});
  }
  return database;
}


std::string suffix ()
{
  return ".sqlite";
}


sqlite3 * connect (const std::string& database)
{
  return connect_file (get_file (database));
}


std::string no_sql_injection (const std::string& sql)
{
  return filter::strings::replace ("'", "''", sql);
}


void exec (sqlite3 * db, const std::string& sql)
{
  char *error = nullptr;
  if (db) {
    sqlite_execute_mutex.lock ();
    const int rc = sqlite3_exec (db, sql.c_str(), nullptr, nullptr, &error);
    sqlite_execute_mutex.unlock ();
    if (rc != SQLITE_OK) 
      database::sqlite::error (db, sql, error);
  } else {
    database::sqlite::error (db, sql, error);
  }
  if (error) 
    sqlite3_free (error);
}


std::map <std::string, std::vector <std::string> > query (sqlite3 * db, const std::string& sql)
{
  char * error = nullptr;
  SqliteReader reader (0);
  if (db) {
    sqlite_execute_mutex.lock ();
    const int rc = sqlite3_exec (db, sql.c_str(), reader.callback, &reader, &error);
    sqlite_execute_mutex.unlock ();
    if (rc != SQLITE_OK) 
      database::sqlite::error (db, sql, error);
  } else {
    database::sqlite::error (db, sql, error);
  }
  if (error) 
    sqlite3_free (error);
  return reader.result;
}


void disconnect (sqlite3 * database)
{
  if (database) 
    sqlite3_close (database);
}


// Does an integrity check on the database.
// Returns true if healthy, false otherwise.
bool healthy (const std::string& database)
{
  const std::string file = database::sqlite::get_file (database);
  bool ok = false;
  // Do an integrity check on the database.
  // An empty file appears healthy too, so deal with that.
  if (filter_url_filesize (file) > 0) {
    sqlite3 * db = connect (database);
    const std::string query = "PRAGMA integrity_check;";
    std::map <std::string, std::vector <std::string> > result = database::sqlite::query (db, query);
    const std::vector <std::string> health = result ["integrity_check"];
    if (health.size () == 1) {
      if (health.at(0) == "ok")
        ok = true;
    }
    disconnect (db);
  }
  return ok;
}


// Logs any error on the database connection,
// The error will be prefixed by $prefix.
void error (sqlite3 * database, const std::string& prefix, char * error)
{
  std::string message = prefix;
  if (error) {
    message.append (" - ");
    message.append (error);
  }
  if (database) {
    const int errcode = sqlite3_errcode (database);
    const char * errstr = sqlite3_errstr (errcode);
    std::string error_string;
    if (errstr) 
      error_string.assign (errstr);
    const int x_errcode = sqlite3_extended_errcode (database);
    const char * x_errstr = sqlite3_errstr (x_errcode);
    std::string extended_error_string;
    if (x_errstr) 
      extended_error_string.assign (x_errstr);
    if (!error_string.empty ()) {
      message.append (" - ");
      message.append (error_string);
    }
    if (extended_error_string != error_string) {
      message.append (" - ");
      message.append (extended_error_string);
    }
    const char * filename = sqlite3_db_filename (database, "main");
    if (filename) {
      message.append (" - ");
      message.append (filename);
    }
  } else {
    if (!message.empty ()) 
      message.append (" - ");
    message.append ("No database connection");
  }
  Database_Logs::log (message);
}


} // Namespace.


SqliteReader::SqliteReader (int dummy)
{
  if (dummy) {};
}


SqliteReader::~SqliteReader ()
{
}


int SqliteReader::callback (void *userdata, int argc, char **argv, char **column_names)
{
  SqliteReader * sqlite_reader = static_cast<SqliteReader *> (userdata);
  for (int i = 0; i < argc; i++) {
    // Handle nullptr field.
    if (argv [i] == nullptr) sqlite_reader->result [column_names [i]].push_back ("");
    else sqlite_reader->result [column_names [i]].push_back (argv[i]);
  }
  return 0;
}


SqliteDatabase::SqliteDatabase (const std::string& filename)
{
  // Connect to the database lazily, only when first needed,
  // i.e. just before execution of SQL or querying SQL.
  m_filename = filename;
}


SqliteDatabase::~SqliteDatabase ()
{
  // When the object goes out of scope, it disconnects from the SQlite database.
  if (db)
    database::sqlite::disconnect (db);
}


void SqliteDatabase::clear ()
{
  m_sql.clear ();
}


void SqliteDatabase::add (const char * fragment)
{
  m_sql.append (" ");
  m_sql.append (fragment);
  m_sql.append (" ");
}


void SqliteDatabase::add (int value)
{
  m_sql.append (" ");
  m_sql.append (std::to_string (value));
  m_sql.append (" ");
}


void SqliteDatabase::add (std::string value)
{
  m_sql.append (" '");
  value = database::sqlite::no_sql_injection (value);
  m_sql.append (value);
  m_sql.append ("' ");
}


const std::string& SqliteDatabase::get_sql() const
{
  return m_sql;
}


void SqliteDatabase::set_sql (const std::string& sql)
{
  m_sql = sql;
}


void SqliteDatabase::push_sql()
{
  m_save_restore = std::move (m_sql);
}


void SqliteDatabase::pop_sql()
{
  m_sql = std::move (m_save_restore);

}


void SqliteDatabase::execute ()
{
  // Connect to the database if not yet connected, i.e. connect lazily.
  if (!db)
    db = database::sqlite::connect (m_filename);
  // Execute the query.
  database::sqlite::exec (db, m_sql);
}


std::map <std::string, std::vector <std::string> > SqliteDatabase::query ()
{
  // Connect to the database if not yet connected, i.e. connect lazily.
  if (!db)
    db = database::sqlite::connect (m_filename);
  // Execute the query.
  return database::sqlite::query (db, m_sql);
}


// Manually disconnect from the database if so required.
void SqliteDatabase::disconnect ()
{
  if (db)
    database::sqlite::disconnect (db);
  db = nullptr;
}
