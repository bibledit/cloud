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


#include <sync/files.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <database/logs.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <checksum/logic.h>


std::string sync_files_url ()
{
  return "sync/files";
}


std::string sync_files (Webserver_Request& webserver_request)
{
  Sync_Logic sync_logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    webserver_request.response_code = 426;
    return std::string();
  }

  // If the client's IP address very recently made a prioritized server call,
  // then delay the current call.
  // This is the way to give priority to the other call:
  // Not clogging the client's internet connection.
  if (sync_logic.prioritized_ip_address_active ()) {
    std::this_thread::sleep_for (std::chrono::seconds (5));
  }
  
  if (webserver_request.post.empty ()) {
    for (auto& [key, value] : webserver_request.query) {
      webserver_request.post.emplace_back(key, value);
    }
  }
  const std::string user = filter::strings::hex2bin (webserver_request.post_get("u"));
  const int action = filter::strings::convert_to_int (webserver_request.post_get("a"));
  const int version = filter::strings::convert_to_int (webserver_request.post_get("v"));
  const size_t d = static_cast<size_t>(filter::strings::convert_to_int (webserver_request.post_get("d")));
  const std::string file = webserver_request.post_get("f");

  // For security reasons a client does not specify the directory of the file to be downloaded.
  // Rather it specifies the offset within the list of allowed directories for the version.
  const std::vector <std::string> directories = Sync_Logic::files_get_directories (version, user);
  if (d >= directories.size ()) {
    webserver_request.response_code = 400;
    return std::string();
  }
  const std::string directory = directories [d];
  
  if (action == Sync_Logic::files_total_checksum) {
    return std::to_string (Sync_Logic::files_get_total_checksum (version, user));
  }
  
  else if (action == Sync_Logic::files_directory_checksum) {
    int checksum = Sync_Logic::files_get_directory_checksum (directory);
    return std::to_string (checksum);
  }

  else if (action == Sync_Logic::files_directory_files) {
    std::vector <std::string> paths = Sync_Logic::files_get_files (directory);
    return filter::strings::implode (paths, "\n");
  }

  else if (action == Sync_Logic::files_file_checksum) {
    int checksum = Sync_Logic::files_get_file_checksum (directory, file);
    return std::to_string (checksum);
  }
  
  else if (action == Sync_Logic::files_file_download) {
    // This triggers the correct mime type.
    webserver_request.get = "file.download";
    // Return the file's contents.
    const std::string path = filter_url_create_root_path ({directory, file});
    return filter_url_file_get_contents (path);
  }
  
  // Bad request. Delay flood of bad requests.
  std::this_thread::sleep_for (std::chrono::seconds (1));
  webserver_request.response_code = 400;
  return std::string();
}

