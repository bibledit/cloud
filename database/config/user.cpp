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


// Cache values in memory for better speed.
// The speed improvement comes from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
static std::map <std::string, std::string> database_config_user_cache;


Database_Config_User::Database_Config_User (Webserver_Request& webserver_request):
m_webserver_request (webserver_request)
{
}


// Functions for getting and setting values or lists of values follow here:


std::string Database_Config_User::file (const std::string& user) const
{
  return filter_url_create_root_path ({database_logic_databases (), "config", "user", user});
}


std::string Database_Config_User::file (const std::string& user, const char * key) const
{
  return filter_url_create_path ({file (user), key});
}


// The key in the cache for this setting.
std::string Database_Config_User::mapkey (const std::string& user, const char * key) const
{
  return user + key;
}


std::string Database_Config_User::getValue (const char * key, const char * default_value) const
{
  const std::string& user = m_webserver_request.session_logic ()->get_username ();
  return getValueForUser (user, key, default_value);
}


bool Database_Config_User::getBValue (const char * key, bool default_value) const
{
  const std::string value = getValue (key, filter::strings::convert_to_string (default_value).c_str());
  return filter::strings::convert_to_bool (value);
}


int Database_Config_User::getIValue (const char * key, int default_value) const
{
  const std::string value = getValue (key, std::to_string (default_value).c_str());
  return filter::strings::convert_to_int (value);
}


std::string Database_Config_User::getValueForUser (const std::string& user, const char * key, const char * default_value) const
{
  // Check the memory cache. If it is there, read it from the memory cache.
  const std::string cachekey = mapkey (user, key);
  if (database_config_user_cache.count (cachekey)) {
    return database_config_user_cache.at (cachekey);
  }
  // Read from file.
  std::string value;
  const std::string filename = file (user, key);
  if (file_or_dir_exists (filename))
    value = filter_url_file_get_contents (filename);
  else 
    value = default_value;
  // Cache it: Improved speed next time getting this value.
  database_config_user_cache.insert_or_assign (cachekey, value);
  // Done.
  return value;
}


bool Database_Config_User::getBValueForUser (const std::string& user, const char * key, bool default_value) const
{
  const auto value {getValueForUser (user, key, filter::strings::convert_to_string (default_value).c_str())};
  return filter::strings::convert_to_bool (value);
}


int Database_Config_User::getIValueForUser (const std::string& user, const char * key, int default_value) const
{
  const auto value {getValueForUser (user, key, std::to_string (default_value).c_str())};
  return filter::strings::convert_to_int (value);
}


void Database_Config_User::setValue (const char * key, const std::string& value) const
{
  const std::string& user = m_webserver_request.session_logic ()->get_username ();
  setValueForUser (user, key, value);
}


void Database_Config_User::setBValue (const char * key, bool value) const
{
  setValue (key, filter::strings::convert_to_string (value));
}


void Database_Config_User::setIValue (const char * key, int value) const
{
  setValue (key, std::to_string (value));
}


void Database_Config_User::setValueForUser (const std::string& user, const char * key, const std::string& value) const
{
  // Store in memory cache.
  database_config_user_cache [mapkey (user, key)] = value;
  // Store on disk.
  const std::string filename {file (user, key)};
  const std::string directory {filter_url_dirname (filename)};
  if (!file_or_dir_exists (directory))
    filter_url_mkdir (directory);
  filter_url_file_put_contents (filename, value);
}


void Database_Config_User::setBValueForUser (const std::string& user, const char * key, bool value) const
{
  setValueForUser (user, key, filter::strings::convert_to_string (value));
}


std::vector <std::string> Database_Config_User::getList (const char * key) const
{
  const std::string& user = m_webserver_request.session_logic ()->get_username ();
  return getListForUser (user, key);
}


std::vector <std::string> Database_Config_User::getListForUser (const std::string& user, const char * key) const
{
  // Check whether value is in cache.
  const std::string cachekey = mapkey (user, key);
  if (database_config_user_cache.count (cachekey)) {
    const std::string value = database_config_user_cache [cachekey];
    return filter::strings::explode (value, '\n');
  }
  // Read setting from disk.
  const std::string filename = file (user, key);
  if (file_or_dir_exists (filename)) {
    const std::string value = filter_url_file_get_contents (filename);
    // Cache it in memory.
    database_config_user_cache [cachekey] = value;
    // Done.
    return filter::strings::explode (value, '\n');
  }
  // Empty value.
  return {};
}


void Database_Config_User::setList (const char * key, const std::vector <std::string>& values) const
{
  const std::string& user = m_webserver_request.session_logic ()->get_username ();
  setListForUser (user, key, values);
}


void Database_Config_User::setListForUser (const std::string& user, const char * key, const std::vector <std::string>& values) const
{
  // Store it on disk.
  const std::string filename = file (user, key);
  const std::string directory = filter_url_dirname (filename);
  if (!file_or_dir_exists (directory))
    filter_url_mkdir (directory);
  const std::string value = filter::strings::implode (values, "\n");
  filter_url_file_put_contents (filename, value);
  // Put it in the memory cache.
  const std::string cachekey = mapkey (user, key);
  database_config_user_cache [cachekey] = value;
}


std::vector <int> Database_Config_User::getIList (const char * key) const
{
  const std::vector <std::string> lines = getList (key);
  std::vector <int> result;
  for (const auto& line : lines) {
    result.push_back (filter::strings::convert_to_int (line));
  }
  return result;
}


void Database_Config_User::setIList (const char * key, const std::vector <int>& values) const
{
  std::vector <std::string> lines {};
  for (const auto& value : values) {
    lines.push_back (std::to_string (value));
  }
  setList (key, lines);
}


void Database_Config_User::trim () const
{
  // Reset the sprint month and year after some time.
  // When a user visits the Sprint page after a few days, it will then display the current Sprint.
  // If the Sprint is not reset, the user may enter new tasks in the wrong sprint.
  const int time = filter::date::seconds_since_epoch () - (2 * 24 * 3600);
  Database_Users database_users {};
  for (const auto& user : database_users.get_users ()) {
    std::string filename = file (user, sprint_month_key ());
    if (file_or_dir_exists (filename)) {
      if (filter_url_file_modification_time (filename) < time) {
        // Remove from disk.
        filter_url_unlink (filename);
        filename = file (user, sprint_year_key ());
        filter_url_unlink (filename);
        // Clear cache.
        database_config_user_cache.clear ();
      }
    }
  }
}


// Remove any configuration setting of $username.
void Database_Config_User::remove (const std::string& username) const
{
  // Remove from disk.
  const std::string folder = file (username);
  filter_url_rmdir (folder);
  // Clear cache.
  database_config_user_cache.clear ();
}


// Clear the settings cache.
void Database_Config_User::clear_cache () const
{
  database_config_user_cache.clear ();
}


// Named configuration functions.


std::string Database_Config_User::getBible () const
{
  std::string bible = getValue ("bible", "");
  // If the Bible does not exist, take the first one available.
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  if (find (bibles.begin (), bibles.end (), bible) == bibles.end ()) {
    // There may not even be a first Bible: Create sample Bible.
    if (bibles.empty ()) {
      bible = demo_sample_bible_name ();
      demo_create_sample_bible ();
      database::bibles::create_bible (bible);
    } else {
      bible = bibles.at (0);
    }
    setBible (bible);
  }
  return bible;
}
void Database_Config_User::setBible (const std::string& bible) const
{
  setValue ("bible", bible);
}


constexpr const auto subscribe_to_consultation_notes_edited_by_me_key {"subscribe-to-consultation-notes-edited-by-me"};
bool Database_Config_User::getSubscribeToConsultationNotesEditedByMe () const
{
  return getBValue (subscribe_to_consultation_notes_edited_by_me_key, false);
}
void Database_Config_User::setSubscribeToConsultationNotesEditedByMe (bool value) const
{
  setBValue (subscribe_to_consultation_notes_edited_by_me_key, value);
}


constexpr const auto notify_me_of_any_consultation_notes_edits_key {"notify-me-of-any-consultation-notes-edits"};
bool Database_Config_User::getNotifyMeOfAnyConsultationNotesEdits () const
{
  return getBValue (notify_me_of_any_consultation_notes_edits_key, false);
}
bool Database_Config_User::getNotifyUserOfAnyConsultationNotesEdits (const std::string& username) const
{
  return getBValueForUser (username, notify_me_of_any_consultation_notes_edits_key, false);
}
void Database_Config_User::setNotifyMeOfAnyConsultationNotesEdits (bool value) const
{
  setBValue (notify_me_of_any_consultation_notes_edits_key, value);
}


constexpr const auto subscribed_consultation_note_notification_key {"subscribed-consultation-note-notification"};
bool Database_Config_User::getSubscribedConsultationNoteNotification () const
{
  return getBValue (subscribed_consultation_note_notification_key, true);
}
bool Database_Config_User::getUserSubscribedConsultationNoteNotification (const std::string& username) const
{
  return getBValueForUser (username, subscribed_consultation_note_notification_key, true);
}
void Database_Config_User::setSubscribedConsultationNoteNotification (bool value) const
{
  setBValue (subscribed_consultation_note_notification_key, value);
}


constexpr const auto get_assigned_to_consultation_notes_changes_key {"get-assigned-to-consultation-notes-changes"};
bool Database_Config_User::getAssignedToConsultationNotesChanges () const
{
  return getBValue (get_assigned_to_consultation_notes_changes_key, false);
}
bool Database_Config_User::getUserAssignedToConsultationNotesChanges (const std::string& username) const
{
  return getBValueForUser (username, get_assigned_to_consultation_notes_changes_key, false);
}
void Database_Config_User::setAssignedToConsultationNotesChanges (bool value) const
{
  setBValue (get_assigned_to_consultation_notes_changes_key, value);
}


constexpr const auto generate_change_notifications_key {"generate-change-notifications"};
bool Database_Config_User::getGenerateChangeNotifications () const
{
  return getBValue (generate_change_notifications_key, false);
}
bool Database_Config_User::getUserGenerateChangeNotifications (const std::string& username) const
{
  return getBValueForUser (username, generate_change_notifications_key, false);
}
void Database_Config_User::setGenerateChangeNotifications (bool value) const
{
  setBValue (generate_change_notifications_key, value);
}


constexpr const auto assigned_consultation_note_notification_key {"assigned-consultation-note-notification"};
bool Database_Config_User::getAssignedConsultationNoteNotification () const
{
  return getBValue (assigned_consultation_note_notification_key, true);
}
bool Database_Config_User::getUserAssignedConsultationNoteNotification (const std::string& username) const
{
  return getBValueForUser (username, assigned_consultation_note_notification_key, true);
}
void Database_Config_User::setAssignedConsultationNoteNotification (bool value) const
{
  setBValue (assigned_consultation_note_notification_key, value);
}


constexpr const auto consultation_notes_passage_selector_key {"consultation-notes-passage-selector"};
// 0: current verse; 1: current chapter; 2: current book; 3: any passage.
int Database_Config_User::getConsultationNotesPassageSelector () const
{
  // Default value is to select notes of the current chapter.
  // It used to be the current verse.
  // But that led to a situation where a user created a note,
  // navigated to another verse within the same chapter,
  // and then was confused because the user could not find the note just created.
  // With the updated selection, current chapter, this confusing situation does not occur.
  return getIValue (consultation_notes_passage_selector_key, 1);
}
void Database_Config_User::setConsultationNotesPassageSelector (int value) const
{
  setIValue (consultation_notes_passage_selector_key, value);
}


constexpr const auto consultation_notes_edit_selector_key {"consultation-notes-edit-selector"};
// 0: any time; 1: last 30 days; 2: last 7 days; 3: since yesterday; 4: today.
int Database_Config_User::getConsultationNotesEditSelector () const
{
  return getIValue (consultation_notes_edit_selector_key, 0);
}
void Database_Config_User::setConsultationNotesEditSelector (int value) const
{
  setIValue (consultation_notes_edit_selector_key, value);
}


constexpr const auto consultation_notes_non_edit_selector_key {"consultation-notes-non-edit-selector"};
// 0: don't care; 1: for last 30 days; 2: for last 7 days; 3: since yesterday; 4: today.
int Database_Config_User::getConsultationNotesNonEditSelector () const
{
  return getIValue (consultation_notes_non_edit_selector_key, 0);
}
void Database_Config_User::setConsultationNotesNonEditSelector (int value) const
{
  setIValue (consultation_notes_non_edit_selector_key, value);
}


constexpr const auto consultation_notes_status_selector_key {"consultation-notes-status-selector"};
// Status is a string; can be empty as well.
std::string Database_Config_User::getConsultationNotesStatusSelector () const
{
  return getValue (consultation_notes_status_selector_key, "");
}
void Database_Config_User::setConsultationNotesStatusSelector (const std::string& value) const
{
  setValue (consultation_notes_status_selector_key, value);
}


constexpr const auto consultation_notes_bible_selector_key {"consultation-notes-bible-selector"};
// "": any Bible; <bible>: named Bible.
std::string Database_Config_User::getConsultationNotesBibleSelector () const
{
  return getValue (consultation_notes_bible_selector_key, "");
}
void Database_Config_User::setConsultationNotesBibleSelector (const std::string& value) const
{
  setValue (consultation_notes_bible_selector_key, value);
}


constexpr const auto consultation_notes_assignment_selector_key {"consultation-notes-assignment-selector"};
// "": don't care; "user": notes assigned to "user".
std::string Database_Config_User::getConsultationNotesAssignmentSelector () const
{
  return getValue (consultation_notes_assignment_selector_key, "");
}
void Database_Config_User::setConsultationNotesAssignmentSelector (const std::string& value) const
{
  setValue (consultation_notes_assignment_selector_key, value);
}


constexpr const auto consultation_notes_subscription_selector_key {"consultation-notes-subscription-selector"};
// false: don't care; true: subscribed.
bool Database_Config_User::getConsultationNotesSubscriptionSelector () const
{
  return getBValue (consultation_notes_subscription_selector_key, false);
}
void Database_Config_User::setConsultationNotesSubscriptionSelector (bool value) const
{
  setBValue (consultation_notes_subscription_selector_key, value);
}


constexpr const auto consultation_notes_severity_selector_key {"consultation-notes-severity-selector"};
int Database_Config_User::getConsultationNotesSeveritySelector () const
{
  return getIValue (consultation_notes_severity_selector_key, -1);
}
void Database_Config_User::setConsultationNotesSeveritySelector (int value) const
{
  setIValue (consultation_notes_severity_selector_key, value);
}


constexpr const auto consultation_notes_text_selector_key {"consultation-notes-text-selector"};
int Database_Config_User::getConsultationNotesTextSelector () const
{
  return getIValue (consultation_notes_text_selector_key, 0);
}
void Database_Config_User::setConsultationNotesTextSelector (int value) const
{
  setIValue (consultation_notes_text_selector_key, value);
}


constexpr const auto consultation_notes_search_text_key {"consultation-notes-search-text"};
std::string Database_Config_User::getConsultationNotesSearchText () const
{
  return getValue (consultation_notes_search_text_key, "");
}
void Database_Config_User::setConsultationNotesSearchText (const std::string& value) const
{
  setValue (consultation_notes_search_text_key, value);
}


constexpr const auto consultation_notes_passage_inclusion_selector_key {"consultation-notes-passage-inclusion-selector"};
int Database_Config_User::getConsultationNotesPassageInclusionSelector () const
{
  return getIValue (consultation_notes_passage_inclusion_selector_key, 0);
}
void Database_Config_User::setConsultationNotesPassageInclusionSelector (int value) const
{
  setIValue (consultation_notes_passage_inclusion_selector_key, value);
}


constexpr const auto consultation_notes_text_inclusion_selector_key {"consultation-notes-text-inclusion-selector"};
int Database_Config_User::getConsultationNotesTextInclusionSelector () const
{
  return getIValue (consultation_notes_text_inclusion_selector_key, 0);
}
void Database_Config_User::setConsultationNotesTextInclusionSelector (int value) const
{
  setIValue (consultation_notes_text_inclusion_selector_key, value);
}


constexpr const auto bible_changes_notification_key {"bible-changes-notification"};
bool Database_Config_User::getBibleChangesNotification () const
{
  return getBValue (bible_changes_notification_key, false);
}
bool Database_Config_User::getUserBibleChangesNotification (const std::string& username) const
{
  return getBValueForUser (username, bible_changes_notification_key, false);
}
void Database_Config_User::setBibleChangesNotification (bool value) const
{
  setBValue (bible_changes_notification_key, value);
}


constexpr const auto deleted_consultation_note_notification_key {"deleted-consultation-note-notification"};
bool Database_Config_User::getDeletedConsultationNoteNotification () const
{
  return getBValue (deleted_consultation_note_notification_key, false);
}
bool Database_Config_User::getUserDeletedConsultationNoteNotification (const std::string& username) const
{
  return getBValueForUser (username, deleted_consultation_note_notification_key, false);
}
void Database_Config_User::setDeletedConsultationNoteNotification (bool value) const
{
  setBValue (deleted_consultation_note_notification_key, value);
}


bool Database_Config_User::default_bible_checks_notification () const
{
#ifdef HAVE_CLIENT
  return false;
#else
  const int level = m_webserver_request.session_logic ()->get_level ();
  return (level >= Filter_Roles::translator () && level <= Filter_Roles::manager ());
#endif
}
constexpr const auto bible_checks_notification_key {"bible-checks-notification"};
bool Database_Config_User::getBibleChecksNotification () const
{
  return getBValue (bible_checks_notification_key, default_bible_checks_notification ());
}
bool Database_Config_User::getUserBibleChecksNotification (const std::string& username) const
{
  return getBValueForUser (username, bible_checks_notification_key, default_bible_checks_notification ());
}
void Database_Config_User::setBibleChecksNotification (bool value) const
{
  setBValue (bible_checks_notification_key, value);
}


constexpr const auto pending_changes_notification_key {"pending-changes-notification"};
bool Database_Config_User::getPendingChangesNotification () const
{
  return getBValue (pending_changes_notification_key, false);
}
bool Database_Config_User::getUserPendingChangesNotification (const std::string& username) const
{
  return getBValueForUser (username, pending_changes_notification_key, false);
}
void Database_Config_User::setPendingChangesNotification (bool value) const
{
  setBValue (pending_changes_notification_key, value);
}


constexpr const auto user_changes_notification_key {"user-changes-notification"};
bool Database_Config_User::getUserChangesNotification () const
{
  return getBValue (user_changes_notification_key, false);
}
bool Database_Config_User::getUserUserChangesNotification (const std::string& username) const
{
  return getBValueForUser (username, user_changes_notification_key, false);
}
void Database_Config_User::setUserChangesNotification (bool value) const
{
  setBValue (user_changes_notification_key, value);
}


constexpr const auto assigned_notes_statistics_notification_key {"assigned-notes-statistics-notification"};
bool Database_Config_User::getAssignedNotesStatisticsNotification () const
{
  return getBValue (assigned_notes_statistics_notification_key, false);
}
bool Database_Config_User::getUserAssignedNotesStatisticsNotification (const std::string& username) const
{
  return getBValueForUser (username, assigned_notes_statistics_notification_key, false);
}
void Database_Config_User::setAssignedNotesStatisticsNotification (bool value) const
{
  setBValue (assigned_notes_statistics_notification_key, value);
}


constexpr const auto subscribed_notes_statistics_notification_key {"subscribed-notes-statistics-notification"};
bool Database_Config_User::getSubscribedNotesStatisticsNotification () const
{
  return getBValue (subscribed_notes_statistics_notification_key, false);
}
bool Database_Config_User::getUserSubscribedNotesStatisticsNotification (const std::string& username) const
{
  return getBValueForUser (username, subscribed_notes_statistics_notification_key, false);
}
void Database_Config_User::setSubscribedNotesStatisticsNotification (bool value) const
{
  setBValue (subscribed_notes_statistics_notification_key, value);
}


constexpr const auto notify_me_of_my_posts_key {"notify-me-of-my-posts"};
bool Database_Config_User::getNotifyMeOfMyPosts () const
{
  return getBValue (notify_me_of_my_posts_key, true);
}
bool Database_Config_User::getUserNotifyMeOfMyPosts (const std::string& username) const
{
  return getBValueForUser (username, notify_me_of_my_posts_key, true);
}
void Database_Config_User::setNotifyMeOfMyPosts (bool value) const
{
  setBValue (notify_me_of_my_posts_key, value);
}


constexpr const auto suppress_mail_my_updated_notes_key {"suppress-mail-my-updated-notes"};
bool Database_Config_User::getSuppressMailFromYourUpdatesNotes () const
{
  return getBValue (suppress_mail_my_updated_notes_key, false);
}
bool Database_Config_User::getUserSuppressMailFromYourUpdatesNotes (const std::string& username) const
{
  return getBValueForUser (username, suppress_mail_my_updated_notes_key, false);
}
void Database_Config_User::setSuppressMailFromYourUpdatesNotes (bool value) const
{
  setBValue (suppress_mail_my_updated_notes_key, value);
}


constexpr const auto active_resources_key {"active-resources"};
std::vector <std::string> Database_Config_User::getActiveResources () const
{
  // Default values.
  return getList (active_resources_key);
}
void Database_Config_User::setActiveResources (std::vector <std::string> values) const
{
  setList (active_resources_key, values);
}


constexpr const auto consistency_bibles_key {"consistency-bibles"};
std::vector <std::string> Database_Config_User::getConsistencyResources () const
{
  return getList (consistency_bibles_key);
}
void Database_Config_User::setConsistencyResources (std::vector <std::string> values) const
{
  setList (consistency_bibles_key, values);
}


const char * Database_Config_User::sprint_month_key ()
{
  return "sprint-month";
}
int Database_Config_User::getSprintMonth () const
{
  return getIValue (sprint_month_key (), filter::date::numerical_month (filter::date::seconds_since_epoch ()));
}
void Database_Config_User::setSprintMonth (int value) const
{
  setIValue (sprint_month_key (), value);
}


const char * Database_Config_User::sprint_year_key ()
{
  return "sprint-year";
}
int Database_Config_User::getSprintYear () const
{
  return getIValue (sprint_year_key (), filter::date::numerical_year (filter::date::seconds_since_epoch ()));
}
void Database_Config_User::setSprintYear (int value) const
{
  setIValue (sprint_year_key (), value);
}


constexpr const auto sprint_progress_notification_key {"sprint-progress-notification"};
bool Database_Config_User::getSprintProgressNotification () const
{
  return getBValue (sprint_progress_notification_key, false);
}
bool Database_Config_User::getUserSprintProgressNotification (const std::string& username) const
{
  return getBValueForUser (username, sprint_progress_notification_key, false);
}
void Database_Config_User::setSprintProgressNotification (bool value) const
{
  setBValue (sprint_progress_notification_key, value);
}


constexpr const auto user_changes_notifications_online_key {"user-changes-notifications-online"};
bool Database_Config_User::getUserChangesNotificationsOnline () const
{
  return getBValue (user_changes_notifications_online_key, false);
}
bool Database_Config_User::getUserUserChangesNotificationsOnline (const std::string& username) const
{
  return getBValueForUser (username, user_changes_notifications_online_key, false);
}
void Database_Config_User::setUserChangesNotificationsOnline (bool value) const
{
  setBValue (user_changes_notifications_online_key, value);
}


constexpr const auto contributor_changes_notifications_online_key {"contributor-changes-notifications-online"};
bool Database_Config_User::getContributorChangesNotificationsOnline () const
{
  return getBValue (contributor_changes_notifications_online_key, false);
}
bool Database_Config_User::getContributorChangesNotificationsOnline (const std::string& username) const
{
  return getBValueForUser (username, contributor_changes_notifications_online_key, false);
}
void Database_Config_User::setContributorChangesNotificationsOnline (bool value) const
{
  setBValue (contributor_changes_notifications_online_key, value);
}


constexpr const auto workbench_urls_key {"workbench-urls"};
std::string Database_Config_User::getWorkspaceURLs () const
{
  return getValue (workbench_urls_key, "");
}
void Database_Config_User::setWorkspaceURLs (const std::string& value) const
{
  setValue (workbench_urls_key, value);
}


constexpr const auto workbench_widths_key {"workbench-widths"};
std::string Database_Config_User::getWorkspaceWidths () const
{
  return getValue (workbench_widths_key, "");
}
void Database_Config_User::setWorkspaceWidths (const std::string& value) const
{
  setValue (workbench_widths_key, value);
}


constexpr const auto workbench_heights_key {"workbench-heights"};
std::string Database_Config_User::getWorkspaceHeights () const
{
  return getValue (workbench_heights_key, "");
}
void Database_Config_User::setWorkspaceHeights (const std::string& value) const
{
  setValue (workbench_heights_key, value);
}


constexpr const auto entire_workbench_widths_key {"entire-workbench-widths"};
std::string Database_Config_User::getEntireWorkspaceWidths () const
{
  return getValue (entire_workbench_widths_key, "");
}
void Database_Config_User::setEntireWorkspaceWidths (const std::string& value) const
{
  setValue (entire_workbench_widths_key, value);
}


constexpr const auto active_workbench_key {"active-workbench"};
std::string Database_Config_User::getActiveWorkspace () const
{
  return getValue (active_workbench_key, "");
}
void Database_Config_User::setActiveWorkspace (const std::string& value) const
{
  setValue (active_workbench_key, value);
}


constexpr const auto postpone_new_notes_mails_key {"postpone-new-notes-mails"};
bool Database_Config_User::getPostponeNewNotesMails () const
{
  return getBValue (postpone_new_notes_mails_key, false);
}
void Database_Config_User::setPostponeNewNotesMails (bool value) const
{
  setBValue (postpone_new_notes_mails_key, value);
}


constexpr const auto recently_applied_styles_key {"recently-applied-styles"};
std::string Database_Config_User::getRecentlyAppliedStyles () const
{
  return getValue (recently_applied_styles_key, "p s add nd f x v");
}
void Database_Config_User::setRecentlyAppliedStyles (const std::string& values) const
{
  setValue (recently_applied_styles_key, values);
}


constexpr const auto print_resources_key {"print-resources"};
std::vector <std::string> Database_Config_User::getPrintResources () const
{
  return getList (print_resources_key);
}
std::vector <std::string> Database_Config_User::getPrintResourcesForUser (const std::string& user) const
{
  return getListForUser (user, print_resources_key);
}
void Database_Config_User::setPrintResources (std::vector <std::string> values) const
{
  setList (print_resources_key, values);
}


static Passage database_config_user_fix_passage (const std::string& value, const char * fallback)
{
  std::vector <std::string> values = filter::strings::explode (value, '.');
  if (values.size () != 3) 
    values = filter::strings::explode (fallback, '.');
  const Passage passage = Passage ("", filter::strings::convert_to_int (values[0]), filter::strings::convert_to_int (values[1]), values[2]);
  return passage;
}


constexpr const auto print_passage_from_key {"print-passage-from"};
Passage Database_Config_User::getPrintPassageFrom () const
{
  return database_config_user_fix_passage (getValue (print_passage_from_key, ""), "1.1.1");
}
Passage Database_Config_User::getPrintPassageFromForUser (const std::string& user) const
{
  return database_config_user_fix_passage (getValueForUser (user, print_passage_from_key, ""), "1.1.1");
}
void Database_Config_User::setPrintPassageFrom (Passage value) const
{
  const std::string s = std::to_string (value.m_book) + "." + std::to_string (value.m_chapter) + "." + value.m_verse;
  setValue (print_passage_from_key, s);
}


constexpr const auto print_passage_to_key {"print-passage-to"};
Passage Database_Config_User::getPrintPassageTo () const
{
  return database_config_user_fix_passage (getValue (print_passage_to_key, ""), "1.1.31");
}
Passage Database_Config_User::getPrintPassageToForUser (const std::string& user) const
{
  return database_config_user_fix_passage (getValueForUser (user, print_passage_to_key, ""), "1.1.31");
}
void Database_Config_User::setPrintPassageTo (const Passage& value) const
{
  const std::string s = std::to_string (value.m_book) + "." + std::to_string (value.m_chapter) + "." + value.m_verse;
  setValue (print_passage_to_key, s);
}


constexpr const auto focused_book_key {"focused-book"};
int Database_Config_User::getFocusedBook () const
{
  return getIValue (focused_book_key, 1);
}
void Database_Config_User::setFocusedBook (int book) const
{
  setIValue (focused_book_key, book);
}


constexpr const auto focused_chapter_key {"focused-chapter"};
int Database_Config_User::getFocusedChapter () const
{
  return getIValue (focused_chapter_key, 1);
}
void Database_Config_User::setFocusedChapter (int chapter) const
{
  setIValue (focused_chapter_key, chapter);
}


constexpr const auto focused_verse_key {"focused-verse"};
int Database_Config_User::getFocusedVerse () const
{
  return getIValue (focused_verse_key, 1);
}
void Database_Config_User::setFocusedVerse (int verse) const
{
  setIValue (focused_verse_key, verse);
}


constexpr const auto updated_settings_key {"updated-settings"};
std::vector <int> Database_Config_User::getUpdatedSettings () const
{
  return getIList (updated_settings_key);
}
void Database_Config_User::setUpdatedSettings (const std::vector <int>& values) const
{
  setIList (updated_settings_key, values);
}
void Database_Config_User::addUpdatedSetting (int value) const
{
  std::vector <int> settings = getUpdatedSettings ();
  settings.push_back (value);
  settings = filter::strings::array_unique (settings);
  setUpdatedSettings (settings);
}
void Database_Config_User::removeUpdatedSetting (int value) const
{
  std::vector <int> settings = getUpdatedSettings ();
  const std::vector <int> against {value};
  settings = filter::strings::array_diff (settings, against);
  setUpdatedSettings (settings);
}


constexpr const auto removed_changes_key {"removed-changes"};
std::vector <int> Database_Config_User::getRemovedChanges () const
{
  return getIList (removed_changes_key);
}
void Database_Config_User::setRemovedChanges (const std::vector <int>& values) const
{
  setIList (removed_changes_key, values);
}
void Database_Config_User::addRemovedChange (int value) const
{
  std::vector <int> settings = getRemovedChanges ();
  settings.push_back (value);
  settings = filter::strings::array_unique (settings);
  setRemovedChanges (settings);
}
void Database_Config_User::removeRemovedChange (int value) const
{
  std::vector <int> settings = getRemovedChanges ();
  std::vector <int> against {value};
  settings = filter::strings::array_diff (settings, against);
  setRemovedChanges (settings);
}


constexpr const auto change_notifications_checksum_key {"change-notifications-checksum"};
std::string Database_Config_User::getChangeNotificationsChecksum () const
{
  return getValue (change_notifications_checksum_key, "");
}
void Database_Config_User::setChangeNotificationsChecksum (const std::string& value) const
{
  setValue (change_notifications_checksum_key, value);
}
void Database_Config_User::setUserChangeNotificationsChecksum (const std::string& user, const std::string& value) const
{
  setValueForUser (user, change_notifications_checksum_key, value);
}


constexpr const auto live_bible_editor_key {"live-bible-editor"};
int Database_Config_User::getLiveBibleEditor () const
{
  return getIValue (live_bible_editor_key, 0);
}
void Database_Config_User::setLiveBibleEditor (int time) const
{
  setIValue (live_bible_editor_key, time);
}


constexpr const auto resource_verses_before_key {"resource-verses-before"};
int Database_Config_User::getResourceVersesBefore () const
{
  return getIValue (resource_verses_before_key, 0);
}
void Database_Config_User::setResourceVersesBefore (int verses) const
{
  setIValue (resource_verses_before_key, verses);
}


constexpr const auto resource_verses_after_key {"resource-verses-after"};
int Database_Config_User::getResourceVersesAfter () const
{
  return getIValue (resource_verses_after_key, 0);
}
void Database_Config_User::setResourceVersesAfter (int verses) const
{
  setIValue (resource_verses_after_key, verses);
}


constexpr const auto sync_key_key {"sync-key"};
// Encryption key storage on server.
std::string Database_Config_User::get_sync_key () const
{
  return getValue (sync_key_key, "");
}
void Database_Config_User::set_sync_key (const std::string& key) const
{
  setValue (sync_key_key, key);
}


constexpr const auto general_font_size_key {"general-font-size"};
int Database_Config_User::getGeneralFontSize () const
{
  // Default value, see https://github.com/bibledit/cloud/issues/509
  return getIValue (general_font_size_key, 112);
}
void Database_Config_User::setGeneralFontSize (int size) const
{
  setIValue (general_font_size_key, size);
}


constexpr const auto menu_font_size_key {"menu-font-size"};
int Database_Config_User::getMenuFontSize () const
{
  // Default value, see https://github.com/bibledit/cloud/issues/509
  return getIValue (menu_font_size_key, 112);
}
void Database_Config_User::setMenuFontSize (int size) const
{
  setIValue (menu_font_size_key, size);
}


constexpr const auto bible_editors_font_size_key {"bible-editors-font-size"};
int Database_Config_User::getBibleEditorsFontSize () const
{
  return getIValue (bible_editors_font_size_key, 100);
}
void Database_Config_User::setBibleEditorsFontSize (int size) const
{
  setIValue (bible_editors_font_size_key, size);
}


constexpr const auto resources_font_size_key {"resources-font-size"};
int Database_Config_User::getResourcesFontSize () const
{
  return getIValue (resources_font_size_key, 100);
}
void Database_Config_User::setResourcesFontSize (int size) const
{
  setIValue (resources_font_size_key, size);
}


constexpr const auto hebrew_font_size_key {"hebrew-font-size"};
int Database_Config_User::getHebrewFontSize () const
{
  return getIValue (hebrew_font_size_key, 100);
}
void Database_Config_User::setHebrewFontSize (int size) const
{
  setIValue (hebrew_font_size_key, size);
}


constexpr const auto greek_font_size_key {"greek-font-size"};
int Database_Config_User::getGreekFontSize () const
{
  return getIValue (greek_font_size_key, 100);
}
void Database_Config_User::setGreekFontSize (int size) const
{
  setIValue (greek_font_size_key, size);
}


constexpr const auto vertical_caret_position_key {"vertical-caret-position"};
int Database_Config_User::getVerticalCaretPosition () const
{
  // Updated default value, see https://github.com/bibledit/cloud/issues/509
  return getIValue (vertical_caret_position_key, 30);
}
void Database_Config_User::setVerticalCaretPosition (int position) const
{
  setIValue (vertical_caret_position_key, position);
}


constexpr const auto current_theme_style_key {"current-theme-style"};
int Database_Config_User::getCurrentTheme () const
{
  return getIValue (current_theme_style_key, 0);
}
void Database_Config_User::setCurrentTheme (int index) const
{
  setIValue (current_theme_style_key, index);
}


constexpr const auto display_breadcrumbs_key {"display-breadcrumbs"};
bool Database_Config_User::getDisplayBreadcrumbs () const
{
  return getBValue (display_breadcrumbs_key, false);
}
void Database_Config_User::setDisplayBreadcrumbs (bool value) const
{
  setBValue (display_breadcrumbs_key, value);
}


constexpr const auto workspace_menu_fadeout_delay_key {"workspace-menu-fadeout-delay"};
int Database_Config_User::getWorkspaceMenuFadeoutDelay () const
{
  return getIValue (workspace_menu_fadeout_delay_key, 4);
}
void Database_Config_User::setWorkspaceMenuFadeoutDelay (int value) const
{
  setIValue (workspace_menu_fadeout_delay_key, value);
}


constexpr const auto editing_allowed_difference_chapter_key {"editing-allowed-difference-chapter"};
int Database_Config_User::getEditingAllowedDifferenceChapter () const
{
  return getIValue (editing_allowed_difference_chapter_key, 20);
}
void Database_Config_User::setEditingAllowedDifferenceChapter (int value) const
{
  setIValue (editing_allowed_difference_chapter_key, value);
}


constexpr const auto editing_allowed_difference_verse_key {"editing-allowed-difference-verse"};
int Database_Config_User::getEditingAllowedDifferenceVerse () const
{
  return getIValue (editing_allowed_difference_verse_key, 75);
}
void Database_Config_User::setEditingAllowedDifferenceVerse (int value) const
{
  setIValue (editing_allowed_difference_verse_key, value);
}


bool Database_Config_User::getBasicInterfaceModeDefault () const
{
  // Touch devices default to basic mode.
#ifdef HAVE_ANDROID
  return true;
#endif
#ifdef HAVE_IOS
  return true;
#endif
  // The app running on a workspace or laptop have default to basic mode for a lower role.
  const int level = m_webserver_request.session_logic ()->get_level ();
  if (level <= Filter_Roles::manager ())
    return true;
  // Higher role: default to advanced mode.
  return false;
}
constexpr const auto basic_interface_mode_key {"basic-interface-mode"};
bool Database_Config_User::getBasicInterfaceMode () const
{
  return getBValue (basic_interface_mode_key, getBasicInterfaceModeDefault ());
}
void Database_Config_User::setBasicInterfaceMode (bool value) const
{
  setBValue (basic_interface_mode_key, value);
}


constexpr const auto main_menu_always_visible_key {"main-menu-always-visible"};
bool Database_Config_User::getMainMenuAlwaysVisible () const
{
  // Default visible in basic mode.
  // Advanced mode: By default it is not visible.
  return getBValue (main_menu_always_visible_key, getBasicInterfaceModeDefault ());
}
void Database_Config_User::setMainMenuAlwaysVisible (bool value) const
{
  setBValue (main_menu_always_visible_key, value);
}


constexpr const auto swipe_actions_available_key {"swipe-actions-available"};
bool Database_Config_User::getSwipeActionsAvailable () const
{
  return getBValue (swipe_actions_available_key, true);
}
void Database_Config_User::setSwipeActionsAvailable (bool value) const
{
  setBValue (swipe_actions_available_key, value);
}


constexpr const auto fast_editor_switching_available_key {"fast-editor-switching-available"};
bool Database_Config_User::getFastEditorSwitchingAvailable () const
{
  return getBValue (fast_editor_switching_available_key, true);
}
void Database_Config_User::setFastEditorSwitchingAvailable (bool value) const
{
  setBValue (fast_editor_switching_available_key, value);
}


constexpr const auto include_related_passages_key {"include-related-passages"};
bool Database_Config_User::getIncludeRelatedPassages () const
{
  return getBValue (include_related_passages_key, false);
}
void Database_Config_User::setIncludeRelatedPassages (bool value) const
{
  setBValue (include_related_passages_key, value);
}


constexpr const auto enabled_visual_editors_key {"enabled-visual-editors"};
int Database_Config_User::getFastSwitchVisualEditors () const
{
  // Updated default values, see https://github.com/bibledit/cloud/issues/509
  return getIValue (enabled_visual_editors_key, 0);
}
void Database_Config_User::setFastSwitchVisualEditors (int value) const
{
  setIValue (enabled_visual_editors_key, value);
}


constexpr const auto enabled_usfm_editors_key {"enabled-usfm-editors"};
int Database_Config_User::getFastSwitchUsfmEditors () const
{
  // Initially only the USFM chapter editor is enabled.
  return getIValue (enabled_usfm_editors_key, 1);
}
void Database_Config_User::setFastSwitchUsfmEditors (int value) const
{
  setIValue (enabled_usfm_editors_key, value);
}


constexpr const auto enable_styles_button_visual_editors_key {"enable-styles-button-visual-editors"};
bool Database_Config_User::getEnableStylesButtonVisualEditors () const
{
  return getBValue (enable_styles_button_visual_editors_key, true);
}
void Database_Config_User::setEnableStylesButtonVisualEditors (bool value) const
{
  setBValue (enable_styles_button_visual_editors_key, value);
}


constexpr const auto menu_changes_in_basic_mode_key {"menu-changes-in-basic-mode"};
bool Database_Config_User::getMenuChangesInBasicMode () const
{
  return getBValue (menu_changes_in_basic_mode_key, false);
}
void Database_Config_User::setMenuChangesInBasicMode (bool value) const
{
  setBValue (menu_changes_in_basic_mode_key, value);
}


constexpr const auto privilege_use_advanced_mode_key {"privilege-use-advanced-mode"};
bool Database_Config_User::getPrivilegeUseAdvancedMode () const
{
  return getBValue (privilege_use_advanced_mode_key, true);
}
bool Database_Config_User::getPrivilegeUseAdvancedModeForUser (const std::string& username) const
{
  return getBValueForUser (username, privilege_use_advanced_mode_key, true);
}
void Database_Config_User::setPrivilegeUseAdvancedModeForUser (const std::string& username, bool value) const
{
  setBValueForUser (username, privilege_use_advanced_mode_key, value);
}


constexpr const auto privilege_delete_consultation_notes_key {"privilege-delete-consultation-notes"};
bool Database_Config_User::getPrivilegeDeleteConsultationNotes () const
{
  return getBValue (privilege_delete_consultation_notes_key, false);
}
void Database_Config_User::setPrivilegeDeleteConsultationNotes (bool value) const
{
  setBValue (privilege_delete_consultation_notes_key, value);
}
bool Database_Config_User::getPrivilegeDeleteConsultationNotesForUser (const std::string& username) const
{
  return getBValueForUser (username, privilege_delete_consultation_notes_key, false);
}
void Database_Config_User::setPrivilegeDeleteConsultationNotesForUser (const std::string& username, bool value) const
{
  setBValueForUser (username, privilege_delete_consultation_notes_key, value);
}


constexpr const auto privilege_set_stylesheets_key {"privilege-set-stylesheets"};
bool Database_Config_User::getPrivilegeSetStylesheets () const
{
  return getBValue (privilege_set_stylesheets_key, false);
}
bool Database_Config_User::getPrivilegeSetStylesheetsForUser (const std::string& username) const
{
  return getBValueForUser (username, privilege_set_stylesheets_key, false);
}
void Database_Config_User::setPrivilegeSetStylesheetsForUser (const std::string& username, bool value) const
{
  setBValueForUser (username, privilege_set_stylesheets_key, value);
}


constexpr const auto dismiss_changes_at_top_key {"dismiss-changes-at-top"};
bool Database_Config_User::getDismissChangesAtTop () const
{
  return getBValue (dismiss_changes_at_top_key, false);
}
void Database_Config_User::setDismissChangesAtTop (bool value) const
{
  setBValue (dismiss_changes_at_top_key, value);
}


constexpr const auto quick_note_edit_link_key {"quick-note-edit-link"};
bool Database_Config_User::getQuickNoteEditLink () const
{
  return getBValue (quick_note_edit_link_key, false);
}
void Database_Config_User::setQuickNoteEditLink (bool value) const
{
  setBValue (quick_note_edit_link_key, value);
}


constexpr const auto show_bible_in_notes_list_key {"show-bible-in-notes-list"};
bool Database_Config_User::getShowBibleInNotesList () const
{
  return getBValue (show_bible_in_notes_list_key, false);
}
void Database_Config_User::setShowBibleInNotesList (bool value) const
{
  setBValue (show_bible_in_notes_list_key, value);
}


constexpr const auto show_note_status_key {"show-note-status"};
bool Database_Config_User::getShowNoteStatus () const
{
  return getBValue (show_note_status_key, false);
}
void Database_Config_User::setShowNoteStatus (bool value) const
{
  setBValue (show_note_status_key, value);
}


constexpr const auto show_verse_text_at_create_note_key {"show-verse-text-at-create-note"};
bool Database_Config_User::getShowVerseTextAtCreateNote () const
{
  return getBValue (show_verse_text_at_create_note_key, false);
}
void Database_Config_User::setShowVerseTextAtCreateNote (bool value) const
{
  setBValue (show_verse_text_at_create_note_key, value);
}


constexpr const auto order_changes_by_author_key {"order-changes-by-author"};
bool Database_Config_User::getOrderChangesByAuthor () const
{
  return getBValue (order_changes_by_author_key, false);
}
void Database_Config_User::setOrderChangesByAuthor (bool value) const
{
  setBValue (order_changes_by_author_key, value);
}


constexpr const char * automatic_note_assignment_key {"automatic-note-assignment"};
std::vector <std::string> Database_Config_User::getAutomaticNoteAssignment () const
{
  return getList (automatic_note_assignment_key);
}
void Database_Config_User::setAutomaticNoteAssignment (const std::vector <std::string>& values) const
{
  setList (automatic_note_assignment_key, values);
}


constexpr const auto receive_focused_reference_from_paratext_key {"receive-focused-reference-from-paratext"};
bool Database_Config_User::getReceiveFocusedReferenceFromParatext () const
{
  return getBValue (receive_focused_reference_from_paratext_key, true);
}
void Database_Config_User::setReceiveFocusedReferenceFromParatext (bool value) const
{
  setBValue (receive_focused_reference_from_paratext_key, value);
}


constexpr const auto receive_focused_reference_from_accordance_key {"receive-focused-reference-from-accordance"};
bool Database_Config_User::getReceiveFocusedReferenceFromAccordance () const
{
  return getBValue (receive_focused_reference_from_accordance_key, true);
}
void Database_Config_User::setReceiveFocusedReferenceFromAccordance (bool value) const
{
  setBValue (receive_focused_reference_from_accordance_key, value);
}


constexpr const auto use_colored_note_status_labels_key {"use-colored-note-status-labels"};
bool Database_Config_User::getUseColoredNoteStatusLabels () const
{
  return getBValue (use_colored_note_status_labels_key, false);
}
void Database_Config_User::setUseColoredNoteStatusLabels (bool value) const
{
  setBValue (use_colored_note_status_labels_key, value);
}


constexpr const auto notes_date_format_key {"notes_date-format"};
int Database_Config_User::getNotesDateFormat () const
{
  return getIValue (notes_date_format_key, 0);
}
void Database_Config_User::setNotesDateFormat (int value) const
{
  setIValue (notes_date_format_key, value);
}


constexpr const auto change_notifications_bibles_key {"change-notifications-bibles"};
std::vector <std::string> Database_Config_User::getChangeNotificationsBibles () const
{
  return getList (change_notifications_bibles_key);
}
std::vector <std::string> Database_Config_User::getChangeNotificationsBiblesForUser (const std::string& user) const
{
  return getListForUser (user, change_notifications_bibles_key);
}
void Database_Config_User::setChangeNotificationsBibles (const std::vector <std::string>& values) const
{
  setList (change_notifications_bibles_key, values);
}


constexpr const auto enable_spell_check_key {"enable-spell-check"};
bool Database_Config_User::get_enable_spell_check () const
{
  return getBValue (enable_spell_check_key, true);
}
void Database_Config_User::set_enable_spell_check (bool value) const
{
  setBValue (enable_spell_check_key, value);
}
