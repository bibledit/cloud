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

std::string system_logic_bibles_file_name ();
void system_logic_produce_bibles_file (int jobid);
void system_logic_import_bibles_file (std::string tarball);
std::string system_logic_notes_file_name ();
void system_logic_produce_notes_file (int jobid);
void system_logic_import_notes_file (std::string tarball);
std::string system_logic_resources_file_name (std::string resourcename = "");
void system_logic_produce_resources_file (int jobid);
void system_logic_import_resources_file (std::string tarball);
