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


#include <unittests/statistics.h>
#include <unittests/utilities.h>
#include <database/statistics.h>
#include <filter/date.h>


void test_database_statistics ()
{
#ifdef HAVE_CLOUD

  trace_unit_tests (__func__);
  
  {
    refresh_sandbox (true);
    
    Database_Statistics::create ();
    Database_Statistics::optimize ();
    
    int one_thousand = 1000;
    int two_thousand = 2000;
    int now = filter_date_seconds_since_epoch ();
    int now_plus_one = now + 1;
    
    // Record some statistics.
    Database_Statistics::store_changes (one_thousand, "zero", 0);
    Database_Statistics::store_changes (one_thousand, "one", 10);
    Database_Statistics::store_changes (two_thousand, "one", 20);
    Database_Statistics::store_changes (now, "one", 30);
    Database_Statistics::store_changes (now_plus_one, "one", 40);
    Database_Statistics::store_changes (1100, "two", 11);
    Database_Statistics::store_changes (2100, "two", 21);
    Database_Statistics::store_changes (now - one_thousand, "two", 31);
    Database_Statistics::store_changes (now - two_thousand, "two", 41);
    
    // Check all available users.
    vector <string> users = Database_Statistics::get_users ();
    evaluate (__LINE__, __func__, {"one", "two"}, users);
    
    // The changes for all available users for no more than a year ago.
    vector <pair <int, int>> changes = Database_Statistics::get_changes ("");
    evaluate (__LINE__, __func__, 4, changes.size ());
    
    // A known amount of change statistics records for a known user for no more than a year ago.
    changes = Database_Statistics::get_changes ("two");
    unsigned int size = 2;
    evaluate (__LINE__, __func__, size, changes.size ());
    
    // Sort the change statistics most recent first.
    if (changes.size () == size) {
      evaluate (__LINE__, __func__, now - one_thousand, changes[0].first);
      evaluate (__LINE__, __func__, 31, changes[0].second);
      evaluate (__LINE__, __func__, now - two_thousand, changes[1].first);
      evaluate (__LINE__, __func__, 41, changes[1].second);
    }
  }

#endif
}
