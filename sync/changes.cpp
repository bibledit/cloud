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


#include <sync/changes.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/modifications.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <checksum/logic.h>
#include <config/globals.h>


std::string sync_changes_url ()
{
  return "sync/changes";
}


std::string sync_changes (Webserver_Request& webserver_request)
{
  Sync_Logic sync_logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    webserver_request.response_code = 426;
    return std::string();
  }
  
  // Check on the credentials.
  if (!sync_logic.credentials_okay ()) return std::string();

  // Bail out if the change notifications are not now available to clients.
  if (!config_globals_change_notifications_available) {
    webserver_request.response_code = 503;
    return std::string();
  }
  
  // Client makes a prioritized server call: Record the client's IP address.
  sync_logic.prioritized_ip_address_record ();

  // Get the relevant parameters the client may have POSTed to us, the server.
  std::string user = filter::strings::hex2bin (webserver_request.post ["u"]);
  int action = filter::strings::convert_to_int (webserver_request.post ["a"]);
  int id = filter::strings::convert_to_int (webserver_request.post ["i"]);

  switch (action) {
    case Sync_Logic::changes_delete_modification:
    {
      // The server deletes the change notification.
      database::modifications::deleteNotification (id);
      Database_Logs::log ("Client deletes change notification from server: " + std::to_string (id), Filter_Roles::translator ());
      webserver_request.database_config_user ()->setChangeNotificationsChecksum ("");
      return std::string();
    }
    case Sync_Logic::changes_get_checksum:
    {
      // The server responds with the possibly cached total checksum for the user's change notifications.
      std::string checksum = webserver_request.database_config_user ()->getChangeNotificationsChecksum ();
      if (checksum.empty ()) {
        checksum = Sync_Logic::changes_checksum (user);
        webserver_request.database_config_user ()->setChangeNotificationsChecksum (checksum);
      }
      return checksum;
    }
    case Sync_Logic::changes_get_identifiers:
    {
      // The server responds with the identifiers of all the user's change notifications.
      std::string any_bible {};
      std::vector <int> notification_ids = database::modifications::getNotificationIdentifiers (user, any_bible);
      std::string response;
      for (auto & notif_id : notification_ids) {
        if (!response.empty ()) response.append ("\n");
        response.append (std::to_string (notif_id));
      }
      return response;
    }
    case Sync_Logic::changes_get_modification:
    {
      // The server responds with the relevant data of the requested modification.
      std::vector <std::string> lines;
      // category
      lines.push_back (database::modifications::getNotificationCategory (id));
      // bible
      lines.push_back (database::modifications::getNotificationBible (id));
      // book
      // chapter
      // verse
      Passage passage = database::modifications::getNotificationPassage (id);
      lines.push_back (std::to_string (passage.m_book));
      lines.push_back (std::to_string (passage.m_chapter));
      lines.push_back (passage.m_verse);
      // oldtext (ensure it's one line for correct transfer to client)
      std::string oldtext = database::modifications::getNotificationOldText (id);
      oldtext = filter::strings::replace ("\n", " ", oldtext);
      lines.push_back (oldtext);
      // modification (ensure it's one line for correct transfer to client)
      std::string modification = database::modifications::getNotificationModification (id);
      modification = filter::strings::replace ("\n", " ", modification);
      lines.push_back (modification);
      // newtext (ensure it's one line for correct transfer to client)
      std::string newtext = database::modifications::getNotificationNewText (id);
      newtext = filter::strings::replace ("\n", " ", newtext);
      lines.push_back (newtext);
      // Result.
      return filter::strings::implode (lines, "\n");
    }
    default: {};
  }

  
  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  std::this_thread::sleep_for (std::chrono::seconds (1));
  webserver_request.response_code = 400;
  return std::string();
}
