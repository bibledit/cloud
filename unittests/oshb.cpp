/*
Copyright (©) 2003-2023 Teus Benschop.

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
#include <database/oshb.h>


TEST (database, oshb)
{
  Database_OsHb database_oshb = Database_OsHb ();
  int size = 0;
  
  // Job 3:2.
  const std::vector <std::string> data = database_oshb.getVerse (18, 3, 2);
  size = 7;
  EXPECT_EQ (size, data.size());
  if (static_cast<int> (data.size ()) == size) {
    EXPECT_EQ ("וַיַּ֥עַן", data [0]);
    EXPECT_EQ (" ", data [1]);
    EXPECT_EQ ("אִיּ֗וֹב", data [2]);
    EXPECT_EQ (" ", data [3]);
    EXPECT_EQ ("וַיֹּאמַֽר", data [4]);
    EXPECT_EQ (" ", data [5]);
    EXPECT_EQ ("׃", data [6]);
  }

  const std::vector <Passage> passages = database_oshb.searchHebrew ("יָדְע֥וּ");
  EXPECT_EQ (2, static_cast <int> (passages.size()));
  
  EXPECT_EQ (19,   passages[0].m_book);
  EXPECT_EQ (95,   passages[0].m_chapter);
  EXPECT_EQ ("10", passages[0].m_verse);
  
  EXPECT_EQ (30,   passages[1].m_book);
  EXPECT_EQ (3,    passages[1].m_chapter);
  EXPECT_EQ ("10", passages[1].m_verse);
  
  // Job 3:2.
  const std::vector <int> items = database_oshb.rowids (18, 3, 2);
  EXPECT_EQ (7, static_cast<int>(items.size()));
  
  EXPECT_EQ ("c/6030 b", database_oshb.lemma (items[0]));
  EXPECT_EQ ("", database_oshb.lemma (items[1]));
  EXPECT_EQ ("347", database_oshb.lemma (items[2]));
  EXPECT_EQ ("", database_oshb.lemma (items[3]));
  EXPECT_EQ ("c/559", database_oshb.lemma (items[4]));
  EXPECT_EQ ("", database_oshb.lemma (items[5]));
  EXPECT_EQ ("", database_oshb.lemma (items[6]));
}


#endif
