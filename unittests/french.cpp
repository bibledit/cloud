/*
Copyright (©) 2003-2020 Teus Benschop.

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
  string nnbsp = narrow_non_breaking_space_u202F ();
  
  // Test reporting lacking no-break space at French square brackets and other punctuation.
  {
    database_check.truncateOutput (bible);
    map <int, string> texts;
    texts [1] = "This is «French» text.";
    texts [2] = "This is « French » text.";
    texts [3] = "This is «" + nbsp + "French" + nbsp + "» with non-breaking spaces.";
    texts [4] = "This is it ;";
    texts [5] = "This is it;";
    texts [6] = "This is it" + nbsp + ";";
    texts [7] = "This is «" + nnbsp + "French" + nnbsp + "» with narrow non-breaking spaces.";
    Checks_French::spaceBeforeAfterPunctuation (bible, 2, 3, texts);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    int hitcount = 6;
    evaluate (__LINE__, __func__, hitcount, hits.size ());
    if ((int)hits.size () == hitcount) {
      string standard;
      standard = "« - Should be followed by a no-break space in French";
      evaluate (__LINE__, __func__, standard, hits [0].data);
      evaluate (__LINE__, __func__, 1, hits [0].verse);
      standard = "» - Should be preceded by a no-break space in French";
      evaluate (__LINE__, __func__, standard, hits [1].data);
      evaluate (__LINE__, __func__, 1, hits [1].verse);
      standard = "« - Should be followed by a no-break space rather than a plain space in French";
      evaluate (__LINE__, __func__, standard, hits [2].data);
      evaluate (__LINE__, __func__, 2, hits [2].verse);
      standard = "» - Should be preceded by a no-break space rather than a plain space in French";
      evaluate (__LINE__, __func__, standard, hits [3].data);
      evaluate (__LINE__, __func__, 2, hits [3].verse);
      standard = "; - Should be preceded by a no-break space rather than a plain space in French";
      evaluate (__LINE__, __func__, standard, hits [4].data);
      evaluate (__LINE__, __func__, 4, hits [4].verse);
      standard = "; - Should be preceded by a no-break space in French";
      evaluate (__LINE__, __func__, standard, hits [5].data);
      evaluate (__LINE__, __func__, 5, hits [5].verse);
      standard = "» - Should be preceded by a no-break space in French";
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
    int size = 4;
    evaluate (__LINE__, __func__, size, hits.size ());
    if ((int)hits.size () == size) {
      string standard1 = "The previous paragraph contains a citation not closed with a » therefore the current paragraph is expected to start with a « to continue that citation in French";
      string standard2 = "The paragraph contains more right guillements than needed";
      evaluate (__LINE__, __func__, 6, hits [0].verse);
      evaluate (__LINE__, __func__, standard1, hits [0].data);
      evaluate (__LINE__, __func__, 6, hits [1].verse);
      evaluate (__LINE__, __func__, standard2, hits [1].data);
      evaluate (__LINE__, __func__, 8, hits [2].verse);
      evaluate (__LINE__, __func__, standard1, hits [2].data);
      evaluate (__LINE__, __func__, 9, hits [3].verse);
      evaluate (__LINE__, __func__, standard2, hits [3].data);
    }
  }
  
  // Real-life data, fixed, regression test.
  {
    database_check.truncateOutput (bible);
    // In the example following, there is the « in the beginning of the verse 13, it needs to begin a new paragraph, or be considered as an extra «, but the checks don't find it.
    string usfm = R"(
\p
\v 9 Yezu taka kingana yai na bantu yankaka vandaka kudisikisa nde bo kele bantu ya masonga na meso ya Nzambi, kansi bo vandaka kuvweza bantu yankaka yonso :
\v 10 « Bantu zole kwendaka na Tempelo na kusamba. Mosi vandaka mfarizi ; yina yankaka, kalaki ya kitari.
\v 11 Mfarizi telamaka ; yandi sambaka mutindu yai : « E Nzambi, mono ke pesa nge matondo, sambu mono ke fwanana ve na bantu yankaka yonso : bo kele bamiyibi, bantu ya nku, bantu ya bizumba ; mono ke fwanana mpi ve na kalaki yai ya kitari.
\v 12 Konso mposo, mono ke salaka kibansa ya kulala nzala mbala zole ; ibuna bima yonso ya mono ke zwaka, mono ke kabisaka yo na ndambu kumi, sambu na kupesa nge makabu ndambu mosi. »
\v 13 « Kalaki ya kitari telamaka kwa yandi mwa-ntama. Nsoni simbaka yandi na kunangula meso na zulu, kansi yandi vandaka kudibula ntulu na kusonga ntantu. Yandi nde : « E Nzambi na mono, mono nsumuki, wila no mawa ! » Yezu yikaka nde :
\v 14 « Mono ke tubila beno nde : ntangu kalaki ya kitari vutukaka na nzo na yandi, Nzambi talaka yandi na disu ya mbote, kansi yandi talaka ve mfarizi mutindu yina, sambu konso muntu ke kudinangula, Nzambi ta kulumusa yandi ; kansi yina ke kudikulumusa, Nzambi ta nangula yandi. »
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    vector <map <int, string>> verses_paragraphs = filter_text.verses_paragraphs;
    Checks_French::citationStyle (bible, 2, 3, verses_paragraphs);
    vector <Database_Check_Hit> hits = database_check.getHits ();
    int size = 1;
    evaluate (__LINE__, __func__, size, hits.size ());
    if ((int)hits.size () == size) {
      string standard = "The paragraph contains more left guillements than needed";
      evaluate (__LINE__, __func__, 14, hits [0].verse);
      evaluate (__LINE__, __func__, standard, hits [0].data);
    }
  }
}
