/*
Copyright (©) 2003-2026 Teus Benschop.

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


#pragma once

#include <config/libraries.h>
#include "config.h"

class Webserver_Request;

namespace config::logic {

const char * config_folder ();
const char * version ();
void load_settings ();
std::string http_network_port ();
std::string https_network_port ();
bool demo_enabled ();
std::string admin_username ();
std::string admin_password ();
std::string admin_email ();
bool basic_mode (Webserver_Request& webserver_request);
std::string site_url (const Webserver_Request& webserver_request);
std::string manual_user_facing_url ();
std::string server_key_path (bool force);
std::string server_certificate_path (bool force);
std::string authorities_certificates_path (bool force);
bool enforce_https_browser ();
bool enforce_https_client ();
void swipe_enabled (Webserver_Request& webserver_request, std::string& script);
bool indonesian_member_cloud ();
bool default_bibledit_configuration ();
std::string google_translate_json_key_path ();
bool create_no_accounts();

// Quality check whether the platform defines are available,
// as the code below relies on it.
#if not (defined(HAVE_CLOUD) or defined(HAVE_WINDOWS) or defined(HAVE_ANDROID) or defined(HAVE_MACOS) or defined(HAVE_LINUX) or defined(HAVE_IOS))
#error "No platform macro defined"
#endif

// Whether file upload works in the browser on the platform.
consteval bool have_file_upload()
{
    // No upload capabilities on Android and iOS, see issue https://github.com/bibledit/cloud/issues/896
#ifdef HAVE_ANDROID
    return false;
#endif
#ifdef HAVE_IOS
    return false;
#endif
    // Enable on all other platforms.
    return true;
}

// The maximum number of simultaneous background tasks.
consteval std::size_t max_parallel_background_tasks()
{
#ifdef HAVE_WINDOWS
    return 5;
#endif
#ifdef HAVE_ANDROID
    return 3;
#endif
#ifdef HAVE_IOS
    return 3;
#endif
    // Value for all other platforms.
    return 10;
}

// Whether to run a secure web server.
consteval bool run_secure_web_server()
{
#ifdef HAVE_CLOUD
    return true;
#endif
    return false;
}

// Whether the system has a bare browser.
consteval bool has_bare_browser()
{
#ifdef HAVE_CLOUD
    return false;
#endif
    return true;
}

// Whether Bibledit works with Paratext.
consteval bool work_with_paratext()
{
#ifdef HAVE_WINDOWS
    return true;
#endif
#ifdef HAVE_LINUX
    return true;
#endif
    return false;
}


} // End of namespace.
