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
#include <database/check.h>
#include <database/state.h>
#include <database/bibles.h>
#include <filter/passage.h>
#include <checks/issues.h>


TEST (checks, database)
{
  {
    // Test optimize.
    refresh_sandbox (false);
    database::check::optimize ();
  }

  {
    // Test record get truncate.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (0, static_cast<int> (hits.size()));
    
    database::check::record_output ("phpunit", 1, 2, 3, "test");
    hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    
    database::check::delete_output ("");
    hits = database::check::get_hits ();
    EXPECT_EQ (0, static_cast<int> (hits.size()));
  }

  {
    // Test getting details.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 5, 2, 3, "test");
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    EXPECT_EQ ("phpunit", hits [0].bible);
    EXPECT_EQ (5, hits [0].book);
    EXPECT_EQ (2, hits [0].chapter);
    EXPECT_EQ ("test", hits [0].data);
  }

  {
    // Test approvals.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    
    database::check::record_output ("phpunit", 3, 4, 5, "test1");
    database::check::record_output ("phpunit", 3, 4, 5, "test2");
    
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
    
    int id = hits [0].rowid;
    database::check::approve (id);
    hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    
    std::vector <database::check::Hit> suppressions = database::check::get_suppressions ();
    EXPECT_EQ (1, static_cast<int>(suppressions.size()));
    
    id = suppressions [0].rowid;
    EXPECT_EQ (1, id);

    database::check::release (1);
    hits = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
  }

  {
    // Test delete.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 3, 4, 5, "test1");
    database::check::record_output ("phpunit", 3, 4, 5, "test2");
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
    const int id = hits [0].rowid;
    database::check::delete_id (id);
    hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
  }

  {
    // Test passage.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 3, 4, 5, "test1");
    database::check::record_output ("phpunit", 6, 7, 8, "test2");
    Passage passage = database::check::get_passage (2);
    EXPECT_EQ (6, passage.m_book);
    EXPECT_EQ (7, passage.m_chapter);
    EXPECT_EQ ("8", passage.m_verse);
  }

  {
    // Test same checks overflow.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 3, 4, 5, "once");
    for (int i = 0; i < 100; i++) {
      database::check::record_output ("phpunit", i, i, i, "multiple");
    }
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (12, static_cast<int> (hits.size()));
  }
}


// Check that each issue has a translation.
// Check there's no duplicate translations.
TEST (checks, issues)
{
  using namespace checks::issues;
  constexpr auto start {static_cast<int>(issue::start_boundary)};
  constexpr auto stop {static_cast<int>(issue::stop_boundary)};
  std::set<std::string> translations;
  for (int i {start + 1}; i < stop; i++) {
    const auto issue = static_cast<enum issue>(i);
    const auto translation = text(issue);
    EXPECT_FALSE(translation.empty());
    EXPECT_FALSE(translations.count(translation));
    translations.insert(translation);
  }
}


#endif
