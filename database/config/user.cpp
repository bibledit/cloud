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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <database/config/user.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <session/logic.h>
#include <database/users.h>
#include <database/styles.h>
#include <webserver/request.h>
#include <demo/logic.h>
#include <styles/logic.h>
#include <filter/date.h>
#include <database/logic.h>
#include <database/config/general.h>
#include <locale/logic.h>
#include <config/logic.h>
#include <algorithm>


Database_Config_User::Database_Config_User (void * webserver_request_in)
{
  webserver_request = webserver_request_in;
}


// Cache values in memory for better speed.
// The speed improvement comes from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
map <string, string> database_config_user_cache;


// Functions for getting and setting values or lists of values follow here:


string Database_Config_User::file (string user)
{
  return filter_url_create_root_path (database_logic_databases (), "config", "user", user);
}


string Database_Config_User::file (string user, const char * key)
{
  return filter_url_create_path (file (user), key);
}


// The key in the cache for this setting.
string Database_Config_User::mapkey (string user, const char * key)
{
  return user + key;
}


string Database_Config_User::getValue (const char * key, const char * default_value)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic ()->currentUser ();
  return getValueForUser (user, key, default_value);
}


bool Database_Config_User::getBValue (const char * key, bool default_value)
{
  string value = getValue (key, convert_to_string (default_value).c_str());
  return convert_to_bool (value);
}


int Database_Config_User::getIValue (const char * key, int default_value)
{
  string value = getValue (key, convert_to_string (default_value).c_str());
  return convert_to_int (value);
}


string Database_Config_User::getValueForUser (string user, const char * key, const char * default_value)
{
  // Check the memory cache.
  string cachekey = mapkey (user, key);
  if (database_config_user_cache.count (cachekey)) {
    return database_config_user_cache [cachekey];
  }
  // Read from file.
  string value;
  string filename = file (user, key);
  if (file_or_dir_exists (filename)) value = filter_url_file_get_contents (filename);
  else value = default_value;
  // Cache it.
  database_config_user_cache [cachekey] = value;
  // Done.
  return value;
}


bool Database_Config_User::getBValueForUser (string user, const char * key, bool default_value)
{
  string value = getValueForUser (user, key, convert_to_string (default_value).c_str());
  return convert_to_bool (value);
}


int Database_Config_User::getIValueForUser (string user, const char * key, int default_value)
{
  string value = getValueForUser (user, key, convert_to_string (default_value).c_str());
  return convert_to_int (value);
}


void Database_Config_User::setValue (const char * key, string value)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic ()->currentUser ();
  setValueForUser (user, key, value);
}


void Database_Config_User::setBValue (const char * key, bool value)
{
  setValue (key, convert_to_string (value));
}


void Database_Config_User::setIValue (const char * key, int value)
{
  setValue (key, convert_to_string (value));
}


void Database_Config_User::setValueForUser (string user, const char * key, string value)
{
  // Store in memory cache.
  database_config_user_cache [mapkey (user, key)] = value;
  // Store on disk.
  string filename = file (user, key);
  string directory = filter_url_dirname (filename);
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  filter_url_file_put_contents (filename, value);
}


void Database_Config_User::setBValueForUser (string user, const char * key, bool value)
{
  setValueForUser (user, key, convert_to_string (value));
}


vector <string> Database_Config_User::getList (const char * key)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic ()->currentUser ();
  return getListForUser (user, key);
}


vector <string> Database_Config_User::getListForUser (string user, const char * key)
{
  // Check whether value is in cache.
  string cachekey = mapkey (user, key);
  if (database_config_user_cache.count (cachekey)) {
    string value = database_config_user_cache [cachekey];
    return filter_string_explode (value, '\n');
  }
  // Read setting from disk.
  string filename = file (user, key);
  if (file_or_dir_exists (filename)) {
    string value = filter_url_file_get_contents (filename);
    // Cache it in memory.
    database_config_user_cache [cachekey] = value;
    // Done.
    return filter_string_explode (value, '\n');
  }
  // Empty value.
  return {};
}


void Database_Config_User::setList (const char * key, vector <string> values)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string user = request->session_logic ()->currentUser ();
  setListForUser (user, key, values);
}


void Database_Config_User::setListForUser (string user, const char * key, vector <string> values)
{
  // Store it on disk.
  string filename = file (user, key);
  string directory = filter_url_dirname (filename);
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);
  string value = filter_string_implode (values, "\n");
  filter_url_file_put_contents (filename, value);
  // Put it in the memory cache.
  string cachekey = mapkey (user, key);
  database_config_user_cache [cachekey] = value;
}


vector <int> Database_Config_User::getIList (const char * key)
{
  vector <string> lines = getList (key);
  vector <int> result;
  for (auto & line : lines) {
    result.push_back (convert_to_int (line));
  }
  return result;
}


void Database_Config_User::setIList (const char * key, vector <int> values)
{
  vector <string> lines;
  for (auto & value : values) {
    lines.push_back (convert_to_string (value));
  }
  setList (key, lines);
}


void Database_Config_User::trim ()
{
  // Reset the sprint month and year after some time.
  // When a user visits the Sprint page after a few days, it will then display the current Sprint.
  // If the Sprint is not reset, the user may enter new tasks in the wrong sprint.
  int time = filter_date_seconds_since_epoch () - (2 * 24 * 3600);
  Database_Users database_users;
  vector <string> users = database_users.get_users ();
  for (unsigned int i = 0; i < users.size(); i++) {
    string filename = file (users[i], keySprintMonth ());
    if (file_or_dir_exists (filename)) {
      if (filter_url_file_modification_time (filename) < time) {
        // Remove from disk.
        filter_url_unlink (filename);
        filename = file (users[i], keySprintYear ());
        filter_url_unlink (filename);
        // Clear cache.
        database_config_user_cache.clear ();
      }
    }
  }
}


// Remove any configuration setting of $username.
void Database_Config_User::remove (string username)
{
  // Remove from disk.
  string folder = file (username);
  filter_url_rmdir (folder);
  // Clear cache.
  database_config_user_cache.clear ();
}


// Clear the settings cache.
void Database_Config_User::clear_cache ()
{
  database_config_user_cache.clear ();
}


// Named configuration functions.


string Database_Config_User::getBible ()
{
  string bible = getValue ("bible", "");
  // If the Bible does not exist, take the first one available.
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Bibles * database_bibles = request->database_bibles ();
  vector <string> bibles = database_bibles->getBibles ();
  if (find (bibles.begin (), bibles.end (), bible) == bibles.end ()) {
    // There may not even be a first Bible: Create sample Bible.
    if (bibles.empty ()) {
      bible = demo_sample_bible_name ();
      demo_create_sample_bible ();
      database_bibles->createBible (bible);
    } else {
      bible = bibles [0];
    }
    setBible (bible);
  }
  return bible;
}
void Database_Config_User::setBible (string bible)
{
  setValue ("bible", bible);
}


bool Database_Config_User::getSubscribeToConsultationNotesEditedByMe ()
{
  return getBValue ("subscribe-to-consultation-notes-edited-by-me", false);
}
void Database_Config_User::setSubscribeToConsultationNotesEditedByMe (bool value)
{
  setBValue ("subscribe-to-consultation-notes-edited-by-me", value);
}


bool Database_Config_User::getNotifyMeOfAnyConsultationNotesEdits ()
{
  return getBValue ("notify-me-of-any-consultation-notes-edits", false);
}
bool Database_Config_User::getNotifyUserOfAnyConsultationNotesEdits (string username)
{
  return getBValueForUser (username, "notify-me-of-any-consultation-notes-edits", false);
}
void Database_Config_User::setNotifyMeOfAnyConsultationNotesEdits (bool value){
  setBValue ("notify-me-of-any-consultation-notes-edits", value);
}


bool Database_Config_User::getSubscribedConsultationNoteNotification ()
{
  return getBValue ("subscribed-consultation-note-notification", true);
}
bool Database_Config_User::getUserSubscribedConsultationNoteNotification (string username)
{
  return getBValueForUser (username, "subscribed-consultation-note-notification", true);
}
void Database_Config_User::setSubscribedConsultationNoteNotification (bool value)
{
  setBValue ("subscribed-consultation-note-notification", value);
}


bool Database_Config_User::getAssignedToConsultationNotesChanges ()
{
  return getBValue ("get-assigned-to-consultation-notes-changes", false);
}
bool Database_Config_User::getUserAssignedToConsultationNotesChanges (string username)
{
  return getBValueForUser (username, "get-assigned-to-consultation-notes-changes", false);
}
void Database_Config_User::setAssignedToConsultationNotesChanges (bool value)
{
  setBValue ("get-assigned-to-consultation-notes-changes", value);
}


bool Database_Config_User::getGenerateChangeNotifications ()
{
  return getBValue ("generate-change-notifications", false);
}
bool Database_Config_User::getUserGenerateChangeNotifications (string username)
{
  return getBValueForUser (username, "generate-change-notifications", false);
}
void Database_Config_User::setGenerateChangeNotifications (bool value)
{
  setBValue ("generate-change-notifications", value);
}


bool Database_Config_User::getAssignedConsultationNoteNotification ()
{
  return getBValue ("assigned-consultation-note-notification", true);
}
bool Database_Config_User::getUserAssignedConsultationNoteNotification (string username)
{
  return getBValueForUser (username, "assigned-consultation-note-notification", true);
}
void Database_Config_User::setAssignedConsultationNoteNotification (bool value)
{
  setBValue ("assigned-consultation-note-notification", value);
}


// 0: current verse; 1: current chapter; 2: current book; 3: any passage.
int Database_Config_User::getConsultationNotesPassageSelector ()
{
  // Default value is to select notes of the current chapter.
  // It used to be the current verse.
  // But that led to a situation where a user created a note,
  // navigated to another verse within the same chapter,
  // and then was confused because the user could not find the note just created.
  // With the updated selection, current chapter, this confusing situation does not occur.
  return getIValue ("consultation-notes-passage-selector", 1);
}
void Database_Config_User::setConsultationNotesPassageSelector (int value)
{
  setIValue ("consultation-notes-passage-selector", value);
}


// 0: any time; 1: last 30 days; 2: last 7 days; 3: since yesterday; 4: today.
int Database_Config_User::getConsultationNotesEditSelector ()
{
  return getIValue ("consultation-notes-edit-selector", 0);
}
void Database_Config_User::setConsultationNotesEditSelector (int value)
{
  setIValue ("consultation-notes-edit-selector", value);
}


// 0: don't care; 1: for last 30 days; 2: for last 7 days; 3: since yesterday; 4: today.
int Database_Config_User::getConsultationNotesNonEditSelector ()
{
  return getIValue ("consultation-notes-non-edit-selector", 0);
}
void Database_Config_User::setConsultationNotesNonEditSelector (int value)
{
  setIValue ("consultation-notes-non-edit-selector", value);
}


// Status is a string; can be empty as well.
string Database_Config_User::getConsultationNotesStatusSelector ()
{
  return getValue ("consultation-notes-status-selector", "");
}
void Database_Config_User::setConsultationNotesStatusSelector (string value)
{
  setValue ("consultation-notes-status-selector", value);
}


// "": any Bible; <bible>: named Bible.
string Database_Config_User::getConsultationNotesBibleSelector ()
{
  return getValue ("consultation-notes-bible-selector", "");
}
void Database_Config_User::setConsultationNotesBibleSelector (string value)
{
  setValue ("consultation-notes-bible-selector", value);
}


// "": don't care; "user": notes assigned to "user".
string Database_Config_User::getConsultationNotesAssignmentSelector ()
{
  return getValue ("consultation-notes-assignment-selector", "");
}
void Database_Config_User::setConsultationNotesAssignmentSelector (string value)
{
  setValue ("consultation-notes-assignment-selector", value);
}


// false: don't care; true: subscribed.
bool Database_Config_User::getConsultationNotesSubscriptionSelector ()
{
  return getBValue ("consultation-notes-subscription-selector", false);
}
void Database_Config_User::setConsultationNotesSubscriptionSelector (bool value)
{
  setBValue ("consultation-notes-subscription-selector", value);
}


int Database_Config_User::getConsultationNotesSeveritySelector ()
{
  return getIValue ("consultation-notes-severity-selector", -1);
}
void Database_Config_User::setConsultationNotesSeveritySelector (int value)
{
  setIValue ("consultation-notes-severity-selector", value);
}


int Database_Config_User::getConsultationNotesTextSelector ()
{
  return getIValue ("consultation-notes-text-selector", 0);
}
void Database_Config_User::setConsultationNotesTextSelector (int value)
{
  setIValue ("consultation-notes-text-selector", value);
}


string Database_Config_User::getConsultationNotesSearchText ()
{
  return getValue ("consultation-notes-search-text", "");
}
void Database_Config_User::setConsultationNotesSearchText (string value)
{
  setValue ("consultation-notes-search-text", value);
}


int Database_Config_User::getConsultationNotesPassageInclusionSelector ()
{
  return getIValue ("consultation-notes-passage-inclusion-selector", 0);
}
void Database_Config_User::setConsultationNotesPassageInclusionSelector (int value)
{
  setIValue ("consultation-notes-passage-inclusion-selector", value);
}


int Database_Config_User::getConsultationNotesTextInclusionSelector ()
{
  return getIValue ("consultation-notes-text-inclusion-selector", 0);
}
void Database_Config_User::setConsultationNotesTextInclusionSelector (int value)
{
  setIValue ("consultation-notes-text-inclusion-selector", value);
}


bool Database_Config_User::getBibleChangesNotification ()
{
  return getBValue ("bible-changes-notification", false);
}
bool Database_Config_User::getUserBibleChangesNotification (string username)
{
  return getBValueForUser (username, "bible-changes-notification", false);
}
void Database_Config_User::setBibleChangesNotification (bool value)
{
  setBValue ("bible-changes-notification", value);
}


bool Database_Config_User::getDeletedConsultationNoteNotification ()
{
  return getBValue ("deleted-consultation-note-notification", false);
}
bool Database_Config_User::getUserDeletedConsultationNoteNotification (string username)
{
  return getBValueForUser (username, "deleted-consultation-note-notification", false);
}
void Database_Config_User::setDeletedConsultationNoteNotification (bool value)
{
  setBValue ("deleted-consultation-note-notification", value);
}


bool Database_Config_User::defaultBibleChecksNotification ()
{
#ifdef HAVE_CLIENT
  return false;
#else
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  int level = request->session_logic ()->currentLevel ();
  return (level >= Filter_Roles::translator () && level <= Filter_Roles::manager ());
#endif
}
bool Database_Config_User::getBibleChecksNotification ()
{
  return getBValue ("bible-checks-notification", defaultBibleChecksNotification ());
}
bool Database_Config_User::getUserBibleChecksNotification (string username)
{
  return getBValueForUser (username, "bible-checks-notification", defaultBibleChecksNotification ());
}
void Database_Config_User::setBibleChecksNotification (bool value)
{
  setBValue ("bible-checks-notification", value);
}


bool Database_Config_User::getPendingChangesNotification ()
{
  return getBValue ("pending-changes-notification", false);
}
bool Database_Config_User::getUserPendingChangesNotification (string username)
{
  return getBValueForUser (username, "pending-changes-notification", false);
}
void Database_Config_User::setPendingChangesNotification (bool value)
{
  setBValue ("pending-changes-notification", value);
}


bool Database_Config_User::getUserChangesNotification ()
{
  return getBValue ("user-changes-notification", false);
}
bool Database_Config_User::getUserUserChangesNotification (string username)
{
  return getBValueForUser (username, "user-changes-notification", false);
}
void Database_Config_User::setUserChangesNotification (bool value)
{
  setBValue ("user-changes-notification", value);
}


bool Database_Config_User::getAssignedNotesStatisticsNotification ()
{
  return getBValue ("assigned-notes-statistics-notification", false);
}
bool Database_Config_User::getUserAssignedNotesStatisticsNotification (string username)
{
  return getBValueForUser (username, "assigned-notes-statistics-notification", false);
}
void Database_Config_User::setAssignedNotesStatisticsNotification (bool value)
{
  setBValue ("assigned-notes-statistics-notification", value);
}


bool Database_Config_User::getSubscribedNotesStatisticsNotification ()
{
  return getBValue ("subscribed-notes-statistics-notification", false);
}
bool Database_Config_User::getUserSubscribedNotesStatisticsNotification (string username)
{
  return getBValueForUser (username, "subscribed-notes-statistics-notification", false);
}
void Database_Config_User::setSubscribedNotesStatisticsNotification (bool value)
{
  setBValue ("subscribed-notes-statistics-notification", value);
}


bool Database_Config_User::getNotifyMeOfMyPosts ()
{
  return getBValue ("notify-me-of-my-posts", true);
}
bool Database_Config_User::getUserNotifyMeOfMyPosts (string username)
{
  return getBValueForUser (username, "notify-me-of-my-posts", true);
}
void Database_Config_User::setNotifyMeOfMyPosts (bool value)
{
  setBValue ("notify-me-of-my-posts", value);
}


bool Database_Config_User::getSuppressMailFromYourUpdatesNotes ()
{
  return getBValue ("suppress-mail-my-updated-notes", false);
}
bool Database_Config_User::getUserSuppressMailFromYourUpdatesNotes (string username)
{
  return getBValueForUser (username, "suppress-mail-my-updated-notes", false);
}
void Database_Config_User::setSuppressMailFromYourUpdatesNotes (bool value)
{
  setBValue ("suppress-mail-my-updated-notes", value);
}


vector <string> Database_Config_User::getActiveResources ()
{
  if (config_logic_indonesian_cloud_free ()) {
    // In the Indonesian Cloud free, there's one central location for storing the active resources.
    return Database_Config_General::getActiveResources ();
  }
  // Default values.
  return getList ("active-resources");
}
void Database_Config_User::setActiveResources (vector <string> values)
{
  if (config_logic_default_bibledit_configuration ()) {
    setList ("active-resources", values);
  }
  if (config_logic_indonesian_cloud_free ()) {
    Database_Config_General::setActiveResources (values);
  }
}


vector <string> Database_Config_User::getConsistencyResources ()
{
  return getList ("consistency-bibles");
}
void Database_Config_User::setConsistencyResources (vector <string> values)
{
  setList ("consistency-bibles", values);
}


const char * Database_Config_User::keySprintMonth ()
{
  return "sprint-month";
}
int Database_Config_User::getSprintMonth ()
{
  return getIValue (keySprintMonth (), filter_date_numerical_month (filter_date_seconds_since_epoch ()));
}
void Database_Config_User::setSprintMonth (int value)
{
  setIValue (keySprintMonth (), value);
}


const char * Database_Config_User::keySprintYear ()
{
  return "sprint-year";
}
int Database_Config_User::getSprintYear ()
{
  return getIValue (keySprintYear (), filter_date_numerical_year (filter_date_seconds_since_epoch ()));
}
void Database_Config_User::setSprintYear (int value)
{
  setIValue (keySprintYear (), value);
}


bool Database_Config_User::getSprintProgressNotification ()
{
  return getBValue ("sprint-progress-notification", false);
}
bool Database_Config_User::getUserSprintProgressNotification (string username)
{
  return getBValueForUser (username, "sprint-progress-notification", false);
}
void Database_Config_User::setSprintProgressNotification (bool value)
{
  setBValue ("sprint-progress-notification", value);
}


bool Database_Config_User::getUserChangesNotificationsOnline ()
{
  return getBValue ("user-changes-notifications-online", false);
}
bool Database_Config_User::getUserUserChangesNotificationsOnline (string username)
{
  return getBValueForUser (username, "user-changes-notifications-online", false);
}
void Database_Config_User::setUserChangesNotificationsOnline (bool value)
{
  setBValue ("user-changes-notifications-online", value);
}


bool Database_Config_User::getContributorChangesNotificationsOnline ()
{
  return getBValue ("contributor-changes-notifications-online", false);
}
bool Database_Config_User::getContributorChangesNotificationsOnline (string username)
{
  return getBValueForUser (username, "contributor-changes-notifications-online", false);
}
void Database_Config_User::setContributorChangesNotificationsOnline (bool value)
{
  setBValue ("contributor-changes-notifications-online", value);
}


string Database_Config_User::getWorkspaceURLs ()
{
  return getValue ("workbench-urls", "");
}
void Database_Config_User::setWorkspaceURLs (string value)
{
  setValue ("workbench-urls", value);
}


string Database_Config_User::getWorkspaceWidths ()
{
  return getValue ("workbench-widths", "");
}
void Database_Config_User::setWorkspaceWidths (string value)
{
  setValue ("workbench-widths", value);
}


string Database_Config_User::getWorkspaceHeights ()
{
  return getValue ("workbench-heights", "");
}
void Database_Config_User::setWorkspaceHeights (string value)
{
  setValue ("workbench-heights", value);
}


string Database_Config_User::getEntireWorkspaceWidths ()
{
  return getValue ("entire-workbench-widths", "");
}
void Database_Config_User::setEntireWorkspaceWidths (string value)
{
  setValue ("entire-workbench-widths", value);
}


string Database_Config_User::getActiveWorkspace ()
{
  return getValue ("active-workbench", "");
}
void Database_Config_User::setActiveWorkspace (string value)
{
  setValue ("active-workbench", value);
}


bool Database_Config_User::getPostponeNewNotesMails ()
{
  return getBValue ("postpone-new-notes-mails", false);
}
void Database_Config_User::setPostponeNewNotesMails (bool value)
{
  setBValue ("postpone-new-notes-mails", value);
}


string Database_Config_User::getRecentlyAppliedStyles ()
{
  return getValue ("recently-applied-styles", "p s add nd f x v");
}
void Database_Config_User::setRecentlyAppliedStyles (string values)
{
  setValue ("recently-applied-styles", values);
}


vector <string> Database_Config_User::getPrintResources ()
{
  return getList ("print-resources");
}
vector <string> Database_Config_User::getPrintResourcesForUser (string user)
{
  return getListForUser (user, "print-resources");
}
void Database_Config_User::setPrintResources (vector <string> values)
{
  setList ("print-resources", values);
}


Passage database_config_user_fix_passage (string value, const char * fallback)
{
  vector <string> values = filter_string_explode (value, '.');
  if (values.size () != 3) values = filter_string_explode (fallback, '.');
  Passage passage = Passage ("", convert_to_int (values[0]), convert_to_int (values[1]), values[2]);
  return passage;
}


Passage Database_Config_User::getPrintPassageFrom ()
{
  return database_config_user_fix_passage (getValue ("print-passage-from", ""), "1.1.1");
}
Passage Database_Config_User::getPrintPassageFromForUser (string user)
{
  return database_config_user_fix_passage (getValueForUser (user, "print-passage-from", ""), "1.1.1");
}
void Database_Config_User::setPrintPassageFrom (Passage value)
{
  string s = convert_to_string (value.book) + "." + convert_to_string (value.chapter) + "." + value.verse;
  setValue ("print-passage-from", s);
}


Passage Database_Config_User::getPrintPassageTo ()
{
  return database_config_user_fix_passage (getValue ("print-passage-to", ""), "1.1.31");
}
Passage Database_Config_User::getPrintPassageToForUser (string user)
{
  return database_config_user_fix_passage (getValueForUser (user, "print-passage-to", ""), "1.1.31");
}
void Database_Config_User::setPrintPassageTo (Passage value)
{
  string s = convert_to_string (value.book) + "." + convert_to_string (value.chapter) + "." + value.verse;
  setValue ("print-passage-to", s);
}


int Database_Config_User::getFocusedBook ()
{
  return getIValue ("focused-book", 1);
}
void Database_Config_User::setFocusedBook (int book)
{
  setIValue ("focused-book", book);
}


int Database_Config_User::getFocusedChapter ()
{
  return getIValue ("focused-chapter", 1);
}
void Database_Config_User::setFocusedChapter (int chapter)
{
  setIValue ("focused-chapter", chapter);
}


int Database_Config_User::getFocusedVerse ()
{
  return getIValue ("focused-verse", 1);
}
void Database_Config_User::setFocusedVerse (int verse)
{
  setIValue ("focused-verse", verse);
}


vector <int> Database_Config_User::getUpdatedSettings ()
{
  return getIList ("updated-settings");
}
void Database_Config_User::setUpdatedSettings (vector <int> values)
{
  setIList ("updated-settings", values);
}
void Database_Config_User::addUpdatedSetting (int value)
{
  vector <int> settings = getUpdatedSettings ();
  settings.push_back (value);
  settings = array_unique (settings);
  setUpdatedSettings (settings);
}
void Database_Config_User::removeUpdatedSetting (int value)
{
  vector <int> settings = getUpdatedSettings ();
  vector <int> against;
  against.push_back (value);
  settings = filter_string_array_diff (settings, against);
  setUpdatedSettings (settings);
}


vector <int> Database_Config_User::getRemovedChanges ()
{
  return getIList ("removed-changes");
}
void Database_Config_User::setRemovedChanges (vector <int> values)
{
  setIList ("removed-changes", values);
}
void Database_Config_User::addRemovedChange (int value)
{
  vector <int> settings = getRemovedChanges ();
  settings.push_back (value);
  settings = array_unique (settings);
  setRemovedChanges (settings);
}
void Database_Config_User::removeRemovedChange (int value)
{
  vector <int> settings = getRemovedChanges ();
  vector <int> against;
  against.push_back (value);
  settings = filter_string_array_diff (settings, against);
  setRemovedChanges (settings);
}


string Database_Config_User::getChangeNotificationsChecksum ()
{
  return getValue ("change-notifications-checksum", "");
}
void Database_Config_User::setChangeNotificationsChecksum (string value)
{
  setValue ("change-notifications-checksum", value);
}
void Database_Config_User::setUserChangeNotificationsChecksum (string user, string value)
{
  setValueForUser (user, "change-notifications-checksum", value);
}


int Database_Config_User::getLiveBibleEditor ()
{
  return getIValue ("live-bible-editor", 0);
}
void Database_Config_User::setLiveBibleEditor (int time)
{
  setIValue ("live-bible-editor", time);
}


int Database_Config_User::getResourceVersesBefore ()
{
  return getIValue ("resource-verses-before", 0);
}
void Database_Config_User::setResourceVersesBefore (int verses)
{
  setIValue ("resource-verses-before", verses);
}


int Database_Config_User::getResourceVersesAfter ()
{
  return getIValue ("resource-verses-after", 0);
}
void Database_Config_User::setResourceVersesAfter (int verses)
{
  setIValue ("resource-verses-after", verses);
}


// Encryption key storage on server.
string Database_Config_User::getSyncKey ()
{
  return getValue ("sync-key", "");
}
void Database_Config_User::setSyncKey (string key)
{
  setValue ("sync-key", key);
}



//const char * site_language_key ()
//{
//  return "site-language";
//}
//string Database_Config_User::getSiteLanguage ()
//{
//  // The default value is "default".
//  // That means: Take the system setting. The user has no language preference.
//  return getValue (site_language_key (), "");
//}
//void Database_Config_User::setSiteLanguage (string value)
//{
//  setValue (site_language_key (), value);
//}


const char * general_font_size_key ()
{
  return "general-font-size";
}
int Database_Config_User::getGeneralFontSize ()
{
  // Default value, see https://github.com/bibledit/cloud/issues/509
  return getIValue (general_font_size_key (), 112);
}
void Database_Config_User::setGeneralFontSize (int size)
{
  setIValue (general_font_size_key (), size);
}


const char * menu_font_size_key ()
{
  return "menu-font-size";
}
int Database_Config_User::getMenuFontSize ()
{
  // Default value, see https://github.com/bibledit/cloud/issues/509
  return getIValue (menu_font_size_key (), 112);
}
void Database_Config_User::setMenuFontSize (int size)
{
  setIValue (menu_font_size_key (), size);
}


int Database_Config_User::getBibleEditorsFontSize ()
{
  return getIValue ("bible-editors-font-size", 100);
}
void Database_Config_User::setBibleEditorsFontSize (int size)
{
  setIValue ("bible-editors-font-size", size);
}


int Database_Config_User::getResourcesFontSize ()
{
  return getIValue ("resources-font-size", 100);
}
void Database_Config_User::setResourcesFontSize (int size)
{
  setIValue ("resources-font-size", size);
}


int Database_Config_User::getHebrewFontSize ()
{
  return getIValue ("hebrew-font-size", 100);
}
void Database_Config_User::setHebrewFontSize (int size)
{
  setIValue ("hebrew-font-size", size);
}


int Database_Config_User::getGreekFontSize ()
{
  return getIValue ("greek-font-size", 100);
}
void Database_Config_User::setGreekFontSize (int size)
{
  setIValue ("greek-font-size", size);
}


const char * vertical_caret_position_key ()
{
  return "vertical-caret-position";
}
int Database_Config_User::getVerticalCaretPosition ()
{
  // Updated default value, see https://github.com/bibledit/cloud/issues/509
  return getIValue (vertical_caret_position_key (), 30);
}
void Database_Config_User::setVerticalCaretPosition (int position)
{
  setIValue (vertical_caret_position_key (), position);
}


const char * current_theme_style_key ()
{
  return "current-theme-style";
}
int Database_Config_User::getCurrentTheme ()
{
  return getIValue (current_theme_style_key (), 0);
}
void Database_Config_User::setCurrentTheme (int index)
{
  setIValue (current_theme_style_key (), index);
}


bool Database_Config_User::getDisplayBreadcrumbs ()
{
  return getBValue ("display-breadcrumbs", false);
}
void Database_Config_User::setDisplayBreadcrumbs (bool value)
{
  setBValue ("display-breadcrumbs", value);
}


int Database_Config_User::getWorkspaceMenuFadeoutDelay ()
{
  return getIValue ("workspace-menu-fadeout-delay", 4);
}
void Database_Config_User::setWorkspaceMenuFadeoutDelay (int value)
{
  setIValue ("workspace-menu-fadeout-delay", value);
}


const char * editing_allowed_difference_chapter_key ()
{
  return "editing-allowed-difference-chapter";
}
int Database_Config_User::getEditingAllowedDifferenceChapter ()
{
  return getIValue (editing_allowed_difference_chapter_key (), 20);
}
void Database_Config_User::setEditingAllowedDifferenceChapter (int value)
{
  setIValue (editing_allowed_difference_chapter_key (), value);
}


const char * editing_allowed_difference_verse_key ()
{
  return "editing-allowed-difference-verse";
}
int Database_Config_User::getEditingAllowedDifferenceVerse ()
{
  return getIValue (editing_allowed_difference_verse_key (), 75);
}
void Database_Config_User::setEditingAllowedDifferenceVerse (int value)
{
  setIValue (editing_allowed_difference_verse_key (), value);
}


bool Database_Config_User::getBasicInterfaceModeDefault ()
{
  // Touch devices default to basic mode.
#ifdef HAVE_ANDROID
  return true;
#endif
#ifdef HAVE_IOS
  return true;
#endif
  // The app running on a workspace or laptop have default to basic mode for a lower role.
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  int level = request->session_logic ()->currentLevel ();
  if (level <= Filter_Roles::manager ()) return true;
  // Higher role: default to advanced mode.
  return false;
}
bool Database_Config_User::getBasicInterfaceMode ()
{
  return getBValue ("basic-interface-mode", getBasicInterfaceModeDefault ());
}
void Database_Config_User::setBasicInterfaceMode (bool value)
{
  setBValue ("basic-interface-mode", value);
}


bool Database_Config_User::getMainMenuAlwaysVisible ()
{
  // Default visible in basic mode.
  // Advanced mode: By default it is not visible.
  return getBValue ("main-menu-always-visible", getBasicInterfaceModeDefault ());
}
void Database_Config_User::setMainMenuAlwaysVisible (bool value)
{
  setBValue ("main-menu-always-visible", value);
}


bool Database_Config_User::getSwipeActionsAvailable ()
{
  return getBValue ("swipe-actions-available", true);
}
void Database_Config_User::setSwipeActionsAvailable (bool value)
{
  setBValue ("swipe-actions-available", value);
}


bool Database_Config_User::getFastEditorSwitchingAvailable ()
{
  return getBValue ("fast-editor-switching-available", true);
}
void Database_Config_User::setFastEditorSwitchingAvailable (bool value)
{
  setBValue ("fast-editor-switching-available", value);
}


bool Database_Config_User::getIncludeRelatedPassages ()
{
  return getBValue ("include-related-passages", false);
}
void Database_Config_User::setIncludeRelatedPassages (bool value)
{
  setBValue ("include-related-passages", value);
}


const char * enabled_visual_editors_key ()
{
  return "enabled-visual-editors";
}
int Database_Config_User::getFastSwitchVisualEditors ()
{
  // Updated default values, see https://github.com/bibledit/cloud/issues/509
  return getIValue (enabled_visual_editors_key (), 0);
}
void Database_Config_User::setFastSwitchVisualEditors (int value)
{
  setIValue (enabled_visual_editors_key (), value);
}


int Database_Config_User::getFastSwitchUsfmEditors ()
{
  // Initially only the USFM chapter editor is enabled.
  return getIValue ("enabled-usfm-editors", 1);
}
void Database_Config_User::setFastSwitchUsfmEditors (int value)
{
  setIValue ("enabled-usfm-editors", value);
}


bool Database_Config_User::getEnableStylesButtonVisualEditors ()
{
  return getBValue ("enable-styles-button-visual-editors", true);
}
void Database_Config_User::setEnableStylesButtonVisualEditors (bool value)
{
  setBValue ("enable-styles-button-visual-editors", value);
}


bool Database_Config_User::getMenuChangesInBasicMode ()
{
  return getBValue ("menu-changes-in-basic-mode", false);
}
void Database_Config_User::setMenuChangesInBasicMode (bool value)
{
  setBValue ("menu-changes-in-basic-mode", value);
}


const char * privilege_use_advanced_mode_key ()
{
  return "privilege-use-advanced-mode";
}
bool Database_Config_User::getPrivilegeUseAdvancedMode ()
{
  return getBValue (privilege_use_advanced_mode_key (), true);
}
bool Database_Config_User::getPrivilegeUseAdvancedModeForUser (string username)
{
  return getBValueForUser (username, privilege_use_advanced_mode_key (), true);
}
void Database_Config_User::setPrivilegeUseAdvancedModeForUser (string username, bool value)
{
  setBValueForUser (username, privilege_use_advanced_mode_key (), value);
}


const char * privilege_delete_consultation_notes_key ()
{
  return "privilege-delete-consultation-notes";
}
bool Database_Config_User::getPrivilegeDeleteConsultationNotes ()
{
  return getBValue (privilege_delete_consultation_notes_key (), false);
}
void Database_Config_User::setPrivilegeDeleteConsultationNotes (bool value)
{
  setBValue (privilege_delete_consultation_notes_key (), value);
}
bool Database_Config_User::getPrivilegeDeleteConsultationNotesForUser (string username)
{
  return getBValueForUser (username, privilege_delete_consultation_notes_key (), false);
}
void Database_Config_User::setPrivilegeDeleteConsultationNotesForUser (string username, bool value)
{
  setBValueForUser (username, privilege_delete_consultation_notes_key (), value);
}


bool Database_Config_User::getDismissChangesAtTop ()
{
  return getBValue ("dismiss-changes-at-top", false);
}
void Database_Config_User::setDismissChangesAtTop (bool value)
{
  setBValue ("dismiss-changes-at-top", value);
}


const char * quick_note_edit_link_key ()
{
  return "quick-note-edit-link";
}
bool Database_Config_User::getQuickNoteEditLink ()
{
  return getBValue (quick_note_edit_link_key (), false);
}
void Database_Config_User::setQuickNoteEditLink (bool value)
{
  setBValue (quick_note_edit_link_key (), value);
}


const char * show_bible_in_notes_list_key ()
{
  return "show-bible-in-notes-list";
}
bool Database_Config_User::getShowBibleInNotesList ()
{
  return getBValue (show_bible_in_notes_list_key (), false);
}
void Database_Config_User::setShowBibleInNotesList (bool value)
{
  setBValue (show_bible_in_notes_list_key (), value);
}


const char * show_note_status_key ()
{
  return "show-note-status";
}
bool Database_Config_User::getShowNoteStatus ()
{
  return getBValue (show_note_status_key (), false);
}
void Database_Config_User::setShowNoteStatus (bool value)
{
  setBValue (show_note_status_key (), value);
}


const char * show_verse_text_at_create_note_key ()
{
  return "show-verse-text-at-create-note";
}
bool Database_Config_User::getShowVerseTextAtCreateNote ()
{
  return getBValue (show_verse_text_at_create_note_key (), false);
}
void Database_Config_User::setShowVerseTextAtCreateNote (bool value)
{
  setBValue (show_verse_text_at_create_note_key (), value);
}


const char * order_changes_by_author_key ()
{
  return "order-changes-by-author";
}
bool Database_Config_User::getOrderChangesByAuthor ()
{
  return getBValue (order_changes_by_author_key (), false);
}
void Database_Config_User::setOrderChangesByAuthor (bool value)
{
  setBValue (order_changes_by_author_key (), value);
}


const char * automatic_note_assignment_key ()
{
  return "automatic-note-assignment";
}
vector <string> Database_Config_User::getAutomaticNoteAssignment ()
{
  return getList (automatic_note_assignment_key ());
}
void Database_Config_User::setAutomaticNoteAssignment (vector <string> values)
{
  setList (automatic_note_assignment_key (), values);
}


const char * receive_focused_reference_from_paratext_key ()
{
  return "receive-focused-reference-from-paratext";
}
bool Database_Config_User::getReceiveFocusedReferenceFromParatext ()
{
  return getBValue (receive_focused_reference_from_paratext_key (), true);
}
void Database_Config_User::setReceiveFocusedReferenceFromParatext (bool value)
{
  setBValue (receive_focused_reference_from_paratext_key (), value);
}


const char * receive_focused_reference_from_accordance_key ()
{
  return "receive-focused-reference-from-accordance";
}
bool Database_Config_User::getReceiveFocusedReferenceFromAccordance ()
{
  return getBValue (receive_focused_reference_from_accordance_key (), true);
}
void Database_Config_User::setReceiveFocusedReferenceFromAccordance (bool value)
{
  setBValue (receive_focused_reference_from_accordance_key (), value);
}


const char * use_colored_note_status_labels_key ()
{
  return "use-colored-note-status-labels";
}
bool Database_Config_User::getUseColoredNoteStatusLabels ()
{
  return getBValue (use_colored_note_status_labels_key (), false);
}
void Database_Config_User::setUseColoredNoteStatusLabels (bool value)
{
  setBValue (use_colored_note_status_labels_key (), value);
}

