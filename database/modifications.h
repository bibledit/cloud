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


#pragma once

#include <config/libraries.h>
#include <filter/passage.h>


namespace database::modifications {

struct id_bundle
{
  int oldid {0};
  int newid {0};
};

struct text_bundle
{
  std::string oldtext {};
  std::string newtext {};
};

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
std::vector <id_bundle> getUserIdentifiers (const std::string& username, const std::string& bible, int book, int chapter);
text_bundle getUserChapter (const std::string& username, const std::string& bible, int book, int chapter, int newID);
int getUserTimestamp (const std::string& username, const std::string& bible, int book, int chapter, int newID);
int getNextAvailableNotificationIdentifier ();
void recordNotification (const std::vector <std::string> & users, const std::string& category, const std::string& bible, int book, int chapter, int verse, const std::string& oldtext, const std::string& modification, const std::string& newtext);
void indexTrimAllNotifications ();
std::vector <int> getNotificationIdentifiers (std::string username, std::string bible, bool sort_on_category = false);
std::vector <int> getNotificationTeamIdentifiers (const std::string& username, const std::string& category, std::string bible = "");
std::vector <std::string> getNotificationDistinctBibles (std::string username = std::string());
void deleteNotification (int identifier);
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


}



