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


#ifndef INCLUDED_DATABASE_MODIFICATIONS_H
#define INCLUDED_DATABASE_MODIFICATIONS_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Modifications_Id
{
public:
  int oldid;
  int newid;
};


class Database_Modifications_Text
{
public:
  string oldtext;
  string newtext;
};


class Database_Modifications
{
public:
  void erase ();
  void create ();
  bool healthy ();
  void vacuum ();
  bool teamDiffExists (const string& bible, int book, int chapter);
  void storeTeamDiff (const string& bible, int book, int chapter);
  string getTeamDiff (const string& bible, int book, int chapter);
  void storeTeamDiffBook (const string& bible, int book);
  void storeTeamDiffBible (const string& bible);
  void deleteTeamDiffBible (const string& bible);
  void deleteTeamDiffChapter (const string& bible, int book, int chapter);
  vector <int> getTeamDiffChapters (const string& bible, int book);
  int getTeamDiffCount (const string& bible);
  vector <int> getTeamDiffBooks (const string& bible);
  vector <string> getTeamDiffBibles ();
  void truncateTeams ();
  void recordUserSave (const string& username, const string& bible, int book, int chapter, int oldID, const string& oldText, int newID, const string& newText);
  void clearUserUser (const string& username);
  vector <string> getUserUsernames ();
  vector <string> getUserBibles (const string& username);
  vector <int> getUserBooks (const string& username, const string& bible);
  vector <int> getUserChapters (const string& username, const string& bible, int book);
  vector <Database_Modifications_Id> getUserIdentifiers (const string& username, const string& bible, int book, int chapter);
  Database_Modifications_Text getUserChapter (const string& username, const string& bible, int book, int chapter, int newID);
  int getUserTimestamp (const string& username, const string& bible, int book, int chapter, int newID);
  int getNextAvailableNotificationIdentifier ();
  void recordNotification (const vector <string> & users, const string& category, const string& bible, int book, int chapter, int verse, const string& oldtext, const string& modification, const string& newtext);
  void indexTrimAllNotifications ();
  vector <int> getNotificationIdentifiers (const string& username = "", bool limit = false);
  vector <int> getNotificationPersonalIdentifiers (const string& username, const string& category, bool limit = false);
  vector <int> getNotificationTeamIdentifiers (const string& username, const string& category, bool limit = false);
  void deleteNotification (int identifier, sqlite3 * db = NULL);
  int getNotificationTimeStamp (int id);
  string getNotificationCategory (int id);
  string getNotificationBible (int id);
  Passage getNotificationPassage (int id);
  string getNotificationOldText (int id);
  string getNotificationModification (int id);
  string getNotificationNewText (int id);
  void clearNotificationsUser (const string& username);
  vector <int> clearNotificationMatches (const string& username, const string& personal, const string& team);
  void storeClientNotification (int id, string username, string category, string bible, int book, int chapter, int verse, string oldtext, string modification, string newtext);
  void notificationUpdateTime (int identifier, int timestamp);
  vector <string> getCategories ();
private:
  const char * filename ();
  sqlite3 * connect ();
  string teamFolder ();
  string teamFile (const string& bible, int book, int chapter);
  string userMainFolder ();
  string userUserFolder (const string& username);
  string userBibleFolder (const string& username, const string& bible);
  string userBookFolder (const string& username, const string& bible, int book);
  string userChapterFolder (const string& username, const string& bible, int book, int chapter);
  string userNewIDFolder (const string& username, const string& bible, int book, int chapter, int newID);
  string userTimeFile (const string& username, const string& bible, int book, int chapter, int newID);
  string userOldIDFile (const string& username, const string& bible, int book, int chapter, int newID);
  string userOldTextFile (const string& username, const string& bible, int book, int chapter, int newID);
  string userNewTextFile (const string& username, const string& bible, int book, int chapter, int newID);
  string notificationsMainFolder ();
  string notificationIdentifierDatabase (int identifier);
  const char * createNotificationsDbSql ();
  void deleteNotificationFile (int identifier);
};


#endif
