/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <database/privileges.h>
#include <database/sqlite.h>
#include <filter/url.h>
using namespace std;


TEST (database, privileges)
{
  // Test creation, automatic repair of damages.
  refresh_sandbox (false);
  DatabasePrivileges::create ();
  string path = database_sqlite_file (DatabasePrivileges::database ());
  filter_url_file_put_contents (path, "damaged database");
  EXPECT_EQ (false, DatabasePrivileges::healthy ());
  DatabasePrivileges::optimize ();
  EXPECT_EQ (true, DatabasePrivileges::healthy ());
  refresh_sandbox (false);
  
  DatabasePrivileges::create ();
  
  // Upgrade routine should not give errors.
  DatabasePrivileges::upgrade ();
  
  string username = "phpunit";
  string bible = "bible";
  
  // Initially there's no privileges for a Bible book.
  {
    bool read, write;
    DatabasePrivileges::get_bible_book (username, bible, 2, read, write);
    EXPECT_EQ (false, read);
    EXPECT_EQ (false, write);
  }
  
  // Set privileges and read them.
  {
    bool read, write;
    DatabasePrivileges::set_bible_book (username, bible, 3, false);
    DatabasePrivileges::get_bible_book (username, bible, 3, read, write);
    EXPECT_EQ (true, read);
    EXPECT_EQ (false, write);
  }
  
  {
    bool read, write;
    DatabasePrivileges::set_bible_book (username, bible, 4, true);
    DatabasePrivileges::get_bible_book (username, bible, 4, read, write);
    EXPECT_EQ (true, read);
    EXPECT_EQ (true, write);
  }

  {
    bool read, write;
    DatabasePrivileges::remove_bible_book (username, bible, 4);
    DatabasePrivileges::get_bible_book (username, bible, 4, read, write);
    EXPECT_EQ (false, read);
    EXPECT_EQ (false, write);
  }
  
  // Test Bible book entry count.
  {
    DatabasePrivileges::set_bible_book (username, bible, 6, true);
    int count = DatabasePrivileges::get_bible_book_count ();
    EXPECT_EQ (2, count);
  }
  
  // Test removing book zero, that it removes entries for all books.
  {
    DatabasePrivileges::remove_bible_book (username, bible, 0);
    int count = DatabasePrivileges::get_bible_book_count ();
    EXPECT_EQ (0, count);
  }
  
  // Enter a privilege for book = 1, and a different privilege for book 0,
  // and then test that the privilege for book 1 has preference.
  {
    bool read, write;
    DatabasePrivileges::set_bible_book (username, bible, 1, false);
    DatabasePrivileges::get_bible_book (username, bible, 1, read, write);
    EXPECT_EQ (true, read);
    EXPECT_EQ (false, write);
    DatabasePrivileges::set_bible_book (username, bible, 0, true);
    DatabasePrivileges::set_bible_book (username, bible, 1, false);
    DatabasePrivileges::get_bible_book (username, bible, 1, read, write);
    EXPECT_EQ (true, read);
    EXPECT_EQ (false, write);
  }
  
  // Start afresh to not depend too much on previous tests.
  refresh_sandbox (true);
  DatabasePrivileges::create ();
  
  // Test whether an entry for a book exists.
  {
    bool exists = DatabasePrivileges::get_bible_book_exists (username, bible, 0);
    EXPECT_EQ (false, exists);
    DatabasePrivileges::set_bible_book (username, bible, 1, false);
    // Test the record for the correct book.
    exists = DatabasePrivileges::get_bible_book_exists (username, bible, 1);
    EXPECT_EQ (true, exists);
    // The record should also exist for book 0.
    exists = DatabasePrivileges::get_bible_book_exists (username, bible, 0);
    EXPECT_EQ (true, exists);
    // The record should not exist for another book.
    exists = DatabasePrivileges::get_bible_book_exists (username, bible, 2);
    EXPECT_EQ (false, exists);
  }
  
  // Start afresh to not depend on the outcome of previous tests.
  refresh_sandbox (true);
  DatabasePrivileges::create ();
  
  // Test no read access to entire Bible.
  {
    auto [read, write] = DatabasePrivileges::get_bible (username, bible);
    EXPECT_EQ (false, read);
    EXPECT_EQ (false, write);
  }
  // Set Bible read-only and test it.
  DatabasePrivileges::set_bible (username, bible, false);
  {
    auto [read, write] = DatabasePrivileges::get_bible (username, bible);
    EXPECT_EQ (true, read);
    EXPECT_EQ (false, write);
  }
  // Set Bible read-write, and test it.
  DatabasePrivileges::set_bible (username, bible, true);
  {
    auto [read, write] = DatabasePrivileges::get_bible (username, bible);
    EXPECT_EQ (true, read);
    EXPECT_EQ (true, write);
  }
  // Set one book read-write and test that this applies to entire Bible.
  DatabasePrivileges::set_bible (username, bible, false);
  DatabasePrivileges::set_bible_book (username, bible, 1, true);
  {
    auto [read, write] = DatabasePrivileges::get_bible (username, bible);
    EXPECT_EQ (true, read);
    EXPECT_EQ (true, write);
  }
  
  // A feature is off by default.
  bool enabled = DatabasePrivileges::get_feature (username, 123);
  EXPECT_EQ (false, enabled);
  
  // Test setting a feature on and off.
  DatabasePrivileges::set_feature (username, 1234, true);
  enabled = DatabasePrivileges::get_feature (username, 1234);
  EXPECT_EQ (true, enabled);
  DatabasePrivileges::set_feature (username, 1234, false);
  enabled = DatabasePrivileges::get_feature (username, 1234);
  EXPECT_EQ (false, enabled);

  {
    // Test bulk privileges removal.
    refresh_sandbox (true);
    DatabasePrivileges::create ();
    // Set privileges for user for Bible.
    DatabasePrivileges::set_bible (username, bible, false);
    int count = DatabasePrivileges::get_bible_book_count ();
    EXPECT_EQ (1, count);
    // Remove privileges for a Bible and check on them.
    DatabasePrivileges::remove_bible (bible);
    count = DatabasePrivileges::get_bible_book_count ();
    EXPECT_EQ (0, count);
    // Again, set privileges, and remove them by username.
    DatabasePrivileges::set_bible (username, bible, false);
    count = DatabasePrivileges::get_bible_book_count ();
    EXPECT_EQ (1, count);
    DatabasePrivileges::remove_user (username);
    count = DatabasePrivileges::get_bible_book_count ();
    EXPECT_EQ (0, count);
  }
  
  {
    // Set features for user.
    DatabasePrivileges::set_feature (username, 1234, true);
    enabled = DatabasePrivileges::get_feature (username, 1234);
    EXPECT_EQ (true, enabled);
    // Remove features by username.
    DatabasePrivileges::remove_user (username);
    enabled = DatabasePrivileges::get_feature (username, 1234);
    EXPECT_EQ (false, enabled);
  }

  {
    // Test privileges transfer through a text file.
    refresh_sandbox (true);
    DatabasePrivileges::create ();
    // Set privileges.
    DatabasePrivileges::set_bible_book (username, bible, 1, true);
    DatabasePrivileges::set_feature (username, 1234, true);
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
    EXPECT_EQ (privileges, DatabasePrivileges::save (username));
    // Transfer the privileges to another user.
    string clientuser = username + "client";
    DatabasePrivileges::load (clientuser, privileges);
    // Check the privileges for that other user.
    auto [read, write] = DatabasePrivileges::get_bible (clientuser, bible);
    EXPECT_EQ (true, read);
    EXPECT_EQ (true, write);
    enabled = DatabasePrivileges::get_feature (username, 1234);
    EXPECT_EQ (true, enabled);
  }
}


#endif

