/*
Copyright (©) 2003-2020 Teus Benschop.

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


#include <unittests/check.h>
#include <unittests/utilities.h>
#include <database/check.h>
#include <database/state.h>
#include <database/bibles.h>


void test_database_check ()
{
  trace_unit_tests (__func__);
  
  {
    // Test optimize.
    refresh_sandbox (true);
    Database_Check database_check = Database_Check ();
    database_check.optimize ();
  }

  {
    // Test record get truncate.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, (int)hits.size());
    
    database_check.recordOutput ("phpunit", 1, 2, 3, "test");
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
    
    database_check.truncateOutput ("");
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, (int)hits.size());
  }

  {
    // Test getting details.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 5, 2, 3, "test");
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
    evaluate (__LINE__, __func__, "phpunit", hits [0].bible);
    evaluate (__LINE__, __func__, 5, hits [0].book);
    evaluate (__LINE__, __func__, 2, hits [0].chapter);
    evaluate (__LINE__, __func__, "test", hits [0].data);
  }

  {
    // Test approvals.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 3, 4, 5, "test2");
    
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)hits.size());
    
    int id = hits [0].rowid;
    database_check.approve (id);
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
    
    vector <Database_Check_Hit> suppressions = database_check.getSuppressions ();
    evaluate (__LINE__, __func__, 1, (int)suppressions.size());
    
    id = suppressions [0].rowid;
    
    database_check.release (1);
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)hits.size());
  }

  {
    // Test delete.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 3, 4, 5, "test2");
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, (int)hits.size());
    int id = hits [0].rowid;
    database_check.erase (id);
    hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, (int)hits.size());
  }

  {
    // Test passage.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "test1");
    database_check.recordOutput ("phpunit", 6, 7, 8, "test2");
    Passage passage = database_check.getPassage (2);
    evaluate (__LINE__, __func__, 6, passage.book);
    evaluate (__LINE__, __func__, 7, passage.chapter);
    evaluate (__LINE__, __func__, "8", passage.verse);
  }

  {
    // Test same checks overflow.
    refresh_sandbox (true);
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.createBible ("phpunit");
    Database_Check database_check = Database_Check ();
    database_check.create ();
    database_check.recordOutput ("phpunit", 3, 4, 5, "once");
    for (unsigned int i = 0; i < 100; i++) {
      database_check.recordOutput ("phpunit", i, i, i, "multiple");
    }
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 12, (int)hits.size());
  }
}
