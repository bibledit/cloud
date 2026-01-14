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

class Webserver_Request;

std::string menu_logic_href (std::string href);
std::string menu_logic_click (std::string item);

std::string menu_logic_create_item (std::string href, std::string text, bool history, std::string title, std::string colour);

std::string menu_logic_translate_menu ();
std::string menu_logic_search_menu ();
std::string menu_logic_tools_menu ();
std::string menu_logic_settings_menu ();
std::string menu_logic_settings_resources_menu ();

std::string menu_logic_main_categories (Webserver_Request& webserver_request, std::string & tooltip);
std::string menu_logic_basic_categories (Webserver_Request& webserver_request);

std::string menu_logic_workspace_category (Webserver_Request& webserver_request, std::string * tooltip = nullptr);
std::string menu_logic_translate_category (Webserver_Request& webserver_request, std::string * tooltip = nullptr);
std::string menu_logic_search_category (Webserver_Request& webserver_request, std::string * tooltip = nullptr);
std::string menu_logic_tools_category (Webserver_Request& webserver_request, std::string * tooltip = nullptr);
std::string menu_logic_settings_category (Webserver_Request& webserver_request, std::string * tooltip = nullptr);
std::string menu_logic_settings_resources_category (Webserver_Request& webserver_request);
std::string menu_logic_help_category (Webserver_Request& webserver_request);

bool menu_logic_public_or_guest (Webserver_Request& webserver_request);

std::string menu_logic_menu_text (std::string menu_item);
std::string menu_logic_menu_url (std::string menu_item);

std::string menu_logic_translate_text ();
std::string menu_logic_search_text ();
std::string menu_logic_tools_text ();
std::string menu_logic_settings_text ();
std::string menu_logic_help_text ();
std::string menu_logic_public_feedback_text ();
std::string menu_logic_logout_text ();
std::string menu_logic_consultation_notes_text ();
std::string menu_logic_bible_manage_text ();
std::string menu_logic_workspace_organize_text ();
std::string menu_logic_checks_settings_text ();
std::string menu_logic_resources_text ();
std::string menu_logic_manage_users_text ();
std::string menu_logic_versification_index_text ();
std::string menu_logic_mapping_index_text ();
std::string menu_logic_styles_indext_text ();
std::string menu_logic_styles_indexm_text ();
std::string menu_logic_changes_text ();
std::string menu_logic_styles_text ();
std::string menu_logic_menu_text ();
std::string menu_logic_images_index_text ();

std::string menu_logic_editor_settings_text (bool visual, int selection);
bool menu_logic_editor_enabled (Webserver_Request& webserver_request, bool visual, bool chapter);
std::string menu_logic_editor_menu_text (bool visual, bool chapter);

bool menu_logic_can_do_tabbed_mode ();
void menu_logic_tabbed_mode_save_json (Webserver_Request& webserver_request);

std::string menu_logic_verse_separator (std::string separator);
