/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <sync/resources.h>
#include <sync/logic.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <resource/logic.h>
#include <database/cache.h>
#include <database/config/general.h>
#include <tasks/logic.h>


string sync_resources_url ()
{
  return "sync/resources";
}


// Serves general resource content to a client.
string sync_resources (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Sync_Logic sync_logic = Sync_Logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    request->response_code = 426;
    return "";
  }

  // If the client's IP address very recently made a prioritized server call,
  // then delay the current call.
  // This is to give priority to the other calls from the same client:
  // Not clogging that client's internet connection.
  if (sync_logic.prioritized_ip_address_active ()) {
    this_thread::sleep_for (chrono::seconds (5));
  }

  int action = convert_to_int (request->query ["a"]);
  string resource = request->query ["r"];
  int book = convert_to_int (request->query ["b"]);
  int chapter = convert_to_int (request->query ["c"]);
  int verse = convert_to_int (request->query ["v"]);
  
  bool request_ok = true;
  if (book <= 0) request_ok = false;
  if (chapter < 0) request_ok = false;
  if (chapter > 151) request_ok = false;
  if (verse < 0) request_ok = false;
  if (verse > 200) request_ok = false;
  
  if (request_ok) {
    switch (action) {

      case Sync_Logic::resources_request_text:
      {
        return resource_logic_get_contents_for_client (resource, book, chapter, verse);
      }
      
      case Sync_Logic::resources_request_database:
      {
        // If the cache is ready, return its file size.
        if (Database_Cache::exists (resource, book)) {
          if (Database_Cache::ready (resource, book)) {
            return convert_to_string (Database_Cache::size (resource, book));
          }
        }
        // Schedule this resource for caching if that's not yet the case.
        vector <string> signatures = Database_Config_General::getResourcesToCache ();
        string signature = resource + " " + convert_to_string (book);
        if (!in_array (signature, signatures)) {
          signatures.push_back (signature);
          Database_Config_General::setResourcesToCache (signatures);
        }
        if (!tasks_logic_queued (CACHERESOURCES)) {
          tasks_logic_queue (CACHERESOURCES);
        }
        return "0";
      }
      
      case Sync_Logic::resources_request_download:
      {
        return Database_Cache::path (resource, book);
      }
      
      default: break;
    }
  }
    
  // Bad request. Delay flood of bad requests.
  this_thread::sleep_for (chrono::seconds (1));
  request->response_code = 400;
  return "";
}
