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


// Todo
#ifndef INCLUDED_UNITTESTS_UTILITIES_H
#define INCLUDED_UNITTESTS_UTILITIES_H


#include <config/libraries.h>


extern string testing_directory;
extern int error_count;
void refresh_sandbox (bool displayjournal, vector <string> allowed = {});
void error_message (int line, string funct, string desired, string actual);
bool evaluate (int line, string func, string desired, string actual);
void evaluate (int line, string func, int desired, int actual);
void evaluate (int line, string func, int desired, size_t actual);
void evaluate (int line, string func, unsigned int desired, unsigned int actual);
void evaluate (int line, string func, bool desired, bool actual);
void evaluate (int line, string func, vector <string> desired, vector <string> actual);
void evaluate (int line, string func, vector <int> desired, vector <int> actual);
void evaluate (int line, string func, vector <bool> desired, vector <bool> actual);
void evaluate (int line, string func, map <int, string> desired, map <int, string> actual);
void evaluate (int line, string func, map <string, int> desired, map <string, int> actual);
void evaluate (int line, string func, map <int, int> desired, map <int, int> actual);
void evaluate (int line, string func, map <string, string> desired, map <string, string> actual);
void evaluate (int line, string func, vector <pair<int, string>> desired, vector <pair<int, string>> actual);
void trace_unit_tests (string func);
int odf2txt (string odf, string txt);


#endif
