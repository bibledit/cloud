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
#include <filter/text.h>


void test_french ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  Database_Check database_check;
  database_check.create ();
  string bible = "unit test";
  string nbsp = non_breaking_space_u00A0 ();
  
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
  
  // Test French citation style.
  {
    database_check.truncateOutput (bible);
    string usfm = R"(
\c 1
\p
\v 1 This is a «citation».
\v 2 Verse text.
\v 3 Verse text and « citation.
\p
\v 4 « New paragraph with continued citation.
\p
\v 5 « New paragraph with continued citation.
\p
\v 6 New paragraph without continued citation and with end of citation ».
\p
\v 7 New «citation.
\p
\v 8 Forgot citation opener.
\v 9 End of citation ».
)";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    vector <map <int, string>> verses_paragraphs = filter_text.verses_paragraphs;
    Checks_French::citationStyle (bible, 2, 3, verses_paragraphs);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    evaluate (__LINE__, __func__, 2, hits.size ());
    if (hits.size () == 2) {
      string standard = "The previous paragraph contains a citation not closed with a » therefore the current paragraph is expected to start with a « to continue that citation in French";
      evaluate (__LINE__, __func__, 6, hits [0].verse);
      evaluate (__LINE__, __func__, standard, hits [0].data);
      evaluate (__LINE__, __func__, 8, hits [1].verse);
      evaluate (__LINE__, __func__, standard, hits [1].data);
    }
  }
  
}
