/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <sendreceive/settings.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/date.h>
#include <tasks/logic.h>
#include <config/globals.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <sync/settings.h>
#include <user/logic.h>
using namespace std;


int sendreceive_settings_watchdog = 0;


string sendreceive_settings_text ()
{
  return translate("Settings") + ": ";
}


string sendreceive_settings_sendreceive_text ()
{
  return sendreceive_settings_text () + translate ("Send/receive");
}


string sendreceive_settings_up_to_date_text ()
{
  return sendreceive_settings_text () + translate ("Up to date");
}


void sendreceive_settings_done ()
{
  sendreceive_settings_watchdog = 0;
  config_globals_syncing_settings = false;
}


void sendreceive_settings ()
{
  if (sendreceive_settings_watchdog) {
    int time = filter::date::seconds_since_epoch ();
    if (time < (sendreceive_settings_watchdog + 900)) {
      Database_Logs::log ("Settings: " + translate("Still busy"), Filter_Roles::translator ());
      return;
    }
    Database_Logs::log ("Settings: " + translate("Watchdog timeout"), Filter_Roles::translator ());
  }
  sendreceive_settings_kick_watchdog ();
  config_globals_syncing_settings = true;
  
  Database_Logs::log (sendreceive_settings_sendreceive_text (), Filter_Roles::translator ());
  
  Webserver_Request request;
  Sync_Logic sync_logic = Sync_Logic (&request);

  string response = client_logic_connection_setup ("", "");
  int iresponse = convert_to_int (response);
  if (iresponse < Filter_Roles::guest () || iresponse > Filter_Roles::admin ()) {
    Database_Logs::log (translate("Failure sending and receiving Settings"), Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }

  // Set the correct user in the session: The sole user on the Client.
  vector <string> users = request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (translate("No user found"), Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }
  string user = users [0];
  request.session_logic ()->set_username (user);
  
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  string url = client_logic_url (address, port, sync_settings_url ());
  
  // Go through all settings flagged as having been updated on this client.
  vector <int> ids = request.database_config_user()->getUpdatedSettings ();
  if (!ids.empty ()) {
    Database_Logs::log (translate("Sending settings"), Filter_Roles::translator ());
  }
  
  // The POST request contains the credentials.
  map <string, string> post;
  post ["u"] = bin2hex (user);
  post ["p"] = request.database_users ()->get_md5 (user);
  post ["l"] = convert_to_string (request.database_users ()->get_level (user));

  for (auto id : ids) {

    // What to request for.
    post ["a"] = convert_to_string (id);

    string value {};
    switch (id) {
      case Sync_Logic::settings_send_workspace_urls:
        value = request.database_config_user()->getWorkspaceURLs ();
        break;
      case Sync_Logic::settings_send_workspace_widths:
        value = request.database_config_user()->getWorkspaceWidths ();
        break;
      case Sync_Logic::settings_send_workspace_heights:
        value = request.database_config_user()->getWorkspaceHeights ();
        break;
      case Sync_Logic::settings_send_resources_organization:
      {
        vector <string> resources = request.database_config_user()->getActiveResources ();
        value = filter_string_implode (resources, "\n");
        break;
      }
      default: break;
    }
    post ["v"] = value;
    
    // POST the setting to the server.
    string error;
    sync_logic.post (post, url, error);
    
    // Handle server's response.
    if (!error.empty ()) {
      Database_Logs::log ("Failure sending setting to server", Filter_Roles::translator ());
    } else {
      request.database_config_user()->removeUpdatedSetting (id);
    }
  }
  // All changed settings have now been sent to the server.

  // Send the platform to the Cloud.
  {
    post ["a"] = convert_to_string (Sync_Logic::settings_send_platform);
    // No longer in use.
    int platform_id = 0;
    post ["v"] = convert_to_string (platform_id);
    string error;
    sync_logic.post (post, url, error);
  }
    
  // The client will now synchronize its settings with the server's settings.

  // The script requests the checksum of all relevant settings from the server.
  // It includes the local Bibles for the server to include settings for those Bibles.
  // It compares this with the local checksum.
  // If it matches, that means that the local settings match the settings on the server.
  // The script is then ready.
  if (post.count ("v")) post.erase (post.find ("v"));
  vector <string> bibles = request.database_bibles ()->getBibles ();
  post ["a"] = convert_to_string (Sync_Logic::settings_get_total_checksum);
  post ["b"] = filter_string_implode (bibles, "\n");
  string error;
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure synchronizing Settings while requesting totals", Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }
  if (post.count ("b")) post.erase (post.find ("b"));
  string checksum = sync_logic.settings_checksum (bibles);
  if (response == checksum) {
    Database_Logs::log (sendreceive_settings_up_to_date_text (), Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }

  // At this stage the total checksum of all relevant settings on the client differs from the same on the server.
  // Request all settings from the server.

  post ["a"] = convert_to_string (Sync_Logic::settings_get_workspace_urls);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace URLS", Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }
  request.database_config_user()->setWorkspaceURLs (response);

  post ["a"] = convert_to_string (Sync_Logic::settings_get_workspace_widths);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace widths", Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }
  request.database_config_user()->setWorkspaceWidths (response);

  post ["a"] = convert_to_string (Sync_Logic::settings_get_workspace_heights);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace heights", Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }
  request.database_config_user()->setWorkspaceHeights (response);

  post ["a"] = convert_to_string (Sync_Logic::settings_get_resources_organization);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace heights", Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }
  request.database_config_user()->setActiveResources (filter_string_explode (response, '\n'));
  
  // Fetch values for the Bibles.
  for (auto & bible : bibles) {

    post ["b"] = bible;

    // Request the font for the Bible.
    // Note that it requests the font name from the Cloud.
    // When the font is set by the client, it will override the font setting from the Cloud.
    post ["a"] = convert_to_string (Sync_Logic::settings_get_bible_font);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log ("Failure receiving Bible font", Filter_Roles::translator ());
      sendreceive_settings_done ();
      return;
    }
    Database_Config_Bible::setTextFont (bible, response);
  }

  post ["a"] = convert_to_string (Sync_Logic::settings_get_privilege_delete_consultation_notes);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving privilege delete consultation notes", Filter_Roles::translator ());
    sendreceive_settings_done ();
    return;
  }
  request.database_config_user()->setPrivilegeDeleteConsultationNotes (convert_to_bool (response));

  // Done.
  Database_Logs::log ("Settings: Updated", Filter_Roles::translator ());
  sendreceive_settings_done ();
}


void sendreceive_settings_kick_watchdog ()
{
  sendreceive_settings_watchdog = filter::date::seconds_since_epoch ();
}

