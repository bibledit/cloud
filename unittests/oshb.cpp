/*
Copyright (©) 2003-2017 Teus Benschop.

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
  
  // Job 3:2.
  vector <string> data = database_oshb.getVerse (18, 3, 2);
  vector <string> standard = {
    "וַיַּ֥עַן"
    ,
    " "
    ,
    "אִיּ֗וֹב"
    ,
    " "
    ,
    "וַיֹּאמַֽר"
    ,
    "׃"
  };
  evaluate (__LINE__, __func__, standard, data);
  
  vector <Passage> passages = database_oshb.searchHebrew ("יָדְע֥וּ");
  evaluate (__LINE__, __func__, 2, (int)passages.size());
  
  evaluate (__LINE__, __func__, 19,   passages[0].book);
  evaluate (__LINE__, __func__, 95,   passages[0].chapter);
  evaluate (__LINE__, __func__, "10", passages[0].verse);
  
  evaluate (__LINE__, __func__, 30,   passages[1].book);
  evaluate (__LINE__, __func__, 3,    passages[1].chapter);
  evaluate (__LINE__, __func__, "10", passages[1].verse);
  
  // Job 3:2.
  vector <int> items = database_oshb.rowids (18, 3, 2);
  evaluate (__LINE__, __func__, 6, (int)items.size());
  
  evaluate (__LINE__, __func__, "c/6030 b", database_oshb.lemma (items[0]));
  evaluate (__LINE__, __func__, "347", database_oshb.lemma (items[2]));
}
