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


// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
std::map <std::string, std::string> database_config_general_cache;


// Functions for getting and setting values or lists of values follow here:


std::string Database_Config_General::file (const char * key)
{
  return filter_url_create_root_path ({database_logic_databases (), "config", "general", key});
}


std::string Database_Config_General::getValue (const char * key, const char * default_value)
{
  // Check the memory cache.
  if (database_config_general_cache.count (key)) {
    return database_config_general_cache [key];
  }
  // Get value from disk.
  std::string value;
  std::string filename = file (key);
  if (file_or_dir_exists (filename)) value = filter_url_file_get_contents (filename);
  else value = default_value;
  // Cache it.
  database_config_general_cache [key] = value;
  // Done.
  return value;
}


void Database_Config_General::setValue (const char * key, std::string value)
{
  // Store in memory cache.
  database_config_general_cache [key] = value;
  // Store on disk.
  std::string filename = file (key);
  filter_url_file_put_contents (filename, value);
}


bool Database_Config_General::getBValue (const char * key, bool default_value)
{
  std::string value = getValue (key, filter::strings::convert_to_string (default_value).c_str());
  return filter::strings::convert_to_bool (value);
}


void Database_Config_General::setBValue (const char * key, bool value)
{
  setValue (key, filter::strings::convert_to_string (value).c_str());
}


int Database_Config_General::getIValue (const char * key, int default_value)
{
  std::string value = getValue (key, filter::strings::convert_to_string (default_value).c_str());
  return filter::strings::convert_to_int (value);
}


void Database_Config_General::setIValue (const char * key, int value)
{
  setValue (key, filter::strings::convert_to_string (value).c_str());
}


std::vector <std::string> Database_Config_General::getList (const char * key)
{
  std::string contents = getValue (key, "");
  return filter::strings::explode (contents, '\n');
}


void Database_Config_General::setList (const char * key, std::vector <std::string> values)
{
  std::string value = filter::strings::implode (values, "\n");
  setValue (key, value);
}


// Named configuration functions.


std::string Database_Config_General::getSiteMailName ()
{
  return getValue ("site-mail-name", "Cloud");
}
void Database_Config_General::setSiteMailName (std::string value)
{
  setValue ("site-mail-name", value);
}


std::string Database_Config_General::getSiteMailAddress ()
{
  return getValue ("site-mail-address", "");
}
void Database_Config_General::setSiteMailAddress (std::string value)
{
  setValue ("site-mail-address", value);
}


std::string Database_Config_General::getMailStorageHost ()
{
  return getValue ("mail-storage-host", "");
}
void Database_Config_General::setMailStorageHost (std::string value)
{
  setValue ("mail-storage-host", value);
}


std::string Database_Config_General::getMailStorageUsername ()
{
  return getValue ("mail-storage-username", "");
}
void Database_Config_General::setMailStorageUsername (std::string value)
{
  setValue ("mail-storage-username", value);
}


std::string Database_Config_General::getMailStoragePassword ()
{
  return getValue ("mail-storage-password", "");
}


void Database_Config_General::setMailStoragePassword (std::string value)
{
  setValue ("mail-storage-password", value);
}


std::string Database_Config_General::getMailStorageProtocol ()
{
  return getValue ("mail-storage-protocol", "");
}
void Database_Config_General::setMailStorageProtocol (std::string value)
{
  setValue ("mail-storage-protocol", value);
}


std::string Database_Config_General::getMailStoragePort ()
{
  return getValue ("mail-storage-port", "");
}
void Database_Config_General::setMailStoragePort (std::string value)
{
  setValue ("mail-storage-port", value);
}


std::string Database_Config_General::getMailSendHost ()
{
  return getValue ("mail-send-host", "");
}
void Database_Config_General::setMailSendHost (std::string value)
{
  setValue ("mail-send-host", value);
}


std::string Database_Config_General::getMailSendUsername ()
{
  return getValue ("mail-send-username", "");
}
void Database_Config_General::setMailSendUsername (std::string value)
{
  setValue ("mail-send-username", value);
}


std::string Database_Config_General::getMailSendPassword ()
{
  return getValue ("mail-send-password", "");
}
void Database_Config_General::setMailSendPassword (std::string value)
{
  setValue ("mail-send-password", value);
}


std::string Database_Config_General::getMailSendPort ()
{
  return getValue ("mail-send-port", "");
}
void Database_Config_General::setMailSendPort (std::string value)
{
  setValue ("mail-send-port", value);
}


std::string Database_Config_General::getTimerMinute ()
{
  return getValue ("timer-minute", "");
}
void Database_Config_General::setTimerMinute (std::string value)
{
  setValue ("timer-minute", value);
}


int Database_Config_General::getTimezone ()
{
  // If the global offset variable is set, that is,
  // within certain limits, then take that.
  if ((config_globals_timezone_offset_utc < MINIMUM_TIMEZONE)
      || (config_globals_timezone_offset_utc > MAXIMUM_TIMEZONE)) {
    return getIValue ("timezone", 0);
  }
  // Else take variable as set in the configuration.
  return config_globals_timezone_offset_utc;
}
void Database_Config_General::setTimezone (int value)
{
  setIValue ("timezone", value);
}


std::string Database_Config_General::getSiteURL ()
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
  return getValue ("site-url", "");
#endif
}
void Database_Config_General::setSiteURL (std::string value)
{
  setValue ("site-url", value);
}


constexpr const char * general_site_language_key {"site-language"};
std::string Database_Config_General::getSiteLanguage ()
{
  // The default site language is an empty string.
  // It means not to localize the interface.
  // Since the default messages are all in English,
  // the default language for the interface will be English.
  return getValue (general_site_language_key, "");
}
void Database_Config_General::setSiteLanguage (std::string value)
{
  setValue (general_site_language_key, value);
}


bool Database_Config_General::getClientMode ()
{
  return getBValue ("client-mode", false);
}
void Database_Config_General::setClientMode (bool value)
{
  setBValue ("client-mode", value);
}


std::string Database_Config_General::getServerAddress ()
{
  return getValue ("server-address", "");
}
void Database_Config_General::setServerAddress (std::string value)
{
  setValue ("server-address", value);
}


int Database_Config_General::getServerPort ()
{
  return getIValue ("server-port", 8080);
}
void Database_Config_General::setServerPort (int value)
{
  setIValue ("server-port", value);
}


int Database_Config_General::getRepeatSendReceive ()
{
  return getIValue ("repeat-send-receive", 0);
}
void Database_Config_General::setRepeatSendReceive (int value)
{
  setIValue ("repeat-send-receive", value);
}


int Database_Config_General::getLastSendReceive ()
{
  return getIValue ("last-send-receive", 0);
}
void Database_Config_General::setLastSendReceive (int value)
{
  setIValue ("last-send-receive", value);
}


std::string Database_Config_General::getInstalledInterfaceVersion ()
{
  return getValue ("installed-interface-version", "");
}
void Database_Config_General::setInstalledInterfaceVersion (std::string value)
{
  setValue ("installed-interface-version", value);
}


std::string Database_Config_General::getInstalledDatabaseVersion ()
{
  return getValue ("installed-database-version", "");
}
void Database_Config_General::setInstalledDatabaseVersion (std::string value)
{
  setValue ("installed-database-version", value);
}


bool Database_Config_General::getJustStarted ()
{
  return getBValue ("just-started", false);
}
void Database_Config_General::setJustStarted (bool value)
{
  setBValue ("just-started", value);
}


std::string Database_Config_General::getParatextProjectsFolder ()
{
  return getValue ("paratext-projects-folder", "");
}
void Database_Config_General::setParatextProjectsFolder (std::string value)
{
  setValue ("paratext-projects-folder", value);
}


// Encryption / decryption key storage on client.
std::string Database_Config_General::getSyncKey ()
{
  return getValue ("sync-key", "");
}
void Database_Config_General::setSyncKey (std::string key)
{
  setValue ("sync-key", key);
}


std::string Database_Config_General::getLastMenuClick ()
{
  return getValue ("last-menu-click", "");
}
void Database_Config_General::setLastMenuClick (std::string url)
{
  setValue ("last-menu-click", url);
}


// Store the resources to be cached.
// The format is this:
// <resource title><space><book number>
std::vector <std::string> Database_Config_General::getResourcesToCache ()
{
  return getList ("resources-to-cache");
}


void Database_Config_General::setResourcesToCache (std::vector <std::string> values)
{
  setList ("resources-to-cache", values);
}


bool Database_Config_General::getIndexNotes ()
{
  return getBValue ("index-notes", false);
}
void Database_Config_General::setIndexNotes (bool value)
{
  setBValue ("index-notes", value);
}


bool Database_Config_General::getIndexBibles ()
{
  return getBValue ("index-bibles", false);
}
void Database_Config_General::setIndexBibles (bool value)
{
  setBValue ("index-bibles", value);
}


int Database_Config_General::getUnsentBibleDataTime ()
{
  return getIValue ("unsent-bible-data-time", 0);
}
void Database_Config_General::setUnsentBibleDataTime (int value)
{
  setIValue ("unsent-bible-data-time", value);
}


int Database_Config_General::getUnreceivedBibleDataTime ()
{
  return getIValue ("unreceived-bible-data-time", 0);
}
void Database_Config_General::setUnreceivedBibleDataTime (int value)
{
  setIValue ("unreceived-bible-data-time", value);
}


bool Database_Config_General::getAuthorInRssFeed ()
{
  return getBValue ("author-in-rss-feed", false);
}
void Database_Config_General::setAuthorInRssFeed (bool value)
{
  setBValue ("author-in-rss-feed", value);
}


bool Database_Config_General::getJustConnectedToCloud ()
{
  return getBValue ("just-connected-to-cloud", false);
}
void Database_Config_General::setJustConnectedToCloud (bool value)
{
  setBValue ("just-connected-to-cloud", value);
}


constexpr const char * menu_in_tabbed_view_on_key {"menu-in-tabbed-view-on"};
bool Database_Config_General::getMenuInTabbedViewOn ()
{
  return getBValue (menu_in_tabbed_view_on_key, true);
}
void Database_Config_General::setMenuInTabbedViewOn (bool value)
{
  setBValue (menu_in_tabbed_view_on_key, value);
}


constexpr const char * menu_in_tabbed_view_json_key {"menu-in-tabbed-view-json"};
std::string Database_Config_General::getMenuInTabbedViewJSON ()
{
  return getValue (menu_in_tabbed_view_json_key, "");
}
void Database_Config_General::setMenuInTabbedViewJSON (std::string value)
{
  setValue (menu_in_tabbed_view_json_key, value);
}


constexpr const char * disable_selection_popup_chrome_os_key {"disable-selection-popup-chrome-os"};
bool Database_Config_General::getDisableSelectionPopupChromeOS ()
{
  return getBValue (disable_selection_popup_chrome_os_key, false);
}
void Database_Config_General::setDisableSelectionPopupChromeOS (bool value)
{
  setBValue (disable_selection_popup_chrome_os_key, value);
}


constexpr const char * notes_verse_separator_key {"notes-verse-separator"};
std::string Database_Config_General::getNotesVerseSeparator ()
{
  // The colon is the default value. See https://github.com/bibledit/cloud/issues/509
  return getValue (notes_verse_separator_key, ":");
}
void Database_Config_General::setNotesVerseSeparator (std::string value)
{
  setValue (notes_verse_separator_key, value);
}


constexpr const char * comparative_resources_key {"comparative-resources"};
std::vector <std::string> Database_Config_General::getComparativeResources ()
{
  return getList (comparative_resources_key);
}
void Database_Config_General::setComparativeResources (std::vector <std::string> values)
{
  setList (comparative_resources_key, values);
}


constexpr const char * translated_resources_key {"translated-resources"};
std::vector <std::string> Database_Config_General::getTranslatedResources ()
{
  return getList (translated_resources_key);
}
void Database_Config_General::setTranslatedResources (std::vector <std::string> values)
{
  setList (translated_resources_key, values);
}


constexpr const char * default_active_resources_key {"default-active-resources"};
std::vector <std::string> Database_Config_General::getDefaultActiveResources ()
{
  return getList (default_active_resources_key);
}
void Database_Config_General::setDefaultActiveResources (std::vector <std::string> values)
{
  setList (default_active_resources_key, values);
}


constexpr const char * account_creation_times_key {"account-creation-times"};
std::vector <std::string> Database_Config_General::getAccountCreationTimes ()
{
  return getList (account_creation_times_key);
}
void Database_Config_General::setAccountCreationTimes (std::vector <std::string> values)
{
  setList (account_creation_times_key, values);
}


constexpr const char * keep_resources_cache_for_long_key {"keep-resources-cache-for-long"};
bool Database_Config_General::getKeepResourcesCacheForLong ()
{
  return getBValue (keep_resources_cache_for_long_key, false);
}
void Database_Config_General::setKeepResourcesCacheForLong (bool value)
{
  setBValue (keep_resources_cache_for_long_key, value);
}


constexpr const char * default_new_user_access_level_key {"default-new-user-access-level"};
int Database_Config_General::getDefaultNewUserAccessLevel ()
{
  return getIValue (default_new_user_access_level_key, Filter_Roles::member ());
}
void Database_Config_General::setDefaultNewUserAccessLevel (int value)
{
  setIValue (default_new_user_access_level_key, value);
}


constexpr const char * keep_osis_content_in_sword_resources_key {"keep-osis-content-in-sword-resources"};
bool Database_Config_General::getKeepOsisContentInSwordResources ()
{
  return getBValue (keep_osis_content_in_sword_resources_key, false);
}
void Database_Config_General::setKeepOsisContentInSwordResources (bool value)
{
  setBValue (keep_osis_content_in_sword_resources_key, value);
}
