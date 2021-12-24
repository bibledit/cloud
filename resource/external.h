/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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

vector <string> resource_external_names ();
vector <string> resource_external_get_original_language_resources ();
vector <string> resource_external_get_bibles ();
string resource_external_versification (string name);
string resource_external_mapping (string name);
string resource_external_type (string name);
string resource_external_cloud_fetch_cache_extract (string name, int book, int chapter, int verse);
const char * resource_external_net_bible_name ();
const char * resource_external_biblehub_interlinear_name ();
