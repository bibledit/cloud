/*
Copyright (©) 2003-2026 Teus Benschop.

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


#include <database/logic.h>
#include <database/styles.h>
#include <database/users.h>
#include <database/config/user.h>
#include <demo/logic.h>
#include <filter/date.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <session/logic.h>
#include <webserver/request.h>

#include <database/bibles.h>


// Cache values in memory for better speed.
// The speed improvement comes from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
static std::map<std::string, std::string> cache;


// Functions for getting and setting values or lists of values follow here:


static std::string file(const std::string& user)
{
    return filter_url_create_root_path({database_logic_databases(), "config", "user", user});
}


static std::string file(const std::string& user, const char* key)
{
    return filter_url_create_path({file(user), key});
}


// The key in the cache for this setting.
static std::string map_key(const std::string& user, const char* key)
{
    return user + key;
}


static std::string get_value_for_user(const std::string& user, const char* key,
                                      const char* default_value)
{
    // Check the memory cache. If it is there, read it from the memory cache.
    const std::string cache_key = map_key(user, key);
    if (cache.contains(cache_key))
    {
        return cache.at(cache_key);
    }
    // Read from file.
    std::string value;
    const std::string filename = file(user, key);
    if (file_or_dir_exists(filename))
        value = filter_url_file_get_contents(filename);
    else
        value = default_value;
    // Cache it: Improved speed next time getting this value.
    cache.insert_or_assign(cache_key, value);
    // Done.
    return value;
}


static bool get_boolean_value_for_user(const std::string& user, const char* key, bool default_value)
{
    const auto value{get_value_for_user(user, key, filter::string::convert_to_string(default_value).c_str())};
    return filter::string::convert_to_bool(value);
}


static void set_value_for_user(const std::string& user, const char* key, const std::string& value)
{
    // Store in memory cache.
    cache[map_key(user, key)] = value;
    // Store on disk.
    const std::string filename{file(user, key)};
    const std::string directory{filter_url_dirname(filename)};
    if (not file_or_dir_exists(directory))
        filter_url_mkdir(directory);
    filter_url_file_put_contents(filename, value);
}


static void set_boolean_value_for_user(const std::string& user, const char* key, bool value)
{
    set_value_for_user(user, key, filter::string::convert_to_string(value));
}


static std::vector<std::string> get_list_for_user(const std::string& user, const char* key)
{
    // Check whether value is in cache.
    const std::string cache_key = map_key(user, key);
    if (cache.contains(cache_key))
    {
        const std::string& value = cache.at(cache_key);
        return filter::string::explode(value, '\n');
    }
    // Read setting from disk.
    const std::string filename = file(user, key);
    if (file_or_dir_exists(filename))
    {
        const std::string value = filter_url_file_get_contents(filename);
        // Cache it in memory.
        cache[cache_key] = value;
        // Done.
        return filter::string::explode(value, '\n');
    }
    // Empty value.
    return {};
}


static void set_list_for_user(const std::string& user, const char* key,
                              const std::vector<std::string>& values)
{
    // Store it on disk.
    const std::string filename = file(user, key);
    const std::string directory = filter_url_dirname(filename);
    if (not file_or_dir_exists(directory))
        filter_url_mkdir(directory);
    const std::string value = filter::string::implode(values, "\n");
    filter_url_file_put_contents(filename, value);
    // Put it in the memory cache.
    const std::string cache_key = map_key(user, key);
    cache[cache_key] = value;
}


constexpr const char* sprint_month_key{"sprint-month"};
constexpr const char* sprint_year_key{"sprint-year"};


Database_Config_User::Database_Config_User(Webserver_Request& webserver_request)
    : m_webserver_request(webserver_request)
{
}


std::string Database_Config_User::get_value(const char* key, const char* default_value) const
{
    const std::string& user = m_webserver_request.session_logic()->get_username();
    return get_value_for_user(user, key, default_value);
}


bool Database_Config_User::get_boolean_value(const char* key, bool default_value) const
{
    const std::string value = get_value(key, filter::string::convert_to_string(default_value).c_str());
    return filter::string::convert_to_bool(value);
}


int Database_Config_User::get_numeric_value(const char* key, int default_value) const
{
    const std::string value = get_value(key, std::to_string(default_value).c_str());
    return filter::string::convert_to_int(value);
}


void Database_Config_User::set_value(const char* key, const std::string& value) const
{
    const std::string& user = m_webserver_request.session_logic()->get_username();
    set_value_for_user(user, key, value);
}


void Database_Config_User::set_boolean_value(const char* key, bool value) const
{
    set_value(key, filter::string::convert_to_string(value));
}


void Database_Config_User::set_numeric_value(const char* key, int value) const
{
    set_value(key, std::to_string(value));
}


std::vector<std::string> Database_Config_User::get_list(const char* key) const
{
    const std::string& user = m_webserver_request.session_logic()->get_username();
    return get_list_for_user(user, key);
}


void Database_Config_User::set_list(const char* key, const std::vector<std::string>& values) const
{
    const std::string& user = m_webserver_request.session_logic()->get_username();
    set_list_for_user(user, key, values);
}


std::vector<int> Database_Config_User::get_numeric_list(const char* key) const
{
    std::vector<int> result;
    std::ranges::for_each(get_list(key), [&result](const std::string& line) noexcept
    {
        result.push_back(filter::string::convert_to_int(line));
    });
    return result;
}


void Database_Config_User::set_numeric_list(const char* key, const std::vector<int>& values) const
{
    std::vector<std::string> lines{};
    std::ranges::for_each(values, [&lines](const int value) noexcept
    {
        lines.push_back(std::to_string(value));
    });
    set_list(key, lines);
}


void Database_Config_User::trim()
{
    // Reset the sprint month and year after some time.
    // When a user visits the Sprint page after a few days, it will then display the current Sprint.
    // If the Sprint is not reset, the user may enter new tasks in the wrong sprint.
    const int time = filter::date::seconds_since_epoch() - (2 * 24 * 3600);
    Database_Users database_users{};
    std::ranges::for_each(database_users.get_users(), [time](const auto& user)
    {
        if (std::string filename = file(user, sprint_month_key); file_or_dir_exists(filename))
            if (filter_url_file_modification_time(filename) < time)
            {
                // Remove from disk.
                filter_url_unlink(filename);
                filename = file(user, sprint_year_key);
                filter_url_unlink(filename);
                // Clear cache.
                cache.clear();
            }
    });
}


// Remove any configuration setting of $username.
void Database_Config_User::remove(const std::string& username)
{
    // Remove from disk.
    const std::string folder = file(username);
    filter_url_rmdir(folder);
    // Clear cache.
    cache.clear();
}


// Clear the settings cache.
void Database_Config_User::clear_cache()
{
    cache.clear();
}


// Named configuration functions.


std::string Database_Config_User::get_bible() const
{
    std::string bible = get_value("bible", "");
    // If the _bible does not exist, take the first one available.
    if (std::vector<std::string> bibles = database::bibles::get_bibles();
        std::ranges::find(bibles, bible) == bibles.end())
    {
        // There may not even be a first Bible: Create sample Bible.
        if (bibles.empty())
        {
            bible = demo_sample_bible_name();
            demo_create_sample_bible();
            database::bibles::create_bible(bible);
        }
        else
        {
            bible = bibles.at(0);
        }
        set_bible(bible);
    }
    return bible;
}

void Database_Config_User::set_bible(const std::string& bible) const
{
    set_value("bible", bible);
}


constexpr auto subscribe_to_consultation_notes_edited_by_me_key{"subscribe-to-consultation-notes-edited-by-me"};

bool Database_Config_User::get_subscribe_to_consultation_notes_edited_by_me() const
{
    return get_boolean_value(subscribe_to_consultation_notes_edited_by_me_key, false);
}

void Database_Config_User::set_subscribe_to_consultation_notes_edited_by_me(const bool value) const
{
    set_boolean_value(subscribe_to_consultation_notes_edited_by_me_key, value);
}


constexpr auto notify_me_of_any_consultation_notes_edits_key{"notify-me-of-any-consultation-notes-edits"};

bool Database_Config_User::get_notify_me_of_any_consultation_notes_edits() const
{
    return get_boolean_value(notify_me_of_any_consultation_notes_edits_key, false);
}

bool Database_Config_User::get_notify_user_of_any_consultation_notes_edits(const std::string& username)
{
    return get_boolean_value_for_user(username, notify_me_of_any_consultation_notes_edits_key, false);
}

void Database_Config_User::set_notify_me_of_any_consultation_notes_edits(const bool value) const
{
    set_boolean_value(notify_me_of_any_consultation_notes_edits_key, value);
}


constexpr auto subscribed_consultation_note_notification_key{"subscribed-consultation-note-notification"};

bool Database_Config_User::get_subscribed_consultation_note_notification() const
{
    return get_boolean_value(subscribed_consultation_note_notification_key, true);
}

bool Database_Config_User::get_user_subscribed_consultation_note_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, subscribed_consultation_note_notification_key, true);
}

void Database_Config_User::set_subscribed_consultation_note_notification(const bool value) const
{
    set_boolean_value(subscribed_consultation_note_notification_key, value);
}


constexpr auto get_assigned_to_consultation_notes_changes_key{"get-assigned-to-consultation-notes-changes"};

bool Database_Config_User::get_assigned_to_consultation_notes_changes() const
{
    return get_boolean_value(get_assigned_to_consultation_notes_changes_key, false);
}

bool Database_Config_User::get_user_assigned_to_consultation_notes_changes(const std::string& username)
{
    return get_boolean_value_for_user(username, get_assigned_to_consultation_notes_changes_key, false);
}

void Database_Config_User::set_assigned_to_consultation_notes_changes(const bool value) const
{
    set_boolean_value(get_assigned_to_consultation_notes_changes_key, value);
}


constexpr auto generate_change_notifications_key{"generate-change-notifications"};

bool Database_Config_User::get_generate_change_notifications() const
{
    return get_boolean_value(generate_change_notifications_key, false);
}

bool Database_Config_User::get_user_generate_change_notifications(const std::string& username)
{
    return get_boolean_value_for_user(username, generate_change_notifications_key, false);
}

void Database_Config_User::set_generate_change_notifications(const bool value) const
{
    set_boolean_value(generate_change_notifications_key, value);
}


constexpr auto assigned_consultation_note_notification_key{"assigned-consultation-note-notification"};

bool Database_Config_User::get_assigned_consultation_note_notification() const
{
    return get_boolean_value(assigned_consultation_note_notification_key, true);
}

bool Database_Config_User::get_user_assigned_consultation_note_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, assigned_consultation_note_notification_key, true);
}

void Database_Config_User::set_assigned_consultation_note_notification(const bool value) const
{
    set_boolean_value(assigned_consultation_note_notification_key, value);
}


constexpr auto consultation_notes_passage_selector_key{"consultation-notes-passage-selector"};
// 0: current verse; 1: current chapter; 2: current book; 3: any passage.
int Database_Config_User::get_consultation_notes_passage_selector() const
{
    // _default value is to select notes of the current chapter.
    // _it used to be the current verse.
    // _but that led to a situation where a user created a note,
    // navigated to another verse within the same chapter,
    // and then was confused because the user could not find the note just created.
    // _with the updated selection, current chapter, this confusing situation does not occur.
    return get_numeric_value(consultation_notes_passage_selector_key, 1);
}

void Database_Config_User::set_consultation_notes_passage_selector(const int value) const
{
    set_numeric_value(consultation_notes_passage_selector_key, value);
}


constexpr auto consultation_notes_edit_selector_key{"consultation-notes-edit-selector"};
// 0: any time; 1: last 30 days; 2: last 7 days; 3: since yesterday; 4: today.
int Database_Config_User::get_consultation_notes_edit_selector() const
{
    return get_numeric_value(consultation_notes_edit_selector_key, 0);
}

void Database_Config_User::set_consultation_notes_edit_selector(const int value) const
{
    set_numeric_value(consultation_notes_edit_selector_key, value);
}


constexpr auto consultation_notes_non_edit_selector_key{"consultation-notes-non-edit-selector"};
// 0: don't care; 1: for last 30 days; 2: for last 7 days; 3: since yesterday; 4: today.
int Database_Config_User::get_consultation_notes_non_edit_selector() const
{
    return get_numeric_value(consultation_notes_non_edit_selector_key, 0);
}

void Database_Config_User::set_consultation_notes_non_edit_selector(const int value) const
{
    set_numeric_value(consultation_notes_non_edit_selector_key, value);
}


constexpr auto consultation_notes_status_selectors_key{"consultation-notes-status-selectors"};

std::vector<std::string> Database_Config_User::get_consultation_notes_status_selectors() const
{
    return get_list(consultation_notes_status_selectors_key);
}

void Database_Config_User::set_consultation_notes_status_selectors(const std::vector<std::string>& values) const
{
    set_list(consultation_notes_status_selectors_key, values);
}


constexpr auto consultation_notes_bible_selector_key{"consultation-notes-bible-selector"};
// "": any _bible; <bible>: named _bible.
std::string Database_Config_User::get_consultation_notes_bible_selector() const
{
    return get_value(consultation_notes_bible_selector_key, "");
}

void Database_Config_User::set_consultation_notes_bible_selector(const std::string& value) const
{
    set_value(consultation_notes_bible_selector_key, value);
}


constexpr auto consultation_notes_assignment_selector_key{"consultation-notes-assignment-selector"};
// "": don't care; "user": notes assigned to "user".
std::string Database_Config_User::get_consultation_notes_assignment_selector() const
{
    return get_value(consultation_notes_assignment_selector_key, "");
}

void Database_Config_User::set_consultation_notes_assignment_selector(const std::string& value) const
{
    set_value(consultation_notes_assignment_selector_key, value);
}


constexpr auto consultation_notes_subscription_selector_key{"consultation-notes-subscription-selector"};
// false: don't care; true: subscribed.
bool Database_Config_User::get_consultation_notes_subscription_selector() const
{
    return get_boolean_value(consultation_notes_subscription_selector_key, false);
}

void Database_Config_User::set_consultation_notes_subscription_selector(const bool value) const
{
    set_boolean_value(consultation_notes_subscription_selector_key, value);
}


constexpr auto consultation_notes_severity_selector_key{"consultation-notes-severity-selector"};

int Database_Config_User::get_consultation_notes_severity_selector() const
{
    return get_numeric_value(consultation_notes_severity_selector_key, -1);
}

void Database_Config_User::set_consultation_notes_severity_selector(const int value) const
{
    set_numeric_value(consultation_notes_severity_selector_key, value);
}


constexpr auto consultation_notes_text_selector_key{"consultation-notes-text-selector"};

int Database_Config_User::get_consultation_notes_text_selector() const
{
    return get_numeric_value(consultation_notes_text_selector_key, 0);
}

void Database_Config_User::set_consultation_notes_text_selector(const int value) const
{
    set_numeric_value(consultation_notes_text_selector_key, value);
}


constexpr auto consultation_notes_search_text_key{"consultation-notes-search-text"};

std::string Database_Config_User::get_consultation_notes_search_text() const
{
    return get_value(consultation_notes_search_text_key, "");
}

void Database_Config_User::set_consultation_notes_search_text(const std::string& value) const
{
    set_value(consultation_notes_search_text_key, value);
}


constexpr auto consultation_notesPassage_inclusion_selector_key{"consultation-notes-passage-inclusion-selector"};

int Database_Config_User::get_consultation_notes_passage_inclusion_selector() const
{
    return get_numeric_value(consultation_notesPassage_inclusion_selector_key, 0);
}

void Database_Config_User::set_consultation_notes_passage_inclusion_selector(const int value) const
{
    set_numeric_value(consultation_notesPassage_inclusion_selector_key, value);
}


constexpr auto consultation_notes_text_inclusion_selector_key{"consultation-notes-text-inclusion-selector"};

int Database_Config_User::get_consultation_notes_text_inclusion_selector() const
{
    return get_numeric_value(consultation_notes_text_inclusion_selector_key, 0);
}

void Database_Config_User::set_consultation_notes_text_inclusion_selector(const int value) const
{
    set_numeric_value(consultation_notes_text_inclusion_selector_key, value);
}


constexpr auto bible_changes_notification_key{"bible-changes-notification"};

bool Database_Config_User::get_bible_changes_notification() const
{
    return get_boolean_value(bible_changes_notification_key, false);
}

bool Database_Config_User::get_user_bible_changes_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, bible_changes_notification_key, false);
}

void Database_Config_User::set_bible_changes_notification(const bool value) const
{
    set_boolean_value(bible_changes_notification_key, value);
}


constexpr auto deleted_consultation_note_notification_key{"deleted-consultation-note-notification"};

bool Database_Config_User::get_deleted_consultation_note_notification() const
{
    return get_boolean_value(deleted_consultation_note_notification_key, false);
}

bool Database_Config_User::get_user_deleted_consultation_note_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, deleted_consultation_note_notification_key, false);
}

void Database_Config_User::set_deleted_consultation_note_notification(const bool value) const
{
    set_boolean_value(deleted_consultation_note_notification_key, value);
}


bool Database_Config_User::default_bible_checks_notification() const
{
    const int level = m_webserver_request.session_logic()->get_level();
    return level >= roles::translator and level <= roles::manager;
}

constexpr auto bible_checks_notification_key{"bible-checks-notification"};

bool Database_Config_User::get_bible_checks_notification() const
{
    return get_boolean_value(bible_checks_notification_key, default_bible_checks_notification());
}

bool Database_Config_User::get_user_bible_checks_notification(const std::string& username) const
{
    return get_boolean_value_for_user(username, bible_checks_notification_key, default_bible_checks_notification());
}

void Database_Config_User::set_bible_checks_notification(const bool value) const
{
    set_boolean_value(bible_checks_notification_key, value);
}


constexpr auto pending_changes_notification_key{"pending-changes-notification"};

bool Database_Config_User::get_pending_changes_notification() const
{
    return get_boolean_value(pending_changes_notification_key, false);
}

bool Database_Config_User::get_user_pending_changes_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, pending_changes_notification_key, false);
}

void Database_Config_User::set_pending_changes_notification(const bool value) const
{
    set_boolean_value(pending_changes_notification_key, value);
}


constexpr auto user_changes_notification_key{"user-changes-notification"};

bool Database_Config_User::get_user_changes_notification() const
{
    return get_boolean_value(user_changes_notification_key, false);
}

bool Database_Config_User::get_user_user_changes_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, user_changes_notification_key, false);
}

void Database_Config_User::set_user_changes_notification(const bool value) const
{
    set_boolean_value(user_changes_notification_key, value);
}


constexpr auto assigned_notes_statistics_notification_key{"assigned-notes-statistics-notification"};

bool Database_Config_User::get_assigned_notes_statistics_notification() const
{
    return get_boolean_value(assigned_notes_statistics_notification_key, false);
}

bool Database_Config_User::get_user_assigned_notes_statistics_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, assigned_notes_statistics_notification_key, false);
}

void Database_Config_User::set_assigned_notes_statistics_notification(const bool value) const
{
    set_boolean_value(assigned_notes_statistics_notification_key, value);
}


constexpr auto subscribed_notes_statistics_notification_key{"subscribed-notes-statistics-notification"};

bool Database_Config_User::get_subscribed_notes_statistics_notification() const
{
    return get_boolean_value(subscribed_notes_statistics_notification_key, false);
}

bool Database_Config_User::get_user_subscribed_notes_statistics_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, subscribed_notes_statistics_notification_key, false);
}

void Database_Config_User::set_subscribed_notes_statistics_notification(const bool value) const
{
    set_boolean_value(subscribed_notes_statistics_notification_key, value);
}


constexpr auto notify_me_of_my_posts_key{"notify-me-of-my-posts"};

bool Database_Config_User::get_notify_me_of_my_posts() const
{
    return get_boolean_value(notify_me_of_my_posts_key, true);
}

bool Database_Config_User::get_user_notify_me_of_my_posts(const std::string& username)
{
    return get_boolean_value_for_user(username, notify_me_of_my_posts_key, true);
}

void Database_Config_User::set_notify_me_of_my_posts(const bool value) const
{
    set_boolean_value(notify_me_of_my_posts_key, value);
}


constexpr auto suppress_mail_my_updated_notes_key{"suppress-mail-my-updated-notes"};

bool Database_Config_User::get_suppress_mail_from_your_updates_notes() const
{
    return get_boolean_value(suppress_mail_my_updated_notes_key, false);
}

bool Database_Config_User::get_user_suppress_mail_from_your_updates_notes(const std::string& username)
{
    return get_boolean_value_for_user(username, suppress_mail_my_updated_notes_key, false);
}

void Database_Config_User::set_suppress_mail_from_your_updates_notes(const bool value) const
{
    set_boolean_value(suppress_mail_my_updated_notes_key, value);
}


constexpr auto active_resources_key{"active-resources"};

std::vector<std::string> Database_Config_User::get_active_resources() const
{
    // _default values.
    return get_list(active_resources_key);
}

void Database_Config_User::set_active_resources(const std::vector<std::string>& values) const
{
    set_list(active_resources_key, values);
}


constexpr auto consistency_bibles_key{"consistency-bibles"};

std::vector<std::string> Database_Config_User::get_consistency_resources() const
{
    return get_list(consistency_bibles_key);
}

void Database_Config_User::set_consistency_resources(const std::vector<std::string>& values) const
{
    set_list(consistency_bibles_key, values);
}


int Database_Config_User::get_sprint_month() const
{
    return get_numeric_value(sprint_month_key, filter::date::numerical_month(filter::date::seconds_since_epoch()));
}

void Database_Config_User::set_sprint_month(const int value) const
{
    set_numeric_value(sprint_month_key, value);
}


int Database_Config_User::get_sprint_year() const
{
    return get_numeric_value(sprint_year_key, filter::date::numerical_year(filter::date::seconds_since_epoch()));
}

void Database_Config_User::set_sprint_year(const int value) const
{
    set_numeric_value(sprint_year_key, value);
}


constexpr auto sprint_progress_notification_key{"sprint-progress-notification"};

bool Database_Config_User::get_sprint_progress_notification() const
{
    return get_boolean_value(sprint_progress_notification_key, false);
}

bool Database_Config_User::get_user_sprint_progress_notification(const std::string& username)
{
    return get_boolean_value_for_user(username, sprint_progress_notification_key, false);
}

void Database_Config_User::set_sprint_progress_notification(const bool value) const
{
    set_boolean_value(sprint_progress_notification_key, value);
}


constexpr auto user_changes_notifications_online_key{"user-changes-notifications-online"};

bool Database_Config_User::get_user_changes_notifications_online() const
{
    return get_boolean_value(user_changes_notifications_online_key, false);
}

bool Database_Config_User::get_user_user_changes_notifications_online(const std::string& username)
{
    return get_boolean_value_for_user(username, user_changes_notifications_online_key, false);
}

void Database_Config_User::set_user_changes_notifications_online(const bool value) const
{
    set_boolean_value(user_changes_notifications_online_key, value);
}


constexpr auto contributor_changes_notifications_online_key{"contributor-changes-notifications-online"};

bool Database_Config_User::get_contributor_changes_notifications_online() const
{
    return get_boolean_value(contributor_changes_notifications_online_key, false);
}

bool Database_Config_User::get_contributor_changes_notifications_online(const std::string& username)
{
    return get_boolean_value_for_user(username, contributor_changes_notifications_online_key, false);
}

void Database_Config_User::set_contributor_changes_notifications_online(const bool value) const
{
    set_boolean_value(contributor_changes_notifications_online_key, value);
}


constexpr auto workbench_urls_key{"workbench-urls"};

std::string Database_Config_User::get_workspace_urls() const
{
    return get_value(workbench_urls_key, "");
}

void Database_Config_User::set_workspace_urls(const std::string& value) const
{
    set_value(workbench_urls_key, value);
}


constexpr auto workbench_widths_key{"workbench-widths"};

std::string Database_Config_User::get_workspace_widths() const
{
    return get_value(workbench_widths_key, "");
}

void Database_Config_User::set_workspace_widths(const std::string& value) const
{
    set_value(workbench_widths_key, value);
}


constexpr auto workbench_heights_key{"workbench-heights"};

std::string Database_Config_User::get_workspace_heights() const
{
    return get_value(workbench_heights_key, "");
}

void Database_Config_User::set_workspace_heights(const std::string& value) const
{
    set_value(workbench_heights_key, value);
}


constexpr auto entire_workbench_widths_key{"entire-workbench-widths"};

std::string Database_Config_User::get_entire_workspace_widths() const
{
    return get_value(entire_workbench_widths_key, "");
}

void Database_Config_User::set_entire_workspace_widths(const std::string& value) const
{
    set_value(entire_workbench_widths_key, value);
}


constexpr auto active_workbench_key{"active-workbench"};

std::string Database_Config_User::get_active_workspace() const
{
    return get_value(active_workbench_key, "");
}

void Database_Config_User::set_active_workspace(const std::string& value) const
{
    set_value(active_workbench_key, value);
}


constexpr auto postpone_new_notes_mails_key{"postpone-new-notes-mails"};

bool Database_Config_User::get_postpone_new_notes_mails() const
{
    return get_boolean_value(postpone_new_notes_mails_key, false);
}

void Database_Config_User::set_postpone_new_notes_mails(const bool value) const
{
    set_boolean_value(postpone_new_notes_mails_key, value);
}


constexpr auto recently_applied_styles_key{"recently-applied-styles"};

std::string Database_Config_User::get_recently_applied_styles() const
{
    return get_value(recently_applied_styles_key, "p s add nd f x v");
}

void Database_Config_User::set_recently_applied_styles(const std::string& values) const
{
    set_value(recently_applied_styles_key, values);
}


constexpr auto focused_book_key{"focused-book"};

std::vector<int> Database_Config_User::get_focused_books() const
{
    return get_numeric_list(focused_book_key);
}

void Database_Config_User::set_focused_books(const std::vector<int>& books) const
{
    set_numeric_list(focused_book_key, books);
}


constexpr auto focused_chapter_key{"focused-chapter"};

std::vector<int> Database_Config_User::get_focused_chapters() const
{
    return get_numeric_list(focused_chapter_key);
}

void Database_Config_User::set_focused_chapters(const std::vector<int>& chapters) const
{
    set_numeric_list(focused_chapter_key, chapters);
}


constexpr auto focused_verse_key{"focused-verse"};

std::vector<int> Database_Config_User::get_focused_verses() const
{
    return get_numeric_list(focused_verse_key);
}

void Database_Config_User::set_focused_verses(const std::vector<int>& verses) const
{
    set_numeric_list(focused_verse_key, verses);
}


constexpr auto updated_settings_key{"updated-settings"};

std::vector<int> Database_Config_User::get_updated_settings() const
{
    return get_numeric_list(updated_settings_key);
}

void Database_Config_User::set_updated_settings(const std::vector<int>& values) const
{
    set_numeric_list(updated_settings_key, values);
}

void Database_Config_User::add_updated_setting(const int value) const
{
    std::vector<int> settings = get_updated_settings();
    settings.push_back(value);
    settings = filter::string::array_unique(settings);
    set_updated_settings(settings);
}

void Database_Config_User::remove_updated_setting(const int value) const
{
    std::vector<int> settings = get_updated_settings();
    const std::vector against{value};
    settings = filter::string::array_diff(settings, against);
    set_updated_settings(settings);
}


constexpr auto removed_changes_key{"removed-changes"};

std::vector<int> Database_Config_User::get_removed_changes() const
{
    return get_numeric_list(removed_changes_key);
}

void Database_Config_User::set_removed_changes(const std::vector<int>& values) const
{
    set_numeric_list(removed_changes_key, values);
}

void Database_Config_User::add_removed_change(const int value) const
{
    std::vector<int> settings = get_removed_changes();
    settings.push_back(value);
    settings = filter::string::array_unique(settings);
    set_removed_changes(settings);
}

void Database_Config_User::remove_removed_change(const int value) const
{
    std::vector<int> settings = get_removed_changes();
    std::vector against{value};
    settings = filter::string::array_diff(settings, against);
    set_removed_changes(settings);
}


constexpr auto change_notifications_checksum_key{"change-notifications-checksum"};

std::string Database_Config_User::get_change_notifications_checksum() const
{
    return get_value(change_notifications_checksum_key, "");
}

void Database_Config_User::set_change_notifications_checksum(const std::string& value) const
{
    set_value(change_notifications_checksum_key, value);
}

void Database_Config_User::set_user_change_notifications_checksum(const std::string& user,
                                                                  const std::string& value)
{
    set_value_for_user(user, change_notifications_checksum_key, value);
}


constexpr auto live_bible_editor_key{"live-bible-editor"};

int Database_Config_User::get_live_bible_editor() const
{
    return get_numeric_value(live_bible_editor_key, 0);
}

void Database_Config_User::set_live_bible_editor(const int time) const
{
    set_numeric_value(live_bible_editor_key, time);
}


constexpr auto resource_verses_before_key{"resource-verses-before"};

int Database_Config_User::get_resource_verses_before() const
{
    return get_numeric_value(resource_verses_before_key, 0);
}

void Database_Config_User::set_resource_verses_before(const int verses) const
{
    set_numeric_value(resource_verses_before_key, verses);
}


constexpr auto resource_verses_after_key{"resource-verses-after"};

int Database_Config_User::get_resource_verses_after() const
{
    return get_numeric_value(resource_verses_after_key, 0);
}

void Database_Config_User::set_resource_verses_after(int verses) const
{
    set_numeric_value(resource_verses_after_key, verses);
}


constexpr auto sync_key_key{"sync-key"};
// _encryption key storage on server.
std::string Database_Config_User::get_sync_key() const
{
    return get_value(sync_key_key, "");
}

void Database_Config_User::set_sync_key(const std::string& key) const
{
    set_value(sync_key_key, key);
}


constexpr auto general_font_size_key{"general-font-size"};

int Database_Config_User::get_general_font_size() const
{
    // _default value, see https://github.com/bibledit/cloud/issues/509
    return get_numeric_value(general_font_size_key, 112);
}

void Database_Config_User::set_general_font_size(int size) const
{
    set_numeric_value(general_font_size_key, size);
}


constexpr auto menu_font_size_key{"menu-font-size"};

int Database_Config_User::get_menu_font_size() const
{
    // _default value, see https://github.com/bibledit/cloud/issues/509
    return get_numeric_value(menu_font_size_key, 112);
}

void Database_Config_User::set_menu_font_size(int size) const
{
    set_numeric_value(menu_font_size_key, size);
}


constexpr auto bible_editors_font_size_key{"bible-editors-font-size"};

int Database_Config_User::get_bible_editors_font_size() const
{
    return get_numeric_value(bible_editors_font_size_key, 100);
}

void Database_Config_User::set_bible_editors_font_size(int size) const
{
    set_numeric_value(bible_editors_font_size_key, size);
}


constexpr auto resources_font_size_key{"resources-font-size"};

int Database_Config_User::get_resources_font_size() const
{
    return get_numeric_value(resources_font_size_key, 100);
}

void Database_Config_User::set_resources_font_size(int size) const
{
    set_numeric_value(resources_font_size_key, size);
}


constexpr auto hebrew_font_size_key{"hebrew-font-size"};

int Database_Config_User::get_hebrew_font_size() const
{
    return get_numeric_value(hebrew_font_size_key, 100);
}

void Database_Config_User::set_hebrew_font_size(int size) const
{
    set_numeric_value(hebrew_font_size_key, size);
}


constexpr auto greek_font_size_key{"greek-font-size"};

int Database_Config_User::get_greek_font_size() const
{
    return get_numeric_value(greek_font_size_key, 100);
}

void Database_Config_User::set_greek_font_size(int size) const
{
    set_numeric_value(greek_font_size_key, size);
}


constexpr auto vertical_caret_position_key{"vertical-caret-position"};

int Database_Config_User::get_vertical_caret_position() const
{
    // _updated default value, see https://github.com/bibledit/cloud/issues/509
    return get_numeric_value(vertical_caret_position_key, 30);
}

void Database_Config_User::set_vertical_caret_position(int position) const
{
    set_numeric_value(vertical_caret_position_key, position);
}


constexpr auto current_theme_style_key{"current-theme-style"};

int Database_Config_User::get_current_theme() const
{
    return get_numeric_value(current_theme_style_key, 0);
}

void Database_Config_User::set_current_theme(int index) const
{
    set_numeric_value(current_theme_style_key, index);
}


constexpr auto workspace_menu_fadeout_delay_key{"workspace-menu-fadeout-delay"};

int Database_Config_User::get_workspace_menu_fadeout_delay() const
{
    return get_numeric_value(workspace_menu_fadeout_delay_key, 4);
}

void Database_Config_User::set_workspace_menu_fadeout_delay(const int value) const
{
    set_numeric_value(workspace_menu_fadeout_delay_key, value);
}


constexpr auto editing_allowed_difference_chapter_key{"editing-allowed-difference-chapter"};

int Database_Config_User::get_editing_allowed_difference_chapter() const
{
    return get_numeric_value(editing_allowed_difference_chapter_key, 20);
}

void Database_Config_User::set_editing_allowed_difference_chapter(const int value) const
{
    set_numeric_value(editing_allowed_difference_chapter_key, value);
}


constexpr auto editing_allowed_difference_verse_key{"editing-allowed-difference-verse"};

int Database_Config_User::get_editing_allowed_difference_verse() const
{
    return get_numeric_value(editing_allowed_difference_verse_key, 75);
}

void Database_Config_User::set_editing_allowed_difference_verse(const int value) const
{
    set_numeric_value(editing_allowed_difference_verse_key, value);
}


bool Database_Config_User::get_basic_interface_mode_default() const
{
    // _touch devices default to basic mode.
#ifdef _h_a_v_e__a_n_d_r_o_i_d
    return true;
#endif
#ifdef _h_a_v_e__i_o_s
    return true;
#endif
    // _the app running on a workspace or laptop have default to basic mode for a lower role.
    if (const int level = m_webserver_request.session_logic()->get_level(); level <= roles::manager)
        return true;
    // _higher role: default to advanced mode.
    return false;
}

constexpr auto basic_interface_mode_key{"basic-interface-mode"};

bool Database_Config_User::get_basic_interface_mode() const
{
    return get_boolean_value(basic_interface_mode_key, get_basic_interface_mode_default());
}

void Database_Config_User::set_basic_interface_mode(const bool value) const
{
    set_boolean_value(basic_interface_mode_key, value);
}


constexpr auto main_menu_always_visible_key{"main-menu-always-visible"};

bool Database_Config_User::get_main_menu_always_visible() const
{
    // _default visible in basic mode.
    // _advanced mode: _by default it is not visible.
    return get_boolean_value(main_menu_always_visible_key, get_basic_interface_mode_default());
}

void Database_Config_User::set_main_menu_always_visible(const bool value) const
{
    set_boolean_value(main_menu_always_visible_key, value);
}


constexpr auto swipe_actions_available_key{"swipe-actions-available"};

bool Database_Config_User::get_swipe_actions_available() const
{
    return get_boolean_value(swipe_actions_available_key, true);
}

void Database_Config_User::set_swipe_actions_available(const bool value) const
{
    set_boolean_value(swipe_actions_available_key, value);
}


constexpr auto fast_editor_switching_available_key{"fast-editor-switching-available"};

bool Database_Config_User::get_fast_editor_switching_available() const
{
    return get_boolean_value(fast_editor_switching_available_key, true);
}

void Database_Config_User::set_fast_editor_switching_available(const bool value) const
{
    set_boolean_value(fast_editor_switching_available_key, value);
}


constexpr auto include_related_passages_key{"include-related-passages"};

bool Database_Config_User::get_include_related_passages() const
{
    return get_boolean_value(include_related_passages_key, false);
}

void Database_Config_User::set_include_related_passages(const bool value) const
{
    set_boolean_value(include_related_passages_key, value);
}


constexpr auto enabled_visual_editors_key{"enabled-visual-editors"};

int Database_Config_User::get_fast_switch_visual_editors() const
{
    // _updated default values, see https://github.com/bibledit/cloud/issues/509
    return get_numeric_value(enabled_visual_editors_key, 0);
}

void Database_Config_User::set_fast_switch_visual_editors(const int value) const
{
    set_numeric_value(enabled_visual_editors_key, value);
}


constexpr auto enabled_usfm_editors_key{"enabled-usfm-editors"};

int Database_Config_User::get_fast_switch_usfm_editors() const
{
    // _initially only the _u_s_f_m chapter editor is enabled.
    return get_numeric_value(enabled_usfm_editors_key, 1);
}

void Database_Config_User::set_fast_switch_usfm_editors(const int value) const
{
    set_numeric_value(enabled_usfm_editors_key, value);
}


constexpr auto enable_styles_button_visual_editors_key{"enable-styles-button-visual-editors"};

bool Database_Config_User::get_enable_styles_button_visual_editors() const
{
    return get_boolean_value(enable_styles_button_visual_editors_key, true);
}

void Database_Config_User::set_enable_styles_button_visual_editors(const bool value) const
{
    set_boolean_value(enable_styles_button_visual_editors_key, value);
}


constexpr auto menu_changes_in_basic_mode_key{"menu-changes-in-basic-mode"};

bool Database_Config_User::get_menu_changes_in_basic_mode() const
{
    return get_boolean_value(menu_changes_in_basic_mode_key, false);
}

void Database_Config_User::set_menu_changes_in_basic_mode(const bool value) const
{
    set_boolean_value(menu_changes_in_basic_mode_key, value);
}


constexpr auto privilege_use_advanced_mode_key{"privilege-use-advanced-mode"};

bool Database_Config_User::get_privilege_use_advanced_mode() const
{
    return get_boolean_value(privilege_use_advanced_mode_key, true);
}

bool Database_Config_User::get_privilege_use_advanced_mode_for_user(const std::string& username)
{
    return get_boolean_value_for_user(username, privilege_use_advanced_mode_key, true);
}

void Database_Config_User::set_privilege_use_advanced_mode_for_user(const std::string& username, const bool value)
{
    set_boolean_value_for_user(username, privilege_use_advanced_mode_key, value);
}


constexpr auto privilege_delete_consultation_notes_key{"privilege-delete-consultation-notes"};

bool Database_Config_User::get_privilege_delete_consultation_notes() const
{
    return get_boolean_value(privilege_delete_consultation_notes_key, false);
}

void Database_Config_User::set_privilege_delete_consultation_notes(const bool value) const
{
    set_boolean_value(privilege_delete_consultation_notes_key, value);
}

bool Database_Config_User::get_privilege_delete_consultation_notes_for_user(const std::string& username)
{
    return get_boolean_value_for_user(username, privilege_delete_consultation_notes_key, false);
}

void Database_Config_User::set_privilege_delete_consultation_notes_for_user(
    const std::string& username, bool value)
{
    set_boolean_value_for_user(username, privilege_delete_consultation_notes_key, value);
}


constexpr auto privilege_set_stylesheets_key{"privilege-set-stylesheets"};

bool Database_Config_User::get_privilege_set_stylesheets() const
{
    return get_boolean_value(privilege_set_stylesheets_key, false);
}

bool Database_Config_User::get_privilege_set_stylesheets_for_user(const std::string& username)
{
    return get_boolean_value_for_user(username, privilege_set_stylesheets_key, false);
}

void Database_Config_User::set_privilege_set_stylesheets_for_user(const std::string& username, bool value)
{
    set_boolean_value_for_user(username, privilege_set_stylesheets_key, value);
}


constexpr auto dismiss_changes_at_top_key{"dismiss-changes-at-top"};

bool Database_Config_User::get_dismiss_changes_at_top() const
{
    return get_boolean_value(dismiss_changes_at_top_key, false);
}

void Database_Config_User::set_dismiss_changes_at_top(const bool value) const
{
    set_boolean_value(dismiss_changes_at_top_key, value);
}


constexpr auto quick_note_edit_link_key{"quick-note-edit-link"};

bool Database_Config_User::get_quick_note_edit_link() const
{
    return get_boolean_value(quick_note_edit_link_key, false);
}

void Database_Config_User::set_quick_note_edit_link(const bool value) const
{
    set_boolean_value(quick_note_edit_link_key, value);
}


constexpr auto show_bible_in_notes_list_key{"show-bible-in-notes-list"};

bool Database_Config_User::get_show_bible_in_notes_list() const
{
    return get_boolean_value(show_bible_in_notes_list_key, false);
}

void Database_Config_User::set_show_bible_in_notes_list(const bool value) const
{
    set_boolean_value(show_bible_in_notes_list_key, value);
}


constexpr auto show_note_status_key{"show-note-status"};

bool Database_Config_User::get_show_note_status() const
{
    return get_boolean_value(show_note_status_key, false);
}

void Database_Config_User::set_show_note_status(const bool value) const
{
    set_boolean_value(show_note_status_key, value);
}


constexpr auto show_verse_text_at_create_note_key{"show-verse-text-at-create-note"};

bool Database_Config_User::get_show_verse_text_at_create_note() const
{
    return get_boolean_value(show_verse_text_at_create_note_key, false);
}

void Database_Config_User::set_show_verse_text_at_create_note(const bool value) const
{
    set_boolean_value(show_verse_text_at_create_note_key, value);
}


constexpr auto order_changes_by_author_key{"order-changes-by-author"};

bool Database_Config_User::get_order_changes_by_author() const
{
    return get_boolean_value(order_changes_by_author_key, false);
}

void Database_Config_User::set_order_changes_by_author(const bool value) const
{
    set_boolean_value(order_changes_by_author_key, value);
}


constexpr auto automatic_note_assignment_key{"automatic-note-assignment"};

std::vector<std::string> Database_Config_User::get_automatic_note_assignment() const
{
    return get_list(automatic_note_assignment_key);
}

void Database_Config_User::set_automatic_note_assignment(const std::vector<std::string>& values) const
{
    set_list(automatic_note_assignment_key, values);
}


constexpr auto receive_focused_reference_from_paratext_key{"receive-focused-reference-from-paratext"};

bool Database_Config_User::get_receive_focused_reference_from_paratext() const
{
    return get_boolean_value(receive_focused_reference_from_paratext_key, true);
}

void Database_Config_User::set_receive_focused_reference_from_paratext(const bool value) const
{
    set_boolean_value(receive_focused_reference_from_paratext_key, value);
}


constexpr auto receive_focused_reference_from_accordance_key{"receive-focused-reference-from-accordance"};

bool Database_Config_User::get_receive_focused_reference_from_accordance() const
{
    return get_boolean_value(receive_focused_reference_from_accordance_key, true);
}

void Database_Config_User::set_receive_focused_reference_from_accordance(const bool value) const
{
    set_boolean_value(receive_focused_reference_from_accordance_key, value);
}


constexpr auto use_colored_note_status_labels_key{"use-colored-note-status-labels"};

bool Database_Config_User::get_use_colored_note_status_labels() const
{
    return get_boolean_value(use_colored_note_status_labels_key, false);
}

void Database_Config_User::set_use_colored_note_status_labels(const bool value) const
{
    set_boolean_value(use_colored_note_status_labels_key, value);
}


constexpr auto notes_date_format_key{"notes_date-format"};

int Database_Config_User::get_notes_date_format() const
{
    return get_numeric_value(notes_date_format_key, 0);
}

void Database_Config_User::set_notes_date_format(const int value) const
{
    set_numeric_value(notes_date_format_key, value);
}


constexpr auto change_notifications_bibles_key{"change-notifications-bibles"};

std::vector<std::string> Database_Config_User::get_change_notifications_bibles() const
{
    return get_list(change_notifications_bibles_key);
}

std::vector<std::string> Database_Config_User::get_change_notifications_bibles_for_user(const std::string& user)
{
    return get_list_for_user(user, change_notifications_bibles_key);
}

void Database_Config_User::set_change_notifications_bibles(const std::vector<std::string>& values) const
{
    set_list(change_notifications_bibles_key, values);
}


constexpr auto enable_spell_check_key{"enable-spell-check"};

bool Database_Config_User::get_enable_spell_check() const
{
    return get_boolean_value(enable_spell_check_key, true);
}

void Database_Config_User::set_enable_spell_check(const bool value) const
{
    set_boolean_value(enable_spell_check_key, value);
}


constexpr auto show_navigation_arrows_key{"show-navigation-arrows"};

bool Database_Config_User::get_show_navigation_arrows() const
{
    // On systems usually without a touch screen, the navigation arrows are on by default.
    // On mobile devices they will be off by default.
#if defined(HAVE_CLOUD)
    constexpr const bool default_value{true};
#elif defined(HAVE_WINDOWS)
    constexpr const bool default_value{true};
#elif defined(HAVE_MACOS)
    constexpr const bool default_value{true};
#elif defined(HAVE_LINUX)
    constexpr const bool default_value{true};
#elif defined(HAVE_ANDROID)
    constexpr const bool default_value{false};
#elif defined(HAVE_IOS)
    constexpr const bool default_value{false};
#endif
    return get_boolean_value(show_navigation_arrows_key, default_value);
}

void Database_Config_User::set_show_navigation_arrows(const bool value) const
{
    set_boolean_value(show_navigation_arrows_key, value);
}
