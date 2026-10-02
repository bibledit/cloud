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
#include <config/config.h>

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
std::string server_key_path (const bool force);
std::string server_certificate_path (const bool force);
std::string authorities_certificates_path (const bool force);
bool enforce_https_browser ();
bool enforce_https_client ();
void swipe_enabled (Webserver_Request& webserver_request, std::string& script);
bool indonesian_member_cloud ();
bool default_bibledit_configuration ();
std::string google_translate_json_key_path ();
bool create_no_accounts();

// Whether file upload works in the browser on the platform.
consteval bool have_upload()
{
#if defined(HAVE_ANDROID) or defined(HAVE_IOS)
    // No upload see https://github.com/bibledit/cloud/issues/896
    return false;
#else
    return true;
#endif
}


// Quality check that exactly one platform has been defined.
consteval int count_defined_platforms()
{
    int count {0};
#ifdef HAVE_CLOUD
    count++;
#endif
#ifdef HAVE_WINDOWS
    count++;
#endif
#ifdef HAVE_ANDROID
    count++;
#endif
#ifdef HAVE_MACOS
    count++;
#endif
#ifdef HAVE_LINUX
    count++;
#endif
#ifdef HAVE_IOS
    count++;
#endif
    return count;
}
static_assert(count_defined_platforms() == 1, "Exactly one platform should be defined");

} // End of namespace.
