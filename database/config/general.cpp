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


static std::string getValue (const char * key, const char * default_value)
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


static void setValue (const char * key, const std::string& value)
{
  // Store in memory cache.
  database_config_general_cache [key] = value;
  // Store on disk.
  const std::string filename = file (key);
  filter_url_file_put_contents (filename, value);
}


static bool getBValue (const char * key, bool default_value)
{
  const std::string value = getValue (key, filter::strings::convert_to_string (default_value).c_str());
  return filter::strings::convert_to_bool (value);
}


static void setBValue (const char * key, bool value)
{
  setValue (key, filter::strings::convert_to_string (value).c_str());
}


static int getIValue (const char * key, int default_value)
{
  const std::string value = getValue (key, filter::strings::convert_to_string (default_value).c_str());
  return filter::strings::convert_to_int (value);
}


static void setIValue (const char * key, int value)
{
  setValue (key, filter::strings::convert_to_string (value).c_str());
}


static std::vector <std::string> getList (const char * key)
{
  const std::string contents = getValue (key, "");
  return filter::strings::explode (contents, '\n');
}


static void setList (const char * key, const std::vector <std::string>& values)
{
  const std::string value = filter::strings::implode (values, "\n");
  setValue (key, value);
}


// Named configuration functions.


constexpr const auto site_mail_name_key {"site-mail-name"};
std::string getSiteMailName ()
{
  return getValue (site_mail_name_key, "Cloud");
}
void setSiteMailName (std::string value)
{
  setValue (site_mail_name_key, value);
}


constexpr const auto site_mail_address_key {"site-mail-address"};
std::string getSiteMailAddress ()
{
  return getValue (site_mail_address_key, "");
}
void setSiteMailAddress (std::string value)
{
  setValue (site_mail_address_key, value);
}


constexpr const auto mail_storage_host_key {"mail-storage-host"};
std::string getMailStorageHost ()
{
  return getValue (mail_storage_host_key, "");
}
void setMailStorageHost (std::string value)
{
  setValue (mail_storage_host_key, value);
}


constexpr const auto mail_storage_username_key {"mail-storage-username"};
std::string getMailStorageUsername ()
{
  return getValue (mail_storage_username_key, "");
}
void setMailStorageUsername (std::string value)
{
  setValue (mail_storage_username_key, value);
}


constexpr const auto mail_storage_password_key {"mail-storage-password"};
std::string getMailStoragePassword ()
{
  return getValue (mail_storage_password_key, "");
}
void setMailStoragePassword (std::string value)
{
  setValue (mail_storage_password_key, value);
}


constexpr const auto mail_storage_protocol_key {"mail-storage-protocol"};
std::string getMailStorageProtocol ()
{
  return getValue (mail_storage_protocol_key, "");
}
void setMailStorageProtocol (std::string value)
{
  setValue (mail_storage_protocol_key, value);
}


constexpr const auto mail_storage_port_key {"mail-storage-port"};
std::string getMailStoragePort ()
{
  return getValue (mail_storage_port_key, "");
}
void setMailStoragePort (std::string value)
{
  setValue (mail_storage_port_key, value);
}


constexpr const auto mail_send_host_key {"mail-send-host"};
std::string getMailSendHost ()
{
  return getValue (mail_send_host_key, "");
}
void setMailSendHost (std::string value)
{
  setValue (mail_send_host_key, value);
}


constexpr const auto mail_send_username_key {"mail-send-username"};
std::string getMailSendUsername ()
{
  return getValue (mail_send_username_key, "");
}
void setMailSendUsername (std::string value)
{
  setValue (mail_send_username_key, value);
}


constexpr const auto mail_send_password_key {"mail-send-password"};
std::string getMailSendPassword ()
{
  return getValue (mail_send_password_key, "");
}
void setMailSendPassword (std::string value)
{
  setValue (mail_send_password_key, value);
}


constexpr const auto mail_send_port_key {"mail-send-port"};
std::string getMailSendPort ()
{
  return getValue (mail_send_port_key, "");
}
void setMailSendPort (std::string value)
{
  setValue (mail_send_port_key, value);
}


constexpr const auto timer_minute_key {"timer-minute"};
std::string getTimerMinute ()
{
  return getValue (timer_minute_key, "");
}
void setTimerMinute (std::string value)
{
  setValue (timer_minute_key, value);
}


constexpr const auto timezone_key {"timezone"};
int getTimezone ()
{
  // If the global offset variable is set, that is,
  // within certain limits, then take that.
  if ((config_globals_timezone_offset_utc < MINIMUM_TIMEZONE)
      || (config_globals_timezone_offset_utc > MAXIMUM_TIMEZONE)) {
    return getIValue (timezone_key, 0);
  }
  // Else take variable as set in the configuration.
  return config_globals_timezone_offset_utc;
}
void setTimezone (int value)
{
  setIValue (timezone_key, value);
}


constexpr const auto site_url_key {"site-url"};
std::string getSiteURL ()
{
  // The site URL is set upon login, normally.
  // In a client setup, there is never a login.
  // Consequently the site URL is never set.
#ifdef HAVE_CLIENT
  // In case of a client, return a predefined URL.
  std::string url = "http://localhost:";
  url.append (config::logic::http_network_port ());
  url.append ("/");
  return url;
#else
  // Get the URL that was set upon login.
  return getValue (site_url_key, "");
#endif
}
void setSiteURL (std::string value)
{
  setValue (site_url_key, value);
}


constexpr const auto general_site_language_key {"site-language"};
std::string getSiteLanguage ()
{
  // The default site language is an empty string.
  // It means not to localize the interface.
  // Since the default messages are all in English,
  // the default language for the interface will be English.
  return getValue (general_site_language_key, "");
}
void setSiteLanguage (std::string value)
{
  setValue (general_site_language_key, value);
}


bool getClientMode ()
{
  return getBValue ("client-mode", false);
}
void setClientMode (bool value)
{
  setBValue ("client-mode", value);
}


std::string getServerAddress ()
{
  return getValue ("server-address", "");
}
void setServerAddress (std::string value)
{
  setValue ("server-address", value);
}


int getServerPort ()
{
  return getIValue ("server-port", 8080);
}
void setServerPort (int value)
{
  setIValue ("server-port", value);
}


int getRepeatSendReceive ()
{
  return getIValue ("repeat-send-receive", 0);
}
void setRepeatSendReceive (int value)
{
  setIValue ("repeat-send-receive", value);
}


int getLastSendReceive ()
{
  return getIValue ("last-send-receive", 0);
}
void setLastSendReceive (int value)
{
  setIValue ("last-send-receive", value);
}


std::string getInstalledInterfaceVersion ()
{
  return getValue ("installed-interface-version", "");
}
void setInstalledInterfaceVersion (std::string value)
{
  setValue ("installed-interface-version", value);
}


std::string getInstalledDatabaseVersion ()
{
  return getValue ("installed-database-version", "");
}
void setInstalledDatabaseVersion (std::string value)
{
  setValue ("installed-database-version", value);
}


bool getJustStarted ()
{
  return getBValue ("just-started", false);
}
void setJustStarted (bool value)
{
  setBValue ("just-started", value);
}


std::string getParatextProjectsFolder ()
{
  return getValue ("paratext-projects-folder", "");
}
void setParatextProjectsFolder (std::string value)
{
  setValue ("paratext-projects-folder", value);
}


// Encryption / decryption key storage on client.
std::string getSyncKey ()
{
  return getValue ("sync-key", "");
}
void setSyncKey (std::string key)
{
  setValue ("sync-key", key);
}


std::string getLastMenuClick ()
{
  return getValue ("last-menu-click", "");
}
void setLastMenuClick (std::string url)
{
  setValue ("last-menu-click", url);
}


// Store the resources to be cached.
// The format is this:
// <resource title><space><book number>
std::vector <std::string> getResourcesToCache ()
{
  return getList ("resources-to-cache");
}


void setResourcesToCache (std::vector <std::string> values)
{
  setList ("resources-to-cache", values);
}


bool getIndexNotes ()
{
  return getBValue ("index-notes", false);
}
void setIndexNotes (bool value)
{
  setBValue ("index-notes", value);
}


bool getIndexBibles ()
{
  return getBValue ("index-bibles", false);
}
void setIndexBibles (bool value)
{
  setBValue ("index-bibles", value);
}


int getUnsentBibleDataTime ()
{
  return getIValue ("unsent-bible-data-time", 0);
}
void setUnsentBibleDataTime (int value)
{
  setIValue ("unsent-bible-data-time", value);
}


int getUnreceivedBibleDataTime ()
{
  return getIValue ("unreceived-bible-data-time", 0);
}
void setUnreceivedBibleDataTime (int value)
{
  setIValue ("unreceived-bible-data-time", value);
}


bool getAuthorInRssFeed ()
{
  return getBValue ("author-in-rss-feed", false);
}
void setAuthorInRssFeed (bool value)
{
  setBValue ("author-in-rss-feed", value);
}


bool getJustConnectedToCloud ()
{
  return getBValue ("just-connected-to-cloud", false);
}
void setJustConnectedToCloud (bool value)
{
  setBValue ("just-connected-to-cloud", value);
}


constexpr const auto menu_in_tabbed_view_on_key {"menu-in-tabbed-view-on"};
bool getMenuInTabbedViewOn ()
{
  return getBValue (menu_in_tabbed_view_on_key, true);
}
void setMenuInTabbedViewOn (bool value)
{
  setBValue (menu_in_tabbed_view_on_key, value);
}


constexpr const auto menu_in_tabbed_view_json_key {"menu-in-tabbed-view-json"};
std::string getMenuInTabbedViewJSON ()
{
  return getValue (menu_in_tabbed_view_json_key, "");
}
void setMenuInTabbedViewJSON (std::string value)
{
  setValue (menu_in_tabbed_view_json_key, value);
}


constexpr const auto disable_selection_popup_chrome_os_key {"disable-selection-popup-chrome-os"};
bool getDisableSelectionPopupChromeOS ()
{
  return getBValue (disable_selection_popup_chrome_os_key, false);
}
void setDisableSelectionPopupChromeOS (bool value)
{
  setBValue (disable_selection_popup_chrome_os_key, value);
}


constexpr const auto notes_verse_separator_key {"notes-verse-separator"};
std::string getNotesVerseSeparator ()
{
  // The colon is the default value. See https://github.com/bibledit/cloud/issues/509
  return getValue (notes_verse_separator_key, ":");
}
void setNotesVerseSeparator (std::string value)
{
  setValue (notes_verse_separator_key, value);
}


constexpr const auto comparative_resources_key {"comparative-resources"};
std::vector <std::string> getComparativeResources ()
{
  return getList (comparative_resources_key);
}
void setComparativeResources (std::vector <std::string> values)
{
  setList (comparative_resources_key, values);
}


constexpr const auto translated_resources_key {"translated-resources"};
std::vector <std::string> getTranslatedResources ()
{
  return getList (translated_resources_key);
}
void setTranslatedResources (std::vector <std::string> values)
{
  setList (translated_resources_key, values);
}


constexpr const auto default_active_resources_key {"default-active-resources"};
std::vector <std::string> getDefaultActiveResources ()
{
  return getList (default_active_resources_key);
}
void setDefaultActiveResources (std::vector <std::string> values)
{
  setList (default_active_resources_key, values);
}


constexpr const auto account_creation_times_key {"account-creation-times"};
std::vector <std::string> getAccountCreationTimes ()
{
  return getList (account_creation_times_key);
}
void setAccountCreationTimes (std::vector <std::string> values)
{
  setList (account_creation_times_key, values);
}


constexpr const auto keep_resources_cache_for_long_key {"keep-resources-cache-for-long"};
bool getKeepResourcesCacheForLong ()
{
  return getBValue (keep_resources_cache_for_long_key, false);
}
void setKeepResourcesCacheForLong (bool value)
{
  setBValue (keep_resources_cache_for_long_key, value);
}


constexpr const auto default_new_user_access_level_key {"default-new-user-access-level"};
int getDefaultNewUserAccessLevel ()
{
  return getIValue (default_new_user_access_level_key, Filter_Roles::member ());
}
void setDefaultNewUserAccessLevel (int value)
{
  setIValue (default_new_user_access_level_key, value);
}


constexpr const auto keep_osis_content_in_sword_resources_key {"keep-osis-content-in-sword-resources"};
bool getKeepOsisContentInSwordResources ()
{
  return getBValue (keep_osis_content_in_sword_resources_key, false);
}
void setKeepOsisContentInSwordResources (bool value)
{
  setBValue (keep_osis_content_in_sword_resources_key, value);
}

}
