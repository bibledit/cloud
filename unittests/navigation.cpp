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


#include <unittests/odf.h>
#include <unittests/utilities.h>
#include <database/navigation.h>
#include <filter/date.h>


void test_database_navigation ()
{
  trace_unit_tests (__func__);
  
  {
    refresh_sandbox (true);
    Database_Navigation database = Database_Navigation ();
    database.create ();
    database.trim ();
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    
    // Record one entry. As a result there should be no previous entry.
    database.record (time, "phpunit", 1, 2, 3);
    bool previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry, with the same time.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry 4 seconds later.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    time += 4;
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry 5 seconds later.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    time += 5;
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry 6 seconds later.
    // This should not remove the already existing entry.
    // As a result there should be a previous entry.
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    previous = database.previousExists ("phpunit");
    evaluate (__LINE__, __func__, true, previous);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record one entry, and another 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    // Get previous entry, which should be the first one entered.
    Passage passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 1, passage.book);
    evaluate (__LINE__, __func__, 2, passage.chapter);
    evaluate (__LINE__, __func__, "3", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record one entry, and another 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    // Get previous entry for another user: It should not be there.
    Passage passage = database.getPrevious ("phpunit2");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record three entries, each one 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    time += 6;
    database.record (time, "phpunit", 7, 8, 9);
    // Get previous entry, which should be the second one entered.
    Passage passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 4, passage.book);
    evaluate (__LINE__, __func__, 5, passage.chapter);
    evaluate (__LINE__, __func__, "6", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record five entries, each one 6 seconds later.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    time += 6;
    database.record (time, "phpunit", 7, 8, 9);
    time += 6;
    database.record (time, "phpunit", 10, 11, 12);
    time += 6;
    database.record (time, "phpunit", 13, 14, 15);
    // Get previous entry, which should be the last but one passage recorded.
    Passage passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 10, passage.book);
    evaluate (__LINE__, __func__, 11, passage.chapter);
    evaluate (__LINE__, __func__, "12", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // There should be no next passage.
    Passage passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter_date_seconds_since_epoch ();
    // Record several entries, all spaced apart by 6 seconds.
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 1, 2, 3);
    Passage passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Record two entries at an interval.
    int time = filter_date_seconds_since_epoch ();
    database.record (time, "phpunit", 1, 2, 3);
    time += 6;
    database.record (time, "phpunit", 4, 5, 6);
    // Next entry is not there.
    Passage passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
    // Previous entry should be there.
    passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 1, passage.book);
    evaluate (__LINE__, __func__, 2, passage.chapter);
    evaluate (__LINE__, __func__, "3", passage.verse);
    // Next entry should be there since we moved to the previous one.
    passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 4, passage.book);
    evaluate (__LINE__, __func__, 5, passage.chapter);
    evaluate (__LINE__, __func__, "6", passage.verse);
    // Previous entry should be there.
    passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 1, passage.book);
    evaluate (__LINE__, __func__, 2, passage.chapter);
    evaluate (__LINE__, __func__, "3", passage.verse);
    // Previous entry before previous entry should not be there.
    passage = database.getPrevious ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
    // Next entry should be there since we moved to the previous one.
    passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 4, passage.book);
    evaluate (__LINE__, __func__, 5, passage.chapter);
    evaluate (__LINE__, __func__, "6", passage.verse);
    // The entry next to the next entry should not be there.
    passage = database.getNext ("phpunit");
    evaluate (__LINE__, __func__, 0, passage.book);
    evaluate (__LINE__, __func__, 0, passage.chapter);
    evaluate (__LINE__, __func__, "", passage.verse);
  }
}
