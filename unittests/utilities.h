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

extern std::string testing_directory;
extern int error_count;
void refresh_sandbox (bool displayjournal, std::vector <std::string> allowed = {});
void error_message (int line, std::string funct, std::string desired, std::string actual);
bool evaluate (int line, std::string func, std::string desired, std::string actual);
void evaluate (int line, std::string func, int desired, int actual);
void evaluate (int line, std::string func, int desired, size_t actual);
void evaluate (int line, std::string func, unsigned int desired, unsigned int actual);
void evaluate (int line, std::string func, bool desired, bool actual);
void evaluate (int line, std::string func, float desired, float actual);
void evaluate (int line, std::string func, std::vector <std::string> desired, std::vector <std::string> actual);
void evaluate (int line, std::string func, std::vector <int> desired, std::vector <int> actual);
void evaluate (int line, std::string func, std::vector <bool> desired, std::vector <bool> actual);
void evaluate (int line, std::string func, std::map <int, std::string> desired, std::map <int, std::string> actual);
void evaluate (int line, std::string func, std::map <std::string, int> desired, std::map <std::string, int> actual);
void evaluate (int line, std::string func, std::map <int, int> desired, std::map <int, int> actual);
void evaluate (int line, std::string func, std::map <std::string, std::string> desired, std::map <std::string, std::string> actual);
void evaluate (int line, std::string func, std::vector <std::pair<int, std::string>> desired, std::vector <std::pair<int, std::string>> actual);
void trace_unit_tests (std::string func);
int odf2txt (std::string odf, std::string txt);
