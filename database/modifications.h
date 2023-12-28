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


#pragma once

#include <config/libraries.h>
#include <filter/passage.h>

struct Database_Modifications_Id
{
  int oldid {0};
  int newid {0};
};

struct Database_Modifications_Text
{
  std::string oldtext {};
  std::string newtext {};
};

class Database_Modifications
{
public:
  void erase ();
  void create ();
  bool healthy ();
  void vacuum ();
  bool teamDiffExists (const std::string& bible, int book, int chapter);
  void storeTeamDiff (const std::string& bible, int book, int chapter);
  std::string getTeamDiff (const std::string& bible, int book, int chapter);
  void storeTeamDiffBook (const std::string& bible, int book);
  void storeTeamDiffBible (const std::string& bible);
  void deleteTeamDiffBible (const std::string& bible);
  void deleteTeamDiffChapter (const std::string& bible, int book, int chapter);
  std::vector <int> getTeamDiffChapters (const std::string& bible, int book);
  int getTeamDiffCount (const std::string& bible);
  std::vector <int> getTeamDiffBooks (const std::string& bible);
  std::vector <std::string> getTeamDiffBibles ();
  void truncateTeams ();
  void recordUserSave (const std::string& username, const std::string& bible, int book, int chapter, int oldID, const std::string& oldText, int newID, const std::string& newText);
  void clearUserUser (const std::string& username);
  std::vector <std::string> getUserUsernames ();
  std::vector <std::string> getUserBibles (const std::string& username);
  std::vector <int> getUserBooks (const std::string& username, const std::string& bible);
  std::vector <int> getUserChapters (const std::string& username, const std::string& bible, int book);
  std::vector <Database_Modifications_Id> getUserIdentifiers (const std::string& username, const std::string& bible, int book, int chapter);
  Database_Modifications_Text getUserChapter (const std::string& username, const std::string& bible, int book, int chapter, int newID);
  int getUserTimestamp (const std::string& username, const std::string& bible, int book, int chapter, int newID);
  int getNextAvailableNotificationIdentifier ();
  void recordNotification (const std::vector <std::string> & users, const std::string& category, const std::string& bible, int book, int chapter, int verse, const std::string& oldtext, const std::string& modification, const std::string& newtext);
  void indexTrimAllNotifications ();
  std::vector <int> getNotificationIdentifiers (std::string username, std::string bible, bool sort_on_category = false);
  std::vector <int> getNotificationTeamIdentifiers (const std::string& username, const std::string& category, std::string bible = "");
  std::vector <std::string> getNotificationDistinctBibles (std::string username = std::string());
  void deleteNotification (int identifier, sqlite3 * db = nullptr);
  int getNotificationTimeStamp (int id);
  std::string getNotificationCategory (int id);
  std::string getNotificationBible (int id);
  Passage getNotificationPassage (int id);
  std::string getNotificationOldText (int id);
  std::string getNotificationModification (int id);
  std::string getNotificationNewText (int id);
  int clearNotificationsUser (const std::string& username);
  std::vector <int> clearNotificationMatches (std::string username, std::string personal, std::string team, std::string bible = "");
  void storeClientNotification (int id, std::string username, std::string category, std::string bible, int book, int chapter, int verse, std::string oldtext, std::string modification, std::string newtext);
  void notificationUpdateTime (int identifier, int timestamp);
  std::vector <std::string> getCategories ();
private:
  const char * filename ();
  sqlite3 * connect ();
  std::string teamFolder ();
  std::string teamFile (const std::string& bible, int book, int chapter);
  std::string userMainFolder ();
  std::string userUserFolder (const std::string& username);
  std::string userBibleFolder (const std::string& username, const std::string& bible);
  std::string userBookFolder (const std::string& username, const std::string& bible, int book);
  std::string userChapterFolder (const std::string& username, const std::string& bible, int book, int chapter);
  std::string userNewIDFolder (const std::string& username, const std::string& bible, int book, int chapter, int newID);
  std::string userTimeFile (const std::string& username, const std::string& bible, int book, int chapter, int newID);
  std::string userOldIDFile (const std::string& username, const std::string& bible, int book, int chapter, int newID);
  std::string userOldTextFile (const std::string& username, const std::string& bible, int book, int chapter, int newID);
  std::string userNewTextFile (const std::string& username, const std::string& bible, int book, int chapter, int newID);
  std::string notificationsMainFolder ();
  std::string notificationIdentifierDatabase (int identifier);
  const char * createNotificationsDbSql ();
  void deleteNotificationFile (int identifier);
};
