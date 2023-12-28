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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <filter/shell.h>
using namespace std;


TEST (filter, shell)
{
  EXPECT_EQ (true, filter_shell_is_present ("zip"));
  EXPECT_EQ (false, filter_shell_is_present ("xxxxx"));
  
  string output;
  int result;
  
  result = filter_shell_vfork (output, "", "ls", "-l");
  EXPECT_EQ (0, result);
  if (output.find ("unittest") == string::npos) {
    EXPECT_EQ ("Supposed to list files", output);
  }
  
  result = filter_shell_vfork (output, "/", "ls", "-l");
  EXPECT_EQ (0, result);
  if (output.find ("tmp") == string::npos) {
    EXPECT_EQ ("Supposed to list folder /", output);
  }
}

#endif

