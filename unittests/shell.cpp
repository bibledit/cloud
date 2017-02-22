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


#include <unittests/shell.h>
#include <unittests/utilities.h>
#include <filter/shell.h>


void test_shell ()
{
  trace_unit_tests (__func__);
  
  evaluate (__LINE__, __func__, true, filter_shell_is_present ("zip"));
  evaluate (__LINE__, __func__, false, filter_shell_is_present ("xxxxx"));
  
  string output;
  int result;
  
  result = filter_shell_vfork (output, "", "ls", "-l");
  evaluate (__LINE__, __func__, 0, result);
  if (output.find ("unittest") == string::npos) {
    evaluate (__LINE__, __func__, "Supposed to list files", output);
  }
  
  result = filter_shell_vfork (output, "/", "ls", "-l");
  evaluate (__LINE__, __func__, 0, result);
  if (output.find ("tmp") == string::npos) {
    evaluate (__LINE__, __func__, "Supposed to list folder /", output);
  }
}
