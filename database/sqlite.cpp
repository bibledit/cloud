/*
Copyright (©) 2003-2019 Teus Benschop.

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
mutex sqlite_execute_mutex;


sqlite3 * database_sqlite_connect_file (string filename)
{
  sqlite3 *db;
  int rc = sqlite3_open (filename.c_str(), &db);
  if (rc) {
    const char * error = sqlite3_errmsg (db);
    database_sqlite_error (db, "Database " + filename, (char *) error);
    return NULL;
  }
  sqlite3_busy_timeout (db, 1000);
  return db;
}


// The function provides the path to the "database" in the default database folder.
// It does this in case "database" contains no path.
// If it has a path, then it returns the path as given.
string database_sqlite_file (string database)
{
  if (filter_url_dirname (database) == ".") {
    return filter_url_create_root_path (database_logic_databases (), database + database_sqlite_suffix ());
  }
  return database;
}


string database_sqlite_suffix ()
{
  return ".sqlite";
}


sqlite3 * database_sqlite_connect (string database)
{
  return database_sqlite_connect_file (database_sqlite_file (database));
}


string database_sqlite_no_sql_injection (string sql)
{
  return filter_string_str_replace ("'", "''", sql);
}


void database_sqlite_exec (sqlite3 * db, string sql)
{
  char *error = NULL;
  if (db) {
    sqlite_execute_mutex.lock ();
    int rc = sqlite3_exec (db, sql.c_str(), NULL, NULL, &error);
    sqlite_execute_mutex.unlock ();
    if (rc != SQLITE_OK) database_sqlite_error (db, sql, error);
  } else {
    database_sqlite_error (db, sql, error);
  }
  if (error) sqlite3_free (error);
}


map <string, vector <string> > database_sqlite_query (sqlite3 * db, string sql)
{
  char * error = NULL;
  SqliteReader reader (0);
  if (db) {
    sqlite_execute_mutex.lock ();
    int rc = sqlite3_exec (db, sql.c_str(), reader.callback, &reader, &error);
    sqlite_execute_mutex.unlock ();
    if (rc != SQLITE_OK) database_sqlite_error (db, sql, error);
  } else {
    database_sqlite_error (db, sql, error);
  }
  if (error) sqlite3_free (error);
  return reader.result;
}


void database_sqlite_disconnect (sqlite3 * database)
{
  if (database) sqlite3_close (database);
}


// Does an integrity check on the database.
// Returns true if healthy, false otherwise.
bool database_sqlite_healthy (string database)
{
  string file = database_sqlite_file (database);
  bool ok = false;
  // Do an integrity check on the database.
  // An empty file appears healthy too, so deal with that.
  if (filter_url_filesize (file) > 0) {
    sqlite3 * db = database_sqlite_connect (database);
    string query = "PRAGMA integrity_check;";
    map <string, vector <string> > result = database_sqlite_query (db, query);
    vector <string> health = result ["integrity_check"];
    if (health.size () == 1) {
      if (health [0] == "ok") ok = true;
    }
    database_sqlite_disconnect (db);
  }
  return ok;
}


// Logs any error on the database connection,
// The error will be prefixed by $prefix.
void database_sqlite_error (sqlite3 * database, const string & prefix, char * error)
{
  string message = prefix;
  if (error) {
    message.append (" - ");
    message.append (error);
  }
  if (database) {
    int errcode = sqlite3_errcode (database);
    const char * errstr = sqlite3_errstr (errcode);
    string error_string;
    if (errstr) error_string.assign (errstr);
    int x_errcode = sqlite3_extended_errcode (database);
    const char * x_errstr = sqlite3_errstr (x_errcode);
    string extended_error_string;
    if (x_errstr) extended_error_string.assign (x_errstr);
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
    if (!message.empty ()) message.append (" - ");
    message.append ("No database connection");
  }
  Database_Logs::log (message);
}


void SqliteSQL::clear ()
{
  sql.clear ();
}


void SqliteSQL::add (const char * fragment)
{
  sql.append (" ");
  sql.append (fragment);
  sql.append (" ");
}

void SqliteSQL::add (int value)
{
  sql.append (" ");
  sql.append (convert_to_string (value));
  sql.append (" ");
}

void SqliteSQL::add (string value)
{
  sql.append (" '");
  value = database_sqlite_no_sql_injection (value);
  sql.append (value);
  sql.append ("' ");
}


SqliteReader::SqliteReader (int dummy)
{
  if (dummy) {};
}


SqliteReader::~SqliteReader ()
{
}


int SqliteReader::callback (void *userdata, int argc, char **argv, char **column_names)
{
  for (int i = 0; i < argc; i++) {
    // Handle NULL field.
    if (argv [i] == NULL) ((SqliteReader *) userdata)->result [column_names [i]].push_back ("");
    else ((SqliteReader *) userdata)->result [column_names [i]].push_back (argv[i]);
  }
  return 0;
}


SqliteDatabase::SqliteDatabase (string filename)
{
  db = database_sqlite_connect (filename);
}


SqliteDatabase::~SqliteDatabase ()
{
  database_sqlite_disconnect (db);
}


void SqliteDatabase::clear ()
{
  sql.clear ();
}


void SqliteDatabase::add (const char * fragment)
{
  sql.append (" ");
  sql.append (fragment);
  sql.append (" ");
}


void SqliteDatabase::add (int value)
{
  sql.append (" ");
  sql.append (convert_to_string (value));
  sql.append (" ");
}


void SqliteDatabase::add (string value)
{
  sql.append (" '");
  value = database_sqlite_no_sql_injection (value);
  sql.append (value);
  sql.append ("' ");
}


void SqliteDatabase::execute ()
{
  database_sqlite_exec (db, sql);
}


map <string, vector <string> > SqliteDatabase::query ()
{
  return database_sqlite_query (db, sql);
}
