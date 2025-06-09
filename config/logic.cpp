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


#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <database/books.h>
#include <database/config/general.h>
#include <database/config/user.h>
#include <cstdlib>
#include <webserver/request.h>
#include <config/globals.h>


namespace config::logic {


const char * config_folder ()
{
  return "config";
}


// Returns the Bibledit version number.
const char * version ()
{
  return VERSION;
}


// Loads the values from the config folder into memory for faster access.
void load_settings ()
{
  // Read the setting whether to log network connections.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "log-network"});
  config_globals_log_network = file_or_dir_exists (path);
}


// Return the network port configured for the server.
std::string http_network_port ()
{
  // If a port number is known already, take that.
  if (!config_globals_negotiated_port_number.empty()) return config_globals_negotiated_port_number;
  // Read the port number from file.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "network-port"});
  config_globals_negotiated_port_number = filter_url_file_get_contents (path);
  // Remove white-space, e.g. a new line, that easily makes its way into the configuration file.
  config_globals_negotiated_port_number = filter::strings::trim (config_globals_negotiated_port_number);
  // Default port number.
  if (config_globals_negotiated_port_number.empty ()) config_globals_negotiated_port_number = "8080";
  // Done.
  return config_globals_negotiated_port_number;
}


// Return the secure network port for the secure server.
std::string https_network_port ()
{
  // Read the port number from file.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "network-port-secure"});
  std::string port = filter_url_file_get_contents (path);
  // Remove white-space, e.g. a new line, that easily makes its way into the configuration file.
  port = filter::strings::trim (port);
  // Default value.
  if (port.empty ()) {
    // The secure port is the plain http port plus one.
    int iport = filter::strings::convert_to_int (config::logic::http_network_port ());
    iport++;
    port = std::to_string (iport);
  }
  return port;
}


// Returns whether demo mode is enabled during configure.
bool demo_enabled ()
{
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "demo"});
  return file_or_dir_exists (path);
}


// The configured admin's username.
std::string admin_username ()
{
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "admin-username"});
  return filter::strings::trim (filter_url_file_get_contents (path));
}


// The configured admin's password.
std::string admin_password ()
{
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "admin-password"});
  return filter::strings::trim (filter_url_file_get_contents (path));
}


// The configured admin's email.
std::string admin_email ()
{
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "admin-email"});
  return filter::strings::trim (filter_url_file_get_contents (path));
}


// Returns whether the interface is supposed to be in basic mode.
bool basic_mode (Webserver_Request& webserver_request)
{
  const bool basic_mode {webserver_request.database_config_user ()->get_basic_interface_mode ()};
  return basic_mode;
}


// This returns the URL of Bibledit Cloud that faces the user.
std::string site_url (const Webserver_Request& webserver_request)
{
  // When the administrator has entered a fixed value for the user-facing URL, take that.
  // It overrides everything.
  std::string url = config::logic::manual_user_facing_url ();
  if (!url.empty ()) return url;
  
  // If a webserver request is passed, take the host from there.
  // The result is that in a situation where 192.168.2.6 is the same as localhost,
  // user can connect from localhost and also from 192.168.2.6.
  // In the past there was a situation that the admin set up a central server for the whole team on his localhost.
  // Then team members that connected to 192.168.2.6 were forwarded to localhost (which of course failed).
  // This solution deals with that.
  if (!webserver_request.host.empty ()) {
    url = get_base_url (webserver_request);
    return url;
  }

  // No URL found yet.
  // This occurs during scheduled tasks that require the URL to add it to emails sent out.
  // Take the URL stored on login.
  url = database::config::general::get_site_url ();
  return url;
}


// This returns the filtered value of file userfacingurl.conf.
std::string manual_user_facing_url ()
{
#ifdef HAVE_CLIENT
  return std::string();
#else
  // Read the configuration file.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "userfacingurl.conf"});
  std::string url = filter_url_file_get_contents (path);
  // Remove white space.
  url = filter::strings::trim (url);
  // The previous file contained dummy text by default. Remove that.
  if (url.length () <= 6) url.clear ();
  // Ensure it ends with a slash.
  if (url.find_last_of ("/") != url.length () - 1) url.append ("/");
  // Done.
  return url;
#endif
}


// Returns the path to the secure server's private key.
std::string server_key_path (const bool force)
{
  // Try the correct config file first.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "privkey.pem"});
  if (force || file_or_dir_exists (path)) return path;
  // Nothing found.
  return std::string();
}


// Returns the path to the secure server's public certificate.
std::string server_certificate_path (const bool force)
{
  // Try the correct config file first.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "cert.pem"});
  if (force || file_or_dir_exists (path)) return path;
  // Nothing found.
  return std::string();
}


// Returns the path to the secure server's certificates of the signing authorities.
// This file contains the chain of trusted certificate authorities that have issued the server certificate.
// At the top of the file will be the intermediate authority that issued the server certificate.
// Next can be more intermediate authorities.
// At the bottom of the file should be the trusted root certificate.
std::string authorities_certificates_path (const bool force)
{
  // Try the correct config file first.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "chain.pem"});
  if (force || file_or_dir_exists (path)) return path;
  // Nothing found.
  return std::string();
}


// Whether to enforce https traffic for browser communications.
bool enforce_https_browser ()
{
  return file_or_dir_exists (filter_url_create_root_path ({config::logic::config_folder (), "browser.https"}));
}


// Whether to enforce https traffic for client communications.
bool enforce_https_client ()
{
  return file_or_dir_exists (filter_url_create_root_path ({config::logic::config_folder (), "client.https"}));
}


void swipe_enabled (Webserver_Request& webserver_request, std::string& script)
{
  bool swipe_operations {false};
  std::string true_false {"false"};
  if (webserver_request.session_logic ()->get_touch_enabled ()) {
    if (webserver_request.database_config_user ()->get_swipe_actions_available ()) {
      true_false = "true";
      swipe_operations = true;
    }
  }
  
  script.append ("\n");
  script.append ("var swipe_operations = ");
  script.append (filter::strings::convert_to_true_false(swipe_operations));
  script.append (";");
}


// Whether the Indonesian Member Cloud is enabled.
bool indonesian_member_cloud ()
{
  // This is to speed up things.
  static bool read {false};
  static bool status {false};
  if (read) return status;
  
  // Read the status from disk and cache it.
  const std::string path = filter_url_create_root_path ({config::logic::config_folder (), "indonesianmembercloud"});
  status = file_or_dir_exists (path);
  read = true;

  // Done.
  return status;
}


// Whether the default Bibledit configuration is enabled.
bool default_bibledit_configuration ()
{
  // If any other configuration is active, then the default Bibledit configuration is not active.
  if (indonesian_member_cloud ()) return false;
  // No other configuration is active: Default Bibledit config is active.
  return true;
}


std::string google_translate_json_key_path ()
{
  return filter_url_create_root_path ({config::logic::config_folder (), "googletranslate.json"});
}


bool create_no_accounts()
{
  return file_or_dir_exists (filter_url_create_root_path ({config::logic::config_folder (), "create-no-accounts"}));
}


} // End of namespace.
