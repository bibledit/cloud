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
#include <database/sample.h>
#include <filter/string.h>
using namespace std;


TEST (database, sample)
{
  refresh_sandbox (false);
  Database_Sample::create ();
  
  // Store samples.
  for (int i = 1; i <= 5; i++) {
    int file = (1000 * i);
    int data = (10'000 * i);
    Database_Sample::store (filter::strings::convert_to_string (file), filter::strings::convert_to_string (data));
  }

  // Check amount of stored sample.
  std::vector <int> rowids = Database_Sample::get ();
  EXPECT_EQ ((std::vector <int>{ 1, 2, 3, 4, 5}), rowids);
  
  // Retrieve and check the samples.
  for (int i = 1; i <= 5; i++) {
    std::string standard_file = filter::strings::convert_to_string (1000 * i);
    std::string standard_data = filter::strings::convert_to_string (10'000 * i);
    std::string file, data;
    Database_Sample::get (i, file, data);
    EXPECT_EQ (standard_file, file);
    EXPECT_EQ (standard_data, data);
  }
}

#endif

