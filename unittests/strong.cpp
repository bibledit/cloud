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
#include <database/strong.h>


TEST (database, strong)
{
  Database_Strong database;
  
  std::string result = database.definition ("G0");
  EXPECT_EQ ("", result);
  
  result = database.definition ("G1");
  size_t length_h = result.length ();
  
  EXPECT_EQ (true, length_h > 100);
  
  std::vector <std::string> results = database.strong ("χρηστοσ");
  EXPECT_EQ (1, results.size ());
  if (!results.empty ()) {
    EXPECT_EQ ("G5543", results[0]);
  }
}

#endif

