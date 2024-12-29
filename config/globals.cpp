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


#include <config/globals.h>


#undef max


std::string config_globals_document_root {DIRECTORY_SEPARATOR};
bool config_globals_unit_testing {false};
bool config_globals_open_installation {false};
bool config_globals_client_prepared {false};
bool config_globals_webserver_running {true};
std::thread * config_globals_http_worker {nullptr};
std::thread * config_globals_https_worker {nullptr};
std::thread * config_globals_timer {nullptr};
bool config_globals_mail_receive_running {false};
bool config_globals_mail_send_running {false};
int config_globals_touch_enabled {0};
int config_globals_timezone_offset_utc {100};
bool config_globals_change_notifications_available {true};
int config_globals_setup_progress {10};
std::string config_globals_setup_message {"install"};
int config_globals_simultaneous_connection_count {0};
bool config_globals_data_initialized {false};
bool config_globals_syncing_bibles {false};
bool config_globals_syncing_notes {false};
bool config_globals_syncing_settings {false};
bool config_globals_syncing_changes {false};
bool config_globals_syncing_files {false};
bool config_globals_syncing_resources {false};
std::map <std::string, int> config_globals_prioritized_ip_addresses {};
bool config_globals_enforce_https_browser {false};
bool config_globals_enforce_https_client {false};
std::default_random_engine config_globals_random_engine ((std::random_device())());
std::uniform_int_distribution <int> config_globals_int_distribution (0, std::numeric_limits <int>::max());
std::string config_globals_external_url {};
std::map <std::string, int> config_globals_resource_window_positions {};
std::string config_globals_pages_to_open {};
bool config_globals_hide_bible_resources {false};
bool config_globals_running_on_chrome_os {false};
bool config_globals_log_network {false};
std::string config_globals_negotiated_port_number {};
bool config_globals_has_crashed_while_mailing {false};

