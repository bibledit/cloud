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


#include <database/config/general.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <config/globals.h>
#include <system/index.h>
#include <database/logic.h>


namespace database::config::general {


// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
std::map <std::string, std::string> database_config_general_cache;


// Functions for getting and setting values or lists of values follow here:


static std::string file (const char * key)
{
  return filter_url_create_root_path ({database_logic_databases (), "config", "general", key});
}


static std::string get_value (const char * key, const char * default_value)
{
  // Check the memory cache.
  if (database_config_general_cache.count (key)) {
    return database_config_general_cache [key];
  }
  // Get value from disk.
  std::string value;
  const std::string filename = file (key);
  if (file_or_dir_exists (filename))
    value = filter_url_file_get_contents (filename);
  else 
    value = default_value;
  // Cache it.
  database_config_general_cache [key] = value;
  // Done.
  return value;
}


static void set_value (const char * key, const std::string& value)
{
  // Store in memory cache.
  database_config_general_cache [key] = value;
  // Store on disk.
  const std::string filename = file (key);
  filter_url_file_put_contents (filename, value);
}


static bool get_boolean_value (const char * key, bool default_value)
{
  const std::string value = get_value (key, filter::strings::convert_to_string (default_value).c_str());
  return filter::strings::convert_to_bool (value);
}


static void set_boolean_value (const char * key, bool value)
{
  set_value (key, filter::strings::convert_to_string (value).c_str());
}


static int get_integer_value (const char * key, int default_value)
{
  const std::string value = get_value (key, std::to_string (default_value).c_str());
  return filter::strings::convert_to_int (value);
}


static void set_integer_value (const char * key, int value)
{
  set_value (key, std::to_string (value).c_str());
}


static std::vector <std::string> get_list (const char * key)
{
  const std::string contents = get_value (key, "");
  return filter::strings::explode (contents, '\n');
}


static void set_list (const char * key, const std::vector <std::string>& values)
{
  const std::string value = filter::strings::implode (values, "\n");
  set_value (key, value);
}


// Named configuration functions.


constexpr const auto site_mail_name_key {"site-mail-name"};
std::string get_site_mail_name ()
{
  return get_value (site_mail_name_key, "Cloud");
}
void set_site_mail_name (const std::string& value)
{
  set_value (site_mail_name_key, value);
}


constexpr const auto site_mail_address_key {"site-mail-address"};
std::string get_site_mail_address ()
{
  return get_value (site_mail_address_key, "");
}
void set_site_mail_address (const std::string& value)
{
  set_value (site_mail_address_key, value);
}


constexpr const auto mail_storage_host_key {"mail-storage-host"};
std::string get_mail_storage_host ()
{
  return get_value (mail_storage_host_key, "");
}
void set_mail_storage_host (const std::string& value)
{
  set_value (mail_storage_host_key, value);
}


constexpr const auto mail_storage_username_key {"mail-storage-username"};
std::string get_mail_storage_username ()
{
  return get_value (mail_storage_username_key, "");
}
void set_mail_storage_username (const std::string& value)
{
  set_value (mail_storage_username_key, value);
}


constexpr const auto mail_storage_password_key {"mail-storage-password"};
std::string get_mail_storage_password ()
{
  return get_value (mail_storage_password_key, "");
}
void set_mail_storage_password (const std::string& value)
{
  set_value (mail_storage_password_key, value);
}


constexpr const auto mail_storage_protocol_key {"mail-storage-protocol"};
std::string get_mail_storage_protocol ()
{
  return get_value (mail_storage_protocol_key, "");
}
void set_mail_storage_protocol (const std::string& value)
{
  set_value (mail_storage_protocol_key, value);
}


constexpr const auto mail_storage_port_key {"mail-storage-port"};
std::string get_mail_storage_port ()
{
  return get_value (mail_storage_port_key, "");
}
void set_mail_storage_port (const std::string& value)
{
  set_value (mail_storage_port_key, value);
}


constexpr const auto mail_send_host_key {"mail-send-host"};
std::string get_mail_send_host ()
{
  return get_value (mail_send_host_key, "");
}
void set_mail_send_host (const std::string& value)
{
  set_value (mail_send_host_key, value);
}


constexpr const auto mail_send_username_key {"mail-send-username"};
std::string get_mail_send_username ()
{
  return get_value (mail_send_username_key, "");
}
void set_mail_send_username (const std::string& value)
{
  set_value (mail_send_username_key, value);
}


constexpr const auto mail_send_password_key {"mail-send-password"};
std::string get_mail_send_password ()
{
  return get_value (mail_send_password_key, "");
}
void set_mail_send_password (const std::string& value)
{
  set_value (mail_send_password_key, value);
}


constexpr const auto mail_send_port_key {"mail-send-port"};
std::string get_mail_send_port ()
{
  return get_value (mail_send_port_key, "");
}
void set_mail_send_port (const std::string& value)
{
  set_value (mail_send_port_key, value);
}


constexpr const auto timer_minute_key {"timer-minute"};
std::string get_timer_minute ()
{
  return get_value (timer_minute_key, "");
}
void set_timer_minute (const std::string& value)
{
  set_value (timer_minute_key, value);
}


constexpr const auto timezone_key {"timezone"};
int get_timezone ()
{
  // If the global offset variable is set, that is,
  // within certain limits, then take that.
  if ((config_globals_timezone_offset_utc < MINIMUM_TIMEZONE)
      || (config_globals_timezone_offset_utc > MAXIMUM_TIMEZONE)) {
    return get_integer_value (timezone_key, 0);
  }
  // Else take variable as set in the configuration.
  return config_globals_timezone_offset_utc;
}
void set_timezone (int value)
{
  set_integer_value (timezone_key, value);
}


constexpr const auto site_url_key {"site-url"};
std::string get_site_url ()
{
  // The site URL is set upon login, normally.
  // In a client setup, there is never a login.
  // Consequently the site URL is never set.
#ifdef HAVE_CLIENT
  // In case of a client, return a predefined URL.
  std::string url = "http://localhost:";
  url.append (::config::logic::http_network_port ());
  url.append ("/");
  return url;
#else
  // Get the URL that was set upon login.
  return get_value (site_url_key, "");
#endif
}
void set_site_url (const std::string& value)
{
  set_value (site_url_key, value);
}


constexpr const auto general_site_language_key {"site-language"};
std::string get_site_language ()
{
  // The default site language is an empty string.
  // It means not to localize the interface.
  // Since the default messages are all in English,
  // the default language for the interface will be English.
  return get_value (general_site_language_key, "");
}
void set_site_language (const std::string& value)
{
  set_value (general_site_language_key, value);
}


constexpr const auto client_mode_key {"client-mode"};
bool get_client_mode ()
{
  return get_boolean_value (client_mode_key, false);
}
void set_client_mode (bool value)
{
  set_boolean_value (client_mode_key, value);
}


constexpr const auto server_address_key {"server-address"};
std::string get_server_address ()
{
  return get_value (server_address_key, "");
}
void set_server_address (const std::string& value)
{
  set_value (server_address_key, value);
}


constexpr const auto server_port_key {"server-port"};
int get_server_port ()
{
  return get_integer_value (server_port_key, 8080);
}
void set_server_port (int value)
{
  set_integer_value (server_port_key, value);
}


constexpr const auto repeat_send_receive_key {"repeat-send-receive"};
int get_repeat_send_receive ()
{
  return get_integer_value (repeat_send_receive_key, 0);
}
void set_repeat_send_receive (int value)
{
  set_integer_value (repeat_send_receive_key, value);
}


constexpr const auto last_send_receive_key {"last-send-receive"};
int get_last_send_receive ()
{
  return get_integer_value (last_send_receive_key, 0);
}
void set_last_send_receive (int value)
{
  set_integer_value (last_send_receive_key, value);
}


constexpr const auto installed_interface_version_key {"installed-interface-version"};
std::string get_installed_interface_version ()
{
  return get_value (installed_interface_version_key, "");
}
void set_installed_interface_version (const std::string& value)
{
  set_value (installed_interface_version_key, value);
}


constexpr const auto installed_database_version_key {"installed-database-version"};
std::string getInstalledDatabaseVersion ()
{
  return get_value (installed_database_version_key, "");
}
void setInstalledDatabaseVersion (const std::string& value)
{
  set_value (installed_database_version_key, value);
}


constexpr const auto just_started_key {"just-started"};
bool getJustStarted ()
{
  return get_boolean_value (just_started_key, false);
}
void setJustStarted (bool value)
{
  set_boolean_value (just_started_key, value);
}


constexpr const auto paratext_projects_folder_key {"paratext-projects-folder"};
std::string get_paratext_projects_folder ()
{
  return get_value (paratext_projects_folder_key, "");
}
void set_paratext_projects_folder (const std::string& value)
{
  set_value (paratext_projects_folder_key, value);
}


// Encryption / decryption key storage on client.
constexpr const auto sync_key_key {"sync-key"};
std::string get_sync_key ()
{
  return get_value (sync_key_key, "");
}
void set_sync_key (const std::string& key)
{
  set_value (sync_key_key, key);
}


constexpr const auto last_menu_click_key {"last-menu-click"};
std::string get_last_menu_click ()
{
  return get_value (last_menu_click_key, "");
}
void set_last_menu_click (const std::string& url)
{
  set_value (last_menu_click_key, url);
}


// Store the resources to be cached.
// The format is this:
// <resource title><space><book number>
constexpr const auto resources_to_cache_key {"resources-to-cache"};
std::vector <std::string> get_resources_to_cache ()
{
  return get_list (resources_to_cache_key);
}
void set_resources_to_cache (const std::vector <std::string>& values)
{
  set_list (resources_to_cache_key, values);
}


constexpr const auto index_notes_key {"index-notes"};
bool getIndexNotes ()
{
  return get_boolean_value (index_notes_key, false);
}
void setIndexNotes (bool value)
{
  set_boolean_value (index_notes_key, value);
}


constexpr const auto index_bibles_key {"index-bibles"};
bool get_index_bibles ()
{
  return get_boolean_value (index_bibles_key, false);
}
void set_index_bibles (bool value)
{
  set_boolean_value (index_bibles_key, value);
}


constexpr const auto unsent_bible_data_time_key {"unsent-bible-data-time"};
int get_unsent_bible_data_time ()
{
  return get_integer_value (unsent_bible_data_time_key, 0);
}
void set_unsent_bible_data_time (int value)
{
  set_integer_value (unsent_bible_data_time_key, value);
}


constexpr const auto unreceived_bible_data_time_key {"unreceived-bible-data-time"};
int get_unreceived_bible_data_time ()
{
  return get_integer_value (unreceived_bible_data_time_key, 0);
}
void set_unreceived_bible_data_time (int value)
{
  set_integer_value (unreceived_bible_data_time_key, value);
}


constexpr const auto author_in_rss_feed_key {"author-in-rss-feed"};
bool get_author_in_rss_feed ()
{
  return get_boolean_value (author_in_rss_feed_key, false);
}
void set_uuthor_in_rss_feed (bool value)
{
  set_boolean_value (author_in_rss_feed_key, value);
}


constexpr const auto just_connected_to_cloud_key {"just-connected-to-cloud"};
bool get_just_connected_to_cloud ()
{
  return get_boolean_value (just_connected_to_cloud_key, false);
}
void set_just_connected_to_cloud (bool value)
{
  set_boolean_value (just_connected_to_cloud_key, value);
}


constexpr const auto menu_in_tabbed_view_on_key {"menu-in-tabbed-view-on"};
bool get_menu_in_tabbed_view_on ()
{
  return get_boolean_value (menu_in_tabbed_view_on_key, true);
}
void set_menu_in_tabbed_view_on (bool value)
{
  set_boolean_value (menu_in_tabbed_view_on_key, value);
}


constexpr const auto menu_in_tabbed_view_json_key {"menu-in-tabbed-view-json"};
std::string get_menu_in_tabbed_view_json ()
{
  return get_value (menu_in_tabbed_view_json_key, "");
}
void set_menu_in_tabbed_view_json (const std::string& value)
{
  set_value (menu_in_tabbed_view_json_key, value);
}


constexpr const auto disable_selection_popup_chrome_os_key {"disable-selection-popup-chrome-os"};
bool get_disable_selection_popup_chrome_os ()
{
  return get_boolean_value (disable_selection_popup_chrome_os_key, false);
}
void set_disable_selection_popup_chrome_os (bool value)
{
  set_boolean_value (disable_selection_popup_chrome_os_key, value);
}


constexpr const auto notes_verse_separator_key {"notes-verse-separator"};
std::string get_notes_verse_separator ()
{
  // The colon is the default value. See https://github.com/bibledit/cloud/issues/509
  return get_value (notes_verse_separator_key, ":");
}
void set_notes_verse_separator (const std::string& value)
{
  set_value (notes_verse_separator_key, value);
}


constexpr const auto comparative_resources_key {"comparative-resources"};
std::vector <std::string> get_comparative_resources ()
{
  return get_list (comparative_resources_key);
}
void set_comparative_resources (const std::vector <std::string>& values)
{
  set_list (comparative_resources_key, values);
}


constexpr const auto translated_resources_key {"translated-resources"};
std::vector <std::string> get_translated_resources ()
{
  return get_list (translated_resources_key);
}
void set_translated_resources (const std::vector <std::string>& values)
{
  set_list (translated_resources_key, values);
}


constexpr const auto default_active_resources_key {"default-active-resources"};
std::vector <std::string> get_default_active_resources ()
{
  return get_list (default_active_resources_key);
}
void set_default_active_resources (const std::vector <std::string>& values)
{
  set_list (default_active_resources_key, values);
}


constexpr const auto account_creation_times_key {"account-creation-times"};
std::vector <std::string> get_account_creation_times ()
{
  return get_list (account_creation_times_key);
}
void set_account_creation_times (const std::vector <std::string>& values)
{
  set_list (account_creation_times_key, values);
}


constexpr const auto keep_resources_cache_for_long_key {"keep-resources-cache-for-long"};
bool get_keep_resources_cache_for_long ()
{
  return get_boolean_value (keep_resources_cache_for_long_key, false);
}
void set_keep_resources_cache_for_long (bool value)
{
  set_boolean_value (keep_resources_cache_for_long_key, value);
}


constexpr const auto default_new_user_access_level_key {"default-new-user-access-level"};
int get_default_new_user_access_level ()
{
  return get_integer_value (default_new_user_access_level_key, Filter_Roles::member ());
}
void set_default_new_user_access_level (int value)
{
  set_integer_value (default_new_user_access_level_key, value);
}


constexpr const auto keep_osis_content_in_sword_resources_key {"keep-osis-content-in-sword-resources"};
bool get_keep_osis_content_in_sword_resources ()
{
  return get_boolean_value (keep_osis_content_in_sword_resources_key, false);
}
void set_keep_osis_content_in_sword_resources (bool value)
{
  set_boolean_value (keep_osis_content_in_sword_resources_key, value);
}

}
