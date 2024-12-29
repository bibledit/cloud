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
const char * DatabasePrivileges::database ()
{
  return "privileges";
}


void DatabasePrivileges::create ()
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


void DatabasePrivileges::upgrade ()
{
}


void DatabasePrivileges::optimize ()
{
  // Recreate damaged database.
  if (!healthy ()) {
    filter_url_unlink (database::sqlite::get_file (database ()));
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


bool DatabasePrivileges::healthy ()
{
  return database::sqlite::healthy (database ());
}


std::string DatabasePrivileges::save (const std::string& username)
{
  SqliteDatabase sql (database ());
  
  std::vector <std::string> lines {};

  lines.emplace_back (bibles_start ());
  sql.add ("SELECT bible, book, write FROM bibles WHERE username =");
  sql.add (username);
  sql.add (";");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  std::vector <std::string> bible = result ["bible"];
  std::vector <std::string> book =  result ["book"];
  std::vector <std::string> write = result ["write"];
  for (size_t i = 0; i < bible.size (); i++) {
    lines.emplace_back (bible [i]);
    lines.emplace_back (book [i]);
    // It could have just stored 0 or 1 for the boolean values.
    // But if that were done, then there would be no change in the length of the file
    // when changing only boolean values.
    // And then the client would not re-download that file.
    // To use "on" and "off", that solves the issue.
    const bool b = filter::strings::convert_to_bool (write[i]);
    if (b) lines.emplace_back (on ());
    else lines.emplace_back (off ());
  }
  lines.emplace_back (bibles_end ());
  
  lines.emplace_back (features_start ());
  sql.clear ();
  sql.add ("SELECT feature FROM features WHERE username =");
  sql.add (username);
  sql.add (";");
  result = sql.query ();
  const std::vector <std::string> feature = result ["feature"];
  for (size_t i = 0; i < feature.size (); i++) {
    lines.emplace_back (feature [i]);
  }
  lines.emplace_back (features_end ());
  
  return filter::strings::implode (lines, "\n");
}


void DatabasePrivileges::load (const std::string& username, const std::string& data)
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
  
  const std::vector <std::string> lines = filter::strings::explode (data, '\n');
  bool loading_bibles {false};
  std::string bible_value {};
  int book_value {0};
  bool write_value {false};
  bool loading_features {false};
  int counter {0};

  for (const auto & line : lines) {

    if (line == bibles_end ()) {
      loading_bibles = false;
    }
    if (line == features_end ()) {
      loading_features = false;
    }
  
    counter++;
    
    if (loading_bibles) {
      if (counter == 1) bible_value = line;
      if (counter == 2) book_value = filter::strings::convert_to_int (line);
      if (counter == 3) {
        write_value = (line == on ());
        set_bible_book (username, bible_value, book_value, write_value);
        counter = 0;
      }
    }
    
    if (loading_features) {
      set_feature (username, filter::strings::convert_to_int (line), true);
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
void DatabasePrivileges::set_bible_book (const std::string& username, const std::string& bible, const int book, const bool write)
{
  // First remove any entry.
  remove_bible_book (username, bible, book);
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
void DatabasePrivileges::set_bible (const std::string& username, const std::string& bible, const bool write)
{
  // First remove any entry.
  remove_bible_book (username, bible, 0);
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
void DatabasePrivileges::get_bible_book (const std::string& username, const std::string& bible, const int book, bool & read, bool & write)
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT write FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add ("AND book =");
  sql.add (book);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["write"];
  if (result.empty()) {
    // Not in database: No access.
    read = false;
    write = false;
  } else {
    // Occurs in database: Read access.
    read = true;
    // Take write access from the database field.
    write = filter::strings::convert_to_bool (result [0]);
  }
}


// Returns a tuple with <read, write> whether the $username has access to the given $bible.
std::tuple <bool, bool> DatabasePrivileges::get_bible (const std::string& username, const std::string& bible)
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT write FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add (";");
  std::vector <std::string> result = sql.query () ["write"];
  const bool read = (!result.empty());
  sql.clear ();
  sql.add ("SELECT write FROM bibles WHERE username =");
  sql.add (username);
  sql.add ("AND bible =");
  sql.add (bible);
  sql.add ("AND write;");
  result = sql.query () ["write"];
  const bool write = (!result.empty());
  return std::make_tuple(read, write);
}


int DatabasePrivileges::get_bible_book_count ()
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT count(*) FROM bibles;");
  const std::vector <std::string> result = sql.query () ["count(*)"];
  if (result.empty ()) return 0;
  return filter::strings::convert_to_int (result [0]);
}


// Returns true if a record for $username / $bible / $book exists in the database.
// When the $book = 0, it takes any book.
bool DatabasePrivileges::get_bible_book_exists (const std::string& username, const std::string& bible, const int book)
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
  const std::vector <std::string> result = sql.query () ["rowid"];
  return !result.empty();
}


// Remove the privilege of a $username to have access to $bible $book.
// Removing the privilege for $book 0 removes them for all possible books.
void DatabasePrivileges::remove_bible_book (const std::string& username, const std::string& bible, const int book)
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
void DatabasePrivileges::remove_bible (const std::string& bible)
{
  SqliteDatabase sql (database ());
  sql.add ("DELETE FROM bibles WHERE bible =");
  sql.add (bible);
  sql.add (";");
  sql.execute ();
}


void DatabasePrivileges::set_feature (const std::string& username, const int feature, const bool enabled)
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


bool DatabasePrivileges::get_feature (const std::string& username, const int feature)
{
  SqliteDatabase sql (database ());
  sql.add ("SELECT rowid FROM features WHERE username =");
  sql.add (username);
  sql.add ("AND feature =");
  sql.add (feature);
  sql.add (";");
  const std::vector <std::string> result = sql.query () ["rowid"];
  if (result.empty()) return false;
  return true;
}


// Remove privileges for $username from the entire database.
void DatabasePrivileges::remove_user (const std::string& username)
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


const char * DatabasePrivileges::bibles_start ()
{
  return "bibles_start";
}


const char * DatabasePrivileges::bibles_end ()
{
  return "bibles_end";
}


const char * DatabasePrivileges::features_start ()
{
  return "features_start";
}


const char * DatabasePrivileges::features_end ()
{
  return "features_start";
}


const char * DatabasePrivileges::on ()
{
  return "on";
}


const char * DatabasePrivileges::off ()
{
  return "off";
}


std::string database_privileges_directory (const std::string& user)
{
  return filter_url_create_path ({database_logic_databases (), "clients", user});
}


std::string database_privileges_file ()
{
  return "privileges.txt";
}


std::string database_privileges_client_path (const std::string& user)
{
  return filter_url_create_root_path ({database_privileges_directory (user), database_privileges_file ()});
}


void database_privileges_client_create (const std::string& user, bool force)
{
  // The path to the file with privileges for the $user.
  std::string path = database_privileges_client_path (user);
  
  // Without $force, if the file exists, we're done.
  if (!force) {
    if (file_or_dir_exists (path)) return;
  }
  
  // If needed, create the folder.
  std::string folder = filter_url_dirname (path);
  if (!file_or_dir_exists (folder)) filter_url_mkdir (folder);
  
  // The bits of privileges in human-readable form.
  std::string privileges = DatabasePrivileges::save (user);
  
  // Write the privileges to disk.
  filter_url_file_put_contents (path, privileges);
}


void database_privileges_client_remove (const std::string& user)
{
  std::string path = database_privileges_client_path (user);
  path = filter_url_dirname (path);
  filter_url_rmdir (path);
}
