/*
Copyright (©) 2003-2026 Teus Benschop.

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
#include <database/temporal.h>


TEST (database, temporal)
{
  // No value initially.
  std::string value = database::temporal::get_value (1, "key");
  EXPECT_EQ (std::string(), value);
  
  // Store value and retrieve it.
  database::temporal::set_value (2, "key2", "value2");
  value = database::temporal::get_value (2, "key2");
  EXPECT_EQ ("value2", value);
  
  // Another key should retrieve nothing.
  value = database::temporal::get_value (2, "key1");
  EXPECT_EQ (std::string(), value);
}


#endif
