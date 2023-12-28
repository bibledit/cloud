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
#include <database/modifications.h>
#include <filter/date.h>
#include <database/state.h>
#include <database/bibles.h>
#include <bb/logic.h>
#include <changes/logic.h>
using namespace std;


TEST (database, modifications_user)
{
  // Create, erase, clear.
  {
    refresh_sandbox (false);
    Database_Modifications database_modifications;
    database_modifications.erase ();
    database_modifications.create ();
    database_modifications.clearUserUser ("phpunit");
  }

  // Usernames
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <string> users = database_modifications.getUserUsernames ();
    EXPECT_EQ (vector <string>{}, users);
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old", 4, "new");
    users = database_modifications.getUserUsernames ();
    EXPECT_EQ (vector <string>{"phpunit1"}, users);
    database_modifications.recordUserSave ("phpunit2", "bible", 1, 2, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit3", "bible", 1, 2, 3, "old", 6, "new");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old", 7, "new");
    database_modifications.recordUserSave ("phpunit2", "bible", 1, 2, 3, "old", 8, "new");
    database_modifications.recordUserSave ("phpunit3", "bible", 1, 2, 3, "old", 9, "new");
    users = database_modifications.getUserUsernames ();
    EXPECT_EQ ((vector <string>{"phpunit1", "phpunit2", "phpunit3"}), users);
  }

  // Bibles
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <string> bibles = database_modifications.getUserBibles ("phpunit1");
    EXPECT_EQ (vector <string>{}, bibles);
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 6, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 7, "new");
    bibles = database_modifications.getUserBibles ("phpunit1");
    EXPECT_EQ (vector <string>{"bible1"}, bibles);
  }

  // Books
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <int> books = database_modifications.getUserBooks ("phpunit1", "bible1");
    EXPECT_EQ (vector <int>{}, books);
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 2, 2, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 3, 2, 3, "old", 6, "new");
    database_modifications.recordUserSave ("phpunit1", "bible2", 4, 2, 3, "old", 7, "new");
    books = database_modifications.getUserBooks ("phpunit1", "bible1");
    EXPECT_EQ ((vector <int>{1, 2, 3}), books);
  }

  // Chapters
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    vector <int> chapters = database_modifications.getUserChapters ("phpunit1", "bible1", 1);
    EXPECT_EQ (vector <int>{}, chapters);
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 3, 3, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible1", 1, 4, 3, "old", 6, "new");
    chapters = database_modifications.getUserChapters ("phpunit1", "bible1", 1);
    EXPECT_EQ ((vector <int>{2, 3, 4}), chapters);
  }

  // Identifiers.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old", 4, "new");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 4, "old", 5, "new");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 5, "old", 6, "new");
    vector <Database_Modifications_Id> identifiers = database_modifications.getUserIdentifiers ("phpunit1", "bible", 1, 2);
    EXPECT_EQ (3, static_cast<int>(identifiers.size()));
    EXPECT_EQ (3, identifiers[0].oldid);
    EXPECT_EQ (4, identifiers[0].newid);
    EXPECT_EQ (4, identifiers[1].oldid);
    EXPECT_EQ (5, identifiers[1].newid);
    EXPECT_EQ (5, identifiers[2].oldid);
    EXPECT_EQ (6, identifiers[2].newid);
  }

  // Chapter
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old1", 4, "new1");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 4, "old2", 5, "new2");
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 5, "old3", 6, "new3");
    Database_Modifications_Text chapter = database_modifications.getUserChapter ("phpunit1", "bible", 1, 2, 4);
    EXPECT_EQ ("old1", chapter.oldtext);
    EXPECT_EQ ("new1", chapter.newtext);
    chapter = database_modifications.getUserChapter ("phpunit1", "bible", 1, 2, 5);
    EXPECT_EQ ("old2", chapter.oldtext);
    EXPECT_EQ ("new2", chapter.newtext);
  }

  // Timestamp
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.recordUserSave ("phpunit1", "bible", 1, 2, 3, "old1", 4, "new1");
    int time = database_modifications.getUserTimestamp ("phpunit1", "bible", 1, 2, 4);
    int currenttime = filter::date::seconds_since_epoch ();
    if ((time < currenttime - 1) || (time > currenttime + 1)) EXPECT_EQ (currenttime, time);
  }
}


TEST (database, modifications_team)
{
  // Basics.
  {
    refresh_sandbox (false);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
  }

  // Team existence
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    // Initially no diff should exist.
    bool exists = database_modifications.teamDiffExists ("phpunit", 1, 2);
    EXPECT_EQ (false, exists);
    
    // After storing a chapter there should be a diff.
    bible_logic::store_chapter ("phpunit", 1, 2, "chapter text one");
    exists = database_modifications.teamDiffExists ("phpunit", 1, 2);
    EXPECT_EQ (true, exists);
    
    // After storing a chapter for the second time, the diff should still exist.
    bible_logic::store_chapter ("none", 1, 2, "chapter text two");
    exists = database_modifications.teamDiffExists ("phpunit", 1, 2);
    EXPECT_EQ (true, exists);
    
    // The diff data should not exist for another chapter.
    exists = database_modifications.teamDiffExists ("phpunit", 1, 1);
    EXPECT_EQ (false, exists);
  }

  // Team book
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    EXPECT_EQ (false, database_modifications.teamDiffExists ("phpunit", 2, 1));
    EXPECT_EQ (false, database_modifications.teamDiffExists ("phpunit", 2, 2));
    EXPECT_EQ (false, database_modifications.teamDiffExists ("phpunit", 2, 3));
    
    bible_logic::store_chapter ("phpunit", 2, 1, "chapter text");
    bible_logic::store_chapter ("phpunit", 2, 2, "chapter text");
    bible_logic::store_chapter ("phpunit", 2, 3, "chapter text");
    
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 2, 1));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 2, 2));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 2, 3));
    
    database_modifications.truncateTeams ();
    
    // Test function storeTeamDiffBook.
    database_modifications.storeTeamDiffBook ("phpunit", 2);
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 2, 1));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 2, 2));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 2, 3));
  }

  // Team Bible
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    EXPECT_EQ (false, database_modifications.teamDiffExists ("phpunit", 3, 1));
    EXPECT_EQ (false, database_modifications.teamDiffExists ("phpunit", 4, 1));
    EXPECT_EQ (false, database_modifications.teamDiffExists ("phpunit", 5, 1));
    
    bible_logic::store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic::store_chapter ("phpunit", 4, 1, "chapter text");
    bible_logic::store_chapter ("phpunit", 5, 1, "chapter text");
    
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 3, 1));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 4, 1));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 5, 1));
    
    database_modifications.truncateTeams ();
    
    // Test function storeTeamDiffBible. It stores diff data for the whole bible.
    database_modifications.storeTeamDiffBible ("phpunit");
    
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 3, 1));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 4, 1));
    EXPECT_EQ (true, database_modifications.teamDiffExists ("phpunit", 5, 1));
  }

  // Team get diff
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    string diff = database_modifications.getTeamDiff ("phpunit", 1, 1);
    EXPECT_EQ ("", diff);
    
    bible_logic::store_chapter ("phpunit", 3, 1, "chapter text");
    database_modifications.truncateTeams ();
    bible_logic::store_chapter ("phpunit", 3, 1, "longer chapter text");
    diff = database_modifications.getTeamDiff ("phpunit", 3, 1);
    EXPECT_EQ ("chapter text", diff);
    
    diff = database_modifications.getTeamDiff ("phpunit", 1, 2);
    EXPECT_EQ ("", diff);
    
    bible_logic::store_chapter ("phpunit", 5, 5, "chapter text");
    database_modifications.truncateTeams ();
    bible_logic::store_chapter ("phpunit", 5, 5, "longer chapter text");
    diff = database_modifications.getTeamDiff ("phpunit", 5, 5);
    EXPECT_EQ ("chapter text", diff);
  }

  // Team get diff chapters
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    bible_logic::store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic::store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic::store_chapter ("phpunit", 3, 5, "chapter text");
    
    vector <int> chapters = database_modifications.getTeamDiffChapters ("phpunit", 1);
    EXPECT_EQ (vector <int>{}, chapters);
    
    chapters = database_modifications.getTeamDiffChapters ("phpunit", 3);
    EXPECT_EQ ((vector <int>{1, 3, 5}), chapters);
    
    chapters = database_modifications.getTeamDiffChapters ("phpunit2", 3);
    EXPECT_EQ (vector <int>{}, chapters);
  }

  // Team diff Bible
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    bible_logic::store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic::store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic::store_chapter ("phpunit", 3, 5, "chapter text");
    
    vector <string> bibles = database_modifications.getTeamDiffBibles ();
    EXPECT_EQ (vector <string>{"phpunit"}, bibles);
    
    database_modifications.deleteTeamDiffBible ("phpunit2");
    
    bibles = database_modifications.getTeamDiffBibles ();
    EXPECT_EQ (vector <string>{"phpunit"}, bibles);
    
    database_modifications.deleteTeamDiffBible ("phpunit");
    
    bibles = database_modifications.getTeamDiffBibles ();
    EXPECT_EQ (vector <string>{}, bibles);
  }

  // Team diff chapter.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    bible_logic::store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic::store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic::store_chapter ("phpunit", 3, 5, "chapter text");
    
    vector <int> chapters = database_modifications.getTeamDiffChapters ("phpunit", 3);
    EXPECT_EQ ((vector <int>{1, 3, 5}), chapters);
    
    database_modifications.deleteTeamDiffChapter ("phpunit", 3, 1);
    
    chapters = database_modifications.getTeamDiffChapters ("phpunit", 3);
    EXPECT_EQ ((vector <int>{3, 5}), chapters);
  }

  // Team diff book
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    bible_logic::store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic::store_chapter ("phpunit", 3, 3, "chapter text");
    bible_logic::store_chapter ("phpunit", 4, 5, "chapter text");
    
    vector <int> books = database_modifications.getTeamDiffBooks ("phpunit");
    EXPECT_EQ ((vector <int>{3, 4}), books);
    
    books = database_modifications.getTeamDiffBooks ("phpunit2");
    EXPECT_EQ (vector <int>{}, books);
  }

  // Get team diff count
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    Database_State::create ();
    Database_Bibles database_bibles;
    database_bibles.create_bible ("phpunit");
    database_bibles.create_bible ("phpunit2");
    
    bible_logic::store_chapter ("phpunit", 3, 1, "chapter text");
    bible_logic::store_chapter ("phpunit2", 3, 3, "chapter text");
    bible_logic::store_chapter ("phpunit2", 3, 5, "chapter text");
    
    int count = database_modifications.getTeamDiffCount ("phpunit");
    EXPECT_EQ (1, count);
    
    count = database_modifications.getTeamDiffCount ("phpunit2");
    EXPECT_EQ (2, count);
    
    count = database_modifications.getTeamDiffCount ("phpunit3");
    EXPECT_EQ (0, count);
  }
}


TEST (database, modifications_notifications)
{
  string any_user = "";
  string any_bible = "";
  
  // Basics.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    vector <int> ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    for (auto id : ids) {
      database_modifications.deleteNotification (id);
    }
  }

  // Trim.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    
    // Record two entries.
    database_modifications.recordNotification ({"phpunit1", "phpunit2"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ ((vector <int>{1, 2}), ids);
    
    // After trimming the two entries should still be there.
    database_modifications.indexTrimAllNotifications ();
    ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ ((vector <int>{1, 2}), ids);
    
    // Set the time back, re-index, filter::strings::trim, and check one entry's gone.
    database_modifications.indexTrimAllNotifications ();
    database_modifications.notificationUpdateTime (1, filter::date::seconds_since_epoch () - 7776001);
    database_modifications.indexTrimAllNotifications ();
    ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ (vector <int>{2}, ids);
  }

  // Next identifier.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    int identifier = database_modifications.getNextAvailableNotificationIdentifier ();
    EXPECT_EQ (1, identifier);
    database_modifications.recordNotification ({"phpunit1"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    identifier = database_modifications.getNextAvailableNotificationIdentifier ();
    EXPECT_EQ (2, identifier);
    database_modifications.deleteNotification (1);
    identifier = database_modifications.getNextAvailableNotificationIdentifier ();
    EXPECT_EQ (1, identifier);
  }

  // Record details retrieval.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    
    // Start with no identifiers.
    vector <int> ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ (vector <int>{}, ids);
    
    // Record three notifications and reindex.
    database_modifications.recordNotification ({"phpunit1", "phpunit2"}, "C", "bible1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.recordNotification ({"phpunit2", "phpunit3"}, "B", "bible2", 4, 5, 6, "old2", "mod2", "new2");
    database_modifications.recordNotification ({"phpunit3", "phpunit4"}, changes_bible_category (), "bible1", 7, 8, 9, "old3", "mod3", "new3");
    database_modifications.indexTrimAllNotifications ();
    
    // There should be six notifications now: Two users per recordNotification call.
    ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ ((vector <int>{1, 2, 3, 4, 5, 6}), ids);

    // Test sorting the notifications on category.
    ids = database_modifications.getNotificationIdentifiers (any_user, any_bible, true);
    EXPECT_EQ ((vector <int>{3, 4, 5, 6, 1, 2}), ids);

    // Test notifications per user.
    ids = database_modifications.getNotificationIdentifiers ("phpunit1", "");
    EXPECT_EQ (vector <int>{1}, ids);
    ids = database_modifications.getNotificationIdentifiers ("phpunit3", "");
    EXPECT_EQ ((vector <int>{4, 5}), ids);
    
    // Test notifications per Bible.
    ids = database_modifications.getNotificationIdentifiers ("", "bible1");
    EXPECT_EQ ((vector <int>{1, 2, 5, 6}), ids);
    ids = database_modifications.getNotificationIdentifiers ("", "bible2");
    EXPECT_EQ ((vector <int>{3, 4}), ids);
    
    // Test distinct Bibles.
    vector <string> bibles = database_modifications.getNotificationDistinctBibles ();
    EXPECT_EQ ((vector <string>{"bible1", "bible2"}), bibles);
    bibles = database_modifications.getNotificationDistinctBibles ("phpunit5");
    EXPECT_EQ (vector <string>{}, bibles);
    bibles = database_modifications.getNotificationDistinctBibles ("phpunit1");
    EXPECT_EQ (vector <string>{"bible1"}, bibles);
  }

  // Timestamps
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    
    int timestamp = database_modifications.getNotificationTimeStamp (0);
    int currenttime = filter::date::seconds_since_epoch ();
    if ((timestamp < currenttime) || (timestamp > currenttime + 1)) EXPECT_EQ (currenttime, timestamp);
    
    int time = filter::date::seconds_since_epoch () - 21600;
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    timestamp = database_modifications.getNotificationTimeStamp (1);
    if ((timestamp < time) || (timestamp > time + 1)) EXPECT_EQ (time, timestamp);
  }

  // Category.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string category = database_modifications.getNotificationCategory (1);
    EXPECT_EQ ("A", category);
    category = database_modifications.getNotificationCategory (2);
    EXPECT_EQ ("", category);
  }

  // Bible.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string bible = database_modifications.getNotificationBible (1);
    EXPECT_EQ ("1", bible);
    bible = database_modifications.getNotificationBible (3);
    EXPECT_EQ ("", bible);
  }

  // Passage.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    Passage passage = database_modifications.getNotificationPassage (1);
    EXPECT_EQ (1, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("3", passage.m_verse);
    passage = database_modifications.getNotificationPassage (3);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
  }
  
  // Old text.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string old = database_modifications.getNotificationOldText (1);
    EXPECT_EQ ("old1", old);
    old = database_modifications.getNotificationOldText (3);
    EXPECT_EQ ("", old);
  }

  // Modification.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string modification = database_modifications.getNotificationModification (1);
    EXPECT_EQ ("mod1", modification);
    modification = database_modifications.getNotificationOldText (3);
    EXPECT_EQ ("", modification);
  }

  // New text.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    string newtext = database_modifications.getNotificationNewText (1);
    EXPECT_EQ ("new1", newtext);
    newtext = database_modifications.getNotificationNewText (3);
    EXPECT_EQ ("", newtext);
  }

  // Clear user.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit1", "phpunit2", "phpunit3"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ (3, static_cast <int>(ids.size ()));
    
    database_modifications.clearNotificationsUser ("phpunit2");
    
    ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ (2, static_cast <int>(ids.size ()));
    
    ids = database_modifications.getNotificationIdentifiers ("phpunit2", "");
    EXPECT_EQ (0, static_cast <int>(ids.size ()));
  }

  // Clear matches one.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit"}, changes_personal_category (), "1", 2, 3, 4, "old1", "mod1", "new1");
    database_modifications.recordNotification ({"phpunit"}, "T", "1", 2, 3, 4, "old1", "mod1", "new1");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ (2, static_cast <int>(ids.size ()));
    database_modifications.clearNotificationMatches ("phpunit", changes_personal_category (), "T");
    database_modifications.indexTrimAllNotifications ();
    ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ (0, static_cast <int>(ids.size ()));
  }

  // Notification team identifiers.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.recordNotification ({"phpunit1", "phpunit2"}, "A", "1", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.recordNotification ({"phpunit2", "phpunit1"}, changes_bible_category (), "1", 1, 2, 3, "old2", "mod2", "new2");
    database_modifications.recordNotification ({"phpunit3", "phpunit4"}, changes_bible_category (), "1", 7, 8, 9, "old3", "mod3", "new3");
    database_modifications.indexTrimAllNotifications ();
    vector <int> ids = database_modifications.getNotificationTeamIdentifiers ("phpunit1", "A");
    EXPECT_EQ (vector <int>{1}, ids);
    ids = database_modifications.getNotificationTeamIdentifiers ("phpunit1", changes_bible_category ());
    EXPECT_EQ (vector <int>{4}, ids);
    ids = database_modifications.getNotificationTeamIdentifiers ("phpunit1", changes_bible_category (), "1");
    EXPECT_EQ (vector <int>{4}, ids);
    ids = database_modifications.getNotificationTeamIdentifiers ("phpunit1", changes_bible_category (), "2");
    EXPECT_EQ (vector <int>{}, ids);
  }

  // Record on client.
  {
    refresh_sandbox (true);
    Database_Modifications database_modifications;
    database_modifications.create ();
    database_modifications.storeClientNotification (3, "phpunit", "A", "bible", 1, 2, 3, "old1", "mod1", "new1");
    database_modifications.storeClientNotification (5, "phpunit", "A", "bible", 1, 2, 3, "old1", "mod1", "new1");
    vector <int> ids = database_modifications.getNotificationIdentifiers (any_user, any_bible);
    EXPECT_EQ ((vector <int>{3, 5}), ids);
  }
}


#endif
