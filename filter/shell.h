/*
Copyright (©) 2003-2016 Teus Benschop.

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


#ifndef INCLUDED_FILTER_SHELL_H
#define INCLUDED_FILTER_SHELL_H


#include <config/libraries.h>


int filter_shell_run (string directory, string command, const vector <string> parameters,
                      string * output, string * error);
int filter_shell_run (string command, const char * parameter, string & output);
int filter_shell_run (string command, string & out_err);
bool filter_shell_is_present (string program);
vector <string> filter_shell_active_processes ();
int filter_shell_vfork (string & output, string directory, string command,
                        const char * p01 = NULL,
                        const char * p02 = NULL,
                        const char * p03 = NULL,
                        const char * p04 = NULL,
                        const char * p05 = NULL,
                        const char * p06 = NULL,
                        const char * p07 = NULL,
                        const char * p08 = NULL,
                        const char * p09 = NULL,
                        const char * p10 = NULL,
                        const char * p11 = NULL,
                        const char * p12 = NULL,
                        const char * p13 = NULL);


#endif
