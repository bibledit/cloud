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
#include <database/sprint.h>
using namespace std;


TEST (database, sprint)
{
#ifdef HAVE_CLOUD

  // Maintenance.
  {
    refresh_sandbox (false);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    database.optimize ();
  }
  // Store Task / GetTask
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    vector <int> ids = database.getTasks ("phpunit", 2014, 1);
    EXPECT_EQ (vector <int>{}, ids);
    database.storeTask ("phpunit", 2014, 1, "phpunit");
    ids = database.getTasks ("phpunit", 2014, 1);
    EXPECT_EQ (1, static_cast <int>(ids.size ()));
    ids = database.getTasks ("phpunit", 2014, 2);
    EXPECT_EQ (0, static_cast <int>(ids.size ()));
    ids = database.getTasks ("phpunit2", 2014, 1);
    EXPECT_EQ (0, static_cast <int>(ids.size ()));
  }
  // GetTitle
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    database.storeTask ("phpunit", 2014, 1, "xyz");
    vector <int> ids = database.getTasks ("phpunit", 2014, 1);
    string title = database.getTitle (ids[0]);
    EXPECT_EQ ("xyz", title);
  }
  // Complete
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    database.storeTask ("phpunit", 2014, 1, "phpunit");
    vector <int> ids = database.getTasks ("phpunit", 2014, 1);
    int id = ids[0];
    int complete = database.getComplete (id);
    EXPECT_EQ (0, complete);
    database.updateComplete (id, 95);
    complete = database.getComplete (id);
    EXPECT_EQ (95, complete);
  }
  // History
  {
    refresh_sandbox (true);
    Database_Sprint database = Database_Sprint ();
    database.create ();
    
    // Expect no history at all for January 2014.
    vector <Database_Sprint_Item> history = database.getHistory ("phpunit", 2014, 1);
    EXPECT_EQ (0, static_cast<int>(history.size()));
    
    // Log values for January 2014, and check that the database returns those values.
    database.logHistory ("phpunit", 2014, 1, 10, 15, 50);
    history = database.getHistory ("phpunit", 2014, 1);
    EXPECT_EQ (10, history[0].day);
    EXPECT_EQ (15, history[0].tasks);
    EXPECT_EQ (50, history[0].complete);
    
    // Log values for February 2014, and don't expect them when requesting the history for January ...
    database.logHistory ("phpunit", 2014, 2, 10, 15, 51);
    history = database.getHistory ("phpunit", 2014, 1);
    EXPECT_EQ (10, history[0].day);
    EXPECT_EQ (15, history[0].tasks);
    EXPECT_EQ (50, history[0].complete);
    
    // ... but get those values when requesting history for February.
    history = database.getHistory ("phpunit", 2014, 2);
    EXPECT_EQ (10, history[0].day);
    EXPECT_EQ (15, history[0].tasks);
    EXPECT_EQ (51, history[0].complete);
    
    // Log another history entry for January 2014, and expect two correct entries for this month.
    database.logHistory ("phpunit", 2014, 1, 11, 16, 55);
    history = database.getHistory ("phpunit", 2014, 1);
    EXPECT_EQ (10, history[0].day);
    EXPECT_EQ (15, history[0].tasks);
    EXPECT_EQ (50, history[0].complete);
    EXPECT_EQ (11, history[1].day);
    EXPECT_EQ (16, history[1].tasks);
    EXPECT_EQ (55, history[1].complete);
  }

#endif
}


#endif
