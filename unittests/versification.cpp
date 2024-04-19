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
#include <database/versifications.h>
#include <database/check.h>
#include <filter/string.h>
#include <checks/versification.h>
#include <database/mappings.h>
using namespace std;


TEST (versification, basic )
{
  
  // Test the available books.
  {
    refresh_sandbox (false);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    Database_Check database_check;
    database_check.create ();
    std::vector <int> books = database_versifications.getBooks (filter::strings::english ());
    std::vector <int> fault = filter::strings::array_diff (books, {10});
    checks_versification::books ("Bible", fault);
    std::vector <Database_Check_Hit> results = database_check.getHits ();
    EXPECT_EQ (1, static_cast<int>(results.size()));
    if (results.size ()) {
      Database_Check_Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (10, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (1, hit.verse);
      EXPECT_EQ ("This book is absent from the Bible", hit.data);
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
    std::vector <int> chapters = {0, 2, 3, 4, 5};
    // Run check.
    checks_versification::chapters ("Bible", book, chapters);

    // Verify results.
    std::vector <Database_Check_Hit> results = database_check.getHits ();
    EXPECT_EQ (2, static_cast<int>(results.size()));
    if (results.size () == 2) {
      Database_Check_Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (1, hit.verse);
      EXPECT_EQ ("This chapter is missing", hit.data);
      hit = results[1];
      EXPECT_EQ (2, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (5, hit.chapter);
      EXPECT_EQ (1, hit.verse);
      EXPECT_EQ ("This chapter is extra", hit.data);
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
    std::vector <int> verses = {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24};

    // Run check.
    checks_versification::verses ("Bible", book, chapter, verses);

    // Verify results.
    std::vector <Database_Check_Hit> results = database_check.getHits ();
    EXPECT_EQ (4, static_cast<int>(results.size()));
    if (results.size () == 4) {
      Database_Check_Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (1, hit.verse);
      EXPECT_EQ ("This verse is missing according to the versification system", hit.data);
      hit = results[1];
      EXPECT_EQ (2, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (24, hit.verse);
      EXPECT_EQ ("This verse is extra according to the versification system", hit.data);
      hit = results[2];
      EXPECT_EQ (3, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ ("The verse is out of sequence", hit.data);
      hit = results[3];
      EXPECT_EQ (4, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (24, hit.verse);
      EXPECT_EQ ("The verse is out of sequence", hit.data);
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
    std::vector <int> verses = {0, 2, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
    
    // Run check.
    checks_versification::verses ("Bible", book, chapter, verses);
    
    // Verify results.
    std::vector <Database_Check_Hit> results = database_check.getHits ();
    EXPECT_EQ (3, static_cast<int>(results.size()));
    if (results.size () == 3) {
      Database_Check_Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ ("The verse is out of sequence", hit.data);
      hit = results[1];
      EXPECT_EQ (2, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (1, hit.verse);
      EXPECT_EQ ("The verse is out of sequence", hit.data);
      hit = results[2];
      EXPECT_EQ (3, hit.rowid);
      EXPECT_EQ ("Bible", hit.bible);
      EXPECT_EQ (8, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (3, hit.verse);
      EXPECT_EQ ("The verse is out of sequence", hit.data);
    }
  }
}


TEST (database, mappings)
{
  // Setup.
  {
    refresh_sandbox (false);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    database_mappings.defaults ();
    database_mappings.create2 ();
    database_mappings.optimize ();
    std::vector <std::string> names = database_mappings.names ();
    std::vector <std::string> standard_names{"Dutch Traditional", filter::strings::english (), "French Louise Segond", "Hebrew Greek", "Russian Canonical", "Russian Orthodox", "Russian Protestant", "Spanish", "Vulgate"};
    EXPECT_EQ (standard_names, names);
  }

  // Import and export.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import =
    "2 Chronicles 14:15 = 2 Chronicles 14:14\n"
    "Nehemiah 4:1 = Nehemiah 3:33\n"
    "Song of Solomon 7:2 = Song of Solomon 7:3\n";
    database_mappings.import ("phpunit", import);
    std::vector <std::string> names = database_mappings.names ();
    std::vector <std::string> standard_names{"Hebrew Greek", "phpunit"};
    EXPECT_EQ (standard_names, names);
    std::string output = database_mappings.output ("phpunit");
    EXPECT_EQ (filter::strings::trim (import), filter::strings::trim (output));
  }
  
  // Create.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    database_mappings.create ("phpunit");
    const std::vector <std::string> names = database_mappings.names ();
    const std::vector <std::string> standard_names {"Hebrew Greek", "phpunit"};
    EXPECT_EQ (standard_names, names);
  }
  
  // Translate the same.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::vector <Passage> passages = database_mappings.translate ("ABC", "ABC", 14, 14, 15);
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    Passage standard = Passage ("", 14, 14, "15");
    EXPECT_EQ (true, passages[0].equal (standard));
    passages = database_mappings.translate ("--X", "--X", 15, 16, 17);
    standard = Passage ("", 15, 16, "17");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    EXPECT_EQ (true, passages[0].equal (standard));
  }
  
  // Translate.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import =
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
    std::vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    Passage standard = Passage ("", 14, 14, "15");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    EXPECT_EQ (true, passages[0].equal (standard));
  }

  // Translate.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import =
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
    std::vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    Passage standard = Passage ("", 14, 14, "13");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    EXPECT_EQ (true, passages[0].equal (standard));
  }

  // Translate double result.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import =
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
    std::vector <Passage> passages = database_mappings.translate ("ABC", "XYZ", 14, 14, 15);
    EXPECT_EQ (2, static_cast<int>(passages.size ()));
    Passage standard = Passage ("", 14, 14, "12");
    EXPECT_EQ (true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "13");
    EXPECT_EQ (true, passages[1].equal (standard));
  }

  // Translate from original.
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("VVV", import);
    std::vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 14, 14);
    Passage standard = Passage ("", 14, 14, "12");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    EXPECT_EQ (true, passages[0].equal (standard));
  }

  // Translate From Original Double
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import =
    "2 Chronicles 14:12 = 2 Chronicles 14:14\n"
    "2 Chronicles 14:13 = 2 Chronicles 14:14\n";
    database_mappings.import ("VVV", import);
    std::vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 14, 14);
    EXPECT_EQ (2, static_cast<int>(passages.size ()));
    Passage standard = Passage ("", 14, 14, "12");
    EXPECT_EQ (true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "13");
    EXPECT_EQ (true, passages[1].equal (standard));
  }

  // Translate From Original No Mapping
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("VVV", import);
    std::vector <Passage> passages = database_mappings.translate ("Hebrew Greek", "VVV", 14, 15, 14);
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    Passage standard = Passage ("", 14, 15, "14");
    EXPECT_EQ (true, passages[0].equal (standard));
  }

  // Translate To Original
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import = "2 Chronicles 14:12 = 2 Chronicles 14:14";
    database_mappings.import ("ABA", import);
    std::vector <Passage> passages = database_mappings.translate ("ABA", "Hebrew Greek", 14, 14, 12);
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    Passage standard = Passage ("", 14, 14, "14");
    EXPECT_EQ (true, passages[0].equal (standard));
  }

  // Translate To Original Double
  {
    refresh_sandbox (true);
    Database_Mappings database_mappings;
    database_mappings.create1 ();
    std::string import =
    "2 Chronicles 14:12 = 2 Chronicles 14:13\n"
    "2 Chronicles 14:12 = 2 Chronicles 14:14\n";
    database_mappings.import ("ABA", import);
    std::vector <Passage> passages = database_mappings.translate ("ABA", "Hebrew Greek", 14, 14, 12);
    EXPECT_EQ (2, static_cast<int>(passages.size ()));
    Passage standard = Passage ("", 14, 14, "13");
    EXPECT_EQ (true, passages[0].equal (standard));
    standard = Passage ("", 14, 14, "14");
    EXPECT_EQ (true, passages[1].equal (standard));
  }
  
}


TEST (database, versifications)
{
  // Basic operations, create, delete.
  {
    refresh_sandbox (false);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.optimize ();
    int id = database_versifications.createSystem ("phpunit");
    EXPECT_EQ (1000, id);
    id = database_versifications.getID ("phpunit");
    EXPECT_EQ (1000, id);
    std::vector <std::string> systems = database_versifications.getSystems ();
    EXPECT_EQ (std::vector <std::string>{"phpunit"}, systems);
    database_versifications.erase ("phpunit");
    systems = database_versifications.getSystems ();
    EXPECT_EQ (std::vector <std::string>{}, systems);
  }
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    
    // GetID
    int id = database_versifications.getID (filter::strings::english ());
    EXPECT_EQ (4 , id);
    
    // Test books.
    std::vector <int> books = database_versifications.getBooks (filter::strings::english ());
    std::vector <int> standard;
    for (int i = 1; i <= 66; i++) standard.push_back (i);
    EXPECT_EQ (standard, books);
    
    // Test chapters.
    std::vector <int> chapters = database_versifications.getChapters (filter::strings::english (), 1);
    standard.clear ();
    for (int i = 1; i <= 50; i++) standard.push_back (i);
    EXPECT_EQ (standard, chapters);
    chapters = database_versifications.getChapters (filter::strings::english (), 1, true);
    standard.clear ();
    for (int i = 0; i <= 50; i++) standard.push_back (i);
    EXPECT_EQ (standard, chapters);
    
    // Test verses.
    std::vector <int> verses = database_versifications.getVerses (filter::strings::english (), 1, 2);
    standard.clear ();
    for (int i = 0; i <= 25; i++) standard.push_back (i);
    EXPECT_EQ (standard, verses);
    
    // Verses in chapter 0.
    verses = database_versifications.getVerses (filter::strings::english (), 1, 0);
    EXPECT_EQ (std::vector <int>{0}, verses);
    
    // Books Chapters Verses.
    std::vector <Passage> data = database_versifications.getBooksChaptersVerses (filter::strings::english ());
    EXPECT_EQ (1189, static_cast<int>(data.size()));
    EXPECT_EQ ("31", data [0].m_verse);
    
    // Maximum number of books.
    books = database_versifications.getMaximumBooks ();
    standard = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 70, 71, 73, 74, 75, 80, 81 };
    EXPECT_EQ (standard, books);
    
    // Maximum number of chapters.
    chapters = database_versifications.getMaximumChapters (5);
    standard.clear ();
    for (int i = 0; i <= 34; i++) standard.push_back (i);
    EXPECT_EQ (standard, chapters);
    
    // Maximum number of verses.
    verses = database_versifications.getMaximumVerses (1, 2);
    standard.clear ();
    for (int i = 0; i <= 25; i++) standard.push_back (i);
    EXPECT_EQ (standard, verses);
  }
  // Import Export
  {
    refresh_sandbox (true);
    Database_Versifications database_versifications;
    database_versifications.create ();
    std::string input =
    "Genesis 1:31\n"
    "Genesis 2:25\n";
    database_versifications.input (input, "phpunit");
    int id = database_versifications.getID ("phpunit");
    EXPECT_EQ (1000, id);
    std::vector <Passage> data = database_versifications.getBooksChaptersVerses ("phpunit");
    EXPECT_EQ (2, static_cast<int> (data.size ()));
    EXPECT_EQ ("25", data [1].m_verse);
    std::string output = database_versifications.output ("phpunit");
    EXPECT_EQ (filter::strings::trim (input), filter::strings::trim (output));
  }
}


#endif

