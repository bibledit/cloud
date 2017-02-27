/*
Copyright (©) 2003-2016 Teus Benschop.

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
