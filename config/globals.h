/*
Copyright (©) 2003-2024 Teus Benschop.

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


extern std::string config_globals_document_root;
extern bool config_globals_unit_testing;
extern bool config_globals_open_installation;
extern bool config_globals_client_prepared;
extern bool config_globals_webserver_running;
extern std::thread * config_globals_http_worker;
extern std::thread * config_globals_https_worker;
extern std::thread * config_globals_timer;
extern bool config_globals_mail_receive_running;
extern bool config_globals_mail_send_running;
extern int config_globals_touch_enabled;
extern int config_globals_timezone_offset_utc;
extern bool config_globals_change_notifications_available;
extern int config_globals_setup_progress;
extern std::string config_globals_setup_message;
extern bool config_globals_data_initialized;
extern bool config_globals_syncing_bibles;
extern bool config_globals_syncing_notes;
extern bool config_globals_syncing_settings;
extern bool config_globals_syncing_changes;
extern bool config_globals_syncing_files;
extern bool config_globals_syncing_resources;
extern std::map <std::string, int> config_globals_prioritized_ip_addresses;
extern bool config_globals_enforce_https_browser;
extern bool config_globals_enforce_https_client;
extern std::default_random_engine config_globals_random_engine;
extern std::uniform_int_distribution <int> config_globals_int_distribution;
extern std::string config_globals_external_url;
extern std::map <std::string, int> config_globals_resource_window_positions;
extern std::string config_globals_pages_to_open;
extern bool config_globals_hide_bible_resources;
extern bool config_globals_running_on_chrome_os;
extern bool config_globals_log_network;
extern std::string config_globals_negotiated_port_number;
extern bool config_globals_has_crashed_while_mailing;
