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
#include <database/state.h>
#include <database/bibles.h>
using namespace std;


TEST (database, check)
{
  {
    // Test optimize.
    refresh_sandbox (false);
    Database_Check database_check = Database_Check ();
    database_check.optimize ();
  }

  {
    // Test record get truncate.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    
    std::vector <Database_Check_Hit> hits = database_check.getHits ();
    EXPECT_EQ (0, static_cast<int> (hits.size()));
    
    database_check.recordOutput ("phpunit", 1, 2, 3, "test");
    hits = database_check.getHits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    
    database_check.truncateOutput ("");
    hits = database_check.getHits ();
    EXPECT_EQ (0, static_cast<int> (hits.size()));
  }

  {
    // Test getting details.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 5, 2, 3, "test");
    std::vector <Database_Check_Hit> hits = database_check.getHits ();
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
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 3, 4, 5, "test2");
    
    std::vector <Database_Check_Hit> hits = database_check.getHits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
    
    int id = hits [0].rowid;
    database_check.approve (id);
    hits = database_check.getHits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    
    std::vector <Database_Check_Hit> suppressions = database_check.getSuppressions ();
    EXPECT_EQ (1, static_cast<int>(suppressions.size()));
    
    id = suppressions [0].rowid;
    EXPECT_EQ (1, id);

    database_check.release (1);
    hits = database_check.getHits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
  }

  {
    // Test delete.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 3, 4, 5, "test2");
    std::vector <Database_Check_Hit> hits = database_check.getHits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
    int id = hits [0].rowid;
    database_check.erase (id);
    hits = database_check.getHits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
  }

  {
    // Test passage.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 6, 7, 8, "test2");
    Passage passage = database_check.getPassage (2);
    EXPECT_EQ (6, passage.m_book);
    EXPECT_EQ (7, passage.m_chapter);
    EXPECT_EQ ("8", passage.m_verse);
  }

  {
    // Test same checks overflow.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "once");
    for (int i = 0; i < 100; i++) {
      database_check.recordOutput ("phpunit", i, i, i, "multiple");
    }
    std::vector <Database_Check_Hit> hits = database_check.getHits ();
    EXPECT_EQ (12, static_cast<int> (hits.size()));
  }
}

#endif
