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
#include <webserver/request.h>
#include <consistency/logic.h>


void test_omit_verse()
{
  Webserver_Request webserver_request{};
  constexpr const int id = 1;
  Consistency_Logic consistency_logic(webserver_request, id);
  {
    constexpr const auto input = "Genesis 12:7 Yahweh appeared to Abram and said, “I will give this land to your offspring.” He built an altar there to Yahweh, who had appeared to him. 12:7 or, seed";
    EXPECT_EQ(consistency_logic.omit_verse_text(input), "Genesis 12:7");
  }
  {
    constexpr const auto input = "1 Peter 1 1 Verse text";
    EXPECT_EQ(consistency_logic.omit_verse_text(input), "1 Peter 1 1");
  }
  {
    constexpr const auto input = "Gen";
    EXPECT_EQ(consistency_logic.omit_verse_text(input), "Gen");
  }
  {
    constexpr const auto input = "2 Moses";
    EXPECT_EQ(consistency_logic.omit_verse_text(input), "2 Moses");
  }
  {
    constexpr const auto input = "2 2";
    EXPECT_EQ(consistency_logic.omit_verse_text(input), "2 2");
  }
  {
    constexpr const auto input = "1 2 Verse";
    EXPECT_EQ(consistency_logic.omit_verse_text(input), "1 2 Verse");
  }
}

TEST (consistency, logic)
{
  test_omit_verse();
}

#endif
