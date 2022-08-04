/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/kjv.h>
#include <unittests/utilities.h>
#include <database/kjv.h>


void test_database_kjv ()
{
  trace_unit_tests (__func__);
  
  Database_Kjv database_kjv = Database_Kjv ();
  
  {
    vector <Database_Kjv_Item> data = database_kjv.getVerse (43, 11, 35);
    evaluate (__LINE__, __func__, 6, static_cast<int>(data.size()));
    
    evaluate (__LINE__, __func__, "G3588", data[0].strong);
    evaluate (__LINE__, __func__, "Jesus", data[0].english);
    
    // There's a slash (/) between "Jesus" and "Jesus", to separate the words, so they are not joined.
    
    evaluate (__LINE__, __func__, "G2424", data[2].strong);
    evaluate (__LINE__, __func__, "Jesus", data[2].english);
    
    evaluate (__LINE__, __func__, "G1145", data[4].strong);
    evaluate (__LINE__, __func__, "wept",  data[4].english);
  }
  
  {
    // Testing space between the end of the canonical text and a note following it.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (1, 1, 5);
    evaluate (__LINE__, __func__, 23, static_cast<int>(data.size()));
    evaluate (__LINE__, __func__, " [And the evening…: Heb. And the evening was, and the morning was etc.]", data[22].english);
  }
  
  {
    // Testing proper parsing of the <q> element in Luke 13.2.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (42, 13, 2);
    evaluate (__LINE__, __func__, 40, static_cast<int>(data.size()));
    evaluate (__LINE__, __func__, "Suppose ye", data[12].english);
  }
  
  {
    // Check parsing of <inscription> in Exodus 28.36.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (2, 28, 36);
    evaluate (__LINE__, __func__, 23, static_cast<int>(data.size()));
    evaluate (__LINE__, __func__, "HOLINESS", data[18].english);
  }
  
  {
    // Check parsing of <divineName> in Genesis 2.4.
    vector <Database_Kjv_Item> data = database_kjv.getVerse (1, 2, 4);
    evaluate (__LINE__, __func__, 25, static_cast<int>(data.size()));
    evaluate (__LINE__, __func__, "Lord", data[15].english);
  }
  
  {
    vector <Passage> passages = database_kjv.searchStrong ("G909");
    evaluate (__LINE__, __func__, 4, static_cast <int> (passages.size()));
    
    evaluate (__LINE__, __func__, 41,   passages[0].m_book);
    evaluate (__LINE__, __func__, 7,    passages[0].m_chapter);
    evaluate (__LINE__, __func__, "4",  passages[0].m_verse);
    
    evaluate (__LINE__, __func__, 41,   passages[1].m_book);
    evaluate (__LINE__, __func__, 7,    passages[1].m_chapter);
    evaluate (__LINE__, __func__, "8",  passages[1].m_verse);
    
    evaluate (__LINE__, __func__, 58,   passages[2].m_book);
    evaluate (__LINE__, __func__, 6,    passages[2].m_chapter);
    evaluate (__LINE__, __func__, "2",  passages[2].m_verse);
    
    evaluate (__LINE__, __func__, 58,   passages[3].m_book);
    evaluate (__LINE__, __func__, 9,    passages[3].m_chapter);
    evaluate (__LINE__, __func__, "10", passages[3].m_verse);
  }
}
