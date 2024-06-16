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
#include <checks/pairs.h>


TEST (checks, pairs)
{
  refresh_sandbox (false);
  database::check::create ();
  
  const std::string bible = "bible";
  const int book = 2;
  const int chapter = 3;
  std::map <int, std::string> verses;
  const std::vector <std::pair <std::string, std::string> > pairs = {
    std::pair ("[", "]"),
    std::pair ("(", ")"),
    std::pair ("“", "”"),
  };
  std::vector <database::check::Hit> results;
  
  {
    verses = {
      std::pair (2, "Verse two."),
      std::pair (3, "Verse three."),
      std::pair (4, "Verse four.")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (0, results.size());
    database::check::truncate_output ("");
  }

  {
    verses = {
      std::pair (2, "Verse [two."),
      std::pair (3, "Verse three]."),
      std::pair (4, "Verse four.")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (0, results.size());
    database::check::truncate_output ("");
  }

  {
    verses = {
      std::pair (2, "Verse [two."),
      std::pair (3, "Verse (three."),
      std::pair (4, "Verse four.")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (2, results.size());
    if (results.size () == 2) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ (bible, hit.bible);
      EXPECT_EQ (book, hit.book);
      EXPECT_EQ (chapter, hit.chapter);
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ (R"(Opening character "[" without its matching closing character "]")", hit.data);
      hit = results[1];
      EXPECT_EQ ("Opening character \"(\" without its matching closing character \")\"", hit.data);
    }
    database::check::truncate_output ("");
  }

  {
    verses = {
      std::pair (2, "Verse [two."),
      std::pair (3, "Verse three."),
      std::pair (4, "Verse four).")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (2, results.size());
    if (results.size () == 2) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ (bible, hit.bible);
      EXPECT_EQ (book, hit.book);
      EXPECT_EQ (chapter, hit.chapter);
      EXPECT_EQ (4, hit.verse);
      EXPECT_EQ ("Closing character \")\" without its matching opening character \"(\"", hit.data);
      hit = results[1];
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ (R"(Opening character "[" without its matching closing character "]")", hit.data);
    }
    database::check::truncate_output ("");
  }
}

#endif

