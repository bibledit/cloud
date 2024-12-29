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


#pragma once

#include <config/libraries.h>
#include <webserver/request.h>

namespace database::config::general {

std::string get_site_mail_name();
void set_site_mail_name (const std::string& value);

std::string get_site_mail_address();
void set_site_mail_address (const std::string& value);

std::string get_mail_storage_host();
void set_mail_storage_host (const std::string& value);

std::string get_mail_storage_username();
void set_mail_storage_username (const std::string& value);

std::string get_mail_storage_password();
void set_mail_storage_password (const std::string& value);

std::string get_mail_storage_protocol();
void set_mail_storage_protocol (const std::string& value);

std::string get_mail_storage_port();
void set_mail_storage_port (const std::string& value);

std::string get_mail_send_host();
void set_mail_send_host (const std::string& value);

std::string get_mail_send_username();
void set_mail_send_username (const std::string& value);

std::string get_mail_send_password();
void set_mail_send_password (const std::string& value);

std::string get_mail_send_port();
void set_mail_send_port (const std::string& value);

std::string get_timer_minute();
void set_timer_minute (const std::string& value);

int get_timezone();
void set_timezone (int value);

std::string get_site_url();
void set_site_url (const std::string& value);

std::string get_site_language ();
void set_site_language (const std::string& value);

bool get_client_mode ();
void set_client_mode (bool value);

std::string get_server_address ();
void set_server_address (const std::string& value);

int get_server_port ();
void set_server_port (int value);

int get_repeat_send_receive ();
void set_repeat_send_receive (int value);

int get_last_send_receive ();
void set_last_send_receive (int value);

std::string get_installed_interface_version ();
void set_installed_interface_version (const std::string& value);

std::string getInstalledDatabaseVersion ();
void setInstalledDatabaseVersion (const std::string& value);

bool getJustStarted ();
void setJustStarted (bool value);

std::string get_paratext_projects_folder ();
void set_paratext_projects_folder (const std::string& value);

std::string get_sync_key ();
void set_sync_key (const std::string& key);

std::string get_last_menu_click ();
void set_last_menu_click (const std::string& url);

std::vector <std::string> get_resources_to_cache ();
void set_resources_to_cache (const std::vector <std::string>& values);

bool getIndexNotes ();
void setIndexNotes (bool value);

bool get_index_bibles ();
void set_index_bibles (bool value);

int get_unsent_bible_data_time ();
void set_unsent_bible_data_time (int value);

int get_unreceived_bible_data_time ();
void set_unreceived_bible_data_time (int value);

bool get_author_in_rss_feed ();
void set_uuthor_in_rss_feed (bool value);

bool get_just_connected_to_cloud ();
void set_just_connected_to_cloud (bool value);

bool get_menu_in_tabbed_view_on ();
void set_menu_in_tabbed_view_on (bool value);
std::string get_menu_in_tabbed_view_json ();
void set_menu_in_tabbed_view_json (const std::string& value);

bool get_disable_selection_popup_chrome_os ();
void set_disable_selection_popup_chrome_os (bool value);

std::string get_notes_verse_separator ();
void set_notes_verse_separator (const std::string& url);

std::vector <std::string> get_comparative_resources ();
void set_comparative_resources (const std::vector <std::string>& values);

std::vector <std::string> get_translated_resources ();
void set_translated_resources (const std::vector <std::string>& values);

std::vector <std::string> get_default_active_resources ();
void set_default_active_resources (const std::vector <std::string>& values);

std::vector <std::string> get_account_creation_times ();
void set_account_creation_times (const std::vector <std::string>& values);

bool get_keep_resources_cache_for_long ();
void set_keep_resources_cache_for_long (bool value);

int get_default_new_user_access_level ();
void set_default_new_user_access_level (int value);

bool get_keep_osis_content_in_sword_resources ();
void set_keep_osis_content_in_sword_resources (bool value);

}
