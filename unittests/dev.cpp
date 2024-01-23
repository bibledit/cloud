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


#ifdef HAVE_GTEST

#include <config/libraries.h>
#include <unittests/utilities.h>
#include "gtest/gtest.h"


TEST(bibledit, dev)
{
  refresh_sandbox (true);
  constexpr int vms_height {25};
  constexpr int vms_width {200};
  const unsigned short char_spacing {1};
  const unsigned short line_spacing {5};
  const std::string font_path {"UnitTests/Bitmaps/16_23SE1-20-unicode.bdf"};
  EXPECT_EQ (3, 3);
  refresh_sandbox (true);
}

#endif
