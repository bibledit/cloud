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


#include <unittests/versification.h>
#include <unittests/utilities.h>
#include <database/versifications.h>
#include <database/check.h>
#include <filter/string.h>
#include <checks/versification.h>
#include <database/mappings.h>


void test_versification ()
{
  trace_unit_tests (__func__);
  
  // Test the available books.
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    Database_Check database_check;
    database_check.create ();
    vector <int> books = database_versifications.getBooks (english ());
    vector <int> fault = filter_string_array_diff (books, {10});
    Checks_Versification::books ("Bible", fault);
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)results.size());
    if (results.size ()) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 10, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 1, hit.verse);
      evaluate (__LINE__, __func__, "This book is absent from the Bible", hit.data);
    }
  }
  
  // Test the available chapters.
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    Database_Check database_check;
    database_check.create ();

    // Ruth.
    int book = 8;
    // One missing and one extra chapter.
    vector <int> chapters = {0, 2, 3, 4, 5};
    // Run check.
    Checks_Versification::chapters ("Bible", book, chapters);

    // Verify results.
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)results.size());
    if (results.size () == 2) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 1, hit.verse);
      evaluate (__LINE__, __func__, "This chapter is missing", hit.data);
      hit = results[1];
      evaluate (__LINE__, __func__, 2, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 5, hit.chapter);
      evaluate (__LINE__, __func__, 1, hit.verse);
      evaluate (__LINE__, __func__, "This chapter is extra", hit.data);
    }
  }

  // Test available verses.
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    Database_Check database_check;
    database_check.create ();

    // Ruth 1.
    int book = 8;
    int chapter = 1;
    // One missing and one extra verse.
    vector <int> verses = {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24};

    // Run check.
    Checks_Versification::verses ("Bible", book, chapter, verses);

    // Verify results.
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 4, (int)results.size());
    if (results.size () == 4) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 1, hit.verse);
      evaluate (__LINE__, __func__, "This verse is missing according to the versification system", hit.data);
      hit = results[1];
      evaluate (__LINE__, __func__, 2, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 24, hit.verse);
      evaluate (__LINE__, __func__, "This verse is extra according to the versification system", hit.data);
      hit = results[2];
      evaluate (__LINE__, __func__, 3, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 2, hit.verse);
      evaluate (__LINE__, __func__, "The verse is out of sequence", hit.data);
      hit = results[3];
      evaluate (__LINE__, __func__, 4, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 24, hit.verse);
      evaluate (__LINE__, __func__, "The verse is out of sequence", hit.data);
    }
  }

  // Test the available verses.
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    Database_Check database_check;
    database_check.create ();
    
    // Ruth 1.
    int book = 8;
    int chapter = 1;

    // Verses out of order.
    vector <int> verses = {0, 2, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
    
    // Run check.
    Checks_Versification::verses ("Bible", book, chapter, verses);
    
    // Verify results.
    vector <Database_Check_Hit> results = database_check.getHits ();
    evaluate (__LINE__, __func__, 3, (int)results.size());
    if (results.size () == 3) {
      Database_Check_Hit hit = results[0];
      evaluate (__LINE__, __func__, 1, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 2, hit.verse);
      evaluate (__LINE__, __func__, "The verse is out of sequence", hit.data);
      hit = results[1];
      evaluate (__LINE__, __func__, 2, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 1, hit.verse);
      evaluate (__LINE__, __func__, "The verse is out of sequence", hit.data);
      hit = results[2];
      evaluate (__LINE__, __func__, 3, hit.rowid);
      evaluate (__LINE__, __func__, "Bible", hit.bible);
      evaluate (__LINE__, __func__, 8, hit.book);
      evaluate (__LINE__, __func__, 1, hit.chapter);
      evaluate (__LINE__, __func__, 3, hit.verse);
      evaluate (__LINE__, __func__, "The verse is out of sequence", hit.data);
    }
  }
}


void test_database_mappings ()
{
  trace_unit_tests (__func__);
  
  // Setup.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    database_mappings.defaults ();
    database_mappings.create2 ();
    database_mappings.optimize ();
    vector <string> names = database_mappings.names ();
    evaluate (__LINE__, __func__, {"Dutch Traditional", english (), "French Louise Segond", "Hebrew Greek", "Russian Canonical", "Russian Orthodox", "Russian Protestant", "Spanish", "Vulgate"}, names);
  }

  // Import and export.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
    "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("phpunit", import);
    vector <string> names = database_mappings.names ();
    evaluate (__LINE__, __func__, {"Hebrew Greek", "phpunit"}, names);
    string output = database_mappings.output ("phpunit");
    evaluate (__LINE__, __func__, filter_string_trim (import), filter_string_trim (output));
  }
  
  // Create.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    database_mappings.create ("phpunit");
    vector <string> names = database_mappings.names ();
    evaluate (__LINE__, __func__, {"Hebrew Greek", "phpunit"}, names);
  }
  
  // Translate the same.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    vector <Passage> passages = database_mappings.translate ("ABC", "ABC", 14, 14, 15);
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "15");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    passages = database_mappings.translate ("--X", "--X", 15, 16, 17);
    standard = Passage ("", 15, 16, "17");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }
  
  // Translate.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
    "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("ABC", import);
    import =
    "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("XYZ", import);
    // Test mapping 2 Chronicles.
    vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    Passage standard = Passage ("", 14, 14, "15");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }

  // Translate.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
    "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("ABC", import);
    import =
    "2 Chronicles 14:13 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("XYZ", import);
    // Test mapping 2 Chronicles.
    vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    Passage standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }

  // Translate double result.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
    "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("ABC", import);
    import =
    "2 Chronicles 14:12 = 2 Chronicles 14:14\n"
    "2 Chronicles 14:13 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("XYZ", import);
    // Test mapping 2 Chronicles.
    vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    evaluate (__LINE__, __func__, 2, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "12");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, true, passages[1].equal (standard));
  }

  // Translate from original.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("VVV", import);
    vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 14, 14);
    Passage standard = Passage ("", 14, 14, "12");
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }

  // Translate From Original Double
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
    "2 Chronicles 14:12 = 2 Chronicles 14:14\n"
    "2 Chronicles 14:13 = 2 Chronicles 14:14\n";
    database_mappings.import ("VVV", import);
    vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 14, 14);
    evaluate (__LINE__, __func__, 2, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "12");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, true, passages[1].equal (standard));
  }

  // Translate From Original No Mapping
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("VVV", import);
    vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 15, 14);
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    Passage standard = Passage ("", 14, 15, "14");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }

  // Translate To Original
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("ABA", import);
    vector <Passage> passages = database_mappings.translate ("ABA", "Hebrew Greek", 14, 14, 12);
    evaluate (__LINE__, __func__, 1, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "14");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
  }

  // Translate To Original Double
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    string import =
    "2 Chronicles 14:12 = 2 Chronicles 14:13\n"
    "2 Chronicles 14:12 = 2 Chronicles 14:14\n";
    database_mappings.import ("ABA", import);
    vector <Passage> passages = database_mappings.translate ("ABA", "Hebrew Greek", 14, 14, 12);
    evaluate (__LINE__, __func__, 2, (int)passages.size ());
    Passage standard = Passage ("", 14, 14, "13");
    evaluate (__LINE__, __func__, true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "14");
    evaluate (__LINE__, __func__, true, passages[1].equal (standard));
  }
  
}


void test_database_versifications ()
{
  trace_unit_tests (__func__);
  
  // Basic operations, create, delete.
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.optimize ();
    int id = database_versifications.createSystem ("phpunit");
    evaluate (__LINE__, __func__, 1000, id);
    id = database_versifications.getID ("phpunit");
    evaluate (__LINE__, __func__, 1000, id);
    vector <string> systems = database_versifications.getSystems ();
    evaluate (__LINE__, __func__, {"phpunit"}, systems);
    database_versifications.erase ("phpunit");
    systems = database_versifications.getSystems ();
    evaluate (__LINE__, __func__, {}, systems);
  }
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    
    // GetID
    int id = database_versifications.getID (english ());
    evaluate (__LINE__, __func__, 4 , id);
    
    // Test books.
    vector <int> books = database_versifications.getBooks (english ());
    vector <int> standard;
    for (int i = 1; i <= 66; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, books);
    
    // Test chapters.
    vector <int> chapters = database_versifications.getChapters (english (), 1);
    standard.clear ();
    for (int i = 1; i <= 50; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, chapters);
    chapters = database_versifications.getChapters (english (), 1, true);
    standard.clear ();
    for (int i = 0; i <= 50; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, chapters);
    
    // Test verses.
    vector <int> verses = database_versifications.getVerses (english (), 1, 2);
    standard.clear ();
    for (int i = 0; i <= 25; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, verses);
    
    // Verses in chapter 0.
    verses = database_versifications.getVerses (english (), 1, 0);
    evaluate (__LINE__, __func__, {0}, verses);
    
    // Books Chapters Verses.
    vector <Passage> data = database_versifications.getBooksChaptersVerses (english ());
    evaluate (__LINE__, __func__, 1189, (int)data.size());
    evaluate (__LINE__, __func__, "31", data [0].m_verse);
    
    // Maximum number of books.
    books = database_versifications.getMaximumBooks ();
    standard = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 70, 71, 73, 74, 75, 80, 81 };
    evaluate (__LINE__, __func__, standard, books);
    
    // Maximum number of chapters.
    chapters = database_versifications.getMaximumChapters (5);
    standard.clear ();
    for (int i = 0; i <= 34; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, chapters);
    
    // Maximum number of verses.
    verses = database_versifications.getMaximumVerses (1, 2);
    standard.clear ();
    for (int i = 0; i <= 25; i++) standard.push_back (i);
    evaluate (__LINE__, __func__, standard, verses);
  }
  // Import Export
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    string input =
    "Genesis 1:31\n"
    "Genesis 2:25\n";
    database_versifications.input (input, "phpunit");
    int id = database_versifications.getID ("phpunit");
    evaluate (__LINE__, __func__, 1000, id);
    vector <Passage> data = database_versifications.getBooksChaptersVerses ("phpunit");
    evaluate (__LINE__, __func__, 2, (int)data.size ());
    evaluate (__LINE__, __func__, "25", data [1].m_verse);
    string output = database_versifications.output ("phpunit");
    evaluate (__LINE__, __func__, filter_string_trim (input), filter_string_trim (output));
  }
}
