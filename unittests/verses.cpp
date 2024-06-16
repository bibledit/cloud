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
#include <database/check.h>
#include <checks/verses.h>


TEST (verses, basic)
{
  refresh_sandbox (false);
  database::check::create ();
  // Test Missing Punctuation At End
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said."),
      std::pair (5, "He said: “Jesus.”")
    };
    checks_verses::missing_punctuation_at_end ("1", 1, 1, verses, ", ;", ". ! ? :", "”");
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int>(results.size()));
    if (results.size ()) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (3, hit.verse);
      EXPECT_EQ ("No punctuation at end of verse: y", hit.data);
    }
  }
  database::check::truncate_output ("");
  // Test Pattern
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said.")
    };
    checks_verses::patterns ("1", 1, 1, verses, {"did"});
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int>(results.size()));
    if (results.size ()) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (3, hit.verse);
      EXPECT_EQ ("Pattern found in text: did", hit.data);
    }
  }
  database::check::truncate_output ("");
  // Test Pattern
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said.")
    };
    checks_verses::patterns ("1", 1, 1, verses, {"Did"});
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (0, static_cast<int>(results.size()));
  }
  database::check::truncate_output ("");
  // Test Pattern
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said.")
    };
    checks_verses::patterns ("1", 1, 1, verses, {"said"});
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int>(results.size()));
    if (results.size () == 2) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ ("Pattern found in text: said", hit.data);
      hit = results[1];
      EXPECT_EQ (2, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (4, hit.verse);
      EXPECT_EQ ("Pattern found in text: said", hit.data);
    }
  }
  database::check::truncate_output ("");
}

#endif

