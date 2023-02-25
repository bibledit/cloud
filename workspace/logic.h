/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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

std::vector <std::string> workspace_get_default_names ();
std::map <int, std::string> workspace_get_default_urls (int id);
std::map <int, std::string> workspace_get_default_widths (int id);
std::map <int, std::string> workspace_get_default_heights (int id);
void workspace_create_defaults (void * webserver_request);
std::string workspace_get_active_name (void * webserver_request);
std::string workspace_process_units (std::string length);
void workspace_set_values (void * webserver_request, int selector, const std::map <int, std::string> & values);
void workspace_set_urls (void * webserver_request, const std::map <int, std::string> & values);
void workspace_set_widths (void * webserver_request, const std::map <int, std::string> & values);
void workspace_set_heights (void * webserver_request, const std::map <int, std::string> & values);
void workspace_set_entire_width (void * webserver_request, std::string value);
std::map <int, std::string> workspace_get_values (void * webserver_request, int selector, bool use);
std::map <int, std::string> workspace_get_urls (void * webserver_request, bool use);
std::map <int, std::string> workspace_get_widths (void * webserver_request);
std::map <int, std::string> workspace_get_heights (void * webserver_request);
std::string workspace_get_entire_width (void * webserver_request);
std::vector <std::string> workspace_get_names (void * webserver_request, bool add_default = true);
void workspace_delete (void * webserver_request, std::string workspace);
void workspace_reorder (void * webserver_request, const std::vector <std::string> & workspaces);
void workspace_copy (void * webserver_request, std::string source, std::string destination);
void workspace_cache_for_cloud (void * webserver_request, bool urls, bool widths, bool heights);
std::string workspace_get_default_name ();
void workspace_send (void * webserver_request, std::string workspace, std::string user);
std::map <int, int> workspace_add_bible_editor_number (std::map <int, std::string> & urls);
