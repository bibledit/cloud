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


#include <sendreceive/resources.h>
#include <sendreceive/logic.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <config/globals.h>
#include <database/config/general.h>
#include <database/logs.h>
#include <database/versifications.h>
#include <database/usfmresources.h>
#include <database/books.h>
#include <database/cache.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <sync/resources.h>
#include <sword/logic.h>
#include <demo/logic.h>


int sendreceive_resources_watchdog = 0;


void sendreceive_resources_kick_watchdog ()
{
  sendreceive_resources_watchdog = filter_date_seconds_since_epoch ();
}


void sendreceive_resources_done ()
{
  sendreceive_resources_watchdog = 0;
  config_globals_syncing_resources = false;
}


bool sendreceive_resources_interrupt = false;


void sendreceive_resources_delay_during_prioritized_tasks ()
{
  if (sendreceive_logic_prioritized_task_is_active ()) {
    // Before, when prioritized sync actions ran,
    // it used to interrupt and reschedule a resource installation.
    // This gave a lot of clutter in the journal,
    // and it takes some time to restart the resource installation.
    // Rather it now delays the installation a bit while the priority tasks flag is on.
    // That delay is not visible in the Journal, it just happens silently.
    this_thread::sleep_for (chrono::seconds (10));
  }
}


void sendreceive_resources ()
{
  if (sendreceive_resources_watchdog) {
    int time = filter_date_seconds_since_epoch ();
    if (time < (sendreceive_resources_watchdog + 900)) {
      return;
    }
    Database_Logs::log ("Resources: " + translate("Watchdog timeout"), Filter_Roles::translator ());
    sendreceive_resources_done ();
  }

  // If any of the prioritized synchronization tasks run, postpone the current task and do not start it.
  if (sendreceive_logic_prioritized_task_is_active ()) {
    sendreceive_resources_done ();
    this_thread::sleep_for (chrono::seconds (5));
    tasks_logic_queue (SYNCRESOURCES);
    return;
  }

  sendreceive_resources_interrupt = false;

  // If there's nothing to cache, bail out.
  vector <string> resources = Database_Config_General::getResourcesToCache ();
  if (resources.empty ()) return;
  
  sendreceive_resources_kick_watchdog ();
  config_globals_syncing_resources = true;

  // Counters.
  int error_count = 0;
  int wait_count = 0;
  
  // Resource to cache.
  string resource = resources [0];
  
  // Erase the two older storage locations that were used to cache resources in earlier versions of Bibledit.
  {
    Database_UsfmResources database_usfmresources;
    database_usfmresources.deleteResource (resource);
  }

  Database_Logs::log ("Starting to install resource:" " " + resource, Filter_Roles::consultant ());

  // Server address and port.
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  // If the client has not been connected to a cloud instance,
  // fetch the resource from the Bibledit Cloud demo.
  if (!client_logic_client_enabled ()) {
    address = demo_address ();
    port = demo_port ();
  }
  string cloud_url = client_logic_url (address, port, sync_resources_url ());
  string resource_url = filter_url_build_http_query (cloud_url, "r", filter_url_urlencode (resource));

  // Go through all Bible books.
  Database_Versifications database_versifications;
  vector <int> books = database_versifications.getMaximumBooks ();
  for (auto & book : books) {
    sendreceive_resources_delay_during_prioritized_tasks ();
    if (sendreceive_resources_interrupt) continue;

    // The URL fragment that contains the current book in its query.
    string book_url = filter_url_build_http_query (resource_url, "b", convert_to_string (book));
    
    // The URL to request the resource database for this book from the Cloud.
    string url = filter_url_build_http_query (book_url, "a", convert_to_string (Sync_Logic::resources_request_database));
    string error;
    string response = filter_url_http_get (url, error, false);
    if (error.empty ()) {
      // When the Cloud responds with a "0", it means that the database is not yet ready for distribution.
      // The Cloud will be working on preparing it.
      int server_size = convert_to_int (response);
      if (server_size > 0) {
        // The Cloud has now responded with the file size of the resource database, in bytes.
        // Now request the path to download it.
        url = filter_url_build_http_query (book_url, "a", convert_to_string (Sync_Logic::resources_request_download));
        error.clear ();
        string response = filter_url_http_get (url, error, false);
        if (error.empty ()) {
          // At this stage the file size is known, plus the fragment of the path in the Cloud.
          // Check whether the file is already available on the client, fully downloaded.
          string client_path = filter_url_create_root_path_cpp17 ({filter_url_urldecode (response)});
          int client_size = filter_url_filesize (client_path);
          if (server_size != client_size) {
            // Download it.
            string url = client_logic_url (address, port, response);
            error.clear ();
            filter_url_download_file (url, client_path, error, false);
            if (error.empty ()) {
              string bookname = Database_Books::getEnglishFromId (book);
              Database_Logs::log ("Downloaded " + resource + " " + bookname, Filter_Roles::consultant ());
            } else {
              Database_Logs::log ("Failed to download resource " + response + " :" + error, Filter_Roles::consultant ());
              error_count++;
            }
          }
        } else {
          Database_Logs::log (error, Filter_Roles::consultant ());
          error_count++;
        }
      } else {
        wait_count++;
      }
    } else {
      Database_Logs::log (error, Filter_Roles::consultant ());
      error_count++;
    }

    sendreceive_resources_kick_watchdog ();
    
  }
  
  // Done.
  if (error_count) {
    string msg = "Error count while downloading resource: " + convert_to_string (error_count);
    Database_Logs::log (msg, Filter_Roles::consultant ());
  } else if (wait_count) {
    string msg = "Waiting for Cloud to prepare resource for download. Remaining books: " + convert_to_string (wait_count);
    Database_Logs::log (msg, Filter_Roles::consultant ());
  } else {
    Database_Logs::log ("Completed installing resource:" " " + resource, Filter_Roles::consultant ());
  }
  // In case of errors, of when waiting for the Cloud, schedule the resource download again.
  bool re_schedule_download = false;
  if (error_count || wait_count) {
    if (!sendreceive_resources_interrupt) {
      // Wait a bit so as not to generate too many journal entries
      // when there were errors of when it had to wait for the Cloud.
      this_thread::sleep_for (chrono::minutes (1));
      if (!sendreceive_resources_interrupt) {
        re_schedule_download = true;
        Database_Logs::log ("Re-scheduling resource installation", Filter_Roles::consultant ());
      }
    }
  }
  // Store new download schedule.
  resources = Database_Config_General::getResourcesToCache ();
  resources = filter_string_array_diff (resources, {resource});
  if (re_schedule_download) {
    resources.push_back (resource);
  }
  Database_Config_General::setResourcesToCache (resources);

  sendreceive_resources_done ();
  
  sendreceive_resources_interrupt = false;

  // If there's another resource waiting to be cached, schedule it for caching.
  if (!resources.empty ()) tasks_logic_queue (SYNCRESOURCES);
}


// This stops all resources that have been scheduled to install.
void sendreceive_resources_clear_all ()
{
  sendreceive_resources_interrupt = true;
  Database_Logs::log ("Interrupting resource installation", Filter_Roles::consultant ());
  Database_Config_General::setResourcesToCache ({});
}


/*

 It used to take a very long time on low-power devices, like Android tablets and phones,
 to install a complete resource.
 
 Some measurements were done to see where the bottle neck was.
 
 1. Installed Staten Bible Resource on Mac via Cloud.
    Measured time: 13:46:57 to 14:08:38 = 20 minutes.
 
 2. Install Staten Bible Resource on Android via Cloud.
    Measured time: 15:10:52 to 17:06:57 = 116 minutes.
 
 Both devices were on the same network and connected to the same Cloud.

 Before doing the measurements, the Cloud's cache was filled with all data it had to fetch externally, so that external influences were excluded.
 
 There was a big difference between a Mac and an Android phone.
 Likely this was due to the difference in performance: 
 The Mac device is fast and powerful and the Android device is a low-power device.
 
 To resolve this, a new faster way to download resources was made available:
 1. The Cloud prepares the SQLite database with part of the resource.
 2. The Client downloads the entire database in one go.

*/
