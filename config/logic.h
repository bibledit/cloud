/*
Copyright (©) 2003-2022 Teus Benschop.

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

namespace config::logic {

const char * config_folder ();
const char * version ();
void load_settings ();
string http_network_port ();
string https_network_port ();
bool demo_enabled ();
string admin_username ();
string admin_password ();
string admin_email ();
int my_stoi (const string& str, void * idx = nullptr, int base = 10);
bool basic_mode (void * webserver_request);
string site_url (void * webserver_request);
string manual_user_facing_url ();
string server_key_path (const bool force);
string server_certificate_path (const bool force);
string authorities_certificates_path (const bool force);
bool enforce_https_browser ();
bool enforce_https_client ();
void swipe_enabled (void * webserver_request, string & script);
bool indonesian_cloud_free ();
bool indonesian_cloud_free_simple ();
bool indonesian_cloud_free_individual ();
bool default_bibledit_configuration ();
string google_translate_json_key_path ();

} // End of namespace.
