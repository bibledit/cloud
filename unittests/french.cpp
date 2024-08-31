/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <checks/french.h>
#include <database/check.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/text.h>


TEST (checks, french)
{
  refresh_sandbox (false);
  database::check::create ();
  std::string bible = "unit test";
  std::string nbsp = filter::strings::non_breaking_space_u00A0 ();
  std::string nnbsp = filter::strings::narrow_non_breaking_space_u202F ();
  
  // Test reporting lacking no-break space at French square brackets and other punctuation.
  {
    database::check::truncate_output (bible);
    std::map <int, std::string> texts;
    texts [1] = "This is «French» text.";
    texts [2] = "This is « French » text.";
    texts [3] = "This is «" + nbsp + "French" + nbsp + "» with non-breaking spaces.";
    texts [4] = "This is it ;";
    texts [5] = "This is it;";
    texts [6] = "This is it" + nbsp + ";";
    texts [7] = "This is «" + nnbsp + "French" + nnbsp + "» with narrow non-breaking spaces.";
    checks_french::space_before_after_punctuation (bible, 2, 3, texts);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    int hitcount = 6;
    EXPECT_EQ (hitcount, hits.size ());
    if (static_cast<int> (hits.size ()) == hitcount) {
      std::string standard;
      standard = "« - Should be followed by a no-break space in French";
      EXPECT_EQ (standard, hits [0].data);
      EXPECT_EQ (1, hits [0].verse);
      standard = "» - Should be preceded by a no-break space in French";
      EXPECT_EQ (standard, hits [1].data);
      EXPECT_EQ (1, hits [1].verse);
      standard = "« - Should be followed by a no-break space rather than a plain space in French";
      EXPECT_EQ (standard, hits [2].data);
      EXPECT_EQ (2, hits [2].verse);
      standard = "» - Should be preceded by a no-break space rather than a plain space in French";
      EXPECT_EQ (standard, hits [3].data);
      EXPECT_EQ (2, hits [3].verse);
      standard = "; - Should be preceded by a no-break space rather than a plain space in French";
      EXPECT_EQ (standard, hits [4].data);
      EXPECT_EQ (4, hits [4].verse);
      standard = "; - Should be preceded by a no-break space in French";
      EXPECT_EQ (standard, hits [5].data);
      EXPECT_EQ (5, hits [5].verse);
      standard = "» - Should be preceded by a no-break space in French";
    }
  }
  
  // Test French citation style.
  {
    database::check::truncate_output (bible);
    std::string usfm = R"(
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
    filter_text.add_usfm_code (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    std::vector <std::map <int, std::string>> verses_paragraphs = filter_text.verses_paragraphs;
    checks_french::citation_style (bible, 2, 3, verses_paragraphs);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    int size = 4;
    EXPECT_EQ (size, hits.size ());
    if (static_cast<int> (hits.size ()) == size) {
      std::string standard1 = "The previous paragraph contains a citation not closed with a » therefore the current paragraph is expected to start with a « to continue that citation in French";
      std::string standard2 = "The paragraph contains more right guillements than needed";
      EXPECT_EQ (6, hits [0].verse);
      EXPECT_EQ (standard1, hits [0].data);
      EXPECT_EQ (6, hits [1].verse);
      EXPECT_EQ (standard2, hits [1].data);
      EXPECT_EQ (8, hits [2].verse);
      EXPECT_EQ (standard1, hits [2].data);
      EXPECT_EQ (9, hits [3].verse);
      EXPECT_EQ (standard2, hits [3].data);
    }
  }
  
  // Real-life data, fixed, regression test.
  {
    database::check::truncate_output (bible);
    // In the example following, there is the « in the beginning of the verse 13, it needs to begin a new paragraph, or be considered as an extra «, but the checks don't find it.
    std::string usfm = R"(
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
    filter_text.add_usfm_code (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    std::vector <std::map <int, std::string>> verses_paragraphs = filter_text.verses_paragraphs;
    checks_french::citation_style (bible, 2, 3, verses_paragraphs);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    int size = 1;
    EXPECT_EQ (size, hits.size ());
    if (static_cast<int> (hits.size ()) == size) {
      std::string standard = "The paragraph contains more left guillements than needed";
      EXPECT_EQ (14, hits [0].verse);
      EXPECT_EQ (standard, hits [0].data);
    }
  }
}

#endif

