/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <sync/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/md5.h>
#include <filter/roles.h>
#include <filter/date.h>
#include <database/notes.h>
#include <database/noteactions.h>
#include <database/mail.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/usfmresources.h>
#include <database/modifications.h>
#include <config/globals.h>
#include <trash/handler.h>
#include <user/logic.h>


mutex sync_logic_mutex;


Sync_Logic::Sync_Logic (void * webserver_request_in)
{
  webserver_request = webserver_request_in;
}


// Returns true if the request coming from the client is considered secure enough.
bool Sync_Logic::security_okay ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  // If the request is made via https, the security is OK.
  if (request->secure) return true;
  
  // At this stage the request is made via plain http.
  // If https is not enforced for the client, the security is considered good enough.
  if (!config_globals_enforce_https_client) return true;
  
  // Not secure enough:
  return false;
}


bool Sync_Logic::credentials_okay ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  // Brute force attack mitigating?
  if (!user_logic_login_failure_check_okay ()) {
    request->response_code = 401;
    return false;
  }
  
  // Get the credentials the client POSTed to the us, the server.
  string username = hex2bin (request->post ["u"]);
  string password = request->post ["p"];
  int level = convert_to_int (request->post ["l"]);
  
  // Check all credentials.
  bool user_ok = request->database_users ()->usernameExists (username);
  if (!user_ok) Database_Logs::log ("Non existing user: " + username, Filter_Roles::manager ());
  bool pass_ok = (password == request->database_users ()->get_md5 (username));
  if (!pass_ok) Database_Logs::log ("Incorrect password: " + password, Filter_Roles::manager ());
  bool level_ok = (level == request->database_users ()->get_level (username));
  if (!level_ok) Database_Logs::log ("Incorrect role: " + Filter_Roles::text (level), Filter_Roles::manager ());
  if (!user_ok || !pass_ok || !level_ok) {
    // Register possible brute force attack.
    user_logic_login_failure_register ();
    // Unauthorized.
    request->response_code = 401;
    return false;
  }
  
  // Set username in session.
  request->session_logic ()->setUsername (username);
  
  // OK.
  return true;
}


// Calculates the checksum of the array of note identifiers.
string Sync_Logic::checksum (const vector <int> & identifiers)
{
  Database_Notes database_notes (webserver_request);
  vector <string> checksums;
  for (const auto & identifier : identifiers) {
    checksums.push_back (database_notes.get_checksum (identifier));
  }
  string checksum = filter_string_implode (checksums, "");
  checksum = md5 (checksum);
  return checksum;
}


// This function takes a start-ing note identifier,
// and an end-ing note identifier.
// It divides this range of identifier into ten 
// smaller ranges.
// It returns an array of array (start, end) with the new ranges 
// for the note identifiers.
vector <Sync_Logic_Range> Sync_Logic::create_range (int start, int end)
{
  int range = end - start;
  range = round (range / 10);
  vector <Sync_Logic_Range> ranges;
  for (unsigned int i = 0; i <= 9; i++) {
    int first = start + (i * range);
    int last = start + ((i + 1) * range) - 1;
    if (i == 9) {
      last = end;
    }
    Sync_Logic_Range item;
    item.low = first;
    item.high = last;
    ranges.push_back (item);
  }
  return ranges;
}


// Sends a post request to the url.
// It returns the server's response, or an empty string on failure.
// burst: Set the connection timing for a burst response after a relatively long silence.
string Sync_Logic::post (map <string, string> & post, const string& url, string & error, bool burst)
{
  error.clear ();
  string response = filter_url_http_post (url, post, error, burst, true);
  if (error.empty ()) {
    // Success: Return response.
    return response;
  }
  // Failure.
  return "";
}


// Calculates the checksum of all settings to be kept in sync between server and client.
string Sync_Logic::settings_checksum (const vector <string> & bibles)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string checksum;
  checksum.append (request->database_config_user()->getWorkspaceURLs ());
  checksum.append (request->database_config_user()->getWorkspaceWidths ());
  checksum.append (request->database_config_user()->getWorkspaceHeights ());
  vector <string> resources = request->database_config_user()->getActiveResources ();
  checksum.append (filter_string_implode (resources, "\n"));
  for (auto & bible : bibles) {
    checksum.append (bible);
    // Download Bible text font name: It is the default name for the clients.
    checksum.append (Database_Config_Bible::getTextFont (bible));
  }
  checksum.append (convert_to_string (request->database_config_user()->getPrivilegeDeleteConsultationNotes ()));
  return md5 (checksum);
}


// Calculates the checksum of all USFM resources.
string Sync_Logic::usfm_resources_checksum ()
{
  vector <string> vchecksum;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  vector <string> resources = database_usfmresources.getResources ();
  for (auto & resource : resources) {
    vchecksum.push_back (usfm_resource_checksum (resource));
  }
  string checksum = filter_string_implode (vchecksum, "");
  checksum = md5 (checksum);
  return checksum;
}


// Calculates the checksum of USFM resource name.
string Sync_Logic::usfm_resource_checksum (const string& name)
{
  vector <string> vchecksum;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  vector <int> books = database_usfmresources.getBooks (name);
  for (auto & book : books) {
    vchecksum.push_back (convert_to_string (book));
    vchecksum.push_back (usfm_resource_book_checksum (name, book));
  }
  string checksum = filter_string_implode (vchecksum, "");
  checksum = md5 (checksum);
  return checksum;
}


// Calculates the checksum of USFM resource name book.
string Sync_Logic::usfm_resource_book_checksum (const string& name, int book)
{
  vector <string> vchecksum;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  vector <int> chapters = database_usfmresources.getChapters (name, book);
  for (auto & chapter : chapters) {
    vchecksum.push_back (convert_to_string (chapter));
    vchecksum.push_back (usfm_resource_chapter_checksum (name, book, chapter));
  }
  string checksum = filter_string_implode (vchecksum, "");
  checksum = md5 (checksum);
  return checksum;
}


// Calculates the checksum of USFM resource name book chapter.
string Sync_Logic::usfm_resource_chapter_checksum (const string& name, int book, int chapter)
{
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  int checksum = database_usfmresources.getSize (name, book, chapter);
  return convert_to_string (checksum);
}


// Calculates the total checksum for all the changes for $username.
string Sync_Logic::changes_checksum (const string & username)
{
  Database_Modifications database_modifications;
  string any_bible = "";
  vector <int> ids = database_modifications.getNotificationIdentifiers (username, any_bible);
  string checksum;
  for (auto & id : ids) {
    checksum.append (convert_to_string (id));
  }
  checksum = md5 (checksum);
  return checksum;
}


// This function returns the root directories to go though on files sync.
// The $version influences which root directories to include.
// The $version is passed by the client to the server,
// so the server can adapt to the client's capabilities.
vector <string> Sync_Logic::files_get_directories (int version, const string & user)
{
  vector <string> directories;
  switch (version) {
    case 1:
      directories = {
        "fonts",
        "databases/usfmresources",
        "databases/offlineresources"
      };
      break;
    case 2:
      directories = {
        "fonts",
        "databases/usfmresources",
        "databases/offlineresources",
        "databases/imageresources"
      };
      break;
    case 3:
      directories = {
        "fonts",
        "databases/usfmresources",
        "databases/offlineresources",
        "databases/imageresources",
        "databases/client"
      };
      break;
    case 4:
      directories = {
        "fonts",
        "databases/imageresources",
        "databases/client"
      };
      break;
    case 5:
      directories = {
        "fonts",
        "databases/imageresources",
        "databases/client",
        "databases/clients/" + user
      };
      break;
    default:
      break;
  }
  return directories;
}


// This returns the total checksum for all directories and files relevant to $version and $user.
int Sync_Logic::files_get_total_checksum (int version, const string & user)
{
  int checksum = 0;
  vector <string> directories = files_get_directories (version, user);
  for (auto directory : directories) {
    checksum += files_get_directory_checksum (directory);
  }
  return checksum;
}


// This returns the total checksum for all files in one root directory.
// It does a recursive scan for the files.
int Sync_Logic::files_get_directory_checksum (string directory)
{
  int checksum = 0;
  vector <string> files = files_get_files (directory);
  for (string file : files) {
    checksum += files_get_file_checksum (directory, file);
  }
  return checksum;
}


// This returns all the paths of the files within $directory.
// $directory is relative to the web root.
// It does a recursive scan for the files.
vector <string> Sync_Logic::files_get_files (string directory)
{
  directory = filter_url_create_root_path (directory);
  vector <string> result;
  vector <string> paths;
  filter_url_recursive_scandir (directory, paths);
  for (string path : paths) {
    if (filter_url_is_dir (path)) continue;
    string extension = filter_url_get_extension (path);
    if (extension == "o") continue;
    if (extension == "h") continue;
    if (extension == "cpp") continue;
    path.erase (0, directory.length () + 1);
    result.push_back (path);
  }
  return result;
}


// This returns the checksum of a $file in $directory.
int Sync_Logic::files_get_file_checksum (string directory, string file)
{
  string path = filter_url_create_root_path (directory, file);
  int checksum = filter_url_filesize (path);
  return checksum;
}


// Makes a global record of the IP address of a client that made a prioritized server call.
void Sync_Logic::prioritized_ip_address_record ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  sync_logic_mutex.lock ();
  config_globals_prioritized_ip_addresses [request->remote_address] = filter_date_seconds_since_epoch ();
  sync_logic_mutex.unlock ();
}


// Checks whether the IP address of the current client has very recently made a prioritized server call.
bool Sync_Logic::prioritized_ip_address_active ()
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string ip = request->remote_address;
  int time = filter_date_seconds_since_epoch ();
  bool active = false;
  sync_logic_mutex.lock ();
  bool record_present = config_globals_prioritized_ip_addresses.count (ip);
  if (record_present) {
    time -= config_globals_prioritized_ip_addresses [ip];
    active = (time < 5);
  }
  sync_logic_mutex.unlock ();
  return active;
}


/*

 Notes about possible data encryptiion between server and client.
 
 Make an option to use encryption between the server and the client.
 But is this really needed at this stage?
 The client keeps using the simple http client.
 The user can generate the encryption key on the server.
 This is generated per user.
 When the user generates a key, that key will be available to the client during ten minutes after enabling that option to be available.
 The client can then download that key during the window of ten minutes.
 Once the client has downloaded the key, all future communications will be using that key between server and client.
 Thus it does not use openssl or any other encryption client.
 
 It is easier to enable encryption by default.
 When a client connects to the cloud, the encryption is enabled by default.
 Enable works automatically.
 If the server does not yet have a key for the user, the server generates one.
 Upon connection, the server sends a key to the client.
 That occurs only once.
 All subsequent communications then use that key.
 The initial connection setup works through a standard key set that is hard coded in the software.
 There may be a key that works in both directions.
 This encrypted service uses a new API on the server, so old clients continue to work with the old API.
 
 On the setup page for the cloud connection mention that 128 bits encryption is used.
 No, it's more than that.
 It's double 128 bit encryption.
 
 If a client has no key yet (a newer client) then it uses the old API.
 It also put a message in the logbook calling for a new connection setup, for encryption.
 If a client has a key, it uses the newer API, the encrypted API.
 
 The server may have to browser through the available keys for all users to find which key a certain client is using, because the server does not yet know which user is going to connect.
 
 The client / server should generate a challenge and response, and the client keeps it for the duration of the session, and the server stores it in a state.sqlite database, for the remainder of the session.
 Every night a certain counter related to the key in state.sqlite is increased, starting from 0.
 And when it is 1, then the key is one day old, and can be deleted.
 
 Create mechanism for starting sync:
 When no key is found, it uses the old client and old api.
 When a key is found, it uses the new client and new api.
 Test it well by adding and removing the key.
 
 After some releases, when the server receives a sync request from an unencrypted client, or a client that sends no key yet, then the server tries to determine the username, and then emails the user about it. Perhaps to mail manager or administrator also.
 
 A good approach to security would be to implement it in three phases:
 1. The server accepts an extra POST field with a varying key per session, but does not yet require it.
 2. Clients gets updated over a few months so they supply that key also over time.
 3. The server enforces this new security mechanism.

*/
