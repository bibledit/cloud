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

std::vector <std::string> resource_external_names ();
std::vector <std::string> resource_external_get_original_language_resources ();
std::vector <std::string> resource_external_get_bibles ();
std::string resource_external_versification (std::string name);
std::string resource_external_mapping (std::string name);
std::string resource_external_type (std::string name);
std::string resource_external_cloud_fetch_cache_extract (const std::string & name, int book, int chapter, int verse);
const char* resource_external_net_bible_name();
const char* resource_external_biblehub_interlinear_name();
const char* resource_external_elberfelder_bibel_name();

