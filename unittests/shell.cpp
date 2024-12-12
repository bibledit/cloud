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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <filter/shell.h>


TEST (filter, shell)
{
  EXPECT_EQ (true, filter::shell::is_present (filter::shell::get_executable(filter::shell::Executable::zip)));
  EXPECT_EQ (false, filter::shell::is_present ("xxxxx"));
  
  {
    std::string output;
    const int result = filter::shell::vfork (output, "", filter::shell::get_executable(filter::shell::Executable::ls), "-l");
    EXPECT_EQ (0, result);
    if (output.find ("unittest") == std::string::npos) {
      EXPECT_EQ ("Supposed to list files", output);
    }
  }

  {
    std::string output;
    const int result = filter::shell::vfork (output, "/", filter::shell::get_executable(filter::shell::Executable::ls), "-l");
    EXPECT_EQ (0, result);
    if (output.find ("tmp") == std::string::npos) {
      EXPECT_EQ ("Supposed to list folder /", output);
    }
  }
}

#endif
