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


#include <unittests/resources.h>
#include <unittests/utilities.h>
#include <resource/external.h>
#include <database/usfmresources.h>
#include <database/imageresources.h>
#include <database/userresources.h>
#include <filter/url.h>
#include <filter/string.h>


void test_database_resources ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  vector <string> names = resource_external_names ();
  bool hit = false;
  for (auto & name : names) if (name == "Statenbijbel GBS") hit = true;
  evaluate (__LINE__, __func__, true, hit);
}


void test_database_usfmresources ()
{
  trace_unit_tests (__func__);
  
  // Empty
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    vector <string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    evaluate (__LINE__, __func__, false, hit);
  }
  // Names
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "");
    vector <string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    evaluate (__LINE__, __func__, true, hit);
  }
  // Store Get Chapter
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    string usfm = database_usfmresources.getUsfm ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, "usfm", usfm);
    usfm = database_usfmresources.getUsfm ("bibledit", 2, 4);
    evaluate (__LINE__, __func__, "", usfm);
  }
  // Books
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    database_usfmresources.storeChapter ("bibledit", 3, 4, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 1, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 2, "usfm");
    vector <int> books = database_usfmresources.getBooks ("bibledit");
    evaluate (__LINE__, __func__, {1, 2, 3}, books);
  }
  // Chapters
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    database_usfmresources.storeChapter ("bibledit", 3, 4, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 1, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 2, "usfm");
    vector <int> chapters = database_usfmresources.getChapters ("bibledit", 1);
    evaluate (__LINE__, __func__, {1, 2}, chapters);
    chapters = database_usfmresources.getChapters ("bibledit", 2);
    evaluate (__LINE__, __func__, {3}, chapters);
  }
  // Sizes
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    
    int size = database_usfmresources.getSize ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, 0, size);
    
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    size = database_usfmresources.getSize ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, 4, size);
    
    database_usfmresources.storeChapter ("bibledit", 2, 3, "chapter");
    size = database_usfmresources.getSize ("bibledit", 2, 3);
    evaluate (__LINE__, __func__, 7, size);
  }
  // Delete Book
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    vector <int> books = database_usfmresources.getBooks ("bibledit");
    evaluate (__LINE__, __func__, {2}, books);
    database_usfmresources.deleteBook ("bibledit", 2);
    books = database_usfmresources.getBooks ("bibledit");
    evaluate (__LINE__, __func__, {}, books);
  }
}


void test_database_imageresources ()
{
  trace_unit_tests (__func__);
  
  Database_ImageResources database_imageresources;
  string image = filter_url_create_root_path ({"unittests", "tests", "Genesis-1-1-18.gif"});
  
  // Empty
  {
    refresh_sandbox (true);
    vector <string> resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 0, resources.size());
  }
  
  // Create, names, erase.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    vector <string> resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 1, resources.size());
    bool hit = false;
    for (auto & resource : resources) if (resource == "unittest") hit = true;
    evaluate (__LINE__, __func__, true, hit);
    
    database_imageresources.erase ("none-existing");
    resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 1, resources.size());
    
    database_imageresources.erase ("unittest");
    resources = database_imageresources.names ();
    evaluate (__LINE__, __func__, 0, resources.size());
  }
  
  // Store, get, erase images.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    
    string path = "/tmp/unittest.jpg";
    filter_url_file_cp (image, path);
    database_imageresources.store ("unittest", path);
    filter_url_file_cp (image, path);
    database_imageresources.store ("unittest", path);
    filter_url_unlink (path);
    
    vector <string> images = database_imageresources.get ("unittest");
    evaluate (__LINE__, __func__, images, {"unittest.jpg", "unittest0.jpg"});
    
    database_imageresources.erase ("unittest", "unittest.jpg");
    
    images = database_imageresources.get ("unittest");
    evaluate (__LINE__, __func__, images, {"unittest0.jpg"});
  }
  // Assign passage and get image based on passage.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    
    for (int i = 10; i < 20; i++) {
      string image = "unittest" + convert_to_string (i) + ".jpg";
      string path = "/tmp/" + image;
      filter_url_file_cp (image, path);
      database_imageresources.store ("unittest", path);
      filter_url_unlink (path);
      database_imageresources.assign ("unittest", image, i, i, i, i, i, i+10);
    }
    
    vector <string> images = database_imageresources.get ("unittest", 11, 11, 13);
    evaluate (__LINE__, __func__, images, {"unittest11.jpg"});
    
    images = database_imageresources.get ("unittest", 11, 11, 100);
    evaluate (__LINE__, __func__, images, {});
  }
  // Assign passage to image, and retrieve it.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    
    string image = "unittest.jpg";
    string path = "/tmp/" + image;
    filter_url_file_cp (image, path);
    database_imageresources.store ("unittest", path);
    filter_url_unlink (path);
    database_imageresources.assign ("unittest", image, 1, 2, 0, 1, 2, 10);
    
    int book1, chapter1, verse1, book2, chapter2, verse2;
    database_imageresources.get ("unittest", "none-existing",
                                 book1, chapter1, verse1, book2, chapter2, verse2);
    evaluate (__LINE__, __func__, book1, 0);
    evaluate (__LINE__, __func__, chapter1, 0);
    
    database_imageresources.get ("unittest", image,
                                 book1, chapter1, verse1, book2, chapter2, verse2);
    evaluate (__LINE__, __func__, book1, 1);
    evaluate (__LINE__, __func__, chapter1, 2);
    evaluate (__LINE__, __func__, verse1, 0);
    evaluate (__LINE__, __func__, book2, 1);
    evaluate (__LINE__, __func__, chapter2, 2);
    evaluate (__LINE__, __func__, verse2, 10);
  }
}


void test_database_userresources ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  
  vector <string> names;
  string name = "unit//test";
  string url = "https://website.org/resources/<book>/<chapter>/<verse>.html";
  int book = 99;
  string abbrev = "Book 99";
  string fragment;
  string value;
  vector <string> specialnames = { "abc\\def:ghi", name };
  
  names = Database_UserResources::names ();
  evaluate (__LINE__, __func__, {}, names);
  
  Database_UserResources::url (name, url);
  value = Database_UserResources::url (name);
  evaluate (__LINE__, __func__, url, value);
  
  for (auto name : specialnames) {
    Database_UserResources::url (name, name + url);
  }
  names = Database_UserResources::names ();
  evaluate (__LINE__, __func__, specialnames, names);
  
  for (auto name : specialnames) {
    Database_UserResources::remove (name);
  }
  names = Database_UserResources::names ();
  evaluate (__LINE__, __func__, {}, names);
  
  Database_UserResources::book (name, book, abbrev);
  fragment = Database_UserResources::book (name, book);
  evaluate (__LINE__, __func__, abbrev, fragment);
  
  fragment = Database_UserResources::book (name + "x", book);
  evaluate (__LINE__, __func__, "", fragment);
  
  fragment = Database_UserResources::book (name, book + 1);
  evaluate (__LINE__, __func__, "", fragment);
}
