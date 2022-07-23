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


#include <unittests/oshb.h>
#include <unittests/utilities.h>
#include <database/oshb.h>


void test_database_oshb ()
{
  trace_unit_tests (__func__);
  
  Database_OsHb database_oshb = Database_OsHb ();
  int size = 0;
  
  // Job 3:2.
  vector <string> data = database_oshb.getVerse (18, 3, 2);
  size = 7;
  evaluate (__LINE__, __func__, size, data.size());
  if ((int)data.size () == size) {
    evaluate (__LINE__, __func__, "וַיַּ֥עַן", data [0]);
    evaluate (__LINE__, __func__, " ", data [1]);
    evaluate (__LINE__, __func__, "אִיּ֗וֹב", data [2]);
    evaluate (__LINE__, __func__, " ", data [3]);
    evaluate (__LINE__, __func__, "וַיֹּאמַֽר", data [4]);
    evaluate (__LINE__, __func__, " ", data [5]);
    evaluate (__LINE__, __func__, "׃", data [6]);
  }

  vector <Passage> passages = database_oshb.searchHebrew ("יָדְע֥וּ");
  evaluate (__LINE__, __func__, 2, (int)passages.size());
  
  evaluate (__LINE__, __func__, 19,   passages[0].m_book);
  evaluate (__LINE__, __func__, 95,   passages[0].m_chapter);
  evaluate (__LINE__, __func__, "10", passages[0].m_verse);
  
  evaluate (__LINE__, __func__, 30,   passages[1].m_book);
  evaluate (__LINE__, __func__, 3,    passages[1].m_chapter);
  evaluate (__LINE__, __func__, "10", passages[1].m_verse);
  
  // Job 3:2.
  vector <int> items = database_oshb.rowids (18, 3, 2);
  evaluate (__LINE__, __func__, 7, (int)items.size());
  
  evaluate (__LINE__, __func__, "c/6030 b", database_oshb.lemma (items[0]));
  evaluate (__LINE__, __func__, "", database_oshb.lemma (items[1]));
  evaluate (__LINE__, __func__, "347", database_oshb.lemma (items[2]));
  evaluate (__LINE__, __func__, "", database_oshb.lemma (items[3]));
  evaluate (__LINE__, __func__, "c/559", database_oshb.lemma (items[4]));
  evaluate (__LINE__, __func__, "", database_oshb.lemma (items[5]));
  evaluate (__LINE__, __func__, "", database_oshb.lemma (items[6]));
}
