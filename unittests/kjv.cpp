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
#include <database/kjv.h>


TEST (database, kjv)
{
  Database_Kjv database_kjv = Database_Kjv ();
  
  {
    std::vector <Database_Kjv_Item> data = database_kjv.getVerse (43, 11, 35);
    EXPECT_EQ (6, static_cast<int>(data.size()));
    
    EXPECT_EQ ("G3588", data[0].strong);
    EXPECT_EQ ("Jesus", data[0].english);
    
    // There's a slash (/) between "Jesus" and "Jesus", to separate the words, so they are not joined.
    
    EXPECT_EQ ("G2424", data[2].strong);
    EXPECT_EQ ("Jesus", data[2].english);
    
    EXPECT_EQ ("G1145", data[4].strong);
    EXPECT_EQ ("wept",  data[4].english);
  }
  
  {
    // Testing space between the end of the canonical text and a note following it.
    std::vector <Database_Kjv_Item> data = database_kjv.getVerse (1, 1, 5);
    EXPECT_EQ (23, static_cast<int>(data.size()));
    EXPECT_EQ (" [And the evening…: Heb. And the evening was, and the morning was etc.]", data[22].english);
  }
  
  {
    // Testing proper parsing of the <q> element in Luke 13.2.
    std::vector <Database_Kjv_Item> data = database_kjv.getVerse (42, 13, 2);
    EXPECT_EQ (40, static_cast<int>(data.size()));
    EXPECT_EQ ("Suppose ye", data[12].english);
  }
  
  {
    // Check parsing of <inscription> in Exodus 28.36.
    std::vector <Database_Kjv_Item> data = database_kjv.getVerse (2, 28, 36);
    EXPECT_EQ (23, static_cast<int>(data.size()));
    EXPECT_EQ ("HOLINESS", data[18].english);
  }
  
  {
    // Check parsing of <divineName> in Genesis 2.4.
    std::vector <Database_Kjv_Item> data = database_kjv.getVerse (1, 2, 4);
    EXPECT_EQ (25, static_cast<int>(data.size()));
    EXPECT_EQ ("Lord", data[15].english);
  }
  
  {
    std::vector <Passage> passages = database_kjv.searchStrong ("G909");
    EXPECT_EQ (4, static_cast <int> (passages.size()));
    
    EXPECT_EQ (41,   passages[0].m_book);
    EXPECT_EQ (7,    passages[0].m_chapter);
    EXPECT_EQ ("4",  passages[0].m_verse);
    
    EXPECT_EQ (41,   passages[1].m_book);
    EXPECT_EQ (7,    passages[1].m_chapter);
    EXPECT_EQ ("8",  passages[1].m_verse);
    
    EXPECT_EQ (58,   passages[2].m_book);
    EXPECT_EQ (6,    passages[2].m_chapter);
    EXPECT_EQ ("2",  passages[2].m_verse);
    
    EXPECT_EQ (58,   passages[3].m_book);
    EXPECT_EQ (9,    passages[3].m_chapter);
    EXPECT_EQ ("10", passages[3].m_verse);
  }
}

#endif
