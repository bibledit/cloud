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

bool client_logic_client_enabled ();
void client_logic_enable_client (bool enable);
std::string client_logic_url (const std::string & address, int port, const std::string & path);
std::string client_logic_connection_setup (std::string user, std::string hash);
std::string client_logic_create_note_encode (const std::string & bible, int book, int chapter, int verse,
                                             const std::string & summary, const std::string & contents, bool raw);
void client_logic_create_note_decode (const std::string & data,
                                      std::string& bible, int& book, int& chapter, int& verse,
                                      std::string& summary, std::string& contents, bool& raw);
std::string client_logic_link_to_cloud (std::string path, std::string linktext);
std::string client_logic_usfm_resources_path ();
void client_logic_usfm_resources_update ();
std::vector <std::string> client_logic_usfm_resources_get ();
std::string client_logic_get_username ();
std::string client_logic_no_cache_resources_path ();
void client_logic_no_cache_resource_add (std::string name);
void client_logic_no_cache_resource_remove (std::string name);
std::vector <std::string> client_logic_no_cache_resources_get ();
