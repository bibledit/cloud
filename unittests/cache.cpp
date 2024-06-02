/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <database/cache.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <filter/string.h>


TEST (database, cache)
{
  refresh_sandbox (false);
  
  // Initially the database should not exist.
  bool exists = database::cache::sql::exists ("");
  EXPECT_EQ (false, exists);
  exists = database::cache::sql::exists ("unittests");
  EXPECT_EQ (false, exists);
  
  // Copy an old cache database in place.
  // It contains cached data in the old layout.
  // Test that it now exists and contains data.
  std::string testdatapath = filter_url_create_root_path ({"unittests", "tests", "cache_resource_test.sqlite"});
  std::string databasepath = filter_url_create_root_path ({"databases",  "cache_resource_unittests.sqlite"});
  std::string out_err;
  filter_shell_run ("cp " + testdatapath + " " + databasepath, out_err);
  int count = database::cache::sql::count ("unittests");
  EXPECT_EQ (1, count);
  exists = database::cache::sql::exists ("unittests", 8, 1, 16);
  EXPECT_EQ (true, exists);
  std::string value = database::cache::sql::retrieve ("unittests", 8, 1, 16);
  EXPECT_EQ ("And Ruth said, Entreat me not to leave you, or to return from following you; for wherever you go, I will go, and wherever you lodge, I will lodge; your people shall be my people, and your God my God.", value);
  
  // Now remove the (old) cache and verify that it no longer exists or contains data.
  database::cache::sql::remove ("unittests");
  exists = database::cache::sql::exists ("unittests");
  EXPECT_EQ (false, exists);
  count = database::cache::sql::count ("unittests");
  EXPECT_EQ (0, count);
  
  // Create a cache for one book.
  database::cache::sql::create ("unittests", 10);
  // It should exists for the correct book, but not for another book.
  exists = database::cache::sql::exists ("unittests", 10);
  EXPECT_EQ (true, exists);
  exists = database::cache::sql::exists ("unittests", 11);
  EXPECT_EQ (false, exists);
  // The cache should have one book.
  count = database::cache::sql::count ("unittests");
  EXPECT_EQ (1, count);
  
  // Cache and retrieve value.
  database::cache::sql::create ("unittests", 1);
  database::cache::sql::cache ("unittests", 1, 2, 3, "cached");
  value = database::cache::sql::retrieve ("unittests", 1, 2, 3);
  EXPECT_EQ ("cached", value);
  
  // Book count check.
  count = database::cache::sql::count ("unittests");
  EXPECT_EQ (2, count);
  
  // Cache does not exist for one passage, but does exist for the other passage.
  exists = database::cache::sql::exists ("unittests", 1, 2, 4);
  EXPECT_EQ (false, exists);
  exists = database::cache::sql::exists ("unittests", 1, 2, 3);
  EXPECT_EQ (true, exists);
  
  // Excercise book cache removal.
  database::cache::sql::remove ("unittests");
  exists = database::cache::sql::exists ("unittests", 1);
  EXPECT_EQ (false, exists);
  
  // Excercise the file-based cache.
  {
    std::string url = "https://netbible.org/bible/1/2/3";
    std::string contents = "Bible contents";
    EXPECT_EQ (false, database::cache::file::exists (url));
    EXPECT_EQ ("", database::cache::file::get (url));
    database::cache::file::put (url, contents);
    EXPECT_EQ (true, database::cache::file::exists (url));
    EXPECT_EQ (contents, database::cache::file::get (url));
    database::cache::file::trim (false);
  }
  
  // Excercise the ready-flag.
  {
    std::string bible = "ready";
    int book = 11;
    database::cache::sql::create (bible, book);
    
    bool ready = database::cache::sql::ready (bible, book);
    EXPECT_EQ (false, ready);
    
    database::cache::sql::ready (bible, book, false);
    ready = database::cache::sql::ready (bible, book);
    EXPECT_EQ (false, ready);
    
    database::cache::sql::ready (bible, book, true);
    ready = database::cache::sql::ready (bible, book);
    EXPECT_EQ (true, ready);
  }
  
  // Check the file size function.
  {
    std::string bible = "size";
    int book = 12;
    database::cache::sql::create (bible, book);
    
    int size = database::cache::sql::size (bible, book);
    if ((size < 10'000) || (size > 15'000)) {
      EXPECT_EQ ("between 3072 and 5120", std::to_string (size));
    }
    
    size = database::cache::sql::size (bible, book + 1);
    EXPECT_EQ (0, size);
  }
  
  // Check file naming for downloading a cache.
  {
    EXPECT_EQ ("cache_resource_", database::cache::sql::fragment ());
    EXPECT_EQ ("databases/cache_resource_download_23.sqlite", database::cache::sql::path ("download", 23));
  }
  
  // Remove some logs.
  refresh_sandbox (false);
}

#endif

