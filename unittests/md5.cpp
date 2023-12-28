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
#include <filter/md5.h>
#include <mbedtls/md5.h>


// Test the md5 from the mbedtls library.
TEST (filter, md5)
{
  std::string apple = "apple";
  std::string apple_hexits = "1f3870be274f6c49b3e31a0c6728957f";

  std::string pear = "pear";
  std::string pear_hexits = "8893dc16b1b2534bab7b03727145a2bb";

  EXPECT_EQ (apple_hexits, md5 (apple));
  EXPECT_EQ (pear_hexits, md5 (pear));

  // The md5 digest hash of an empty string.
  EXPECT_EQ ("d41d8cd98f00b204e9800998ecf8427e", md5 (std::string()));
}

#endif

