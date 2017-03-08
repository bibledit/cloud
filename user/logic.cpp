/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <user/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/md5.h>
#include <filter/date.h>
#include <database/users.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <email/send.h>
#include <ldap/logic.h>
#include <webserver/request.h>


typedef struct
{
  int identifier;
  const char * name;
  const char * url;
  const char * prefix;
  const char * suffix;
  bool join;
  const char * info;
  const char * installer;
} platform_record;


platform_record platform_table [] =
{
  {
    PLATFORM_WINDOWS,
    "Windows",
    "http://bibledit.org/windows",
    "<a href=\"", "\">",
    true,
    "https://bibledit.org:8081/help/installwindows",
    ""
  },
  {
    PLATFORM_ANDROID,
    "Android",
    "https://play.google.com/store/apps/details?id=org.bibledit.android",
    "\"softwareVersion\">", "</div>",
    false,
    "https://bibledit.org:8081/help/installandroid",
    ""
  },
  { PLATFORM_MACOS,
    "macOS",
    "https://itunes.apple.com/us/app/bibledit/id996639148?mt=8",
    "\"softwareVersion\">", "</span>",
    false,
    "https://bibledit.org:8081/help/installosx",
    ""
  },
  { PLATFORM_LINUX,
    "Linux",
    "http://bibledit.org/linux",
    "<a href=\"", "\">",
    true,
    "https://bibledit.org:8081/help/installlinux",
    "https://bibledit.org:8081/help/install.sh"
  },
  { PLATFORM_IOS,
    "iOS",
    "https://itunes.apple.com/us/app/bibledit/id967595683?mt=8",
    "\"softwareVersion\">", "</span>",
    false,
    "https://bibledit.org:8081/help/installios",
    ""
  },
  { PLATFORM_CHROMEOS,
    "Chrome OS",
    "https://chrome.google.com/webstore/detail/bibledit/aiaanakhppdclmabkcnpmnidajanaoda",
    "\"version\" content=\"", "\"",
    false,
    "https://bibledit.org:8081/help/installchromeos",
    ""
  },
  { PLATFORM_CLOUD,
    "Cloud",
    "http://bibledit.org/cloud",
    "<a href=\"", "\">",
    true,
    "https://bibledit.org:8081/help/installcloudubuntu",
    ""
  }
};


void user_logic_software_updates_notify ()
{
  // The first check is to find out whether there's users who have enabled the notifications for software updates.
  bool notify = false;
  Database_Users database_users;
  Database_Config_User database_config_user (NULL);
  vector <string> users = database_users.getUsers ();
  for (auto & user : users) {
    if (database_config_user.getAllSoftwareUpdatesNotificationForUser (user)) notify = true;
    if (database_config_user.getYourSoftwareUpdatesNotificationForUser (user)) notify = true;
  }
  // If there's nobody to be notified, bail out.
  if (!notify) return;
  
  unsigned int platform_count = sizeof (platform_table) / sizeof (*platform_table);
  
  // Go through the platforms to get the version numbers.
  map <int, string> online_version_numbers;
  for (unsigned int platform = 0; platform < platform_count; platform++) {
    
    string name = platform_table[platform].name;
    
    string url = platform_table[platform].url;
    string error;
    string text = filter_url_http_get (url, error, false);
    if (!error.empty ()) {
      Database_Logs::log ("Checking software update: " + name + ": " + error);
      continue;
    }

    string prefix = platform_table[platform].prefix;
    size_t pos = text.find (prefix);
    if (pos == string::npos) {
      Database_Logs::log ("Failure locating software release: " + name);
      continue;
    }
    while ((pos = text.find (prefix)) != string::npos) {
      text.erase (0, pos + prefix.length ());
    }
    
    string suffix = platform_table[platform].suffix;
    pos = text.find (suffix);
    if (pos == string::npos) {
      Database_Logs::log ("Malformed software release: " + name);
      continue;
    }
    text.erase (pos);

    // Store the version.
    text = filter_string_trim (text);
    online_version_numbers [platform] = text;
  }
  
  // Go through the users to see whom to notify.
  for (auto & user : users) {
    
    // Get the version numbers that were last mailed to this user.
    vector <string> user_version_numbers = database_config_user.getLastMailedSoftwareVersionForUser (user);
    // Expand the container when it's smaller than the number of platforms.
    for (size_t i = user_version_numbers.size (); i < platform_count; i++) {
      user_version_numbers.push_back ("");
    }
    
    // Get the platforms this user runs as client(s).
    vector <string> client_platforms = database_config_user.getConnectedClientsForUser (user);

    bool user_versions_updated = false;
    
    for (unsigned int platform = 0; platform < platform_count; platform++) {
      
      // Whether to check for this platform for this user.
      bool check_platform = database_config_user.getAllSoftwareUpdatesNotificationForUser (user);
      string platform_identifier = convert_to_string (platform_table[platform].identifier);
      if (!check_platform) {
        if (in_array (platform_identifier, client_platforms)) check_platform = true;
      }
      
      if (check_platform) {
        // The current version number.
        string online_version_number = online_version_numbers [platform];
        // If the version number was empty, it means it could not be fetched this time.
        if (!online_version_number.empty ()) {
          // The version number the user was last notified of.
          string user_version_number = user_version_numbers [platform];
          if (online_version_number != user_version_number) {
            // Send mail.
            string name = platform_table[platform].name;
            string url = platform_table[platform].url;
            string version (online_version_number);
            version = filter_string_str_replace ("bibledit-", "", version);
            version = filter_string_str_replace (".exe", "", version);
            version = filter_string_str_replace (".tar.gz", "", version);
            vector <string> body;
            body.push_back ("Updated Bibledit software is available.");
            body.push_back ("Platform: " + name);
            body.push_back ("Version: " + version);
            bool join = platform_table[platform].join;
            if (join) url = filter_url_create_path (url, online_version_number);
            body.push_back ("Download: " + url);
            string info = platform_table[platform].info;
            body.push_back ("Info: " + info);
            string installer = platform_table[platform].installer;
            if (!installer.empty ()) body.push_back ("Installer: " + installer);
            email_schedule (user, "Bibledit " + name + " update", filter_string_implode (body, "<br>"));
            user_version_numbers [platform] = online_version_number;
            user_versions_updated = true;
            // Remove the platform from the list of platforms the user runs clients on.
            client_platforms = filter_string_array_diff (client_platforms, {platform_identifier});
          }
        }
      }
    }
    
    if (user_versions_updated) {
      // Store the updated version numbers.
      database_config_user.setLastMailedSoftwareVersionForUser (user, user_version_numbers);
      // Clear the list of clients the user runs.
      // The purpose of this is that if a user stops running on a certain platform,
      // the user won't keep getting update notifications for this platform.
      database_config_user.setConnectedClientsForUser (user, {});
    }
  }
}


void user_logic_optional_ldap_authentication (void * webserver_request, string user, string pass)
{
  if (ldap_logic_is_on ()) {
    // Query the LDAP server and log the response.
    bool ldap_okay;
    string email;
    int role;
    ldap_logic_fetch (user, pass, ldap_okay, email, role, true);
    if (ldap_okay) {
      Webserver_Request * request = (Webserver_Request *) webserver_request;
      if (request->database_users ()->usernameExists (user)) {
        // Verify and/or update the fields for the user in the local database.
        if (request->database_users ()->get_md5 (user) != md5 (pass)) {
          request->database_users ()->set_password (user, pass);
        }
        if (request->database_users ()->get_level (user) != role) {
          request->database_users ()->set_level (user, role);
        }
        if (request->database_users ()->get_email (user) != email) {
          request->database_users ()->updateUserEmail (user, email);
        }
        if (!request->database_users ()->get_enabled (user)) {
          request->database_users ()->set_enabled (user, true);
        }
      } else {
        // Enter the user into the database.
        request->database_users ()->add_user (user, pass, role, email);
      }
    }
  }
}


int user_logic_login_failure_time = 0;


bool user_logic_login_failure_check_okay ()
{
  // No time set yet: OK.
  if (!user_logic_login_failure_time) return true;
  // A login failure was recorded during this very second: Check fails.
  if (user_logic_login_failure_time == filter_date_seconds_since_epoch ()) return false;
  // Default: OK.
  return true;
}


void user_logic_login_failure_register ()
{
  // Register a login failure for the current second.
  user_logic_login_failure_time = filter_date_seconds_since_epoch ();
}


void user_logic_login_failure_clear ()
{
  // Clear login failure tracker.
  user_logic_login_failure_time = 0;
}
