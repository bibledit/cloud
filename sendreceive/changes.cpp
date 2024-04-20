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


#include <sendreceive/changes.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <config/globals.h>
#include <database/config/general.h>
#include <database/modifications.h>
#include <database/logs.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <sync/changes.h>
#include <checksum/logic.h>


int sendreceive_changes_watchdog {0};


void send_receive_changes_done ()
{
  sendreceive_changes_watchdog = 0;
  config_globals_syncing_changes = false;
}


std::string sendreceive_changes_text ()
{
  return translate("Changes") + ": ";
}


std::string sendreceive_changes_sendreceive_text ()
{
  return sendreceive_changes_text () + translate ("Send/receive");
}


std::string sendreceive_changes_up_to_date_text ()
{
  return sendreceive_changes_text () + translate ("Up to date");
}


void sendreceive_changes ()
{
  if (sendreceive_changes_watchdog) {
    int time = filter::date::seconds_since_epoch ();
    if (time < (sendreceive_changes_watchdog + 900)) {
      Database_Logs::log (sendreceive_changes_text () + translate("Still busy"), Filter_Roles::translator ());
      return;
    }
    Database_Logs::log (sendreceive_changes_text () + translate("Watchdog timeout"), Filter_Roles::translator ());
  }
  sendreceive_changes_kick_watchdog ();
  config_globals_syncing_changes = true;

  
  Database_Logs::log (sendreceive_changes_sendreceive_text (), Filter_Roles::translator ());
  

  Webserver_Request webserver_request;
  Sync_Logic sync_logic (webserver_request);
  Database_Modifications database_modifications;
  
  
  if (!database_modifications.healthy ()) {
    Database_Logs::log (sendreceive_changes_text () + translate("Recreate damaged modifications database"), Filter_Roles::translator ());
    database_modifications.erase ();
    database_modifications.create ();
  }
  
  
  std::string response = client_logic_connection_setup ("", "");
  int iresponse = filter::strings::convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (sendreceive_changes_text () + translate("Failure to initiate connection"), Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  
  
  // Set the correct user in the session: The sole user on the Client.
  std::vector <std::string> users = webserver_request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (translate("No user found"), Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  std::string user = users [0];
  webserver_request.session_logic ()->set_username (user);
  std::string password = webserver_request.database_users ()->get_md5 (user);
  
  
  // The basic request to be POSTed to the server.
  // It contains the user's credentials.
  std::map <std::string, std::string> post;
  post ["u"] = filter::strings::bin2hex (user);
  post ["p"] = password;
  post ["l"] = filter::strings::convert_to_string (webserver_request.database_users ()->get_level (user));
  
  
  // Error variables.
  std::string error;
  bool communication_errors = false;
  
  
  // Server URL to call.
  std::string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  std::string url = client_logic_url (address, port, sync_changes_url ());
  
  
  // Send the removed change notifications to the server.
  std::vector <int> ids = webserver_request.database_config_user ()->getRemovedChanges ();
  if (!ids.empty ()) Database_Logs::log (sendreceive_changes_text () + "Sending removed notifications: " + filter::strings::convert_to_string (ids.size()), Filter_Roles::translator ());
  for (auto & id : ids) {
    post ["a"] = filter::strings::convert_to_string (Sync_Logic::changes_delete_modification);
    post ["i"] = filter::strings::convert_to_string (id);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      communication_errors = true;
      Database_Logs::log (sendreceive_changes_text () + "Failure sending removed notification: " + error, Filter_Roles::translator ());
    }
    else {
      webserver_request.database_config_user ()->removeRemovedChange (id);
    }
  }
  
  
  if (communication_errors) {
    Database_Logs::log (sendreceive_changes_text () + translate("Not downloading change notifications due to communication error"), Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  
  
  // Compare the total checksum for the change notifications for the active user on client and server.
  // Checksum is cached for future re-use.
  // Take actions based on that.
  std::string client_checksum = webserver_request.database_config_user ()->getChangeNotificationsChecksum ();
  if (client_checksum.empty ()) {
    client_checksum = Sync_Logic::changes_checksum (user);
    webserver_request.database_config_user ()->setChangeNotificationsChecksum (client_checksum);
  }
  std::string server_checksum;
  post ["a"] = filter::strings::convert_to_string (Sync_Logic::changes_get_checksum);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log (sendreceive_changes_text () + "Failure receiving checksum: " + error, Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  server_checksum = response;
  if (client_checksum == server_checksum) {
    Database_Logs::log (sendreceive_changes_up_to_date_text (), Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  
  
  // Get all identifiers for the notifications on the server for the user.
  // Get the identifiers on the client.
  std::string any_bible = "";
  std::vector <int> client_identifiers = database_modifications.getNotificationIdentifiers (user, any_bible);
  std::vector <int> server_identifiers;
  post ["a"] = filter::strings::convert_to_string (Sync_Logic::changes_get_identifiers);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log (sendreceive_changes_text () + "Failure receiving identifiers: " + error, Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  {
    std::vector <std::string> ids2 = filter::strings::explode (response, '\n');
    for (auto & id : ids2) server_identifiers.push_back (filter::strings::convert_to_int (id));
  }

  
  // Any identifiers on the client, but not on the server, remove them from the client.
  std::vector <int> remove_identifiers = filter::strings::array_diff (client_identifiers, server_identifiers);
  for (auto & id : remove_identifiers) {
    database_modifications.deleteNotification (id);
    webserver_request.database_config_user ()->setChangeNotificationsChecksum ("");
    Database_Logs::log (sendreceive_changes_text () + "Removing notification: " + filter::strings::convert_to_string (id), Filter_Roles::translator ());
  }

  
  // Any identifiers on the server, but not on the client, download them from the server.
  std::vector <int> download_identifiers = filter::strings::array_diff (server_identifiers, client_identifiers);
  for (auto & id : download_identifiers) {
    sendreceive_changes_kick_watchdog ();
    Database_Logs::log (sendreceive_changes_text () + "Downloading notification: " + filter::strings::convert_to_string (id), Filter_Roles::translator ());
    post ["a"] = filter::strings::convert_to_string (Sync_Logic::changes_get_modification);
    post ["i"] = filter::strings::convert_to_string (id);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (sendreceive_changes_text () + "Failure downloading notification: " + error, Filter_Roles::translator ());
    }
    else {
      // The server has put all bits together, one bit per line.
      std::vector <std::string> lines = filter::strings::explode (response, '\n');
      std::string category;
      if (!lines.empty ()) {
        category = lines [0];
        lines.erase (lines.begin ());
      }
      std::string bible;
      if (!lines.empty ()) {
        bible = lines [0];
        lines.erase (lines.begin ());
      }
      int book = 0;
      if (!lines.empty ()) {
        book = filter::strings::convert_to_int (lines [0]);
        lines.erase (lines.begin ());
      }
      int chapter = 0;
      if (!lines.empty ()) {
        chapter = filter::strings::convert_to_int (lines [0]);
        lines.erase (lines.begin ());
      }
      int verse = 0;
      if (!lines.empty ()) {
        verse = filter::strings::convert_to_int (lines [0]);
        lines.erase (lines.begin ());
      }
      std::string oldtext;
      if (!lines.empty ()) {
        oldtext = lines [0];
        lines.erase (lines.begin ());
      }
      std::string modification;
      if (!lines.empty ()) {
        modification = lines [0];
        lines.erase (lines.begin ());
      }
      std::string newtext;
      if (!lines.empty ()) {
        newtext = lines [0];
        lines.erase (lines.begin ());
      }
      database_modifications.storeClientNotification (id, user, category, bible, book, chapter, verse, oldtext, modification, newtext);
      webserver_request.database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  

  // Done.
  Database_Logs::log (sendreceive_changes_text () + "Ready", Filter_Roles::translator ());
  send_receive_changes_done ();
}


void sendreceive_changes_kick_watchdog ()
{
  sendreceive_changes_watchdog = filter::date::seconds_since_epoch ();
}
