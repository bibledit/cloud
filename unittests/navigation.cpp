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
#include <database/navigation.h>
#include <filter/date.h>


constexpr const char* user  {"user"};
constexpr const char* user_1 {"user1"};
constexpr const char* user_2 {"user2"};
constexpr const int focus_group_0 {0};
constexpr const int focus_group_1 {1};
constexpr const int focus_group_2 {2};
constexpr const std::array<int,3> focus_groups {focus_group_0, focus_group_1, focus_group_2};


TEST (database_navigation, create_trim)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  database.trim ();
}


TEST (database_navigation, timed_record)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  
  // Use current time.
  int time = filter::date::seconds_since_epoch ();
  
  // Record one entry.
  // As a result there should be no previous entry.
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 1, 2, 3, focus_group);
    EXPECT_FALSE (database.previous_exists (user, focus_group));
  }

  // Record another entry, with the same time.
  // This should remove the already existing entry.
  // As a result there should still be no previous entry.
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
    EXPECT_FALSE (database.previous_exists (user, focus_group));
  }
  
  // Record another entry 4 seconds later.
  // This should remove the already existing entry.
  // As a result there should be no previous entry.
  time += 4;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
    EXPECT_FALSE (database.previous_exists (user, focus_group));
  }

  // Record another entry 5 seconds later.
  // This should remove the already existing entry.
  // As a result there should be no previous entry.
  time += 5;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
    EXPECT_FALSE (database.previous_exists (user, focus_group));
  }

  // Record another entry 6 seconds later.
  // This should not remove the already existing entry.
  // As a result there should be one previous entry.
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
    EXPECT_TRUE (database.previous_exists (user, focus_group));
  }
}


TEST (database_navigation, previous_same_user)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  // Use current time.
  int time = filter::date::seconds_since_epoch ();
  // Record one entry, and another one 6 seconds later.
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 1, 2, 3, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
  }
  // Get previous entry, which should be the first one entered.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_previous (user, focus_group);
    EXPECT_EQ (1, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("3", passage.m_verse);
  }
}


TEST (database_navigation, previous_other_user)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  // Use current time.
  int time = filter::date::seconds_since_epoch ();
  // Record one entry, and another 6 seconds later.
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 1, 2, 3, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
  }
  // Get previous entry for another user: It should not be there.
  for (const auto focus_group : focus_groups) {
    Passage passage = database.get_previous (user_2, focus_group);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
  }
}


TEST (database_navigation, previous_of_3_entries)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  // Use current time.
  int time = filter::date::seconds_since_epoch ();
  // Record three entries, each one 6 seconds later.
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 1, 2, 3, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 7, 8, 9, focus_group);
  }
  // Get previous entry, which should be the second one entered.
  for (const auto focus_group : focus_groups) {
    Passage passage = database.get_previous (user, focus_group);
    EXPECT_EQ (4, passage.m_book);
    EXPECT_EQ (5, passage.m_chapter);
    EXPECT_EQ ("6", passage.m_verse);
  }
}


TEST (database_navigation, previous_of_5_entries)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  // Use current time.
  int time = filter::date::seconds_since_epoch ();
  // Record five entries, each one 6 seconds later.
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 1, 2, 3, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 4, 5, 6, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 7, 8, 9, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 10, 11, 12, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user, 13, 14, 15, focus_group);
  }
  // Get previous entry, which should be the last but one passage recorded.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_previous (user, focus_group);
    EXPECT_EQ (10, passage.m_book);
    EXPECT_EQ (11, passage.m_chapter);
    EXPECT_EQ ("12", passage.m_verse);
  }
}


TEST (database_navigation, no_next)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  // There should be no next passage.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_next (user, focus_group);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
  }
}


TEST (database_navigation, no_next_n_records)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  // Use current time.
  int time = filter::date::seconds_since_epoch ();
  // Record several entries, all spaced apart by 6 seconds.
  for (const auto focus_group : focus_groups)
    database.record (time, user, 1, 2, 3, focus_group);
  time += 6;
  for (const auto focus_group : focus_groups)
    database.record (time, user, 1, 2, 3, focus_group);
  time += 6;
  for (const auto focus_group : focus_groups)
    database.record (time, user, 1, 2, 3, focus_group);
  time += 6;
  for (const auto focus_group : focus_groups)
    database.record (time, user, 1, 2, 3, focus_group);
  time += 6;
  for (const auto focus_group : focus_groups)
  database.record (time, user, 1, 2, 3, focus_group);
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_next (user, focus_group);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
  }
}


TEST (database_navigation, multiple_previous_next)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  // Record two entries at an interval.
  int time = filter::date::seconds_since_epoch ();
  for (const auto focus_group : focus_groups)
    database.record (time, user, 1, 2, 3, focus_group);
  time += 6;
  for (const auto focus_group : focus_groups)
    database.record (time, user, 4, 5, 6, focus_group);
  // Next entry is not there.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_next (user, focus_group);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
  }
  // Previous entry should be there.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_previous (user, focus_group);
    EXPECT_EQ (1, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("3", passage.m_verse);
  }
  // Next entry should be there since we moved to the previous one.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_next (user, focus_group);
    EXPECT_EQ (4, passage.m_book);
    EXPECT_EQ (5, passage.m_chapter);
    EXPECT_EQ ("6", passage.m_verse);
  }
  // Previous entry should be there.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_previous (user,focus_group);
    EXPECT_EQ (1, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("3", passage.m_verse);
  }
  // Previous entry before previous entry should not be there.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_previous (user, focus_group);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
  }
  // Next entry should be there since we moved to the previous one.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_next (user, focus_group);
    EXPECT_EQ (4, passage.m_book);
    EXPECT_EQ (5, passage.m_chapter);
    EXPECT_EQ ("6", passage.m_verse);
  }
  // The entry next to the next entry should not be there.
  for (const auto focus_group : focus_groups) {
    const Passage passage = database.get_next (user, focus_group);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
  }
}


TEST (database_navigation, history_two_users)
{
  // Initialization.
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  int time = filter::date::seconds_since_epoch ();
  
  // Record three entries at an interval for this user.
  // Record different entries for another user.
  for (const auto focus_group : focus_groups) {
    database.record (time, user,   1, 2, 3, focus_group);
    database.record (time, user_1, 3, 2, 1, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user,   4, 5, 6, focus_group);
    database.record (time, user_1, 6, 5, 4, focus_group);
  }
  time += 6;
  for (const auto focus_group : focus_groups) {
    database.record (time, user,   7, 8, 9, focus_group);
    database.record (time, user_1, 9, 8, 7, focus_group);
  }
  
  // Check the first two entries are there in the backward history.
  // They should be in reverse order of entry.
  // The entries for the other user should not be in the retrieved history.
  for (const auto focus_group : focus_groups) {
    const auto passages = database.get_history(user, -1, focus_group);
    EXPECT_EQ (2, passages.size());
    if (passages.size() == 2) {
      EXPECT_EQ (4, passages[0].m_book);
      EXPECT_EQ (5, passages[0].m_chapter);
      EXPECT_EQ ("6", passages[0].m_verse);
      EXPECT_EQ (1, passages[1].m_book);
      EXPECT_EQ (2, passages[1].m_chapter);
      EXPECT_EQ ("3", passages[1].m_verse);
    }
  }
  
  // At this stage there should not yet be any forward history.
  for (const auto focus_group : focus_groups) {
    const auto passages = database.get_history(user, 1, focus_group);
    EXPECT_EQ (0, passages.size());
  }
  
  // Go backwards one step.
  // As a result there should be one history item going back.
  // And one history item going forward.
  for (const auto focus_group : focus_groups) {
    database.get_previous(user, focus_group);
    const auto passages = database.get_history(user, -1, focus_group);
    EXPECT_EQ (1, passages.size());
    if (passages.size() == 1) {
      EXPECT_EQ (1, passages[0].m_book);
      EXPECT_EQ (2, passages[0].m_chapter);
      EXPECT_EQ ("3", passages[0].m_verse);
    }
  }
  for (const auto focus_group : focus_groups) {
    const auto passages = database.get_history(user, 1, focus_group);
    EXPECT_EQ (1, passages.size());
    if (passages.size() == 1) {
      EXPECT_EQ (7, passages[0].m_book);
      EXPECT_EQ (8, passages[0].m_chapter);
      EXPECT_EQ ("9", passages[0].m_verse);
    }
  }
  
  // Go backwards yet another step.
  // After this there should not be any history going back left.
  // There should now be two forward history items available.
  for (const auto focus_group : focus_groups) {
    database.get_previous(user, focus_group);
    const auto passages = database.get_history(user, -1, focus_group);
    EXPECT_EQ (0, passages.size());
  }
  for (const auto focus_group : focus_groups) {
    const auto passages = database.get_history(user, 1, focus_group);
    EXPECT_EQ (2, passages.size());
    if (passages.size() == 2) {
      EXPECT_EQ (4, passages[0].m_book);
      EXPECT_EQ (5, passages[0].m_chapter);
      EXPECT_EQ ("6", passages[0].m_verse);
      EXPECT_EQ (7, passages[1].m_book);
      EXPECT_EQ (8, passages[1].m_chapter);
      EXPECT_EQ ("9", passages[1].m_verse);
    }
  }
}


TEST (database_navigation, trim_no_loss)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  int time = filter::date::seconds_since_epoch ();
  
  // 1. Record several recent passages.
  // 2. Trim the database.
  // Check the passages are still there.
  for (const auto focus_group : focus_groups) {
    for (int i = 0; i < 5; i++) {
      database.record(time + (10 * i), user, 1, 2, 3, focus_group);
    }
  }
  database.trim ();
  for (const auto focus_group : focus_groups) {
    const auto passages = database.get_history(user, -1, focus_group);
    EXPECT_EQ (4, passages.size());
  }
}


TEST (database_navigation, trim)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  int time = filter::date::seconds_since_epoch ();
  
  // 1. Record several old passages.
  // Check the passages are there.
  for (const auto focus_group : focus_groups) {
    for (int i = 0; i < 5; i++) {
      database.record(time + (10 * i) - (15 * 24 * 3600), user, 1, 2, 3, focus_group);
    }
  }
  for (const auto focus_group : focus_groups) {
    const auto passages = database.get_history(user, -1, focus_group);
    EXPECT_EQ (4, passages.size());
  }
  // 2. Trim the database.
  // 3. Add two new ones.
  // Check the passages are gone now.
  database.trim();
  for (const auto focus_group : focus_groups) {
    for (int i = 0; i < 2; i++) {
      database.record(time + (10 * i), user, 1, 2, 3, focus_group);
    }
  }
  for (const auto focus_group : focus_groups) {
    const auto passages = database.get_history(user, -1, focus_group);
    EXPECT_EQ (1, passages.size());
  }
}


TEST (database_navigation, upgrade)
{
  refresh_sandbox (true);
  Database_Navigation database;
  database.create ();
  int time = filter::date::seconds_since_epoch ();

  const auto record_old_passages = [&](const int focus_group) {
    for (int i = 0; i < 5; i++) {
      database.record(time + (10 * i) - (15 * 24 * 3600), user, 1, 2, 3, focus_group);
    }
  };
  
  // Downgrade the database and check that recording old passages fails.
  database.downgrade();
  for (const auto focus_group : focus_groups) {
    record_old_passages(focus_group);
    EXPECT_TRUE (database.get_history(user, -1, focus_group).empty());
  }

  // Upgrade the database and check that recording old passages now works.
  database.upgrade();
  for (const auto focus_group : focus_groups) {
    record_old_passages(focus_group);
    EXPECT_EQ (database.get_history(user, -1, focus_group).size(), 4);
  }
  
  // Downgrade it and upgrade it and check that the upgrade emptied the database.
  database.downgrade();
  database.upgrade();
  for (const auto focus_group : focus_groups) {
    EXPECT_TRUE (database.get_history(user, -1, focus_group).empty());
  }
}


#endif
