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


#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <database/books.h>
#include <database/config/general.h>
#include <database/config/user.h>
#include <cstdlib>
#include <webserver/request.h>
#include <config/globals.h>


const char * config_logic_config_folder ()
{
  return "config";
}


// Returns the Bibledit version number.
const char * config_logic_version ()
{
  return VERSION;
}


// Loads the values from the config folder into memory for faster access.
void config_logic_load_settings ()
{
  string path;
  // Read the setting whether to log network connections.
  path = filter_url_create_root_path (config_logic_config_folder (), "log-net");
  config_globals_log_incoming_connections = file_or_dir_exists (path);
}


// Return the network port configured for the server.
string config_logic_http_network_port ()
{
  // If a port number is known already, take that.
  if (!config_globals_negotiated_port_number.empty()) return config_globals_negotiated_port_number;
  // Read the port number from file.
  string path = filter_url_create_root_path (config_logic_config_folder (), "network-port");
  config_globals_negotiated_port_number = filter_url_file_get_contents (path);
  // Remove white-space, e.g. a new line, that easily makes its way into the configuration file.
  config_globals_negotiated_port_number = filter_string_trim (config_globals_negotiated_port_number);
  // Default port number.
  if (config_globals_negotiated_port_number.empty ()) config_globals_negotiated_port_number = "8080";
  // Done.
  return config_globals_negotiated_port_number;
}


// Return the secure network port for the secure server.
string config_logic_https_network_port ()
{
  // Read the port number from file.
  string path = filter_url_create_root_path (config_logic_config_folder (), "network-port-secure");
  string port = filter_url_file_get_contents (path);
  // Remove white-space, e.g. a new line, that easily makes its way into the configuration file.
  port = filter_string_trim (port);
  // Default value.
  if (port.empty ()) {
    // The secure port is the plain http port plus one.
    int iport = convert_to_int (config_logic_http_network_port ());
    iport++;
    port = convert_to_string (iport);
  }
  return port;
}


// Returns whether demo mode is enabled during configure.
bool config_logic_demo_enabled ()
{
  string path = filter_url_create_root_path (config_logic_config_folder (), "demo");
  return file_or_dir_exists (path);
}


// The configured admin's username.
string config_logic_admin_username ()
{
  string path = filter_url_create_root_path (config_logic_config_folder (), "admin-username");
  return filter_string_trim (filter_url_file_get_contents (path));
}


// The configured admin's password.
string config_logic_admin_password ()
{
  string path = filter_url_create_root_path (config_logic_config_folder (), "admin-password");
  return filter_string_trim (filter_url_file_get_contents (path));
}


// The configured admin's email.
string config_logic_admin_email ()
{
  string path = filter_url_create_root_path (config_logic_config_folder (), "admin-email");
  return filter_string_trim (filter_url_file_get_contents (path));
}


// Replacement function for missing "stoi" on platform like Android.
int my_stoi (const string& str, void * idx, int base)
{
#ifdef HAVE_STOI
  size_t * index = (size_t *) idx;
  return stoi (str, index, base);
#else
  char ** endptr = reinterpret_cast <char **> (idx);
  int i = strtol (str.c_str(), endptr, base);
  return i;
#endif
}


// Returns whether the interface is supposed to be in basic mode.
bool config_logic_basic_mode (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  bool basic_mode = request->database_config_user ()->getBasicInterfaceMode ();
  return basic_mode;
}


// This returns the URL of Bibledit Cloud that faces the user.
string config_logic_site_url (void * webserver_request)
{
  // When the administrator has entered a fixed value for the user-facing URL, take that.
  // It overrides everything.
  string url = config_logic_manual_user_facing_url ();
  if (!url.empty ()) return url;
  
  // If a webserver request is passed, take the host from there.
  // The results is that in a situation where 192.168.2.6 is the same as localhost,
  // user can connect from localhost and also from 192.168.2.6.
  // In the past there was a situation that the admin set up a central server for the whole team on his localhost.
  // Then team members that connected to 192.168.2.6 were forwarded to localhost (which of course failed).
  // This solution deals with that.
  if (webserver_request) {
    Webserver_Request * request = (Webserver_Request *) webserver_request;
    if (!request->host.empty ()) {
      url = get_base_url (request);
      return url;
    }
  }
  
  // No URL found yet.
  // This occurs during scheduled tasks that require the URL to add it to emails sent out.
  // Take the URL stored on login.
  url = Database_Config_General::getSiteURL ();
  return url;
}


// This returns the filtered value of file userfacingurl.conf.
string config_logic_manual_user_facing_url ()
{
#ifdef HAVE_CLIENT
  return "";
#else
  // Read the configuration file.
  string path = filter_url_create_root_path (config_logic_config_folder (), "userfacingurl.conf");
  string url = filter_url_file_get_contents (path);
  // Remove white space.
  url = filter_string_trim (url);
  // The previous file contained dummy text by default. Remove that.
  if (url.length () <= 6) url.clear ();
  // Ensure it ends with a slash.
  if (url.find_last_of ("/") != url.length () - 1) url.append ("/");
  // Done.
  return url;
#endif
}


// Returns the path to the secure server's private key.
// See also https://github.com/bibledit/cloud/issues/235
string config_logic_server_key_path ()
{
  // Try the correct config file first.
  string path = filter_url_create_root_path (config_logic_config_folder (), "privkey.pem");
  if (file_or_dir_exists (path)) return path;
  // Try the file for localhost next.
  path = filter_url_create_root_path (config_logic_config_folder (), "local.privkey.pem");
  if (file_or_dir_exists (path)) return path;
  // Nothing found.
  return "";
}


// Returns the path to the secure server's public certificate.
// See also https://github.com/bibledit/cloud/issues/235
string config_logic_server_certificate_path ()
{
  // Try the correct config file first.
  string path = filter_url_create_root_path (config_logic_config_folder (), "cert.pem");
  if (file_or_dir_exists (path)) return path;
  // Try the file for localhost next.
  path = filter_url_create_root_path (config_logic_config_folder (), "local.cert.pem");
  if (file_or_dir_exists (path)) return path;
  // Nothing found.
  return "";
}


// Returns the path to the secure server's certificates of the signing authorities.
// This file contains the chain of trusted certificate authorities that have issued the server certificate.
// At the top of the file will be the intermediate authority that issued the server certificate.
// Next can be more intermediate authorities.
// At the bottom of the file should be the trusted root certificate.
// See also https://github.com/bibledit/cloud/issues/235
string config_logic_authorities_certificates_path ()
{
  // Try the correct config file first.
  string path = filter_url_create_root_path (config_logic_config_folder (), "chain.pem");
  if (file_or_dir_exists (path)) return path;
  // Try the file for localhost next.
  path = filter_url_create_root_path (config_logic_config_folder (), "local.chain.pem");
  if (file_or_dir_exists (path)) return path;
  // Nothing found.
  return "";
}


// Whether to enforce https traffic for browser communications.
bool config_logic_enforce_https_browser ()
{
  return file_or_dir_exists (filter_url_create_root_path (config_logic_config_folder (), "browser.https"));
}


// Whether to enforce https traffic for client communications.
bool config_logic_enforce_https_client ()
{
  return file_or_dir_exists (filter_url_create_root_path (config_logic_config_folder (), "client.https"));
}


void config_logic_swipe_enabled (void * webserver_request, string & script)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string true_false = "false";
  if (request->session_logic ()->touchEnabled ()) {
    if (request->database_config_user ()->getSwipeActionsAvailable ()) {
      true_false = "true";
    }
  }
  
  script.append ("\n");
  script.append ("var swipe_operations = ");
  script.append (true_false);
  script.append (";");
}


// Whether to log incoming IP connections to the journal.
bool config_logic_log_incoming_connections ()
{
  return config_globals_log_incoming_connections;
}
