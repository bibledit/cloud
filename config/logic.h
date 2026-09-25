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

// ReSharper disable once CppUnusedIncludeDirective
#include <config/libraries.h>
// ReSharper disable once CppUnusedIncludeDirective
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

enum class Platform { cloud, windows, android, macos, linux_, ios };
consteval Platform platform()
{
#ifdef HAVE_WINDOWS
     return Platform::windows;
#endif
#ifdef HAVE_ANDROID
    return Platform::android;
#endif
#ifdef HAVE_MACOS
    return Platform::macos;
#endif
#ifdef HAVE_LINUX
    return Platform::linux_;
#endif
#ifdef HAVE_IOS
    return Platform::ios;
#endif
    return Platform::cloud;
}

// Whether file upload works in the browser on the platform.
consteval bool have_file_upload()
{
    // No upload capabilities on Android and iOS, see issue https://github.com/bibledit/cloud/issues/896
    // ReSharper disable once CppRedundantBooleanExpressionArgument
    if constexpr (platform() == Platform::android or platform() == Platform::ios)
        // ReSharper disable once CppConstexprIfDiscardedBranch
        return false;
    // Enable on all other platforms.
    return true;
}

// The maximum number of simultaneous background tasks.
consteval std::size_t max_parallel_background_tasks()
{
    switch (platform())
    {
    case Platform::windows:
        return 5;
    case Platform::android:
    case Platform::ios:
        return 3;
    case Platform::macos:
    case Platform::linux_:
    case Platform::cloud:
    default:
        return 10;
    }
}

// Whether to run a secure web server.
consteval bool run_secure_web_server()
{
    return platform() == Platform::cloud;
}

// Whether the system has a bare browser.
consteval bool has_bare_browser()
{
    return platform() != Platform::cloud;
}

// Whether Bibledit works with Paratext.
consteval bool work_with_paratext()
{
    // ReSharper disable once CppRedundantBooleanExpressionArgument
    return platform() == Platform::windows or platform() == Platform::linux_;
}

// Whether the journal is much smaller than normal.
consteval bool have_tiny_journal()
{
    // ReSharper disable once CppRedundantBooleanExpressionArgument
    return platform() == Platform::android or platform() == Platform::ios;
}

// Whether the operating system is mature enough to rely on std::filesystem.
consteval bool use_std_filesystem()
{
    switch (platform())
    {
    case Platform::windows:
    case Platform::macos:
        return true;
    case Platform::android:
        // Testing the std::filesystem on Android in August 2024.
        // Results: 5 out of 6 devices tested had crashes in C++.
        // See https://github.com/bibledit/cloud/issues/952 for more info.
    case Platform::ios:
        // The std::filesystem makes the app very slow in the iOS simulator
        // that it appears to be stuck during the setup phase, where it copies files.
        // This was tested in August 2024.
    case Platform::linux_:
        // Older Ubuntu versions (whatever this means in 2024) did not yet have a good std::filesystem support.
    case Platform::cloud:
    default:
        return false;
    }
}

} // End of namespace.
