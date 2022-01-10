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


#include <database/config/general.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <system/index.h>
#include <database/logic.h>


// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
map <string, string> database_config_general_cache;


// Functions for getting and setting values or lists of values follow here:


string Database_Config_General::file (const char * key)
{
  return filter_url_create_root_path (database_logic_databases (), "config", "general", key);
}


string Database_Config_General::getValue (const char * key, const char * default_value)
{
  // Check the memory cache.
  if (database_config_general_cache.count (key)) {
    return database_config_general_cache [key];
  }
  // Get value from disk.
  string value;
  string filename = file (key);
  if (file_or_dir_exists (filename)) value = filter_url_file_get_contents (filename);
  else value = default_value;
  // Cache it.
  database_config_general_cache [key] = value;
  // Done.
  return value;
}


void Database_Config_General::setValue (const char * key, string value)
{
  // Store in memory cache.
  database_config_general_cache [key] = value;
  // Store on disk.
  string filename = file (key);
  filter_url_file_put_contents (filename, value);
}


bool Database_Config_General::getBValue (const char * key, bool default_value)
{
  string value = getValue (key, convert_to_string (default_value).c_str());
  return convert_to_bool (value);
}


void Database_Config_General::setBValue (const char * key, bool value)
{
  setValue (key, convert_to_string (value).c_str());
}


int Database_Config_General::getIValue (const char * key, int default_value)
{
  string value = getValue (key, convert_to_string (default_value).c_str());
  return convert_to_int (value);
}


void Database_Config_General::setIValue (const char * key, int value)
{
  setValue (key, convert_to_string (value).c_str());
}


vector <string> Database_Config_General::getList (const char * key)
{
  string contents = getValue (key, "");
  return filter_string_explode (contents, '\n');
}


void Database_Config_General::setList (const char * key, vector <string> values)
{
  string value = filter_string_implode (values, "\n");
  setValue (key, value);
}


// Named configuration functions.


string Database_Config_General::getSiteMailName ()
{
  return getValue ("site-mail-name", "Cloud");
}
void Database_Config_General::setSiteMailName (string value)
{
  setValue ("site-mail-name", value);
}


string Database_Config_General::getSiteMailAddress ()
{
  return getValue ("site-mail-address", "");
}
void Database_Config_General::setSiteMailAddress (string value)
{
  setValue ("site-mail-address", value);
}


string Database_Config_General::getMailStorageHost ()
{
  return getValue ("mail-storage-host", "");
}
void Database_Config_General::setMailStorageHost (string value)
{
  setValue ("mail-storage-host", value);
}


string Database_Config_General::getMailStorageUsername ()
{
  return getValue ("mail-storage-username", "");
}
void Database_Config_General::setMailStorageUsername (string value)
{
  setValue ("mail-storage-username", value);
}


string Database_Config_General::getMailStoragePassword ()
{
  return getValue ("mail-storage-password", "");
}


void Database_Config_General::setMailStoragePassword (string value)
{
  setValue ("mail-storage-password", value);
}


string Database_Config_General::getMailStorageProtocol ()
{
  return getValue ("mail-storage-protocol", "");
}
void Database_Config_General::setMailStorageProtocol (string value)
{
  setValue ("mail-storage-protocol", value);
}


string Database_Config_General::getMailStoragePort ()
{
  return getValue ("mail-storage-port", "");
}
void Database_Config_General::setMailStoragePort (string value)
{
  setValue ("mail-storage-port", value);
}


string Database_Config_General::getMailSendHost ()
{
  return getValue ("mail-send-host", "");
}
void Database_Config_General::setMailSendHost (string value)
{
  setValue ("mail-send-host", value);
}


string Database_Config_General::getMailSendUsername ()
{
  return getValue ("mail-send-username", "");
}
void Database_Config_General::setMailSendUsername (string value)
{
  setValue ("mail-send-username", value);
}


string Database_Config_General::getMailSendPassword ()
{
  return getValue ("mail-send-password", "");
}
void Database_Config_General::setMailSendPassword (string value)
{
  setValue ("mail-send-password", value);
}


string Database_Config_General::getMailSendPort ()
{
  return getValue ("mail-send-port", "");
}
void Database_Config_General::setMailSendPort (string value)
{
  setValue ("mail-send-port", value);
}


string Database_Config_General::getTimerMinute ()
{
  return getValue ("timer-minute", "");
}
void Database_Config_General::setTimerMinute (string value)
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


string Database_Config_General::getSiteURL ()
{
  // The site URL is set upon login, normally.
  // In a client setup, there is never a login.
  // Consequently the site URL is never set.
#ifdef HAVE_CLIENT
  // In case of a client, return a predefined URL.
  string url = "http://localhost:";
  url.append (config_logic_http_network_port ());
  url.append ("/");
  return url;
#else
  // Get the URL that was set upon login.
  return getValue ("site-url", "");
#endif
}
void Database_Config_General::setSiteURL (string value)
{
  setValue ("site-url", value);
}


const char * general_site_language_key ()
{
  return "site-language";
}
string Database_Config_General::getSiteLanguage ()
{
  // The default site language is an empty string.
  // It means not to localize the interface.
  // Since the default messages are all in English,
  // the default language for the interface will be English.
  return getValue (general_site_language_key (), "");
}
void Database_Config_General::setSiteLanguage (string value)
{
  setValue (general_site_language_key (), value);
}


bool Database_Config_General::getClientMode ()
{
  return getBValue ("client-mode", false);
}
void Database_Config_General::setClientMode (bool value)
{
  setBValue ("client-mode", value);
}


string Database_Config_General::getServerAddress ()
{
  return getValue ("server-address", "");
}
void Database_Config_General::setServerAddress (string value)
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


string Database_Config_General::getInstalledInterfaceVersion ()
{
  return getValue ("installed-interface-version", "");
}
void Database_Config_General::setInstalledInterfaceVersion (string value)
{
  setValue ("installed-interface-version", value);
}


string Database_Config_General::getInstalledDatabaseVersion ()
{
  return getValue ("installed-database-version", "");
}
void Database_Config_General::setInstalledDatabaseVersion (string value)
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


string Database_Config_General::getParatextProjectsFolder ()
{
  return getValue ("paratext-projects-folder", "");
}
void Database_Config_General::setParatextProjectsFolder (string value)
{
  setValue ("paratext-projects-folder", value);
}


// Encryption / decryption key storage on client.
string Database_Config_General::getSyncKey ()
{
  return getValue ("sync-key", "");
}
void Database_Config_General::setSyncKey (string key)
{
  setValue ("sync-key", key);
}


string Database_Config_General::getLastMenuClick ()
{
  return getValue ("last-menu-click", "");
}
void Database_Config_General::setLastMenuClick (string url)
{
  setValue ("last-menu-click", url);
}


vector <string> Database_Config_General::getResourcesToCache ()
{
  return getList ("resources-to-cache");
}


void Database_Config_General::setResourcesToCache (vector <string> values)
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


const char * menu_in_tabbed_view_on_key ()
{
  return "menu-in-tabbed-view-on";
}
bool Database_Config_General::getMenuInTabbedViewOn ()
{
  return getBValue (menu_in_tabbed_view_on_key (), true);
}
void Database_Config_General::setMenuInTabbedViewOn (bool value)
{
  setBValue (menu_in_tabbed_view_on_key (), value);
}


const char * menu_in_tabbed_view_json_key ()
{
  return "menu-in-tabbed-view-json";
}
string Database_Config_General::getMenuInTabbedViewJSON ()
{
  return getValue (menu_in_tabbed_view_json_key (), "");
}
void Database_Config_General::setMenuInTabbedViewJSON (string value)
{
  setValue (menu_in_tabbed_view_json_key (), value);
}


const char * disable_selection_popup_chrome_os_key ()
{
  return "disable-selection-popup-chrome-os";
}
bool Database_Config_General::getDisableSelectionPopupChromeOS ()
{
  return getBValue (disable_selection_popup_chrome_os_key (), false);
}
void Database_Config_General::setDisableSelectionPopupChromeOS (bool value)
{
  setBValue (disable_selection_popup_chrome_os_key (), value);
}


const char * notes_verse_separator_key ()
{
  return "notes-verse-separator";
}
string Database_Config_General::getNotesVerseSeparator ()
{
  // The colon is the default value. See https://github.com/bibledit/cloud/issues/509
  return getValue (notes_verse_separator_key (), ":");
}
void Database_Config_General::setNotesVerseSeparator (string value)
{
  setValue (notes_verse_separator_key (), value);
}


const char * comparative_resources_key ()
{
  return "comparative-resources";
}
vector <string> Database_Config_General::getComparativeResources ()
{
  return getList (comparative_resources_key ());
}
void Database_Config_General::setComparativeResources (vector <string> values)
{
  setList (comparative_resources_key (), values);
}


const char * active_resources_key ()
{
  return "active-resources";
}
vector <string> Database_Config_General::getActiveResources ()
{
  return getList (active_resources_key ());
}
void Database_Config_General::setActiveResources (vector <string> values)
{
  setList (active_resources_key (), values);
}


const char * account_creation_times_key ()
{
  return "account-creation-times";
}
vector <string> Database_Config_General::getAccountCreationTimes ()
{
  return getList (account_creation_times_key ());
}
void Database_Config_General::setAccountCreationTimes (vector <string> values)
{
  setList (account_creation_times_key (), values);
}


const char * keep_resources_cache_for_long_key ()
{
  return "keep-resources-cache-for-long";
}
bool Database_Config_General::getKeepResourcesCacheForLong ()
{
  return getBValue (keep_resources_cache_for_long_key (), false);
}
void Database_Config_General::setKeepResourcesCacheForLong (bool value)
{
  setBValue (keep_resources_cache_for_long_key (), value);
}

