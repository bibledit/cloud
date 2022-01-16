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


#include <database/privileges.h>
#include <database/sqlite.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/logic.h>


// This database is resilient.
// The data is stored in a SQLite database.
// This part is read often, and infrequently written to.
// Due to the infrequent write operations, there is a low and acceptable change of corruption.


// The name of the database.
const char * Database_Privileges::database ()
{
  return "privileges";
}


void Database_Privileges::create ()
{
  SqliteDatabase sql (database ());
  
  sql.add ("CREATE TABLE IF NOT EXISTS bibles ("
           " username text,"
           " bible text,"
           " book integer,"
           " write boolean"
           ");");
  sql.execute ();
  
  sql.add ("CREATE TABLE IF NOT EXISTS features ("
           " username text,"
           " feature integer"
           ");");
  sql.execute ();
}


void Database_Privileges::upgrade ()
{
}


void Database_Privileges::optimize ()
{
  // Recreate damaged database.
  if (!healthy ()) {
    filter_url_unlink_cpp17 (database_sqlite_file (database ()));
    create ();
  }
  // Vacuum it.
  SqliteDatabase sql (database ());
  // On Android, this pragma prevents the following error: VACUUM; Unable to open database file.
  sql.add ("PRAGMA temp_store = MEMORY;");
  sql.execute ();
  sql.clear ();
  sql.add ("VACUUM;");
  sql.execute ();
}


bool Database_Privileges::healthy ()
{
  return database_sqlite_healthy (database ());
}


string Database_Privileges::save (string username)
{
  SqliteDatabase sql (database ());
  
  vector <string> lines;

  lines.push_back (bibles_start ());
  sql.add ("SELECT bible, book, write FROM bibles WHERE username =");
  sql.add (username);
  sql.add (";");
  map <string, vector <string> > result = sql.query ();
  vector <string> bible = result ["bible"];
  vector <string> book =  result ["book"];
  vector <string> write = result ["write"];
  for (size_t i = 0; i < bible.size (); i++) {
    lines.push_back (bible [i]);
    lines.push_back (book [i]);
    // It could have just stored 0 or 1 for the boolean values.
    // But if that were done, then there would be no change in the length of the file
    // when changing only boolean values.
    // And then the client would not re-download that file.
    // To use "on" and "off", that solves the issue.
    bool b = convert_to_bool (write[i]);
    if (b) lines.push_back (on ());
    else lines.push_back (off ());
  }
  lines.push_back (bibles_end ());
  
  lines.push_back (features_start ());
  sql.clear ();
  sql.add ("SELECT feature FROM features WHERE username =");
  sql.add (username);
  sql.add (";");
  result = sql.query ();
  vector <string> feature = result ["feature"];
  for (size_t i = 0; i < feature.size (); i++) {
    lines.push_back (feature [i]);
  }
  lines.push_back (features_end ());
  
  return filter_string_implode (lines, "\n");
}


void Database_Privileges::load (string username, const string & data)
{
  // Clear all data for the user.
  {
    SqliteDatabase sql (database ());
    sql.add ("DELETE FROM bibles WHERE username =");
    sql.add (username);
    sql.add (";");
    sql.execute ();
    sql.clear ();
    sql.add ("DELETE FROM features WHERE username =");
    sql.add (username);
    sql.add (";");
    sql.execute ();
  }

  vector <string> lines = filter_string_explode (data, '\n');
  bool loading_bibles = false;
  string bible_value;
  int book_value = 0;
  bool write_value = false;
  bool loading_features = false;
  int counter = 0;

  for (auto & line : lines) {

    if (line == bibles_end ()) {
      loading_bibles = false;
    }
    if (line == features_end ()) {
      loading_features = false;
    }
  
    counter++;
    
    if (loading_bibles) {
      if (counter == 1) bible_value = line;
      if (counter == 2) book_value = convert_to_int (line);
      if (counter == 3) {
        write_value = (line == on ());
        setBibleBook (username, bible_value, book_value, write_value);
        counter = 0;
      }
    }
    
    if (loading_features) {
      setFeature (username, convert_to_int (line), true);
    }
    
    if (line == bibles_start ()) {
      loading_bibles = true;
      counter = 0;
    }
    if (line == features_start ()) {
      loading_features = true;
      counter = 0;
    }
    
  }
}


// Give a privilege to a $username to access $bible $book to read it, or also to $write it.
void Database_Privileges::setBibleBook (string username, string bible, int book, bool write)
{
  // First remove any entry.
  removeBibleBook (username, bible, book);
  // Store the new entry.
  SqliteDatabase sql (database ());
  sql.add ("INSERT INTO bibles VALUES (");
  sql.add (username);
  sql.add (",");
  sql.add (bible);
  sql.add (",");
  sql.add (book);
  sql.add (",");
  sql.add (write);
  sql.add (");");
  sql.execute ();
}


// Give a privilege to a $username to access $bible to read it, or also to $write it.
void Database_Privileges::setBible (string username, string bible, bool write)
{
  // First remove any entry.
  removeBibleBook (username, bible, 0);
  // Store the new entry.
  SqliteDatabase sql (database ());
  sql.add ("INSERT INTO bibles VALUES (");
  sql.add (username);
  sql.add (",");
  sql.add (bible);
  sql.add (",");
  sql.add (0);
  sql.add (",");
  sql.add (write);
  sql.add (");");
  sql.execute ();
}


// Read the privilege from the database whether $username has access to $bible $book.
// The privileges are stored in $read for read-only access,
// and in $write for write access.
void Database_Privileges::getBibleBook (string username, string bible, int book, bool & read, bool & write)
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT write FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add ("AND book =");
  sql.add (book);
  sql.add (";");
  vector <string> result = sql.query () ["write"];
  if (result.empty()) {
    // Not in database: No access.
    read = false;
    write = false;
  } else {
    // Occurs in database: Read access.
    read = true;
    // Take write access from the database field.
    write = convert_to_bool (result [0]);
  }
}


// Returns a tuple with <read, write> whether the $username has access to the given $bible.
tuple <bool, bool> Database_Privileges::getBible (string username, string bible)
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT write FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add (";");
  vector <string> result = sql.query () ["write"];
  bool read = (!result.empty());
  sql.clear ();
  sql.add ("SELECT write FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add ("AND write;");
  result = sql.query () ["write"];
  bool write = (!result.empty());
  return make_tuple(read, write);
}


int Database_Privileges::getBibleBookCount ()
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT count(*) FROM bibles;");
  vector <string> result = sql.query () ["count(*)"];
  if (result.empty ()) return 0;
  return convert_to_int (result [0]);
}


// Returns true if a record for $username / $bible / $book exists in the database.
// When the $book = 0, it takes any book.
bool Database_Privileges::getBibleBookExists (string username, string bible, int book)
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT rowid FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  if (book) {
    sql.add ("AND book =");
    sql.add (book);
  }
  sql.add (";");
  vector <string> result = sql.query () ["rowid"];
  return !result.empty();
}


// Remove the privilege of a $username to have access to $bible $book.
// Removing the privilege for $book 0 removes them for all possible books.
void Database_Privileges::removeBibleBook (string username, string bible, int book)
{
  SqliteDatabase sql (database ());
  sql.add ("DELETE FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  if (book) {
    sql.add ("AND book =");
    sql.add (book);
  }
  sql.add (";");
  sql.execute ();
}


// Remove data for $bible from the database.
void Database_Privileges::removeBible (string bible)
{
  SqliteDatabase sql (database ());
  sql.add ("DELETE FROM bibles WHERE bible =");
  sql.add (bible);
  sql.add (";");
  sql.execute ();
}


void Database_Privileges::setFeature (string username, int feature, bool enabled)
{
  SqliteDatabase sql (database ());
  sql.add ("DELETE FROM features WHERE username =");
  sql.add (username);
  sql.add ("AND feature =");
  sql.add (feature);
  sql.add (";");
  sql.execute ();
  if (enabled) {
    sql.clear ();
    sql.add ("INSERT INTO features VALUES (");
    sql.add (username);
    sql.add (",");
    sql.add (feature);
    sql.add (");");
    sql.execute ();
  }
}


bool Database_Privileges::getFeature (string username, int feature)
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT rowid FROM features WHERE username =");
  sql.add (username);
  sql.add ("AND feature =");
  sql.add (feature);
  sql.add (";");
  vector <string> result = sql.query () ["rowid"];
  if (result.empty()) return false;
  return true;
}


// Remove privileges for $username from the entire database.
void Database_Privileges::removeUser (string username)
{
  SqliteDatabase sql (database ());
  sql.add ("DELETE FROM bibles WHERE username =");
  sql.add (username);
  sql.add (";");
  sql.execute ();
  sql.clear ();
  sql.add ("DELETE FROM features WHERE username =");
  sql.add (username);
  sql.add (";");
  sql.execute ();
}


const char * Database_Privileges::bibles_start ()
{
  return "bibles_start";
}


const char * Database_Privileges::bibles_end ()
{
  return "bibles_end";
}


const char * Database_Privileges::features_start ()
{
  return "features_start";
}


const char * Database_Privileges::features_end ()
{
  return "features_start";
}


const char * Database_Privileges::on ()
{
  return "on";
}


const char * Database_Privileges::off ()
{
  return "off";
}


string database_privileges_directory (const string & user)
{
  return filter_url_create_path_cpp17 ({database_logic_databases (), "clients", user});
}


string database_privileges_file ()
{
  return "privileges.txt";
}


string database_privileges_client_path (const string & user)
{
  return filter_url_create_root_path_cpp17 ({database_privileges_directory (user), database_privileges_file ()});
}


void database_privileges_client_create (const string & user, bool force)
{
  // The path to the file with privileges for the $user.
  string path = database_privileges_client_path (user);
  
  // Without $force, if the file exists, we're done.
  if (!force) {
    if (file_or_dir_exists (path)) return;
  }
  
  // If needed, create the folder.
  string folder = filter_url_dirname_cpp17 (path);
  if (!file_or_dir_exists (folder)) filter_url_mkdir (folder);
  
  // The bits of privileges in human-readable form.
  string privileges = Database_Privileges::save (user);
  
  // Write the privileges to disk.
  filter_url_file_put_contents (path, privileges);
}


void database_privileges_client_remove (const string & user)
{
  string path = database_privileges_client_path (user);
  path = filter_url_dirname_cpp17 (path);
  filter_url_rmdir (path);
}
