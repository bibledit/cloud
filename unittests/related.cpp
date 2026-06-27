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
#pragma GCC diagnostic ignored "-Wcharacter-conversion"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <filter/passage.h>
#include <related/logic.h>


// Test the related verses feature.
TEST (related, logic)
{
  {
    // Test situation of one input passage that has several output passages.
    std::vector <Passage> output = related_logic_get_verses ({ Passage ("", 1, 10, "6") });
    int size = 6;
    EXPECT_EQ (size, output.size ());
    if (static_cast<int>(output.size ()) == size) {
      EXPECT_TRUE (Passage ("", 1, 10, "6") == output.at(0));
      EXPECT_TRUE (Passage ("", 1, 10, "7") == output.at(1));
      EXPECT_TRUE (Passage ("", 1, 10, "8") == output.at(2));
      EXPECT_TRUE (Passage ("", 13, 1, "8") == output.at(3));
      EXPECT_TRUE (Passage ("", 13, 1, "9") == output.at(4));
      EXPECT_TRUE (Passage ("", 13, 1, "10") == output.at(5));
    }
  }

  {
    // Test that an input passage that finds no parallel passages in the XML files results in output equal to the input.
    std::vector <Passage> output = related_logic_get_verses ({ Passage ("", 2, 3, "4") });
    EXPECT_EQ (0, output.size ());
  }

  {
    // Test input passages that match two entries in the XML files.
    std::vector <Passage> output = related_logic_get_verses ({ Passage ("", 1, 10, "29"), Passage ("", 1, 11, "12") });
    int size = 12;
    EXPECT_EQ (size, output.size ());
    if (static_cast<int>(output.size ()) == size) {
      EXPECT_TRUE (Passage ("", 1, 10, "26") == output.at(0));
      EXPECT_TRUE (Passage ("", 1, 10, "27") == output.at(1));
      EXPECT_TRUE (Passage ("", 1, 10, "28") == output.at(2));
      EXPECT_TRUE (Passage ("", 1, 10, "29") == output.at(3));
      EXPECT_TRUE (Passage ("", 1, 11, "12") == output.at(4));
      EXPECT_TRUE (Passage ("", 1, 11, "13") == output.at(5));
      EXPECT_TRUE (Passage ("", 1, 11, "14") == output.at(6));
      EXPECT_TRUE (Passage ("", 13, 1, "18") == output.at(7));
      EXPECT_TRUE (Passage ("", 13, 1, "20") == output.at(8));
      EXPECT_TRUE (Passage ("", 13, 1, "21") == output.at(9));
      EXPECT_TRUE (Passage ("", 13, 1, "22") == output.at(10));
      EXPECT_TRUE (Passage ("", 13, 1, "23") == output.at(11));
    }
  }

  {
    // Test third passage of synoptic parallels whether it also gets the first two passages, and the fourth.
    // At the same time it takes data from the quotations XML.
    std::vector <Passage> output = related_logic_get_verses ({ Passage ("", 42, 3, "4") });
    int size = 10;
    EXPECT_EQ (size, output.size ());
    if (static_cast<int>(output.size ()) == size) {
      EXPECT_TRUE (Passage ("", 23, 40, "3") == output.at(0));
      EXPECT_TRUE (Passage ("", 23, 40, "4") == output.at(1));
      EXPECT_TRUE (Passage ("", 23, 40, "5") == output.at(2));
      EXPECT_TRUE (Passage ("", 40, 3, "3") == output.at(3));
      EXPECT_TRUE (Passage ("", 41, 1, "2") == output.at(4));
      EXPECT_TRUE (Passage ("", 41, 1, "3") == output.at(5));
      EXPECT_TRUE (Passage ("", 42, 3, "4") == output.at(6));
      EXPECT_TRUE (Passage ("", 42, 3, "5") == output.at(7));
      EXPECT_TRUE (Passage ("", 42, 3, "6") == output.at(8));
      EXPECT_TRUE (Passage ("", 43, 1, "23") == output.at(9));
    }
  }
}

#endif

