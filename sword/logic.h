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

std::string sword_logic_get_path ();
void sword_logic_refresh_module_list ();
std::string sword_logic_module_list_path ();
std::string sword_logic_get_source (std::string line);
std::string sword_logic_get_remote_module (std::string line);
std::string sword_logic_get_installed_module (std::string line);
std::string sword_logic_get_version (std::string line);
std::string sword_logic_get_name (std::string line);
void sword_logic_install_module_schedule (const std::string& source, const std::string& module);
void sword_logic_install_module (const std::string& source, const std::string& module);
void sword_logic_uninstall_module (const std::string& module);
std::vector <std::string> sword_logic_get_available ();
std::vector <std::string> sword_logic_get_installed ();
std::string sword_logic_get_text (const std::string& source, const std::string& module, const int book, const int chapter, const int verse);
std::map <int, std::string> sword_logic_get_bulk_text (const std::string& module, const int book, const int chapter, const std::vector <int>& verses);
void sword_logic_update_installed_modules ();
void sword_logic_trim_modules ();
std::string sword_logic_installing_module_text ();
std::string sword_logic_fetch_failure_text ();
std::string sword_logic_access_tracker (const std::string & module);
void sword_logic_run_scheduled_module_install (const std::string& source, const std::string& module);
void sword_logic_installmgr_initialize ();
bool sword_logic_installmgr_synchronize_configuration_with_master ();
void sword_logic_installmgr_list_remote_sources (const std::vector <std::string> & sources);
bool sword_logic_installmgr_refresh_remote_source (const std::string& name);
void sword_logic_installmgr_list_remote_modules (const std::string& source_name, std::vector <std::string>& modules);
std::string sword_logic_diatheke (const std::string & module_name, const std::string& osis, int chapter, int verse, bool & available);
void sword_logic_log (std::string message);
std::string sword_logic_clean_verse (const std::string & module, int chapter, int verse, std::string text);
std::string sword_logic_get_resource_name (const std::string & source, const std::string & module);
