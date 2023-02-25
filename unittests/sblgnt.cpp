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


#include <unittests/sblgnt.h>
#include <unittests/utilities.h>
#include <database/sblgnt.h>
using namespace std;


void test_database_sblgnt ()
{
  trace_unit_tests (__func__);
  
  Database_Sblgnt database_sblgnt = Database_Sblgnt ();
  
  vector <string> data = database_sblgnt.getVerse (43, 11, 35);
  evaluate (__LINE__, __func__, { "ἐδάκρυσεν", "ὁ", "Ἰησοῦς" }, data);
  
  vector <Passage> passages = database_sblgnt.searchGreek ("βαπτισμῶν");
  evaluate (__LINE__, __func__, 1,   static_cast <int> (passages.size()));
  evaluate (__LINE__, __func__, 58,  passages[0].m_book);
  evaluate (__LINE__, __func__, 6,   passages[0].m_chapter);
  evaluate (__LINE__, __func__, "2", passages[0].m_verse);
}
