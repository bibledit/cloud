/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <config/globals.h>


#undef max


string config_globals_document_root = DIRECTORY_SEPARATOR;
bool config_globals_unit_testing = false;
bool config_globals_open_installation = false;
bool config_globals_client_prepared = false;
bool config_globals_webserver_running = true;
thread * config_globals_http_worker = NULL;
thread * config_globals_https_worker = NULL;
thread * config_globals_timer = NULL;
bool config_globals_mail_receive_running = false;
bool config_globals_mail_send_running = false;
bool config_globals_quit_at_midnight = false;
int config_globals_touch_enabled = 0;
int config_globals_timezone_offset_utc = 100;
bool config_globals_change_notifications_available = true;
int config_globals_setup_progress = 10;
string config_globals_setup_message = "install";
int config_globals_simultaneous_connection_count = 0;
bool config_globals_data_initialized = false;
bool config_globals_syncing_bibles = false;
bool config_globals_syncing_notes = false;
bool config_globals_syncing_settings = false;
bool config_globals_syncing_changes = false;
bool config_globals_syncing_files = false;
bool config_globals_syncing_resources = false;
map <string, int> config_globals_prioritized_ip_addresses;
bool config_globals_enforce_https_browser = false;
bool config_globals_enforce_https_client = false;
default_random_engine config_globals_random_engine ((random_device())());
uniform_int_distribution <int> config_globals_int_distribution (0, numeric_limits <int>::max());
string config_globals_external_url;
map <string, int> config_globals_resource_window_positions;
