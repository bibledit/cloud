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


#include <unittests/cache.h>
#include <unittests/utilities.h>
#include <database/cache.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <filter/string.h>


void test_database_cache ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  
  // Initially the database should not exist.
  bool exists = Database_Cache::exists ("");
  evaluate (__LINE__, __func__, false, exists);
  exists = Database_Cache::exists ("unittests");
  evaluate (__LINE__, __func__, false, exists);
  
  // Copy an old cache database in place.
  // It contains cached data in the old layout.
  // Test that it now exists and contains data.
  string testdatapath = filter_url_create_root_path ("unittests", "tests", "cache_resource_test.sqlite");
  string databasepath = filter_url_create_root_path ("databases",  "cache_resource_unittests.sqlite");
  string out_err;
  filter_shell_run ("cp " + testdatapath + " " + databasepath, out_err);
  size_t count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 1, count);
  exists = Database_Cache::exists ("unittests", 8, 1, 16);
  evaluate (__LINE__, __func__, true, exists);
  string value = Database_Cache::retrieve ("unittests", 8, 1, 16);
  evaluate (__LINE__, __func__, "And Ruth said, Entreat me not to leave you, or to return from following you; for wherever you go, I will go, and wherever you lodge, I will lodge; your people shall be my people, and your God my God.", value);
  
  // Now remove the (old) cache and verify that it no longer exists or contains data.
  Database_Cache::remove ("unittests");
  exists = Database_Cache::exists ("unittests");
  evaluate (__LINE__, __func__, false, exists);
  count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 0, count);
  
  // Create a cache for one book.
  Database_Cache::create ("unittests", 10);
  // It should exists for the correct book, but not for another book.
  exists = Database_Cache::exists ("unittests", 10);
  evaluate (__LINE__, __func__, true, exists);
  exists = Database_Cache::exists ("unittests", 11);
  evaluate (__LINE__, __func__, false, exists);
  // The cache should have one book.
  count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 1, count);
  
  // Cache and retrieve value.
  Database_Cache::create ("unittests", 1);
  Database_Cache::cache ("unittests", 1, 2, 3, "cached");
  value = Database_Cache::retrieve ("unittests", 1, 2, 3);
  evaluate (__LINE__, __func__, "cached", value);
  
  // Book count check.
  count = Database_Cache::count ("unittests");
  evaluate (__LINE__, __func__, 2, count);
  
  // Cache does not exist for one passage, but does exist for the other passage.
  exists = Database_Cache::exists ("unittests", 1, 2, 4);
  evaluate (__LINE__, __func__, false, exists);
  exists = Database_Cache::exists ("unittests", 1, 2, 3);
  evaluate (__LINE__, __func__, true, exists);
  
  // Excercise book cache removal.
  Database_Cache::remove ("unittests");
  exists = Database_Cache::exists ("unittests", 1);
  evaluate (__LINE__, __func__, false, exists);
  
  // Excercise the file-based cache.
  {
    string url = "https://netbible.org/bible/1/2/3";
    string contents = "Bible contents";
    evaluate (__LINE__, __func__, false, database_filebased_cache_exists (url));
    evaluate (__LINE__, __func__, "", database_filebased_cache_get (url));
    database_filebased_cache_put (url, contents);
    evaluate (__LINE__, __func__, true, database_filebased_cache_exists (url));
    evaluate (__LINE__, __func__, contents, database_filebased_cache_get (url));
    database_cache_trim (false);
  }
  
  // Excercise the ready-flag.
  {
    string bible = "ready";
    int book = 11;
    Database_Cache::create (bible, book);
    
    bool ready = Database_Cache::ready (bible, book);
    evaluate (__LINE__, __func__, false, ready);
    
    Database_Cache::ready (bible, book, false);
    ready = Database_Cache::ready (bible, book);
    evaluate (__LINE__, __func__, false, ready);
    
    Database_Cache::ready (bible, book, true);
    ready = Database_Cache::ready (bible, book);
    evaluate (__LINE__, __func__, true, ready);
  }
  
  // Check the file size function.
  {
    string bible = "size";
    int book = 12;
    Database_Cache::create (bible, book);
    
    int size = Database_Cache::size (bible, book);
    if ((size < 10000) || (size > 15000)) {
      evaluate (__LINE__, __func__, "between 3072 and 5120", convert_to_string (size));
    }
    
    size = Database_Cache::size (bible, book + 1);
    evaluate (__LINE__, __func__, 0, size);
  }
  
  // Check file naming for downloading a cache.
  {
    evaluate (__LINE__, __func__, "cache_resource_", Database_Cache::fragment ());
    evaluate (__LINE__, __func__, "databases/cache_resource_download_23.sqlite", Database_Cache::path ("download", 23));
  }
  
  refresh_sandbox (true);
}
