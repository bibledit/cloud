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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <resource/external.h>
#include <database/usfmresources.h>
#include <database/userresources.h>
#include <filter/url.h>
#include <filter/string.h>


TEST (database, resources)
{
  refresh_sandbox (false);
  std::vector <std::string> names = resource_external_names ();
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
    refresh_sandbox (false);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    std::vector <std::string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    EXPECT_EQ (false, hit);
  }
  // Names
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "");
    std::vector <std::string> resources = database_usfmresources.getResources ();
    bool hit = false;
    for (auto & resource : resources) if (resource == "bibledit") hit = true;
    EXPECT_EQ (true, hit);
  }
  // Store Get Chapter
  {
    refresh_sandbox (true);
    Database_UsfmResources database_usfmresources = Database_UsfmResources ();
    database_usfmresources.storeChapter ("bibledit", 2, 3, "usfm");
    std::string usfm = database_usfmresources.getUsfm ("bibledit", 2, 3);
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
    const std::vector <int> books = database_usfmresources.getBooks ("bibledit");
    const std::vector <int> standard_books {1, 2, 3};
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
    std::vector <int> chapters = database_usfmresources.getChapters ("bibledit", 1);
    const std::vector <int> standard_chapters {1, 2};
    EXPECT_EQ (standard_chapters, chapters);
    chapters = database_usfmresources.getChapters ("bibledit", 2);
    EXPECT_EQ (std::vector <int>{3}, chapters);
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
    std::vector <int> books = database_usfmresources.getBooks ("bibledit");
    EXPECT_EQ (std::vector <int>{2}, books);
    database_usfmresources.deleteBook ("bibledit", 2);
    books = database_usfmresources.getBooks ("bibledit");
    EXPECT_EQ (std::vector <int>{}, books);
  }
}


TEST (database, userresources)
{
  refresh_sandbox (false);
  
  std::vector <std::string> names;
  std::string name = "unit//test";
  std::string url = "https://website.org/resources/<book>/<chapter>/<verse>.html";
  int book = 99;
  std::string abbrev = "Book 99";
  std::string fragment;
  std::string value;
  std::vector <std::string> specialnames = { "abc\\def:ghi", name };
  
  names = Database_UserResources::names ();
  EXPECT_EQ (std::vector <std::string>{}, names);
  
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
  EXPECT_EQ (std::vector <std::string>{}, names);
  
  Database_UserResources::book (name, book, abbrev);
  fragment = Database_UserResources::book (name, book);
  EXPECT_EQ (abbrev, fragment);
  
  fragment = Database_UserResources::book (name + "x", book);
  EXPECT_EQ (std::string(), fragment);
  
  fragment = Database_UserResources::book (name, book + 1);
  EXPECT_EQ (std::string(), fragment);
}

#endif

