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


int sendreceive_changes_watchdog = 0;


void send_receive_changes_done ()
{
  sendreceive_changes_watchdog = 0;
  config_globals_syncing_changes = false;
}


string sendreceive_changes_text ()
{
  return translate("Changes") + ": ";
}


string sendreceive_changes_sendreceive_text ()
{
  return sendreceive_changes_text () + translate ("Send/receive");
}


string sendreceive_changes_up_to_date_text ()
{
  return sendreceive_changes_text () + translate ("Up to date");
}


void sendreceive_changes ()
{
  if (sendreceive_changes_watchdog) {
    int time = filter_date_seconds_since_epoch ();
    if (time < (sendreceive_changes_watchdog + 900)) {
      Database_Logs::log (sendreceive_changes_text () + translate("Still busy"), Filter_Roles::translator ());
      return;
    }
    Database_Logs::log (sendreceive_changes_text () + translate("Watchdog timeout"), Filter_Roles::translator ());
  }
  sendreceive_changes_kick_watchdog ();
  config_globals_syncing_changes = true;

  
  Database_Logs::log (sendreceive_changes_sendreceive_text (), Filter_Roles::translator ());
  

  Webserver_Request request;
  Sync_Logic sync_logic = Sync_Logic (&request);
  Database_Modifications database_modifications;
  
  
  if (!database_modifications.healthy ()) {
    Database_Logs::log (sendreceive_changes_text () + translate("Recreate damaged modifications database"), Filter_Roles::translator ());
    database_modifications.erase ();
    database_modifications.create ();
  }
  
  
  string response = client_logic_connection_setup ("", "");
  int iresponse = convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (sendreceive_changes_text () + translate("Failure to initiate connection"), Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  
  
  // Set the correct user in the session: The sole user on the Client.
  vector <string> users = request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (translate("No user found"), Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  string user = users [0];
  request.session_logic ()->setUsername (user);
  string password = request.database_users ()->get_md5 (user);
  
  
  // The basic request to be POSTed to the server.
  // It contains the user's credentials.
  map <string, string> post;
  post ["u"] = bin2hex (user);
  post ["p"] = password;
  post ["l"] = convert_to_string (request.database_users ()->get_level (user));
  
  
  // Error variables.
  string error;
  bool communication_errors = false;
  
  
  // Server URL to call.
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  string url = client_logic_url (address, port, sync_changes_url ());
  
  
  // Send the removed change notifications to the server.
  vector <int> ids = request.database_config_user ()->getRemovedChanges ();
  if (!ids.empty ()) Database_Logs::log (sendreceive_changes_text () + "Sending removed notifications: " + convert_to_string (ids.size()), Filter_Roles::translator ());
  for (auto & id : ids) {
    post ["a"] = convert_to_string (Sync_Logic::changes_delete_modification);
    post ["i"] = convert_to_string (id);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      communication_errors = true;
      Database_Logs::log (sendreceive_changes_text () + "Failure sending removed notification: " + error, Filter_Roles::translator ());
    }
    else {
      request.database_config_user ()->removeRemovedChange (id);
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
  string client_checksum = request.database_config_user ()->getChangeNotificationsChecksum ();
  if (client_checksum.empty ()) {
    client_checksum = Sync_Logic::changes_checksum (user);
    request.database_config_user ()->setChangeNotificationsChecksum (client_checksum);
  }
  string server_checksum;
  post ["a"] = convert_to_string (Sync_Logic::changes_get_checksum);
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
  string any_bible = "";
  vector <int> client_identifiers = database_modifications.getNotificationIdentifiers (user, any_bible);
  vector <int> server_identifiers;
  post ["a"] = convert_to_string (Sync_Logic::changes_get_identifiers);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log (sendreceive_changes_text () + "Failure receiving identifiers: " + error, Filter_Roles::translator ());
    send_receive_changes_done ();
    return;
  }
  {
    vector <string> ids = filter_string_explode (response, '\n');
    for (auto & id : ids) server_identifiers.push_back (convert_to_int (id));
  }

  
  // Any identifiers on the client, but not on the server, remove them from the client.
  vector <int> remove_identifiers = filter_string_array_diff (client_identifiers, server_identifiers);
  for (auto & id : remove_identifiers) {
    database_modifications.deleteNotification (id);
    request.database_config_user ()->setChangeNotificationsChecksum ("");
    Database_Logs::log (sendreceive_changes_text () + "Removing notification: " + convert_to_string (id), Filter_Roles::translator ());
  }

  
  // Any identifiers on the server, but not on the client, download them from the server.
  vector <int> download_identifiers = filter_string_array_diff (server_identifiers, client_identifiers);
  for (auto & id : download_identifiers) {
    sendreceive_changes_kick_watchdog ();
    Database_Logs::log (sendreceive_changes_text () + "Downloading notification: " + convert_to_string (id), Filter_Roles::translator ());
    post ["a"] = convert_to_string (Sync_Logic::changes_get_modification);
    post ["i"] = convert_to_string (id);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log (sendreceive_changes_text () + "Failure downloading notification: " + error, Filter_Roles::translator ());
    }
    else {
      // The server has put all bits together, one bit per line.
      vector <string> lines = filter_string_explode (response, '\n');
      string category;
      if (!lines.empty ()) {
        category = lines [0];
        lines.erase (lines.begin ());
      }
      string bible;
      if (!lines.empty ()) {
        bible = lines [0];
        lines.erase (lines.begin ());
      }
      int book = 0;
      if (!lines.empty ()) {
        book = convert_to_int (lines [0]);
        lines.erase (lines.begin ());
      }
      int chapter = 0;
      if (!lines.empty ()) {
        chapter = convert_to_int (lines [0]);
        lines.erase (lines.begin ());
      }
      int verse = 0;
      if (!lines.empty ()) {
        verse = convert_to_int (lines [0]);
        lines.erase (lines.begin ());
      }
      string oldtext;
      if (!lines.empty ()) {
        oldtext = lines [0];
        lines.erase (lines.begin ());
      }
      string modification;
      if (!lines.empty ()) {
        modification = lines [0];
        lines.erase (lines.begin ());
      }
      string newtext;
      if (!lines.empty ()) {
        newtext = lines [0];
        lines.erase (lines.begin ());
      }
      database_modifications.storeClientNotification (id, user, category, bible, book, chapter, verse, oldtext, modification, newtext);
      request.database_config_user ()->setChangeNotificationsChecksum ("");
    }
  }
  

  // Done.
  Database_Logs::log (sendreceive_changes_text () + "Ready", Filter_Roles::translator ());
  send_receive_changes_done ();
}


void sendreceive_changes_kick_watchdog ()
{
  sendreceive_changes_watchdog = filter_date_seconds_since_epoch ();
}
