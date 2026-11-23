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

void sword_logic_refresh_module_list();
std::string sword_logic_get_source(std::string line);
std::string sword_logic_get_remote_module(std::string line);
std::string sword_logic_get_installed_module(std::string line);
std::string sword_logic_get_version(std::string line);
std::string sword_logic_get_name(std::string line);
void sword_logic_schedule_module_extraction(const std::string& source, const std::string& module);
std::vector<std::string> sword_logic_get_available();
std::string sword_logic_get_text(const std::string& source, const std::string& module,
                                 int book, int chapter, int verse);
std::string sword_logic_installing_module_text();
void sword_logic_run_scheduled_module_extract(const std::string& source, const std::string& module);
std::string sword_logic_get_resource_name(const std::string& source, const std::string& module);
