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


#include <sync/settings.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>


std::string sync_settings_url ()
{
  return "sync/settings";
}


std::string sync_settings (Webserver_Request& webserver_request)
{
  Sync_Logic sync_logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    webserver_request.response_code = 426;
    return std::string();
  }

  // Check on the credentials.
  if (!sync_logic.credentials_okay ()) return std::string();
  
  // Client makes a prioritized server call: Record the client's IP address.
  sync_logic.prioritized_ip_address_record ();

  // Get the relevant parameters the client POSTed to us, the server.
  int action = filter::strings::convert_to_int (webserver_request.post ["a"]);
  std::string value = webserver_request.post ["v"];
  // The value can be all Bibles, or one Bible.
  std::string bible_s = webserver_request.post ["b"];

  switch (action) {
    case Sync_Logic::settings_get_total_checksum:
    {
      return sync_logic.settings_checksum (filter::strings::explode (bible_s, '\n'));
    }
    case Sync_Logic::settings_send_workspace_urls:
    {
      webserver_request.database_config_user()->setWorkspaceURLs (value);
      return std::string();
    }
    case Sync_Logic::settings_get_workspace_urls:
    {
      return webserver_request.database_config_user()->getWorkspaceURLs ();
    }
    case Sync_Logic::settings_send_workspace_widths:
    {
      webserver_request.database_config_user()->setWorkspaceWidths (value);
      return std::string();
    }
    case Sync_Logic::settings_get_workspace_widths:
    {
      return webserver_request.database_config_user()->getWorkspaceWidths ();
    }
    case Sync_Logic::settings_send_workspace_heights:
    {
      webserver_request.database_config_user()->setWorkspaceHeights (value);
      return std::string();
    }
    case Sync_Logic::settings_get_workspace_heights:
    {
      return webserver_request.database_config_user()->getWorkspaceHeights ();
    }
    case Sync_Logic::settings_send_resources_organization:
    {
      std::vector <std::string> resources = filter::strings::explode (value, '\n');
      webserver_request.database_config_user()->setActiveResources (resources);
      return std::string();
    }
    case Sync_Logic::settings_get_resources_organization:
    {
      std::vector <std::string> resources = webserver_request.database_config_user()->getActiveResources ();
      return filter::strings::implode (resources, "\n");
    }
    case Sync_Logic::settings_get_bible_id:
    {
      // No longer in use since June 2016.
      return "1";
    }
    case Sync_Logic::settings_get_bible_font:
    {
      return database::config::bible::get_text_font (bible_s);
    }
    case Sync_Logic::settings_send_platform:
    {
      // No longer in use, just discard this.
      return std::string();
    }
    case Sync_Logic::settings_get_privilege_delete_consultation_notes:
    {
      return filter::strings::convert_to_string (webserver_request.database_config_user()->getPrivilegeDeleteConsultationNotes ());
    }
    default:
    {
    }
  }

  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  std::this_thread::sleep_for (std::chrono::seconds (1));
  webserver_request.response_code = 400;
  return std::string();
}
