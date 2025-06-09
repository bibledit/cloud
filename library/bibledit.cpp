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


#include <config/libraries.h>
#include <webserver/webserver.h>
#include <library/bibledit.h>
#include <config/globals.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <thread>
#include <timer/index.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/mappings.h>
#include <database/books.h>
#include <setup/index.h>
#include <setup/logic.h>
#include <library/locks.h>
#ifdef HAVE_CLIENT
#else
#include <curl/curl.h>
#endif
#include <sendreceive/logic.h>
#include <ldap/logic.h>
#include <locale/logic.h>
#include <ipc/focus.h>
#include <client/logic.h>


bool bibledit_started {false};


// Get Bibledit's version number.
const char * bibledit_get_version_number ()
{
  return config::logic::version ();
}


// Get the port number that Bibledit's web server listens on.
// If the server already runs, it will give that port number.
// If the server does not yet run, on a client, it will negotiate a free port number, and give that.
const char * bibledit_get_network_port ()
{
  // If the port number has already been set or negotiated, return that port number.
  if (!config_globals_negotiated_port_number.empty()) return config_globals_negotiated_port_number.c_str();

  // On a client device, negotiate a local port number.
#ifdef HAVE_CLIENT
  std::vector <int> ports = { 9876, 9987, 9998 };
  for (auto port : ports) {
    if (!filter_url_port_can_connect ("localhost", port)) {
      config_globals_negotiated_port_number = std::to_string(port);
      break;
    }
  }
#endif

  // Set the port number.
  config::logic::http_network_port ();

  // Give the port number to the caller.
  return config_globals_negotiated_port_number.c_str ();
}


// Initialize library.
// To be called once during the lifetime of the app.
// $package: The folder where the package data resides.
// $webroot: The document root folder for the web server.
void bibledit_initialize_library (const char * package, const char * webroot)
{
  // Must initialize libcurl before any threads are started.
  // Only on the Cloud because it uses libcurl.
  // The client does not use it.
#ifdef HAVE_CLOUD
  curl_global_init (CURL_GLOBAL_ALL);
#endif
  
  // Thread locking.
  thread_setup ();
  
  // Initialize SQLite: Full thread safety: https://www.sqlite.org/c3ref/threadsafe.html.
  // This is supported to prevent "database locked" errors.
  if (!sqlite3_threadsafe ()) {
    std::cerr << "SQLite is not threadsafe" << std::endl;
  }
  sqlite3_config (SQLITE_CONFIG_SERIALIZED);

  // Binary file mode on Windows.
#ifdef HAVE_WINDOWS
  _set_fmode (_O_BINARY);
#endif

  // Set the web root folder.
  config_globals_document_root = webroot;
  
  // Initialize SSL/TLS (after webroot has been set).
  std::thread ssl_tls = std::thread (filter_url_ssl_tls_initialize);
  ssl_tls.detach ();
  
#ifndef HAVE_CLIENT
  // Cloud initializes OpenLDAP server access settings (after webroot has been set).
  ldap_logic_initialize ();
#endif

#ifdef HAVE_CLIENT
  // Set local timezone offset in the library on Windows.
  int hours {0};
#ifdef HAVE_WINDOWS
  TIME_ZONE_INFORMATION tzi;
  [[maybe_unused]] auto dwRet = GetTimeZoneInformation (&tzi);
  hours = 0 - (tzi.Bias / 60);
#else
  // Set local timezone offset in the library on Linux.
  time_t t = time (nullptr);
  struct tm lt = {};
  localtime_r (&t, &lt);
  hours = static_cast<int>(round (lt.tm_gmtoff / 3600));
#endif
  config_globals_timezone_offset_utc = hours;
  Database_Logs::log ("Timezone offset in hours: " + std::to_string(hours));
#endif

  // Initialize obfuscation data.
  locale_logic_obfuscate_initialize ();
  
  // Read some configuration settings into memory for faster access.
  config::logic::load_settings ();
  
  // Initialize data in a thread.
  std::thread setup_thread = std::thread (setup_conditionally, package);
  setup_thread.detach ();
  
  // Multiple start/stop guard.
  bibledit_started = false;
}


// Sets whether the library considers any device that connects to be touch-enabled.
// This is necessary for client devices which are always logged-in.
// The detection of touch-enabled devices happens during login,
// so when the login is skipped, the device is not detected.
// Therefore the calling program can preset touch-enabled here through this library call.
void bibledit_set_touch_enabled (bool enabled)
{
  // Set global variable for use elsewhere in the library.
  // A value of zero does nothing,
  // so set it greater than or smaller than zero to have effect.
  if (enabled) {
    config_globals_touch_enabled = 1;
  }
  else {
    config_globals_touch_enabled = -1;
  }
}


// Start library.
// Can be called multiple times during the lifetime of the app.
void bibledit_start_library ()
{
  // Repeating start guard.
  if (bibledit_started) return;
  bibledit_started = true;

  // Setup server behaviour.
#ifdef HAVE_CLIENT
  config_globals_client_prepared = true;
#else
  config_globals_client_prepared = false;
#endif
  if (config::logic::demo_enabled ()) {
    config_globals_open_installation = true;
  }

  
#ifdef HAVE_CLOUD
  // Indicate that the Cloud has started just now.
  database::config::general::setJustStarted (true);
#endif

  
  // Ignore SIGPIPE signal on Linux: When the browser cancels the request, it won't kill Bibledit.
  // On Windows, this is not needed.
#ifndef HAVE_WINDOWS
  signal (SIGPIPE, SIG_IGN);
#endif
  
  // Set running flag.
  config_globals_webserver_running = true;
  
  // Run the plain web server in a thread.
  config_globals_http_worker = new std::thread (http_server);
  
  // Run the secure web server in a thread.
  config_globals_https_worker = new std::thread (https_server);
  
  // Run the timers in a thread.
  config_globals_timer = new std::thread (timer_index);
  
  // Client should sync right after wake up.
  sendreceive_queue_startup ();
}


// Gets the last page that was opened via the menu.
const char * bibledit_get_last_page ()
{
  static std::string href = database::config::general::get_last_menu_click ();
  return href.c_str();
}


// Returns true if Bibledit is running.
bool bibledit_is_running ()
{
  std::this_thread::sleep_for (std::chrono::milliseconds (10));
  if (config_globals_webserver_running) return true;
  return false;
}


// Returns a non-empty string if the client is synchronizing with or downloading from Bibledit Cloud.
const char * bibledit_is_synchronizing ()
{
  // If any of the sync tasks is running, the function considers bibledit to be synchronizing.
  // On a bad network, it may happen that a task gets stuck.
  // During the time that the task is stuck, till the watchdog kicks in,
  // the sync is considered to be running.
  // When mobile devices use this API call,
  // the devices will remain awake during the time the task is stuck.
  // The user may then have to manually put the device on standby.
  bool syncing = false;
  if (config_globals_syncing_bibles) syncing = true;
  if (config_globals_syncing_changes) syncing = true;
  if (config_globals_syncing_notes) syncing = true;
  if (config_globals_syncing_settings) syncing = true;
  if (config_globals_syncing_files) syncing = true;
  if (config_globals_syncing_resources) syncing = true;
  if (syncing) return "true";
  return "false";
}


// Returns the last external URL the user clicked.
const char * bibledit_get_external_url ()
{
  // The mechanism to return an allocated value for the clicked URL works like this:
  // If there's an URL, it leaves the value untouched, increases a counter, and returns that URL.
  // Next function call it sees the counter incremented, so it clears the URL plus counter.
  // This way the value for the URL remains allocated while it gets returned to the caller.
  // If the URL were clearer during this call, there would only be an empty string to be returned.
  static int counter = 0;
  if (counter) {
    config_globals_external_url.clear ();
    counter = 0;
  }
  if (!config_globals_external_url.empty ()) counter++;
  // Return the URL.
  return config_globals_external_url.c_str ();
}


// Returns the pages the calling app should open.
const char * bibledit_get_pages_to_open ()
{
  config_globals_pages_to_open = database::config::general::get_menu_in_tabbed_view_json ();
  return config_globals_pages_to_open.c_str ();
}


// The normal shutdown procedure works by connecting to the internal webservers,
// and these connections in turn help with shutting down the listening internal webservers.
// In case all the internal webservers no longer are able to accept connections,
// the normal shutdown fails to work.
// This last-ditch function waits a few seconds, and if the app is still running then,
// it exits the app, regardless of the state of the internal webservers.
[[noreturn]]
void bibledit_last_ditch_forced_exit ()
{
  std::this_thread::sleep_for (std::chrono::seconds (2));
  exit (0);
}


// Stop the library.
// Can be called multiple times during the lifetime of the app.
void bibledit_stop_library ()
{
  // Repeating stop guard.
  if (!bibledit_started) return;
  bibledit_started = false;

  // Clear running flag.
  config_globals_webserver_running = false;
  
  std::string url, error;
  
  // Connect to the plain webserver to initiate its shutdown mechanism.
  url = "http://localhost:";
  url.append (config::logic::http_network_port ());
  filter_url_http_get (url, error, false);

#ifdef RUN_SECURE_SERVER
  // If the secure server runs, connect to it to initiate its shutdown mechanism.
  std::string https_port = config::logic::https_network_port ();
  if (https_port.length() > 1) {
    url = "https://localhost:";
    url.append (https_port);
    filter_url_http_get (url, error, false);
    // Let the connection start, then close it.
    // The server will then abort the TLS handshake, and shut down.
  }
#endif

  // Another way of doing the above is to ::raise a signal to each of the listening threads.
  // That signal will unblock the blocking BSD sockets, and so allow the shutdown process to proceed.
  
#ifndef HAVE_ANDROID
#ifndef HAVE_IOS
  // Schedule a timer to exit(0) the program in case the network stack fails to exit the servers.
  // This should not be done on devices like Android and iOS
  // because then the app would quit when the user moves the app to the background,
  // whereas the user expects the app to stay alive in the background.
  new std::thread (bibledit_last_ditch_forced_exit);
#endif
#endif

  // Wait till the servers and the timers shut down.
  config_globals_http_worker->join ();
  config_globals_https_worker->join ();
  config_globals_timer->join ();
  
  // Clear memory.
  delete config_globals_http_worker;
  delete config_globals_https_worker;
  delete config_globals_timer;
}


// Shut the library down.
// To be called exactly once during the lifetime of the app.
void bibledit_shutdown_library ()
{
  // Remove thread locks.
  thread_cleanup ();
  
  // Finalize SSL/TLS.
  filter_url_ssl_tls_finalize ();

  // Multiple start/stop guard.
  bibledit_started = false;
}


// Puts an entry in the journal.
void bibledit_log (const char * message)
{
  Database_Logs::log (message);
}


// The Bibledit outer shell calls this function when it runs on Chrome OS,
// rather than on Android.
// See https://github.com/bibledit/cloud/issues/282
void bibledit_run_on_chrome_os ()
{
  config_globals_running_on_chrome_os = true;
}


// Whether to disable the text selection pop-up that may occur on Chrome OS.
// See https://github.com/bibledit/cloud/issues/282
const char * bibledit_disable_selection_popup_chrome_os ()
{
  if (database::config::general::get_disable_selection_popup_chrome_os ()) {
    return "true";
  }
  return "false";
}


// https://github.com/bibledit/cloud/issues/437
// Accordance expects to receive a standardized verse reference.
// So, for instance, a reference of Psalm 13:3 in the Hebrew Bible
// will instead become the standardized (KJV-like) Psalm 13:2.
const char * bibledit_get_reference_for_accordance ()
{
  // Keep the static reference always in memory as a global reference.
  // The purpose is that the value remains live in memory for the caller,
  // even after the function has returned, and local variables will have been destroyed.
  static std::string reference;
  
  // Wait till all the data has been initialized.
  // If the data is not yet initialized, return an empty reference instead.
  if (!config_globals_data_initialized) return reference.c_str();

  // Get the username on this client device.
  const std::string& user = client_logic_get_username ();

  // Get the active Bible and its versification system.
  Webserver_Request webserver_request;
  webserver_request.session_logic()->set_username(user);
  Database_Config_User database_config_user (webserver_request);
  std::string bible = webserver_request.database_config_user ()->get_bible ();
  std::string versification = database::config::bible::get_versification_system (bible);

  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  int verse = Ipc_Focus::getVerse (webserver_request);

  // Accordance expects a verse reference in the English versification system.
  std::vector <Passage> passages;
  Database_Mappings database_mappings;
  if ((versification != filter::strings::english()) && !versification.empty ()) {
    passages = database_mappings.translate (versification, filter::strings::english (), book, chapter, verse);
  } else {
    passages.push_back (Passage ("", book, chapter, std::to_string(verse)));
  }
  if (passages.empty()) return "";

  // Accordance expects for instance, 2 Corinthians 9:2, to be broadcast as "2CO 9:2".
  book = passages[0].m_book;
  chapter = passages[0].m_chapter;
  std::string verse_s = passages[0].m_verse;
  std::string usfm_id = database::books::get_usfm_from_id (static_cast<book_id>(book));
  reference = usfm_id + " " + std::to_string(chapter) + ":" + filter::strings::convert_to_string (verse_s);

  // Return the reference.
  return reference.c_str ();
}


// https://github.com/bibledit/cloud/issues/437
// Accordance sends a standardized verse reference.
// So, for instance, a reference of Psalm 13:3 in the Hebrew Bible
// will instead become the standardized (KJV-like) Psalm 13:2.
void bibledit_put_reference_from_accordance (const char * reference)
{
  // Get and set the user name on this client device.
  const std::string& user = client_logic_get_username ();
  Webserver_Request webserver_request;
  webserver_request.session_logic()->set_username(user);

  // Setting whether to enable receiving verse references from Accordance.
  bool enabled = webserver_request.database_config_user ()->get_receive_focused_reference_from_accordance ();
  if (!enabled) return;
  
  // Interpret the passage from Accordance, e.g. MAT 1:1.
  // Accordance broadcasts for instance, 2 Corinthians 9:2, as "2CO 9:2".
  std::vector<std::string> book_rest = filter::strings::explode (reference, ' ');
  if (book_rest.size() != 2) return;
  int book = static_cast<int>(database::books::get_id_from_usfm (book_rest[0]));
  std::vector <std::string> chapter_verse = filter::strings::explode(book_rest[1], ':');
  if (chapter_verse.size() != 2) return;
  int chapter = filter::strings::convert_to_int(chapter_verse[0]);
  int verse = filter::strings::convert_to_int(chapter_verse[1]);

  // Get the active Bible and its versification system.
  Database_Config_User database_config_user (webserver_request);
  std::string bible = webserver_request.database_config_user ()->get_bible ();
  std::string versification = database::config::bible::get_versification_system (bible);

  // Accordance expects a verse reference in the English versification system.
  std::vector <Passage> passages;
  Database_Mappings database_mappings;
  if ((versification != filter::strings::english()) && !versification.empty ()) {
    passages = database_mappings.translate (filter::strings::english (), versification, book, chapter, verse);
  } else {
    passages.push_back (Passage ("", book, chapter, std::to_string(verse)));
  }
  if (passages.empty()) return;

  // Set the focused passage in Bibledit.
  book = passages[0].m_book;
  chapter = passages[0].m_chapter;
  std::string verse_s = passages[0].m_verse;
  Ipc_Focus::set (webserver_request, book, chapter, verse);
}
