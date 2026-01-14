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
#include <database/sblgnt.h>


TEST (database, sblgnt)
{
  Database_Sblgnt database_sblgnt = Database_Sblgnt ();
  
  const std::vector <std::string> data = database_sblgnt.getVerse (43, 11, 35);
  const std::vector<std::string> standard_data { "ἐδάκρυσεν", "ὁ", "Ἰησοῦς" };
  EXPECT_EQ (standard_data, data);
  
  const std::vector <Passage> passages = database_sblgnt.searchGreek ("βαπτισμῶν");
  EXPECT_EQ (1,   static_cast <int> (passages.size()));
  EXPECT_EQ (58,  passages[0].m_book);
  EXPECT_EQ (6,   passages[0].m_chapter);
  EXPECT_EQ ("2", passages[0].m_verse);
}


#endif
