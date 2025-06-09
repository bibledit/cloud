/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


int sendreceive_settings_watchdog = 0;


std::string sendreceive_settings_text ()
{
  return translate("Settings") + ": ";
}


std::string sendreceive_settings_sendreceive_text ()
{
  return sendreceive_settings_text () + translate ("Send/receive");
}


std::string sendreceive_settings_up_to_date_text ()
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
      Database_Logs::log ("Settings: " + translate("Still busy"), roles::translator);
      return;
    }
    Database_Logs::log ("Settings: " + translate("Watchdog timeout"), roles::translator);
  }
  sendreceive_settings_kick_watchdog ();
  config_globals_syncing_settings = true;
  
  Database_Logs::log (sendreceive_settings_sendreceive_text (), roles::translator);
  
  Webserver_Request webserver_request;
  Sync_Logic sync_logic (webserver_request);

  std::string response = client_logic_connection_setup ("", "");
  int iresponse = filter::strings::convert_to_int (response);
  if (iresponse < roles::guest || iresponse > roles::admin) {
    Database_Logs::log (translate("Failure sending and receiving Settings"), roles::translator);
    sendreceive_settings_done ();
    return;
  }

  // Set the correct user in the session: The sole user on the Client.
  std::vector <std::string> users = webserver_request.database_users ()->get_users ();
  if (users.empty ()) {
    Database_Logs::log (translate("No user found"), roles::translator);
    sendreceive_settings_done ();
    return;
  }
  std::string user = users [0];
  webserver_request.session_logic ()->set_username (user);
  
  std::string address = database::config::general::get_server_address ();
  int port = database::config::general::get_server_port ();
  std::string url = client_logic_url (address, port, sync_settings_url ());
  
  // Go through all settings flagged as having been updated on this client.
  std::vector <int> ids = webserver_request.database_config_user()->get_updated_settings ();
  if (!ids.empty ()) {
    Database_Logs::log (translate("Sending settings"), roles::translator);
  }
  
  // The POST request contains the credentials.
  std::map <std::string, std::string> post;
  post ["u"] = filter::strings::bin2hex (user);
  post ["p"] = webserver_request.database_users ()->get_md5 (user);
  post ["l"] = std::to_string (webserver_request.database_users ()->get_level (user));

  for (auto id : ids) {

    // What to request for.
    post ["a"] = std::to_string (id);

    std::string value {};
    switch (id) {
      case Sync_Logic::settings_send_workspace_urls:
        value = webserver_request.database_config_user()->get_workspace_urls ();
        break;
      case Sync_Logic::settings_send_workspace_widths:
        value = webserver_request.database_config_user()->get_workspace_widths ();
        break;
      case Sync_Logic::settings_send_workspace_heights:
        value = webserver_request.database_config_user()->get_workspace_heights ();
        break;
      case Sync_Logic::settings_send_resources_organization:
      {
        std::vector <std::string> resources = webserver_request.database_config_user()->get_active_resources ();
        value = filter::strings::implode (resources, "\n");
        break;
      }
      default: break;
    }
    post ["v"] = value;
    
    // POST the setting to the server.
    std::string error;
    sync_logic.post (post, url, error);
    
    // Handle server's response.
    if (!error.empty ()) {
      Database_Logs::log ("Failure sending setting to server", roles::translator);
    } else {
      webserver_request.database_config_user()->remove_updated_setting (id);
    }
  }
  // All changed settings have now been sent to the server.

  // Send the platform to the Cloud.
  {
    post ["a"] = std::to_string (Sync_Logic::settings_send_platform);
    // No longer in use.
    int platform_id = 0;
    post ["v"] = std::to_string (platform_id);
    std::string error;
    sync_logic.post (post, url, error);
  }
    
  // The client will now synchronize its settings with the server's settings.

  // The script requests the checksum of all relevant settings from the server.
  // It includes the local Bibles for the server to include settings for those Bibles.
  // It compares this with the local checksum.
  // If it matches, that means that the local settings match the settings on the server.
  // The script is then ready.
  if (post.count ("v")) post.erase (post.find ("v"));
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  post ["a"] = std::to_string (Sync_Logic::settings_get_total_checksum);
  post ["b"] = filter::strings::implode (bibles, "\n");
  std::string error;
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure synchronizing Settings while requesting totals", roles::translator);
    sendreceive_settings_done ();
    return;
  }
  if (post.count ("b")) post.erase (post.find ("b"));
  std::string checksum = sync_logic.settings_checksum (bibles);
  if (response == checksum) {
    Database_Logs::log (sendreceive_settings_up_to_date_text (), roles::translator);
    sendreceive_settings_done ();
    return;
  }

  // At this stage the total checksum of all relevant settings on the client differs from the same on the server.
  // Request all settings from the server.

  post ["a"] = std::to_string (Sync_Logic::settings_get_workspace_urls);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace URLS", roles::translator);
    sendreceive_settings_done ();
    return;
  }
  webserver_request.database_config_user()->set_workspace_urls (response);

  post ["a"] = std::to_string (Sync_Logic::settings_get_workspace_widths);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace widths", roles::translator);
    sendreceive_settings_done ();
    return;
  }
  webserver_request.database_config_user()->set_workspace_widths (response);

  post ["a"] = std::to_string (Sync_Logic::settings_get_workspace_heights);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace heights", roles::translator);
    sendreceive_settings_done ();
    return;
  }
  webserver_request.database_config_user()->set_workspace_heights (response);

  post ["a"] = std::to_string (Sync_Logic::settings_get_resources_organization);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving workspace heights", roles::translator);
    sendreceive_settings_done ();
    return;
  }
  webserver_request.database_config_user()->set_active_resources (filter::strings::explode (response, '\n'));
  
  // Fetch values for the Bibles.
  for (auto & bible : bibles) {

    post ["b"] = bible;

    // Request the font for the Bible.
    // Note that it requests the font name from the Cloud.
    // When the font is set by the client, it will override the font setting from the Cloud.
    post ["a"] = std::to_string (Sync_Logic::settings_get_bible_font);
    response = sync_logic.post (post, url, error);
    if (!error.empty ()) {
      Database_Logs::log ("Failure receiving Bible font", roles::translator);
      sendreceive_settings_done ();
      return;
    }
    database::config::bible::set_text_font (bible, response);
  }

  post ["a"] = std::to_string (Sync_Logic::settings_get_privilege_delete_consultation_notes);
  response = sync_logic.post (post, url, error);
  if (!error.empty ()) {
    Database_Logs::log ("Failure receiving privilege delete consultation notes", roles::translator);
    sendreceive_settings_done ();
    return;
  }
  webserver_request.database_config_user()->set_privilege_delete_consultation_notes (filter::strings::convert_to_bool (response));

  // Done.
  Database_Logs::log ("Settings: Updated", roles::translator);
  sendreceive_settings_done ();
}


void sendreceive_settings_kick_watchdog ()
{
  sendreceive_settings_watchdog = filter::date::seconds_since_epoch ();
}

