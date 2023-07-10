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
#include <resource/external.h>
#include <database/usfmresources.h>
#include <database/imageresources.h>
#include <database/userresources.h>
#include <filter/url.h>
#include <filter/string.h>
using namespace std;


TEST (database, resources)
{
  refresh_sandbox (true);
  vector <string> names = resource_external_names ();
  bool hit = false;
  for (const auto& name : names)
    if (name == "Statenbijbel GBS")
      hit = true;
  EXPECT_EQ (true, hit);
}


TEST (database, usfmresources)
{
  // Empty
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    vector <string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    EXPECT_EQ (false, hit);
  }
  // Names
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "");
    vector <string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    EXPECT_EQ (true, hit);
  }
  // Store Get Chapter
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    string usfm = database_usfmresources.getUsfm ("bibledit", 2, 3);
    EXPECT_EQ ("usfm", usfm);
    usfm = database_usfmresources.getUsfm ("bibledit", 2, 4);
    EXPECT_EQ ("", usfm);
  }
  // Books
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    database_usfmresources.storeChapter ("bibledit", 3, 4, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 1, "usfm");
    database_usfmresources.storeChapter ("bibledit", 1, 2, "usfm");
    const vector <int> books = database_usfmresources.getBooks ("bibledit");
    const vector <int> standard_books {1, 2, 3};
    EXPECT_EQ (standard_books, books);
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
    const vector <int> standard_chapters {1, 2};
    EXPECT_EQ (standard_chapters, chapters);
    chapters = database_usfmresources.getChapters ("bibledit", 2);
    EXPECT_EQ (vector <int>{3}, chapters);
  }
  // Sizes
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    
    int size = database_usfmresources.getSize ("bibledit", 2, 3);
    EXPECT_EQ (0, size);
    
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    size = database_usfmresources.getSize ("bibledit", 2, 3);
    EXPECT_EQ (4, size);
    
    database_usfmresources.storeChapter ("bibledit", 2, 3, "chapter");
    size = database_usfmresources.getSize ("bibledit", 2, 3);
    EXPECT_EQ (7, size);
  }
  // Delete Book
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    vector <int> books = database_usfmresources.getBooks ("bibledit");
    EXPECT_EQ (vector <int>{2}, books);
    database_usfmresources.deleteBook ("bibledit", 2);
    books = database_usfmresources.getBooks ("bibledit");
    EXPECT_EQ (vector <int>{}, books);
  }
}


TEST (database, imageresources)
{
  Database_ImageResources database_imageresources;
  const string image = filter_url_create_root_path ({"unittests", "tests", "Genesis-1-1-18.gif"});
  
  // Empty.
  {
    refresh_sandbox (true);
    vector <string> resources = database_imageresources.names ();
    EXPECT_EQ (0, resources.size());
  }
  
  // Create, names, erase.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    vector <string> resources = database_imageresources.names ();
    EXPECT_EQ (1, resources.size());
    bool hit = false;
    for (auto & resource : resources) if (resource == "unittest") hit = true;
    EXPECT_EQ (true, hit);
    
    database_imageresources.erase ("none-existing");
    resources = database_imageresources.names ();
    EXPECT_EQ (1, resources.size());
    
    database_imageresources.erase ("unittest");
    resources = database_imageresources.names ();
    EXPECT_EQ (0, resources.size());
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
    const vector <string> standard_images {"unittest.jpg", "unittest0.jpg"};
    EXPECT_EQ (standard_images, images);
    
    database_imageresources.erase ("unittest", "unittest.jpg");
    
    images = database_imageresources.get ("unittest");
    EXPECT_EQ (vector <string>{"unittest0.jpg"}, images);
  }
  // Assign passage and get image based on passage.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    
    for (int i = 10; i < 20; i++) {
      string image_name = "unittest" + filter::strings::convert_to_string (i) + ".jpg";
      string image_path = "/tmp/" + image_name;
      filter_url_file_cp (image_name, image_path);
      database_imageresources.store ("unittest", image_path);
      filter_url_unlink (image_path);
      database_imageresources.assign ("unittest", image_name, i, i, i, i, i, i+10);
    }
    
    vector <string> images = database_imageresources.get ("unittest", 11, 11, 13);
    EXPECT_EQ (images, vector <string>{"unittest11.jpg"});
    
    images = database_imageresources.get ("unittest", 11, 11, 100);
    EXPECT_EQ (images, vector <string>{});
  }
  // Assign passage to image, and retrieve it.
  {
    refresh_sandbox (true);
    
    database_imageresources.create ("unittest");
    
    string image_name = "unittest.jpg";
    string path_path = "/tmp/" + image_name;
    filter_url_file_cp (image_name, path_path);
    database_imageresources.store ("unittest", path_path);
    filter_url_unlink (path_path);
    database_imageresources.assign ("unittest", image_name, 1, 2, 0, 1, 2, 10);
    
    int book1, chapter1, verse1, book2, chapter2, verse2;
    database_imageresources.get ("unittest", "none-existing",
                                 book1, chapter1, verse1, book2, chapter2, verse2);
    EXPECT_EQ (book1, 0);
    EXPECT_EQ (chapter1, 0);
    
    database_imageresources.get ("unittest", image_name,
                                 book1, chapter1, verse1, book2, chapter2, verse2);
    EXPECT_EQ (book1, 1);
    EXPECT_EQ (chapter1, 2);
    EXPECT_EQ (verse1, 0);
    EXPECT_EQ (book2, 1);
    EXPECT_EQ (chapter2, 2);
    EXPECT_EQ (verse2, 10);
  }
}


TEST (database, userresources)
{
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
  EXPECT_EQ (vector <string>{}, names);
  
  Database_UserResources::url (name, url);
  value = Database_UserResources::url (name);
  EXPECT_EQ (url, value);
  
  for (auto special_name : specialnames) {
    Database_UserResources::url (special_name, special_name + url);
  }
  names = Database_UserResources::names ();
  EXPECT_EQ (specialnames, names);
  
  for (auto special_name : specialnames) {
    Database_UserResources::remove (special_name);
  }
  names = Database_UserResources::names ();
  EXPECT_EQ (vector <string>{}, names);
  
  Database_UserResources::book (name, book, abbrev);
  fragment = Database_UserResources::book (name, book);
  EXPECT_EQ (abbrev, fragment);
  
  fragment = Database_UserResources::book (name + "x", book);
  EXPECT_EQ (std::string(), fragment);
  
  fragment = Database_UserResources::book (name, book + 1);
  EXPECT_EQ (std::string(), fragment);
}

#endif

