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


#include <sendreceive/files.h>
#include <sendreceive/logic.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <config/globals.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <sync/files.h>
#include <checksum/logic.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/privileges.h>


int sendreceive_files_watchdog {0};


void sendreceive_files_done ()
{
  sendreceive_files_watchdog = 0;
  config_globals_syncing_files = false;
}


std::string sendreceive_files_text ()
{
  return translate("Files") + ": ";
}


std::string sendreceive_files_sendreceive_text ()
{
  return sendreceive_files_text () + translate ("Synchronizing");
}


std::string sendreceive_files_up_to_date_text ()
{
  return sendreceive_files_text () + translate ("Up to date");
}


void sendreceive_files ()
{
  // Watchdog handler.
  if (sendreceive_files_watchdog) {
    const int time = filter::date::seconds_since_epoch ();
    if (time < (sendreceive_files_watchdog + 900)) {
      Database_Logs::log (sendreceive_files_text () + translate("Still busy"), Filter_Roles::translator ());
      return;
    }
    Database_Logs::log (sendreceive_files_text () + translate("Watchdog timeout"), Filter_Roles::translator ());
  }
  sendreceive_files_kick_watchdog ();
  config_globals_syncing_files = true;
  
  
  // If any of the prioritized synchronization tasks run, postpone the current task and do not start it.
  if (sendreceive_logic_prioritized_task_is_active ()) {
    sendreceive_files_done ();
    std::this_thread::sleep_for (std::chrono::seconds (5));
    tasks_logic_queue (task::sync_files);
    return;
  }
  
  
  Webserver_Request webserver_request {};
  Sync_Logic sync_logic (webserver_request);

  
  Database_Logs::log (sendreceive_files_sendreceive_text (), Filter_Roles::translator ());

  
  const std::string address = database::config::general::get_server_address ();
  const int port = database::config::general::get_server_port ();
  const std::string url = client_logic_url (address, port, sync_files_url ());
  
  
  // The verion number of the set of directories this client uses.
  const int version {5};
  
  
  std::map <std::string, std::string> post {};

  
  // The client user is the sole user registered on the system.
  const std::vector <std::string> users = webserver_request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (translate("No user found"), Filter_Roles::translator ());
    sendreceive_files_done ();
    return;
  }
  const std::string user {users.at(0)};
  post ["u"] = filter::strings::bin2hex (user);

  
  post ["v"] = std::to_string (version);
  std::string error {};
  std::string response {};
  int iresponse {0};
  

  // Request the checksum of all relevant files on the server.
  // Compare it with the local checksum for the same set of files.
  // If the two match: Ready.
  post ["a"] = std::to_string (Sync_Logic::files_total_checksum);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log (sendreceive_files_text () + "Failure requesting total checksum: " + error, Filter_Roles::translator ());
    sendreceive_files_done ();
    return;
  }
  iresponse = filter::strings::convert_to_int (response);
  int checksum = Sync_Logic::files_get_total_checksum (version, user);
  if (iresponse == checksum) {
    Database_Logs::log (sendreceive_files_up_to_date_text (), Filter_Roles::translator ());
    sendreceive_files_done ();
    return;
  }

  
  // Go through the directories relevant to the version.
  const std::vector <std::string> directories = Sync_Logic::files_get_directories (version, user);
  for (size_t d = 0; d < directories.size (); d++) {
    

    // The directory name itself is not posted to the server,
    // but rather the index of the directory in the entire list.
    // This is for security reasons.
    post ["d"] = std::to_string (d);
    const std::string directory = directories [d];
    

    // Request the total checksum of a directory on the server.
    // Compare it with the local checksum for the directory on the client.
    // If the two match: This directory is ready.
    post ["a"] = std::to_string (Sync_Logic::files_directory_checksum);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (sendreceive_files_text () + "Failure requesting directory checksum: " + error, Filter_Roles::translator ());
      sendreceive_files_done ();
      return;
    }
    iresponse = filter::strings::convert_to_int (response);
    int checksum_d = Sync_Logic::files_get_directory_checksum (directory);
    if (iresponse == checksum_d) {
      continue;
    }

    
    // Retrieve the list of files in a directory on the server.
    post ["a"] = std::to_string (Sync_Logic::files_directory_files);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (sendreceive_files_text () + "Failure requesting directory files: " + error, Filter_Roles::translator ());
      sendreceive_files_done ();
      return;
    }
    const std::vector <std::string> server_files = filter::strings::explode (response, '\n');
    
    
    // Delete files that exist locally but not on the server.
    const std::vector <std::string> client_files = Sync_Logic::files_get_files (directory);
    const std::vector <std::string> files = filter::strings::array_diff (client_files, server_files);
    for (const auto& file : files) {
      Database_Logs::log (sendreceive_files_text () + "Deleting file: " + filter_url_create_path ({directory, file}), Filter_Roles::translator ());
      std::string path = filter_url_create_root_path ({directory, file});
      filter_url_unlink (path);
      // Attempt to delete the directory, which will only succeed if it is empty.
      path = filter_url_dirname (path);
      filter_url_unlink (path);
    }
    

    // Deal with each file individually.
    for (const auto& file : server_files) {


      // Request checksum of this file,
      // compare it with the local checksum,
      // and skip the file if the checksums match.
      post ["a"] = std::to_string (Sync_Logic::files_file_checksum);
      post ["f"] = file;
      response = sync_logic.post (post, url, error);
      if (!error.empty ()) {
        Database_Logs::log (sendreceive_files_text () + "Failure requesting checksum file: " + error, Filter_Roles::translator ());
        sendreceive_files_done ();
        return;
      }
      const int iresponse_file = filter::strings::convert_to_int (response);
      const int checksum_file = Sync_Logic::files_get_file_checksum (directory, file);
      if (iresponse_file == checksum_file) {
        continue;
      }
      

      sendreceive_files_kick_watchdog ();
      
      
      // Download the file from the server, and store it locally on the client.
      Database_Logs::log (sendreceive_files_text () + "Downloading " + filter_url_create_path ({directory, file}), Filter_Roles::translator ());
      // Local file path where to save resource.
      const std::string fullpath = filter_url_create_root_path ({directory, file});
      // Create directory if it does not yet exist.
      const std::string dirpath = filter_url_dirname (fullpath);
      if (!file_or_dir_exists (dirpath)) {
        filter_url_mkdir (dirpath);
      }
      std::string download_url = filter_url_build_http_query (url, "a", std::to_string (Sync_Logic::files_file_download));
      download_url = filter_url_build_http_query (download_url, "v", std::to_string (version));
      download_url = filter_url_build_http_query (download_url, "d", std::to_string (d));
      download_url = filter_url_build_http_query (download_url, "u", filter::strings::bin2hex (user));
      download_url = filter_url_build_http_query (download_url, "f", filter_url_urlencode (file));
      // Download and save file locally.
      filter_url_download_file (download_url, fullpath, error, true);
      if (!error.empty ()) {
        Database_Logs::log (sendreceive_files_text () + "Failure downloading file: " + error, Filter_Roles::translator ());
        sendreceive_files_done ();
        return;
      }
      // When downloading privileges, load them in the database on the client.
      if (directory == database_privileges_directory (user)) {
        if (file == database_privileges_file ()) {
          DatabasePrivileges::load (user, filter_url_file_get_contents (fullpath));
        }
      }
    }
  }

  
  // Done.
  Database_Logs::log (sendreceive_files_text () + "Now up to date", Filter_Roles::translator ());
  sendreceive_files_done ();
}


void sendreceive_files_kick_watchdog ()
{
  sendreceive_files_watchdog = filter::date::seconds_since_epoch ();
}
