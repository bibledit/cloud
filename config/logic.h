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

const char * config_logic_config_folder ();
const char * config_logic_version ();
void config_logic_load_settings ();
string config_logic_http_network_port ();
string config_logic_https_network_port ();
bool config_logic_demo_enabled ();
string config_logic_admin_username ();
string config_logic_admin_password ();
string config_logic_admin_email ();
int my_stoi (const string& str, void * idx = NULL, int base = 10);
bool config_logic_basic_mode (void * webserver_request);
string config_logic_site_url (void * webserver_request);
string config_logic_manual_user_facing_url ();
string config_logic_server_key_path ();
string config_logic_server_certificate_path ();
string config_logic_authorities_certificates_path ();
bool config_logic_enforce_https_browser ();
bool config_logic_enforce_https_client ();
void config_logic_swipe_enabled (void * webserver_request, string & script);
bool config_logic_indonesian_cloud_free ();
bool config_logic_indonesian_cloud_free_simple ();
bool config_logic_indonesian_cloud_free_individual ();
bool config_logic_default_bibledit_configuration ();
