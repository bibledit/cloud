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


#include <unittests/odf.h>
#include <unittests/utilities.h>
#include <database/navigation.h>
#include <filter/date.h>


void test_database_navigation ()
{
  trace_unit_tests (__func__);
  
  string user = "user";
  
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
    int time = filter::date::seconds_since_epoch ();
    
    // Record one entry.
    // As a result there should be no previous entry.
    database.record (time, user, 1, 2, 3);
    bool previous = database.previous_exists (user);
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry, with the same time.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    database.record (time, user, 4, 5, 6);
    previous = database.previous_exists (user);
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry 4 seconds later.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    time += 4;
    database.record (time, user, 4, 5, 6);
    previous = database.previous_exists (user);
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry 5 seconds later.
    // This should remove the already existing entry.
    // As a result there should be no previous entry.
    time += 5;
    database.record (time, user, 4, 5, 6);
    previous = database.previous_exists (user);
    evaluate (__LINE__, __func__, false, previous);
    
    // Record another entry 6 seconds later.
    // This should not remove the already existing entry.
    // As a result there should be one previous entry.
    time += 6;
    database.record (time, user, 4, 5, 6);
    previous = database.previous_exists (user);
    evaluate (__LINE__, __func__, true, previous);
  }
  
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter::date::seconds_since_epoch ();
    // Record one entry, and another one 6 seconds later.
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 4, 5, 6);
    // Get previous entry, which should be the first one entered.
    Passage passage = database.get_previous (user);
    evaluate (__LINE__, __func__, 1, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "3", passage.m_verse);
  }
  
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter::date::seconds_since_epoch ();
    // Record one entry, and another 6 seconds later.
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 4, 5, 6);
    // Get previous entry for another user: It should not be there.
    Passage passage = database.get_previous (user + "2");
    evaluate (__LINE__, __func__, 0, passage.m_book);
    evaluate (__LINE__, __func__, 0, passage.m_chapter);
    evaluate (__LINE__, __func__, "", passage.m_verse);
  }
  
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter::date::seconds_since_epoch ();
    // Record three entries, each one 6 seconds later.
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 4, 5, 6);
    time += 6;
    database.record (time, user, 7, 8, 9);
    // Get previous entry, which should be the second one entered.
    Passage passage = database.get_previous (user);
    evaluate (__LINE__, __func__, 4, passage.m_book);
    evaluate (__LINE__, __func__, 5, passage.m_chapter);
    evaluate (__LINE__, __func__, "6", passage.m_verse);
  }
  
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter::date::seconds_since_epoch ();
    // Record five entries, each one 6 seconds later.
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 4, 5, 6);
    time += 6;
    database.record (time, user, 7, 8, 9);
    time += 6;
    database.record (time, user, 10, 11, 12);
    time += 6;
    database.record (time, user, 13, 14, 15);
    // Get previous entry, which should be the last but one passage recorded.
    Passage passage = database.get_previous (user);
    evaluate (__LINE__, __func__, 10, passage.m_book);
    evaluate (__LINE__, __func__, 11, passage.m_chapter);
    evaluate (__LINE__, __func__, "12", passage.m_verse);
  }
  
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // There should be no next passage.
    Passage passage = database.get_next (user);
    evaluate (__LINE__, __func__, 0, passage.m_book);
    evaluate (__LINE__, __func__, 0, passage.m_chapter);
    evaluate (__LINE__, __func__, "", passage.m_verse);
  }
  
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Use current time.
    int time = filter::date::seconds_since_epoch ();
    // Record several entries, all spaced apart by 6 seconds.
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 1, 2, 3);
    Passage passage = database.get_next (user);
    evaluate (__LINE__, __func__, 0, passage.m_book);
    evaluate (__LINE__, __func__, 0, passage.m_chapter);
    evaluate (__LINE__, __func__, "", passage.m_verse);
  }
  
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    // Record two entries at an interval.
    int time = filter::date::seconds_since_epoch ();
    database.record (time, user, 1, 2, 3);
    time += 6;
    database.record (time, user, 4, 5, 6);
    // Next entry is not there.
    Passage passage = database.get_next (user);
    evaluate (__LINE__, __func__, 0, passage.m_book);
    evaluate (__LINE__, __func__, 0, passage.m_chapter);
    evaluate (__LINE__, __func__, "", passage.m_verse);
    // Previous entry should be there.
    passage = database.get_previous (user);
    evaluate (__LINE__, __func__, 1, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "3", passage.m_verse);
    // Next entry should be there since we moved to the previous one.
    passage = database.get_next (user);
    evaluate (__LINE__, __func__, 4, passage.m_book);
    evaluate (__LINE__, __func__, 5, passage.m_chapter);
    evaluate (__LINE__, __func__, "6", passage.m_verse);
    // Previous entry should be there.
    passage = database.get_previous (user);
    evaluate (__LINE__, __func__, 1, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "3", passage.m_verse);
    // Previous entry before previous entry should not be there.
    passage = database.get_previous (user);
    evaluate (__LINE__, __func__, 0, passage.m_book);
    evaluate (__LINE__, __func__, 0, passage.m_chapter);
    evaluate (__LINE__, __func__, "", passage.m_verse);
    // Next entry should be there since we moved to the previous one.
    passage = database.get_next (user);
    evaluate (__LINE__, __func__, 4, passage.m_book);
    evaluate (__LINE__, __func__, 5, passage.m_chapter);
    evaluate (__LINE__, __func__, "6", passage.m_verse);
    // The entry next to the next entry should not be there.
    passage = database.get_next (user);
    evaluate (__LINE__, __func__, 0, passage.m_book);
    evaluate (__LINE__, __func__, 0, passage.m_chapter);
    evaluate (__LINE__, __func__, "", passage.m_verse);
  }
  
  {
    // Initialization.
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    int time = filter::date::seconds_since_epoch ();
    vector<Passage> passages;

    // Record three entries at an interval for this user.
    // Record different entries for another user.
    database.record (time, user,       1, 2, 3);
    database.record (time, user + "1", 3, 2, 1);
    time += 6;
    database.record (time, user,       4, 5, 6);
    database.record (time, user + "1", 6, 5, 4);
    time += 6;
    database.record (time, user,       7, 8, 9);
    database.record (time, user + "1", 9, 8, 7);

    // Check the first two entries are there in the backward history.
    // They should be in reverse order of entry.
    // The entries for the other user should not be in the retrieved history.
    passages = database.get_history(user, -1);
    evaluate (__LINE__, __func__, 2, passages.size());
    if (passages.size() == 2) {
      evaluate (__LINE__, __func__, 4, passages[0].m_book);
      evaluate (__LINE__, __func__, 5, passages[0].m_chapter);
      evaluate (__LINE__, __func__, "6", passages[0].m_verse);
      evaluate (__LINE__, __func__, 1, passages[1].m_book);
      evaluate (__LINE__, __func__, 2, passages[1].m_chapter);
      evaluate (__LINE__, __func__, "3", passages[1].m_verse);
    }
    
    // At this stage there should not yet be any forward history.
    passages = database.get_history(user, 1);
    evaluate (__LINE__, __func__, 0, passages.size());
    
    // Go backwards one step.
    // As a result there should be one history item going back.
    // And one history item going forward.
    database.get_previous(user);
    passages = database.get_history(user, -1);
    evaluate (__LINE__, __func__, 1, passages.size());
    if (passages.size() == 1) {
      evaluate (__LINE__, __func__, 1, passages[0].m_book);
      evaluate (__LINE__, __func__, 2, passages[0].m_chapter);
      evaluate (__LINE__, __func__, "3", passages[0].m_verse);
    }
    passages = database.get_history(user, 1);
    evaluate (__LINE__, __func__, 1, passages.size());
    if (passages.size() == 1) {
      evaluate (__LINE__, __func__, 7, passages[0].m_book);
      evaluate (__LINE__, __func__, 8, passages[0].m_chapter);
      evaluate (__LINE__, __func__, "9", passages[0].m_verse);
    }

    // Go backwards yet another step.
    // After this there should not be any history going back left.
    // There should now be two forward history items available.
    database.get_previous(user);
    passages = database.get_history(user, -1);
    evaluate (__LINE__, __func__, 0, passages.size());
    if (passages.size() == 1) {
    }
    passages = database.get_history(user, 1);
    evaluate (__LINE__, __func__, 2, passages.size());
    if (passages.size() == 2) {
      evaluate (__LINE__, __func__, 4, passages[0].m_book);
      evaluate (__LINE__, __func__, 5, passages[0].m_chapter);
      evaluate (__LINE__, __func__, "6", passages[0].m_verse);
      evaluate (__LINE__, __func__, 7, passages[1].m_book);
      evaluate (__LINE__, __func__, 8, passages[1].m_chapter);
      evaluate (__LINE__, __func__, "9", passages[1].m_verse);
    }
  }

  // Test trimming the navigation database.
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    int time = filter::date::seconds_since_epoch ();
    vector<Passage> passages;
    
    // 1. Record several recent passages.
    // 2. Trim the database.
    // Check the passages are still there.
    for (int i = 0; i < 5; i++) {
      database.record(time + (10 * i), user, 1, 2, 3);
    }
    database.trim ();
    passages = database.get_history(user, -1);
    evaluate (__LINE__, __func__, 4, passages.size());
  }
  // Test trimming the navigation database.
  {
    refresh_sandbox (true);
    Database_Navigation database;
    database.create ();
    int time = filter::date::seconds_since_epoch ();
    vector<Passage> passages;
    
    // 1. Record several old passages.
    // Check the passages are there.
    for (int i = 0; i < 5; i++) {
      database.record(time + (10 * i) - (15 * 24 * 3600), user, 1, 2, 3);
    }
    passages = database.get_history(user, -1);
    evaluate (__LINE__, __func__, 4, passages.size());
    // 2. Trim the database.
    // 3. Add two new ones.
    // Check the passages are gone now.
    database.trim();
    for (int i = 0; i < 2; i++) {
      database.record(time + (10 * i), user, 1, 2, 3);
    }
    passages = database.get_history(user, -1);
    evaluate (__LINE__, __func__, 1, passages.size());
  }

}
