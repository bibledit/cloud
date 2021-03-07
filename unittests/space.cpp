/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <unittests/space.h>
#include <unittests/utilities.h>
#include <checks/space.h>
#include <database/check.h>


void test_space ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  Database_Check database_check;
  database_check.create ();
  string bible = "unit test";
  
  // Test reporting space at end of verse: No spaces here.
  {
    database_check.truncateOutput (bible);
    string usfm =
    "\\v 2 This is verse 2.\n"
    "\\v 3 This is verse 3.\n"
    ;
    Checks_Space::spaceEndVerse (bible, 2, 3, usfm);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, hits.size ());
  }

  // Test reporting space at end of verse: One space here.
  {
    database_check.truncateOutput (bible);
    string usfm =
    "\\v 4 This is verse 4. \n"
    ;
    Checks_Space::spaceEndVerse (bible, 2, 3, usfm);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, hits.size ());
  }

  // Test reporting space at end of verse: One space here.
  // The space is not right at the end in the USFM.
  // The space is followed by empty embedded markup.
  // So in the pure text stream, the space is at the end of a verse.
  // Check that it gets reported.
  {
    database_check.truncateOutput (bible);
    string usfm =
    "\\v 5 This is verse \\add 5. \\add*\n"
    ;
    Checks_Space::spaceEndVerse (bible, 2, 3, usfm);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, hits.size ());
  }
  
  // Test there's no space in the cleaned text at the end of the verse.
  {
    database_check.truncateOutput (bible);
    string usfm =
    "\\v 6 This is verse \\add 6.\\add*\n"
    ;
    Checks_Space::spaceEndVerse (bible, 2, 3, usfm);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 0, hits.size ());
  }
  
  // Check that it catches a double space in USFM.
  {
    database_check.truncateOutput (bible);
    string usfm = R"(\v 1 This contains  a double space.)";
    Checks_Space::doubleSpaceUsfm (bible, 2, 3, 4, usfm);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 1, hits.size ());
  }
  
  // Check that it transposes spaces in notes.
  {
    string usfm = R"(\v 1 Verse\f + \fr 3.1\fk  keyword\ft  Text.\f* one.)";
    string standard = R"(\v 1 Verse\f + \fr 3.1 \fk keyword \ft Text.\f* one.)";
    bool transposed = Checks_Space::transposeNoteSpace (usfm);
    evaluate (__LINE__, __func__, true, transposed);
    evaluate (__LINE__, __func__, standard, usfm);
  }
  {
    string usfm = R"(\v 2 Verse\x + \xo 3.2\xt  Text.\x* two.)";
    string standard = R"(\v 2 Verse\x + \xo 3.2 \xt Text.\x* two.)";
    bool transposed = Checks_Space::transposeNoteSpace (usfm);
    evaluate (__LINE__, __func__, true, transposed);
    evaluate (__LINE__, __func__, standard, usfm);
  }

}
