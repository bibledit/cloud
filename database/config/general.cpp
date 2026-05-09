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


#include <config/globals.h>
#include <database/logic.h>
#include <database/config/general.h>
#include <filter/string.h>
#include <filter/url.h>
#include <system/index.h>


namespace database::config::general {
// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
static std::map<std::string, std::string> cache;


// Functions for getting and setting values or lists of values follow here:


static std::string file(const char* key)
{
    return filter_url_create_root_path({database_logic_databases(), "config", "general", key});
}


template <typename T>
concept is_string_bool_int = std::is_same_v<T, std::string> or std::is_same_v<T, bool> or std::is_same_v<T, int>;


template <typename T>
requires is_string_bool_int<T>
static T get_value(const char* key, const T& default_value)
{
    const auto get_value_internal = [key](const std::string& default_val)
    {
        // Check the memory cache.
        if (cache.contains(key))
            return cache.at(key);
        // Get value from disk.
        std::string value;
        if (const std::string filename = file(key); file_or_dir_exists(filename))
            value = filter_url_file_get_contents(filename);
        else
            value = default_val;
        // Cache it.
        cache[key] = value;
        // Done.
        return value;
    };

    if constexpr (std::is_same_v<T, std::string>)
        return get_value_internal(default_value);
    else if constexpr (std::is_same_v<T, bool>)
        return filter::string::convert_to_bool(get_value_internal(filter::string::convert_to_string(default_value)));
    else if constexpr (std::is_same_v<T, int>)
        return filter::string::convert_to_int(get_value_internal(std::to_string(default_value)));
    else
        static_assert(false, "Not implemented");
    return T{};
}


template <typename T>
requires is_string_bool_int<T>
static void set_value(const char* key, const T& value)
{
    const auto set_value_internal = [key] (const std::string& val)
    {
        // Store in memory cache.
        cache[key] = val;
        // Store on disk.
        const std::string filename = file(key);
        filter_url_file_put_contents(filename, val);
    };

    if constexpr (std::is_same_v<T, std::string>)
        set_value_internal(value);
    else if constexpr (std::is_same_v<T, bool>)
        set_value_internal(filter::string::convert_to_string(value));
    else if constexpr (std::is_same_v<T, int>)
        set_value_internal(std::to_string(value));
    else
        static_assert(false, "Not implemented");
}


static std::vector<std::string> get_list(const char* key)
{
    const std::string contents = get_value(key, std::string());
    return filter::string::explode(contents, '\n');
}


static void set_list(const char* key, const std::vector<std::string>& values)
{
    const std::string value = filter::string::implode(values, "\n");
    set_value<std::string>(key, value);
}


// Named configuration functions.


constexpr auto site_mail_name_key{"site-mail-name"};

std::string get_site_mail_name()
{
    return get_value<std::string>(site_mail_name_key, std::string("Cloud"));
}

void set_site_mail_name(const std::string& value)
{
    set_value<std::string>(site_mail_name_key, value);
}


constexpr auto site_mail_address_key{"site-mail-address"};

std::string get_site_mail_address()
{
    return get_value<std::string>(site_mail_address_key, std::string());
}

void set_site_mail_address(const std::string& value)
{
    set_value<std::string>(site_mail_address_key, value);
}


constexpr auto mail_storage_host_key{"mail-storage-host"};

std::string get_mail_storage_host()
{
    return get_value<std::string>(mail_storage_host_key, std::string());
}

void set_mail_storage_host(const std::string& value)
{
    set_value<std::string>(mail_storage_host_key, value);
}


constexpr auto mail_storage_username_key{"mail-storage-username"};

std::string get_mail_storage_username()
{
    return get_value<std::string>(mail_storage_username_key, std::string());
}

void set_mail_storage_username(const std::string& value)
{
    set_value<std::string>(mail_storage_username_key, value);
}


constexpr auto mail_storage_password_key{"mail-storage-password"};

std::string get_mail_storage_password()
{
    return get_value<std::string>(mail_storage_password_key, std::string());
}

void set_mail_storage_password(const std::string& value)
{
    set_value<std::string>(mail_storage_password_key, value);
}


constexpr auto mail_storage_protocol_key{"mail-storage-protocol"};

std::string get_mail_storage_protocol()
{
    return get_value<std::string>(mail_storage_protocol_key, std::string());
}

void set_mail_storage_protocol(const std::string& value)
{
    set_value<std::string>(mail_storage_protocol_key, value);
}


constexpr auto mail_storage_port_key{"mail-storage-port"};

std::string get_mail_storage_port()
{
    return get_value<std::string>(mail_storage_port_key, std::string());
}

void set_mail_storage_port(const std::string& value)
{
    set_value<std::string>(mail_storage_port_key, value);
}


constexpr auto mail_send_host_key{"mail-send-host"};

std::string get_mail_send_host()
{
    return get_value<std::string>(mail_send_host_key, std::string());
}

void set_mail_send_host(const std::string& value)
{
    set_value<std::string>(mail_send_host_key, value);
}


constexpr auto mail_send_username_key{"mail-send-username"};

std::string get_mail_send_username()
{
    return get_value<std::string>(mail_send_username_key, std::string());
}

void set_mail_send_username(const std::string& value)
{
    set_value<std::string>(mail_send_username_key, value);
}


constexpr auto mail_send_password_key{"mail-send-password"};

std::string get_mail_send_password()
{
    return get_value<std::string>(mail_send_password_key, std::string());
}

void set_mail_send_password(const std::string& value)
{
    set_value<std::string>(mail_send_password_key, value);
}


constexpr auto mail_send_port_key{"mail-send-port"};

std::string get_mail_send_port()
{
    return get_value<std::string>(mail_send_port_key, std::string());
}

void set_mail_send_port(const std::string& value)
{
    set_value<std::string>(mail_send_port_key, value);
}


constexpr auto timer_minute_key{"timer-minute"};

std::string get_timer_minute()
{
    return get_value<std::string>(timer_minute_key, std::string());
}

void set_timer_minute(const std::string& value)
{
    set_value<std::string>(timer_minute_key, value);
}


constexpr auto timezone_key{"timezone"};

int get_timezone()
{
    // If the global offset variable is set, that is,
    // within certain limits, then take that.
    if (config_globals_timezone_offset_utc < MINIMUM_TIMEZONE
        or config_globals_timezone_offset_utc > MAXIMUM_TIMEZONE)
    {
        return get_value<int>(timezone_key, 0);
    }
    // Else take variable as set in the configuration.
    return config_globals_timezone_offset_utc;
}

void set_timezone(const int value)
{
    set_value<int>(timezone_key, value);
}


constexpr auto site_url_key{"site-url"};

std::string get_site_url()
{
    // The site URL is set upon login, normally.
    // In a client setup, there is never a login.
    // Consequently, the site URL is never set.
#ifdef HAVE_CLIENT
    // In case of a client, return a predefined URL.
    std::string url = "http://localhost:";
    url.append(::config::logic::http_network_port());
    url.append("/");
    return url;
#else
    // Get the URL that was set upon login.
    return get_value<std::string>(site_url_key, std::string());
#endif
}

void set_site_url(const std::string& value)
{
    set_value<std::string>(site_url_key, value);
}


constexpr auto general_site_language_key{"site-language"};

std::string get_site_language()
{
    // The default site language is an empty string.
    // It means not to localize the interface.
    // Since the default messages are all in English,
    // the default language for the interface will be English.
    return get_value<std::string>(general_site_language_key, std::string());
}

void set_site_language(const std::string& value)
{
    set_value<std::string>(general_site_language_key, value);
}


constexpr auto client_mode_key{"client-mode"};

bool get_client_mode()
{
    return get_value<bool>(client_mode_key, false);
}

void set_client_mode(const bool value)
{
    set_value<bool>(client_mode_key, value);
}


constexpr auto server_address_key{"server-address"};

std::string get_server_address()
{
    return get_value<std::string>(server_address_key, std::string());
}

void set_server_address(const std::string& value)
{
    set_value<std::string>(server_address_key, value);
}


constexpr auto server_port_key{"server-port"};

int get_server_port()
{
    return get_value<int>(server_port_key, 8080);
}

void set_server_port(const int value)
{
    set_value<int>(server_port_key, value);
}


constexpr auto repeat_send_receive_key{"repeat-send-receive"};

int get_repeat_send_receive()
{
    return get_value<int>(repeat_send_receive_key, 0);
}

void set_repeat_send_receive(const int value)
{
    set_value<int>(repeat_send_receive_key, value);
}


constexpr auto last_send_receive_key{"last-send-receive"};

int get_last_send_receive()
{
    return get_value<int>(last_send_receive_key, 0);
}

void set_last_send_receive(const int value)
{
    set_value<int>(last_send_receive_key, value);
}


constexpr auto installed_interface_version_key{"installed-interface-version"};

std::string get_installed_interface_version()
{
    return get_value<std::string>(installed_interface_version_key, std::string());
}

void set_installed_interface_version(const std::string& value)
{
    set_value<std::string>(installed_interface_version_key, value);
}


constexpr auto installed_database_version_key{"installed-database-version"};

std::string getInstalledDatabaseVersion()
{
    return get_value<std::string>(installed_database_version_key, std::string());
}

void setInstalledDatabaseVersion(const std::string& value)
{
    set_value<std::string>(installed_database_version_key, value);
}


constexpr auto just_started_key{"just-started"};

bool getJustStarted()
{
    return get_value<bool>(just_started_key, false);
}

void setJustStarted(const bool value)
{
    set_value<bool>(just_started_key, value);
}


constexpr auto paratext_projects_folder_key{"paratext-projects-folder"};

std::string get_paratext_projects_folder()
{
    return get_value<std::string>(paratext_projects_folder_key, std::string());
}

void set_paratext_projects_folder(const std::string& value)
{
    set_value<std::string>(paratext_projects_folder_key, value);
}


// Encryption / decryption key storage on client.
constexpr auto sync_key_key{"sync-key"};

std::string get_sync_key()
{
    return get_value<std::string>(sync_key_key, std::string());
}

void set_sync_key(const std::string& key)
{
    set_value<std::string>(sync_key_key, key);
}


constexpr auto last_menu_click_key{"last-menu-click"};

std::string get_last_menu_click()
{
    return get_value<std::string>(last_menu_click_key, std::string());
}

void set_last_menu_click(const std::string& url)
{
    set_value<std::string>(last_menu_click_key, url);
}


// Store the resources to be cached.
// The format is this:
// <resource title><space><book number>
constexpr auto resources_to_cache_key{"resources-to-cache"};

std::vector<std::string> get_resources_to_cache()
{
    return get_list(resources_to_cache_key);
}

void set_resources_to_cache(const std::vector<std::string>& values)
{
    set_list(resources_to_cache_key, values);
}


constexpr auto index_notes_key{"index-notes"};

bool getIndexNotes()
{
    return get_value<bool>(index_notes_key, false);
}

void setIndexNotes(const bool value)
{
    set_value<bool>(index_notes_key, value);
}


constexpr auto index_bibles_key{"index-bibles"};

bool get_index_bibles()
{
    return get_value<bool>(index_bibles_key, false);
}

void set_index_bibles(const bool value)
{
    set_value<bool>(index_bibles_key, value);
}


constexpr auto unsent_bible_data_time_key{"unsent-bible-data-time"};

int get_unsent_bible_data_time()
{
    return get_value<int>(unsent_bible_data_time_key, 0);
}

void set_unsent_bible_data_time(const int value)
{
    set_value<int>(unsent_bible_data_time_key, value);
}


constexpr auto unreceived_bible_data_time_key{"unreceived-bible-data-time"};

int get_unreceived_bible_data_time()
{
    return get_value<int>(unreceived_bible_data_time_key, 0);
}

void set_unreceived_bible_data_time(const int value)
{
    set_value<int>(unreceived_bible_data_time_key, value);
}


constexpr auto author_in_rss_feed_key{"author-in-rss-feed"};

bool get_author_in_rss_feed()
{
    return get_value<bool>(author_in_rss_feed_key, false);
}

void set_author_in_rss_feed(const bool value)
{
    set_value<bool>(author_in_rss_feed_key, value);
}


constexpr auto just_connected_to_cloud_key{"just-connected-to-cloud"};

bool get_just_connected_to_cloud()
{
    return get_value<bool>(just_connected_to_cloud_key, false);
}

void set_just_connected_to_cloud(const bool value)
{
    set_value<bool>(just_connected_to_cloud_key, value);
}


constexpr auto menu_in_tabbed_view_on_key{"menu-in-tabbed-view-on"};

bool get_menu_in_tabbed_view_on()
{
    return get_value<bool>(menu_in_tabbed_view_on_key, true);
}

void set_menu_in_tabbed_view_on(const bool value)
{
    set_value<bool>(menu_in_tabbed_view_on_key, value);
}


constexpr auto menu_in_tabbed_view_json_key{"menu-in-tabbed-view-json"};

std::string get_menu_in_tabbed_view_json()
{
    return get_value<std::string>(menu_in_tabbed_view_json_key, std::string());
}

void set_menu_in_tabbed_view_json(const std::string& value)
{
    set_value<std::string>(menu_in_tabbed_view_json_key, value);
}


constexpr auto disable_selection_popup_chrome_os_key{"disable-selection-popup-chrome-os"};

bool get_disable_selection_popup_chrome_os()
{
    return get_value<bool>(disable_selection_popup_chrome_os_key, false);
}

void set_disable_selection_popup_chrome_os(const bool value)
{
    set_value<bool>(disable_selection_popup_chrome_os_key, value);
}


constexpr auto notes_verse_separator_key{"notes-verse-separator"};

std::string get_notes_verse_separator()
{
    // The colon is the default value. See https://github.com/bibledit/cloud/issues/509
    return get_value<std::string>(notes_verse_separator_key, std::string(":"));
}

void set_notes_verse_separator(const std::string& value)
{
    set_value<std::string>(notes_verse_separator_key, value);
}


constexpr auto comparative_resources_key{"comparative-resources"};

std::vector<std::string> get_comparative_resources()
{
    return get_list(comparative_resources_key);
}

void set_comparative_resources(const std::vector<std::string>& values)
{
    set_list(comparative_resources_key, values);
}


constexpr auto translated_resources_key{"translated-resources"};

std::vector<std::string> get_translated_resources()
{
    return get_list(translated_resources_key);
}

void set_translated_resources(const std::vector<std::string>& values)
{
    set_list(translated_resources_key, values);
}


constexpr auto default_active_resources_key{"default-active-resources"};

std::vector<std::string> get_default_active_resources()
{
    return get_list(default_active_resources_key);
}

void set_default_active_resources(const std::vector<std::string>& values)
{
    set_list(default_active_resources_key, values);
}


constexpr auto keep_resources_cache_for_long_key{"keep-resources-cache-for-long"};

bool get_keep_resources_cache_for_long()
{
    return get_value<bool>(keep_resources_cache_for_long_key, false);
}

void set_keep_resources_cache_for_long(const bool value)
{
    set_value<bool>(keep_resources_cache_for_long_key, value);
}


constexpr auto keep_osis_content_in_sword_resources_key{"keep-osis-content-in-sword-resources"};

bool get_keep_osis_content_in_sword_resources()
{
    return get_value<bool>(keep_osis_content_in_sword_resources_key, false);
}

void set_keep_osis_content_in_sword_resources(const bool value)
{
    set_value<bool>(keep_osis_content_in_sword_resources_key, value);
}
}
