/*
Copyright (©) 2003-2024 Teus Benschop.

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

int filter_shell_run (std::string directory, std::string command, const std::vector <std::string> parameters,
                      std::string * output, std::string * error);
int filter_shell_run (std::string command, const char * parameter, std::string & output);
int filter_shell_run (std::string command, std::string & out_err);
bool filter_shell_is_present (std::string program);
std::vector <std::string> filter_shell_active_processes ();
int filter_shell_vfork (std::string & output, std::string directory, std::string command,
                        const char * p01 = nullptr,
                        const char * p02 = nullptr,
                        const char * p03 = nullptr,
                        const char * p04 = nullptr,
                        const char * p05 = nullptr,
                        const char * p06 = nullptr,
                        const char * p07 = nullptr,
                        const char * p08 = nullptr,
                        const char * p09 = nullptr,
                        const char * p10 = nullptr,
                        const char * p11 = nullptr,
                        const char * p12 = nullptr,
                        const char * p13 = nullptr);
