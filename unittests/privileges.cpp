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


#include <unittests/privileges.h>
#include <unittests/utilities.h>
#include <database/privileges.h>
#include <database/sqlite.h>
#include <filter/url.h>


void test_database_privileges ()
{
  trace_unit_tests (__func__);
  
  // Test creation, automatic repair of damages.
  refresh_sandbox (true);
  Database_Privileges::create ();
  string path = database_sqlite_file (Database_Privileges::database ());
  filter_url_file_put_contents (path, "damaged database");
  evaluate (__LINE__, __func__, false, Database_Privileges::healthy ());
  Database_Privileges::optimize ();
  evaluate (__LINE__, __func__, true, Database_Privileges::healthy ());
  refresh_sandbox (false);
  
  Database_Privileges::create ();
  
  // Upgrade routine should not give errors.
  Database_Privileges::upgrade ();
  
  string username = "phpunit";
  string bible = "bible";
  
  // Initially there's no privileges for a Bible book.
  bool read;
  bool write;
  Database_Privileges::getBibleBook (username, bible, 2, read, write);
  evaluate (__LINE__, __func__, false, read);
  evaluate (__LINE__, __func__, false, write);
  
  // Set privileges and read them.
  Database_Privileges::setBibleBook (username, bible, 3, false);
  Database_Privileges::getBibleBook (username, bible, 3, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);
  
  Database_Privileges::setBibleBook (username, bible, 4, true);
  Database_Privileges::getBibleBook (username, bible, 4, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  
  Database_Privileges::removeBibleBook (username, bible, 4);
  Database_Privileges::getBibleBook (username, bible, 4, read, write);
  evaluate (__LINE__, __func__, false, read);
  evaluate (__LINE__, __func__, false, write);
  
  // Test Bible book entry count.
  Database_Privileges::setBibleBook (username, bible, 6, true);
  int count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 2, count);
  
  // Test removing book zero, that it removes entries for all books.
  Database_Privileges::removeBibleBook (username, bible, 0);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 0, count);
  
  // Enter a privilege for book = 1, and a different privilege for book 0,
  // and then test that the privilege for book 1 has preference.
  Database_Privileges::setBibleBook (username, bible, 1, false);
  Database_Privileges::getBibleBook (username, bible, 1, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);
  Database_Privileges::setBibleBook (username, bible, 0, true);
  Database_Privileges::setBibleBook (username, bible, 1, false);
  Database_Privileges::getBibleBook (username, bible, 1, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);
  
  // Start afresh to not depend too much on previous tests.
  refresh_sandbox (true);
  Database_Privileges::create ();
  
  // Test whether an entry for a book exists.
  bool exists = Database_Privileges::getBibleBookExists (username, bible, 0);
  evaluate (__LINE__, __func__, false, exists);
  Database_Privileges::setBibleBook (username, bible, 1, false);
  // Test the record for the correct book.
  exists = Database_Privileges::getBibleBookExists (username, bible, 1);
  evaluate (__LINE__, __func__, true, exists);
  // The record should also exist for book 0.
  exists = Database_Privileges::getBibleBookExists (username, bible, 0);
  evaluate (__LINE__, __func__, true, exists);
  // The record should not exist for another book.
  exists = Database_Privileges::getBibleBookExists (username, bible, 2);
  evaluate (__LINE__, __func__, false, exists);
  
  // Start afresh to not depend on the outcome of previous tests.
  refresh_sandbox (true);
  Database_Privileges::create ();
  
  // Test no read access to entire Bible.
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, false, read);
  evaluate (__LINE__, __func__, false, write);
  // Set Bible read-only and test it.
  Database_Privileges::setBible (username, bible, false);
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, false, write);
  // Set Bible read-write, and test it.
  Database_Privileges::setBible (username, bible, true);
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  // Set one book read-write and test that this applies to entire Bible.
  Database_Privileges::setBible (username, bible, false);
  Database_Privileges::setBibleBook (username, bible, 1, true);
  Database_Privileges::getBible (username, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  
  // A feature is off by default.
  bool enabled = Database_Privileges::getFeature (username, 123);
  evaluate (__LINE__, __func__, false, enabled);
  
  // Test setting a feature on and off.
  Database_Privileges::setFeature (username, 1234, true);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, true, enabled);
  Database_Privileges::setFeature (username, 1234, false);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, false, enabled);
  
  // Test bulk privileges removal.
  refresh_sandbox (true);
  Database_Privileges::create ();
  // Set privileges for user for Bible.
  Database_Privileges::setBible (username, bible, false);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 1, count);
  // Remove privileges for a Bible and check on them.
  Database_Privileges::removeBible (bible);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 0, count);
  // Again, set privileges, and remove them by username.
  Database_Privileges::setBible (username, bible, false);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 1, count);
  Database_Privileges::removeUser (username);
  count = Database_Privileges::getBibleBookCount ();
  evaluate (__LINE__, __func__, 0, count);
  
  // Set features for user.
  Database_Privileges::setFeature (username, 1234, true);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, true, enabled);
  // Remove features by username.
  Database_Privileges::removeUser (username);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, false, enabled);
  
  // Test privileges transfer through a text file.
  refresh_sandbox (true);
  Database_Privileges::create ();
  // Set privileges.
  Database_Privileges::setBibleBook (username, bible, 1, true);
  Database_Privileges::setFeature (username, 1234, true);
  // Check the transfer text file.
  string privileges =
  "bibles_start\n"
  "bible\n"
  "1\n"
  "on\n"
  "bibles_end\n"
  "features_start\n"
  "1234\n"
  "features_start";
  evaluate (__LINE__, __func__, privileges, Database_Privileges::save (username));
  // Transfer the privileges to another user.
  string clientuser = username + "client";
  Database_Privileges::load (clientuser, privileges);
  // Check the privileges for that other user.
  Database_Privileges::getBible (clientuser, bible, read, write);
  evaluate (__LINE__, __func__, true, read);
  evaluate (__LINE__, __func__, true, write);
  enabled = Database_Privileges::getFeature (username, 1234);
  evaluate (__LINE__, __func__, true, enabled);
}


