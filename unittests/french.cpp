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


#include <unittests/french.h>
#include <unittests/utilities.h>
#include <checks/french.h>
#include <database/check.h>
#include <filter/string.h>
#include <filter/url.h>


void test_french ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  Database_Check database_check;
  database_check.create ();
  string bible = "unit test";
  string nbsp = no_break_space_utf8_00a0 ();
  
  // Test reporting lacking no-break space at French square brackets and other punctuation.
  {
    database_check.truncateOutput (bible);
    map <int, string> texts;
    texts [1] = "This is «French» text.";
    texts [2] = "This is « French » text.";
    texts [3] = "This is «" + nbsp + "French" + nbsp + "» text.";
    texts [4] = "This is it ;";
    texts [5] = "This is it;";
    texts [6] = "This is it" + nbsp + ";";
    Checks_French::spaceBeforeAfterPunctuation (bible, 2, 3, texts);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 6, hits.size ());
    if (hits.size () == 6) {
      string standard;
      standard = "« - Should be followed by a no-break space in French";
      evaluate (__LINE__, __func__, standard, hits [0].data);
      standard = "» - Should be preceded by a no-break space in French";
      evaluate (__LINE__, __func__, standard, hits [1].data);
      standard = "« - Should be followed by a no-break space rather than a plain space in French";
      evaluate (__LINE__, __func__, standard, hits [2].data);
      standard = "» - Should be preceded by a no-break space rather than a plain space in French";
      evaluate (__LINE__, __func__, standard, hits [3].data);
      standard = "; - Should be preceded by a no-break space rather than a plain space in French";
      evaluate (__LINE__, __func__, standard, hits [4].data);
      standard = "; - Should be preceded by a no-break space in French";
      evaluate (__LINE__, __func__, standard, hits [5].data);
    }
  }
}
