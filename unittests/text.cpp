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


#include <unittests/text.h>
#include <unittests/utilities.h>
#include <database/config/bible.h>
#include <filter/text.h>
#include <filter/url.h>
#include <filter/string.h>


void test_filter_text () 
{
  trace_unit_tests (__func__);
  
  string TextTestOdt  = "/tmp/TextTest.odt";
  string TextTestHtml = "/tmp/TextTest.html";
  string TextTestTxt  = "/tmp/TextTest.txt";
  
  string bible = "phpunit";
  
  // The unittests depend on known settings.
  Database_Config_Bible::setExportChapterDropCapsFrames (bible, true);
  Database_Config_Bible::setOdtSpaceAfterVerse (bible, " ");

  // Test extraction of all sorts of information from USFM code.
  // Test basic formatting into OpenDocument.
  {
    string usfm = R"(
\id GEN
\h Header
\h1 Header1
\h2 Header2
\h3 Header3
\toc1 The Book of Genesis
\toc2 Genesis
\toc3 Gen
\c 1
\cp Ⅰ
\p
\v 1 This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1.
\c 2
\cp ②
\h Header4
\p
\v 2 This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    // Check that it finds the running headers.
    int desiredRunningHeaders = 5;
    int actualRunningHeaders = (int)filter_text.runningHeaders.size();
    evaluate (__LINE__, __func__, desiredRunningHeaders, actualRunningHeaders);
    if (actualRunningHeaders == desiredRunningHeaders) {
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[0].verse);
      evaluate (__LINE__, __func__, "h", filter_text.runningHeaders[0].marker);
      evaluate (__LINE__, __func__, "Header", filter_text.runningHeaders[0].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[1].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[1].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[1].verse);
      evaluate (__LINE__, __func__, "h1", filter_text.runningHeaders[1].marker);
      evaluate (__LINE__, __func__, "Header1", filter_text.runningHeaders[1].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[2].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[2].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[2].verse);
      evaluate (__LINE__, __func__, "h2", filter_text.runningHeaders[2].marker);
      evaluate (__LINE__, __func__, "Header2", filter_text.runningHeaders[2].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[3].book);
      evaluate (__LINE__, __func__, 0, filter_text.runningHeaders[3].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[3].verse);
      evaluate (__LINE__, __func__, "h3", filter_text.runningHeaders[3].marker);
      evaluate (__LINE__, __func__, "Header3", filter_text.runningHeaders[3].value);
      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[4].book);
      evaluate (__LINE__, __func__, 2, filter_text.runningHeaders[4].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.runningHeaders[4].verse);
      evaluate (__LINE__, __func__, "h", filter_text.runningHeaders[4].marker);
      evaluate (__LINE__, __func__, "Header4", filter_text.runningHeaders[4].value);
    }

    // Check table of contents items.
    int desiredlongTOCs = 1;
    size_t actuallongTOCs = filter_text.longTOCs.size();
    evaluate (__LINE__, __func__, desiredlongTOCs, actuallongTOCs);
    if (desiredlongTOCs == (int)actuallongTOCs) {
      evaluate (__LINE__, __func__, 1, filter_text.longTOCs[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.longTOCs[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.longTOCs[0].verse);
      evaluate (__LINE__, __func__, "toc1", filter_text.longTOCs[0].marker);
      evaluate (__LINE__, __func__, "The Book of Genesis", filter_text.longTOCs[0].value);
    }
    int desiredshortTOCs = 1;
    size_t actualshortTOCs = filter_text.shortTOCs.size();
    evaluate (__LINE__, __func__, desiredshortTOCs, actualshortTOCs);
    if (desiredlongTOCs == (int)actuallongTOCs) {
      evaluate (__LINE__, __func__, 1, filter_text.shortTOCs[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.shortTOCs[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.shortTOCs[0].verse);
      evaluate (__LINE__, __func__, "toc2", filter_text.shortTOCs[0].marker);
      evaluate (__LINE__, __func__, "Genesis", filter_text.shortTOCs[0].value);
    }

    // Check book abbreviation.
    int desiredbookAbbreviations = 1;
    size_t actualbookAbbreviations = filter_text.bookAbbreviations.size();
    evaluate (__LINE__, __func__, desiredbookAbbreviations, actualbookAbbreviations);
    if (desiredbookAbbreviations == (int)actualbookAbbreviations) {
      evaluate (__LINE__, __func__, 1, filter_text.bookAbbreviations[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.bookAbbreviations[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.bookAbbreviations[0].verse);
      evaluate (__LINE__, __func__, "toc3", filter_text.bookAbbreviations[0].marker);
      evaluate (__LINE__, __func__, "Gen", filter_text.bookAbbreviations[0].value);
    }
    
    // Check published chapter markers.
    int desiredpublishedChapterMarkers = 2;
    size_t actualpublishedChapterMarkers = filter_text.publishedChapterMarkers.size();
    evaluate (__LINE__, __func__, desiredpublishedChapterMarkers, actualpublishedChapterMarkers);
    if (desiredpublishedChapterMarkers == (int)actualpublishedChapterMarkers) {
      evaluate (__LINE__, __func__, 1, filter_text.publishedChapterMarkers[0].book);
      evaluate (__LINE__, __func__, 1, filter_text.publishedChapterMarkers[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.publishedChapterMarkers[0].verse);
      evaluate (__LINE__, __func__, "cp", filter_text.publishedChapterMarkers[0].marker);
      evaluate (__LINE__, __func__, "Ⅰ", filter_text.publishedChapterMarkers[0].value);
      evaluate (__LINE__, __func__, 1, filter_text.publishedChapterMarkers[1].book);
      evaluate (__LINE__, __func__, 2, filter_text.publishedChapterMarkers[1].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.publishedChapterMarkers[1].verse);
      evaluate (__LINE__, __func__, "cp", filter_text.publishedChapterMarkers[1].marker);
      evaluate (__LINE__, __func__, "②", filter_text.publishedChapterMarkers[1].value);
    }
    
    // OpenDocument output.
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = R"(
Header4

Header4 Ⅰ

Ⅰ

This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1.

Header4 ②

②

This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2.
    )";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }

  // There are two books here.
  // This normally gives one new page between these two books.
  // Test that basic USFM code gets transformed correctly.
  {
    string usfm =
    "\\id GEN\n"
    "\\ide XYZ\n"
    "\\c 1\n"
    "\\p Text Genesis 1\n"
    "\\c 2\n"
    "\\p Text Genesis 2\n"
    "\\id MAT\n"
    "\\c 1\n"
    "\\p Text Matthew 1\n"
    "\\c 2\n"
    "\\p Text Matthew 2\n"
    "\\rem Comment\n"
    "\\xxx Unknown markup\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "\n"
    "Genesis 1\n"
    "\n"
    "Text Genesis 1\n"
    "\n"
    "Genesis 2\n"
    "\n"
    "Text Genesis 2\n"
    "\n"
    "\n"
    "\n"
    "Matthew\n"
    "\n"
    "Matthew 1\n"
    "\n"
    "Text Matthew 1\n"
    "\n"
    "Matthew 2\n"
    "\n"
    "Text Matthew 2\n"
    "\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
    evaluate (__LINE__, __func__, {"Matthew 2:0 Unknown marker \\xxx, formatting error: Unknown markup"}, filter_text.fallout);
    evaluate (__LINE__, __func__, {"Genesis 0:0 Text encoding: \\ide XYZ", "Matthew 2:0 Comment: \\rem Comment"}, filter_text.info);
  }

  
  // Test multiple books in one OpenDocument file.
  // The headers of each new book should be correct.
  {
    string directory = filter_url_create_root_path ("unittests", "tests");
    string usfm_ruth = filter_url_file_get_contents (filter_url_create_path (directory, "08-Ruth.usfm"));
    string usfm_1_peter = filter_url_file_get_contents (filter_url_create_path (directory, "60-1Peter.usfm"));
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm_ruth);
    filter_text.addUsfmCode (usfm_1_peter);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Ruth\n"
    "\n"
    "The Book of\n"
    "\n"
    "Ruth\n"
    "\n"
    "Ruth 1\n"
    "\n"
    "1 In the days when the judges judged, there was a famine in the land. A certain man of Bethlehem Judah went to live in the country of Moab with his wife and his two sons.\n"
    "\n"
    "\n"
    "\n"
    "1 Peter\n"
    "\n"
    "Peter’s First Letter\n"
    "\n"
    "1 Peter 1\n"
    "\n"
    "1 Peter, an apostle of Jesus Christ, to the chosen ones who are living as foreigners in the Dispersion in Pontus, Galatia, Cappadocia, Asia, and Bithynia,\n"
    "\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test transformation of verse numbers and text following.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1 Verse One.\n"
    "\\p Paragraph One.\n"
    "\\v 2 Verse Two.\n"
    "\\p\n"
    "\\v 3 Verse Three.\n"
    "\\v 4 Verse Four.\n"
    "\\v 5 Verse Five.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "\n"
    "1 Verse One.\n"
    "\n"
    "Paragraph One. 2 Verse Two.\n"
    "\n"
    "3 Verse Three. 4 Verse Four. 5 Verse Five.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test footnotes and cross references.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1 Text 1\\x + \\xt Isa. 1.1.\\x*\\x - \\xt Isa. 2.2.\\x*\\x + \\xt Isa. 3.3.\\x*, text 2\\f + \\fk Word1: \\fl Heb. \\fq Explanation1.\\f*\\f + \\fk Word2: \\fl Heb. \\fq Explanation2.\\f*, text3.\\f + \\fk Test: \\fl Heb. \\fq Note at the very end.\\f*\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "\n"
    "1 Text 1aIsa. 1.1.\n"
    "\n"
    "Isa. 2.2.\n"
    "\n"
    "bIsa. 3.3.\n"
    "\n"
    ", text 21Word1: Heb. Explanation1.\n"
    "\n"
    "2Word2: Heb. Explanation2.\n"
    "\n"
    ", text3.3Test: Heb. Note at the very end.\n"
    "\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }

  // Test footnotes and cross references and their behaviour in new chapters.
  {
    string usfm = R"(
\id GEN
\c 1
\p
\v 1 Xref 1\x + \xt Xref 1.\x*.
\v 2 Xref 2\x + \xt Xref 2.\x*.
\v 3 Xref 3\x + \xt Xref 3.\x*
\v 4 Note 1\f + \ft Note 1.\f*.
\v 5 Note 2\f + \ft Note 2.\f*.
\v 6 Note 3\f + \ft Note 3.\f*.
\c 2
\p
\v 1 Xref 4\x + \xt Xref 4.\x*.
\v 2 Xref 5\x + \xt Xref 5.\x*.
\v 3 Xref 6\x + \xt Xref 6.\x*
\v 4 Note 4\f + \ft Note 4.\f*.
\v 5 Note 5\f + \ft Note 5.\f*.
\v 6 Note 6\f + \ft Note 6.\f*.
    )";
    usfm = filter_string_trim (usfm);
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = R"(
Genesis

Genesis 1

1

Xref 1aXref 1.

. 2 Xref 2bXref 2.

. 3 Xref 3cXref 3.

 4 Note 11Note 1.

. 5 Note 22Note 2.

. 6 Note 33Note 3.

.

Genesis 2

2

Xref 4aXref 4.

. 2 Xref 5bXref 5.

. 3 Xref 6cXref 6.

 4 Note 41Note 4.

. 5 Note 52Note 5.

. 6 Note 63Note 6.

.
    )";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }

  // Test transformation of published verse numbers.
  {
    string usfm = R"(
\id GEN
\c 1
\p
\v 1 \vp ၁။\vp* ကိုယ်တော်သည် တမန်တော် တစ်ဆယ့်နှစ်ပါးတို့ကို အတူတကွခေါ်ပြီးလျှင် နတ်ဆိုးအပေါင်းတို့ကို နှင်ထုတ်နိုင်ရန်နှင့် အနာရောဂါများကို ပျောက်ကင်းစေနိုင်ရန် တန်ခိုးအာဏာတို့ကို သူတို့အား ပေးတော်မူ၏။-
\v 2 \vp ၂။\vp* ထို့နောက် ကိုယ်တော်သည် ဘုရားသခင်၏ နိုင်ငံတော်အကြောင်းကို ဟောပြောရန်နှင့် ဖျားနာသူများကို ကုသ ပျောက်ကင်းစေရန် သူတို့ကို စေလွှတ်တော်မူ၏။-
\v 3 \vp ၃။\vp* ကိုယ်တော်က သင်တို့သွားရမည့်လမ်းခရီးအတွက် မည်သည့်အရာကိုမျှ ယူဆောင်မသွားကြလေနှင့်။ တောင်ဝှေး၊ လွယ်အိတ်၊  စားစရာနှင့် ငွေကြေးတို့ကို သယ်ဆောင်မသွားကြနှင့်။ ဝတ်ရုံကိုလည်း နှစ်ထည်ယူမသွားလေနှင့်။-
\v 4 \vp ၄။\vp* မည်သည့်အိမ်ကိုမဆို သင်တို့ဝင်ကြလျှင် ထိုအိမ်၌နေထိုင်၍ ထိုနေရာမှပင် ပြန်လည်ထွက်ခွာကြလော့။-
    )";
    usfm = filter_string_trim (usfm);
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    string standard = R"(
Genesis

Genesis 1

၁။ ကိုယ်တော်သည် တမန်တော် တစ်ဆယ့်နှစ်ပါးတို့ကို အတူတကွခေါ်ပြီးလျှင် နတ်ဆိုးအပေါင်းတို့ကို နှင်ထုတ်နိုင်ရန်နှင့် အနာရောဂါများကို ပျောက်ကင်းစေနိုင်ရန် တန်ခိုးအာဏာတို့ကို သူတို့အား ပေးတော်မူ၏။- ၂။ ထို့နောက် ကိုယ်တော်သည် ဘုရားသခင်၏ နိုင်ငံတော်အကြောင်းကို ဟောပြောရန်နှင့် ဖျားနာသူများကို ကုသ ပျောက်ကင်းစေရန် သူတို့ကို စေလွှတ်တော်မူ၏။- ၃။ ကိုယ်တော်က သင်တို့သွားရမည့်လမ်းခရီးအတွက် မည်သည့်အရာကိုမျှ ယူဆောင်မသွားကြလေနှင့်။ တောင်ဝှေး၊ လွယ်အိတ်၊  စားစရာနှင့် ငွေကြေးတို့ကို သယ်ဆောင်မသွားကြနှင့်။ ဝတ်ရုံကိုလည်း နှစ်ထည်ယူမသွားလေနှင့်။- ၄။ မည်သည့်အိမ်ကိုမဆို သင်တို့ဝင်ကြလျှင် ထိုအိမ်၌နေထိုင်၍ ထိုနေရာမှပင် ပြန်လည်ထွက်ခွာကြလော့။-
    )";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }

  // Test that the \vp... markup does not introduce an extra space after the \v marker.
  {
    string usfm = R"(
\id MAT
\c 1
\p
\v 1 \vp A\vp* Verse text.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string output = filter_text.text_text->get ();
    string standard = R"(
1
A Verse text.
    )";
    evaluate (__LINE__, __func__, filter_string_trim (standard), output);
  }
  
  // Test clear text export.
  {
    string usfm =
    "\\id GEN\n"
    "\\h Genesis\n"
    "\\toc2 Genesis\n"
    "\\mt2 The book of\n"
    "\\mt Genesis\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 In the be\\x + \\xt Isa. 1.1.\\x*\\x - \\xt Isa. 2.2.\\x*ginning, God created\\f + \\fk Word1: \\fl Heb. \\fq Explanation1.\\f*\\f + \\fk Word2: \\fl Heb. \\fq Explanation2.\\f* the heavens and the earth.\\f + \\fk Test: \\fl Heb. \\fq Note at the very end.\\f*\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string output = filter_text.text_text->get ();
    string notes = filter_text.text_text->getnote ();
    string standard =
    "The book of\n"
    "Genesis\n"
    "1\n"
    "1 In the beginning, God created the heavens and the earth.";
    evaluate (__LINE__, __func__, standard, output);
    string standardnotes =
    "Isa. 1.1.\n"
    "Isa. 2.2.\n"
    "Word1: Heb. Explanation1.\n"
    "Word2: Heb. Explanation2.\n"
    "Test: Heb. Note at the very end.";
    evaluate (__LINE__, __func__, standardnotes, notes);
  }
  
  // Test clear text export.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Chapter 1, verse one.\n"
    "\\v 2 Verse two.\n"
    "\\c 2\n"
    "\\p\n"
    "\\v 1 Chapter 2, verse one.\n"
    "\\v 2 Verse two.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.text_text = new Text_Text ();
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string output = filter_text.text_text->get ();
    string notes = filter_text.text_text->getnote ();
    string standard =
    "1\n"
    "1 Chapter 1, verse one. 2 Verse two.\n"
    "2\n"
    "1 Chapter 2, verse one. 2 Verse two.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (output));
    evaluate (__LINE__, __func__, "", notes);
  }
  
  // Test verses headings.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse one.\n"
    "\\v 2 Verse two.\n"
    "\\s Heading one\n"
    "\\p\n"
    "\\v 3 Verse three\n"
    "\\p\n"
    "\\s Heading two\n"
    "\\p\n"
    "\\v 4 Verse four.\n"
    "\\v 5 Verse five.\n"
    "\\c 2\n"
    "\\s Heading three\n"
    "\\p\n"
    "\\v 1 Verse one.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.verses_headings;
    map <int, string> standard = { {0, "Heading three"}, {2, "Heading one"}, {3, "Heading two"} };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test verses headings.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse one.\n"
    "\\s \\s Usuku lweN\\nd kosi\\nd* luyeza masinyane\n"
    "\\p\n"
    "\\v 2 Verse two\n"
    "\\p\n"
    "\\s Heading \\add two\\add*\n"
    "\\p\n"
    "\\v 3 Verse three\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.verses_headings;
    map <int, string> standard = { {1, "Usuku lweNkosi luyeza masinyane"}, {2, "Heading two"} };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test verses text.
  {
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 2 Verse\\x + \\xt Isa. 1.1.\\x* two.\n"
    "\\v 3 Verse three\\x + \\xt Isa. 1.1.\\x*.\n"
    "\\s Heading one\n"
    "\\p\n"
    "\\v 4 Verse four.\n"
    "\\p\n"
    "\\s Heading two\n"
    "\\p\n"
    "\\v 5 Verse five.\n"
    "\\v 6 Verse six.\n"
    "\\c 2\n"
    "\\s Heading three\n"
    "\\p\n"
    "\\v 1 Verse one\\x + \\xt Isa. 1.1.\\x*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "Verse one."},
      {2, "Verse two."},
      {3, "Verse three."},
      {4, "Verse four."},
      {5, "Verse five."},
      {6, "Verse six."},
    };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test verses text.
  {
    string usfm =
    "\\c 15\n"
    "\\s Heading\n"
    "\\p\n"
    "\\v 1 He said:\n"
    "\\p I will sing to the Lord.\n"
    "\\v 2 The Lord is my strength.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "He said: I will sing to the Lord."},
      {2, "The Lord is my strength." }
    };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test paragraph starting markers.
  {
    string usfm =
    "\\c 1\n"
    "\\s Heading\n"
    "\\p\n"
    "\\v 1 He said:\n"
    "\\q1 I will sing to the Lord.\n"
    "\\v 2 The Lord is my strength.\n"
    "\\q2 I trust in Him.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    evaluate (__LINE__, __func__, {"p", "q1", "q2"}, filter_text.paragraph_starting_markers);
  }
  
  // Test improved paragraph detection.
  {
    string path = filter_url_create_root_path ("unittests", "tests", "ShonaNumbers23.usfm");
    string usfm = filter_url_file_get_contents (path);
    Filter_Text filter_text = Filter_Text ("");
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    for (size_t i = 0; i < filter_text.verses_paragraphs.size (); i++) {
      map <int, string> verses_paragraph = filter_text.verses_paragraphs [i];
      map <int, string> standard;
      if (i == 0) {
        standard = {
          make_pair (1, "Zvino Bharamu wakati kuna Bharaki: Ndivakire pano aritari nomwe, ugondigadzirire pano nzombe nomwe nemakondohwe manomwe."),
          make_pair (2, "Bharaki ndokuita Bharamu sezvakataura; Bharaki naBharamu ndokupa nzombe diki negondohwe paaritari imwe neimwe."),
          make_pair (3, "Zvino Bharamu wakati kuna Bharaki: Mira pachipo chako chinopiswa, asi ini ndichaenda; zvimwe Jehovha achauya kuzosangana neni; zvino chinhu chaanondiratidza ndichakuudza. Zvino wakaenda pakakwirira."),
          make_pair (4, "Zvino Mwari wakasangana naBharamu, iye ndokuti kwaari: Ndagadzira aritari nomwe uye ndapa nzombe diki negondohwe paaritari imwe neimwe. "),
          make_pair (5, "Jehovha ndokuisa shoko mumuromo waBharamu, ndokuti: Dzokera kuna Bharaki ugotaura seizvi."),
          make_pair (6, "Zvino wakadzokera kwaari, uye tarira, amire pachipo chake chinopiswa, iye nemachinda ese aMoabhu."),
          make_pair (7, "Zvino wakasimudza mufananidzo wake ndokuti: Bharaki mambo waMoabhu wakandivinga kubva Siriya, pamakomo ekumabvazuva achiti: Uya nditukire Jakove, uye uya unyombe Israeri."),
          make_pair (8, "Ini ndingatuka sei Mwari waasina kutuka; uye ndinganyomba sei Jehovha waasina kunyomba?"),
          make_pair (9, "Nokuti kubva panhongonya yemabwe ndinomuona, uye kubva pazvikomo ndinomutarisa; tarira, vanhu vachagara vega, uye havangaverengwi pakati pendudzi."),
          make_pair (10, "Ndiani angaverenga guruva raJakove, nekuverenga chechina chaIsraeri? Mweya wangu ngaufe rufu rwevakarurama, uye magumo angu ngaave seake!"),
          make_pair (11, "Zvino Bharaki wakati kuna Bharamu: Wandiitei? Ndakutora kuti utuke vavengi vangu; zvino tarira, wavaropafadza nekuvaropafadza."),
          make_pair (12, "Zvino wakapindura ndokuti: Handifaniri kuchenjerera kutaura izvo Jehovha zvaanoisa mumuromo mangu here?"),
          make_pair (13, "Zvino Bharaki wakati kwaari: Uya hako neni kune imwe nzvimbo paungavaona kubvapo; uchaona kuguma kwavo chete, uye haungavaoni vese; ugonditukira ivo kubva ipapo.")
        };
      }
      if (i == 1) {
        standard = {
          make_pair (14, "Zvino wakamutora akamuisa mumunda weZofimu, panhongonya yePisiga, ndokuvaka aritari nomwe, ndokupa nzombe diki negondohwe paaritari imwe neimwe."),
          make_pair (15, "Zvino wakati kuna Bharaki: Mira pano pachipo chako chinopiswa, zvino ini ndichasangana naJehovha uko."),
          make_pair (16, "Zvino Jehovha wakasangana naBharamu, ndokuisa shoko mumuromo make, ndokuti: Dzokera kuna Bharaki ugotaura seizvi."),
          make_pair (17, "Zvino asvika kwaari, tarira, amire pachipo chake chinopiswa, nemachinda aMoabhu anaye. Bharaki ndokuti kwaari: Jehovha wataurei?"),
          make_pair (18, "Zvino wakasimudza mufananidzo wake ndokuti: Simuka Bharaki, ugonzwa; urereke nzeve kwandiri, iwe mwanakomana waZipori:"),
          make_pair (19, "Mwari haasi munhu kuti areve nhema, kana mwanakomana wemunhu kuti ashanduke. Iye wakareva, haangaiti kanhi? Kana wakareva, haangasimbisi kanhi?"),
          make_pair (20, "Tarira, ndagamuchira kuti ndiropafadze; zvino waropafadza, uye handigoni kuzvidzosa."),
          make_pair (21, "Haana kutarisira chakaipa kuna Jakove; uye haana kuona kutsauka kuna Israeri; Jehovha Mwari wake anaye, nekudanidzira kwamambo kuri pakati pavo."),
          make_pair (22, "Mwari wakavabudisa Egipita, ane sesimba renyati."),
          make_pair (23, "Zvirokwazvo hakuna un'anga hunopikisa Jakove, kana kuvuka kunopikisa Israeri. Ikozvino zvicharehwa pamusoro paJakove nepamusoro paIsraeri kuti Mwari wakazviita."),
          make_pair (24, "Tarirai, vanhu vachasimuka seshumbakadzi, vazvisimudze seshumba. Havangarari pasi kusvika vadya chakajimbiwa, uye vanwa ropa revakaurawa.")
        };
      }
      if (i == 2) {
        standard = {
          make_pair (25, "Zvino Bharaki wakati kuna Bharamu: Zvino usatongovatuka kana kutongovaropafadza."),
          make_pair (26, "Asi Bharamu wakapindura, ndokuti kuna Bharaki: Handina kukuudza here ndichiti: Zvese Jehovha zvaanotaura, ndizvo zvandichaita?")
        };
      }
      if (i == 3) {
        standard = {
          make_pair (27, "Zvino Bharaki wakati kuna Bharamu: Uya hako, ndichakuendesa kune imwe nzvimbo; zvimwe zvingarurama pameso aMwari kuti unditukire ivo kubva ipapo."),
          make_pair (28, "Zvino Bharaki wakatora ndokuisa Bharamu panhongonya yePeori, pakatarisa pasi pachiso cherenje."),
          make_pair (29, "Zvino Bharamu wakati kuna Bharaki: Ndivakire pano aritari nomwe, undigadzirire pano nzombe diki nomwe nemakondohwe manomwe."),
          make_pair (30, "Bharaki ndokuita Bharamu sezvaakataura, ndokupa nzombe diki negondohwe paaritari imwe neimwe.")
        };
      }
      evaluate (__LINE__, __func__, standard, verses_paragraph);
    }
    evaluate (__LINE__, __func__, {"p", "p", "p", "p"}, filter_text.paragraph_starting_markers);
  }
  
  // Test embedded character styles to text output.
  {
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 He said: I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n"
    "\\v 2 The \\nd Lord\\nd* is my strength.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "He said: I will sing to the Lord."},
      {2, "The Lord is my strength."}
    };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Test embedded character styles to html output.
  {
    // Open character style, and embedded character style, and close both normally.
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new Html_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string html = filter_text.html_text_standard->getInnerHtml ();
    string standard =
    "<p class=\"p\">"
    "<span class=\"v\">1</span>"
    "<span> I will sing </span>"
    "<span class=\"add\">to the </span>"
    "<span class=\"add nd\">Lord</span>"
    "<span>.</span>"
    "</p>\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (html));
  }
  
  // Test embedded character styles to html output.
  {
    // Open character style, open embedded character style, close embedded one, then close the outer one.
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd* God\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new Html_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string html = filter_text.html_text_standard->getInnerHtml ();
    string standard =
    "<p class=\"p\">"
    "<span class=\"v\">1</span>"
    "<span> I will sing </span>"
    "<span class=\"add\">to the </span>"
    "<span class=\"add nd\">Lord</span>"
    "<span class=\"add\"> God</span>"
    "<span>.</span>"
    "</p>\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (html));
  }
  
  // Test embedded character styles to html output.
  {
    // Open character style, open embedded character style, then closing the outer one closes the embedded one also.
    string usfm =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new Html_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string html = filter_text.html_text_standard->getInnerHtml ();
    string standard =
    "<p class=\"p\">"
    "<span class=\"v\">1</span>"
    "<span> I will sing </span>"
    "<span class=\"add\">to the </span>"
    "<span class=\"add nd\">Lord</span>"
    "<span>.</span>"
    "</p>\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (html));
  }
  
  // Test embedded character styles to OpenDocument output.
  {
    // Open character style, and embedded character style, and close both normally.
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "\n"
    "Genesis 1\n"
    "\n"
    "1 I will sing to the Lord.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Exercise bits in document to generate text and note citations.
  {
    string usfm =
    "\\id GEN\n"
    "\\v 1 Text 1\\x + \\xt Isa. 1.1.\\x* text\\f + \\fk Word: \\fl Heb. \\fq Explanation1.\\f* text\\fe + \\fk Word: \\fl Heb. \\fq Explanation1.\\fe*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_text_and_note_citations = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_text_and_note_citations->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    string standard = ""
    "Genesis\n"
    "\n"
    "1 Text 1a text1 text1.";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test embedded character styles to OpenDocument output.
  {
    // Open character style, open embedded character style, close embedded one, then close the outer one.
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\+nd* God\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "\n"
    "Genesis 1\n"
    "\n"
    "1 I will sing to the Lord God.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test embedded character styles to OpenDocument output.
  {
    // Open character style, open embedded character style, then closing the outer one closes the embedded one also.
    string usfm =
    "\\id GEN\n"
    "\\c 1\n"
    "\\p\n"
    "\\v 1 I will sing \\add to the \\+nd Lord\\add*.\n";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = ""
    "Genesis\n"
    "\n"
    "Genesis 1\n"
    "\n"
    "1 I will sing to the Lord.\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test the behaviour of a chapter label put in chapter zero.
  {
    // The following USFM has the \cl - chapter label - before the first chapter.
    // It means that the \cl represents the text for "chapter" to be used throughout the book.
    // So it will output:
    // "Chapter 1"
    // ...
    // "Chapter 2"
    // ... and so on.
    string usfm = R"(
\id GEN
\cl Chapter
\c 1
\p
\v 1 I will sing to the LORD.
\c 2
\p
\v 2 Jesus came to save the people.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    
    // Check chapter labels.
    int desiredchapterLabels = 1;
    size_t actualchapterLabels = filter_text.chapterLabels.size();
    evaluate (__LINE__, __func__, desiredchapterLabels, actualchapterLabels);
    if (desiredchapterLabels == (int)actualchapterLabels) {
      evaluate (__LINE__, __func__, 1, filter_text.chapterLabels[0].book);
      evaluate (__LINE__, __func__, 0, filter_text.chapterLabels[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.chapterLabels[0].verse);
      evaluate (__LINE__, __func__, "cl", filter_text.chapterLabels[0].marker);
      evaluate (__LINE__, __func__, "Chapter", filter_text.chapterLabels[0].value);
    }
    
    // OpenDocument output.
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = R"(
Genesis

Genesis 1

Chapter 1

1 I will sing to the LORD.

Genesis 2

Chapter 2

2 Jesus came to save the people.
    )";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test the behaviour of a chapter label put in each separate chapter.
  {
    // The following USFM has the \cl - chapter label - in each chapter.
    // It means that the \cl represents the particular text to be used
    // for the display of the current chapter heading.
    // Usually this is done if numbers are being presented as words, not numerals.
    // So it will output:
    // "Chapter One"
    // ...
    // "Chapter Two"
    // ... and so on.
    string usfm = R"(
\id GEN
\c 1
\cl Chapter One
\p
\v 1 I will sing to the LORD.
\c 2
\cl Chapter Two
\p
\v 2 Jesus came to save the people.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    
    // Check chapter labels.
    int desiredchapterLabels = 2;
    size_t actualchapterLabels = filter_text.chapterLabels.size();
    evaluate (__LINE__, __func__, desiredchapterLabels, actualchapterLabels);
    if (desiredchapterLabels == (int)actualchapterLabels) {
      evaluate (__LINE__, __func__, 1, filter_text.chapterLabels[0].book);
      evaluate (__LINE__, __func__, 1, filter_text.chapterLabels[0].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.chapterLabels[0].verse);
      evaluate (__LINE__, __func__, "cl", filter_text.chapterLabels[0].marker);
      evaluate (__LINE__, __func__, "Chapter One", filter_text.chapterLabels[0].value);
      evaluate (__LINE__, __func__, 1, filter_text.chapterLabels[1].book);
      evaluate (__LINE__, __func__, 2, filter_text.chapterLabels[1].chapter);
      evaluate (__LINE__, __func__, "0", filter_text.chapterLabels[1].verse);
      evaluate (__LINE__, __func__, "cl", filter_text.chapterLabels[1].marker);
      evaluate (__LINE__, __func__, "Chapter Two", filter_text.chapterLabels[1].value);
    }
    
    // OpenDocument output.
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    odt = filter_string_str_replace ("  ", "", odt);
    string standard = R"(
Genesis

Genesis 1

Chapter One

1 I will sing to the LORD.

Genesis 2

Chapter Two

2 Jesus came to save the people.
    )";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }

  // Test footnotes and cross references in plain text.
  {
    string usfm = R"(
\id GEN
\c 1
\p
\v 1 This is verse one\x + Xref 1\x*.
\v 2 This is verse two\f + Note 2\f*.
\v 3 This is verse three\fe + Endnote 3\fe*.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    int n = 3;
    size_t size = filter_text.notes_plain_text.size();
    evaluate (__LINE__, __func__, n, size);
    if ((int)size == n) {
      evaluate (__LINE__, __func__, "1", filter_text.notes_plain_text[0].first);
      evaluate (__LINE__, __func__, "2", filter_text.notes_plain_text[1].first);
      evaluate (__LINE__, __func__, "3", filter_text.notes_plain_text[2].first);
      evaluate (__LINE__, __func__, "Xref 1", filter_text.notes_plain_text[0].second);
      evaluate (__LINE__, __func__, "Note 2", filter_text.notes_plain_text[1].second);
      evaluate (__LINE__, __func__, "Endnote 3", filter_text.notes_plain_text[2].second);
    }
  }

  // Test plain text and notes for export to.
  {
    string usfm = R"(
\id GEN
\c 1
\p
\v 1 This is verse one\x + Xref 1\x*\f + Note 1\f*.
\v 2 This is verse two\f + Note 2\f*\x + Xref 2\x*.
\v 3 This is verse three\fe + Endnote 3\fe*.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    
    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "This is verse one."},
      {2, "This is verse two."},
      {3, "This is verse three."},
    };
    evaluate (__LINE__, __func__, standard, output);
    
    int n = 5;
    size_t size = filter_text.notes_plain_text.size();
    evaluate (__LINE__, __func__, n, size);
    if ((int)size == n) {
      evaluate (__LINE__, __func__, "1", filter_text.notes_plain_text[0].first);
      evaluate (__LINE__, __func__, "1", filter_text.notes_plain_text[1].first);
      evaluate (__LINE__, __func__, "2", filter_text.notes_plain_text[2].first);
      evaluate (__LINE__, __func__, "2", filter_text.notes_plain_text[3].first);
      evaluate (__LINE__, __func__, "3", filter_text.notes_plain_text[4].first);
      evaluate (__LINE__, __func__, "Xref 1", filter_text.notes_plain_text[0].second);
      evaluate (__LINE__, __func__, "Note 1", filter_text.notes_plain_text[1].second);
      evaluate (__LINE__, __func__, "Note 2", filter_text.notes_plain_text[2].second);
      evaluate (__LINE__, __func__, "Xref 2", filter_text.notes_plain_text[3].second);
      evaluate (__LINE__, __func__, "Endnote 3", filter_text.notes_plain_text[4].second);
    }
    
    evaluate (__LINE__, __func__, 3, filter_text.verses_text_note_positions.size());
    evaluate (__LINE__, __func__, {}, filter_text.verses_text_note_positions [0]);
    evaluate (__LINE__, __func__, {17, 17}, filter_text.verses_text_note_positions [1]);
    evaluate (__LINE__, __func__, {17, 17}, filter_text.verses_text_note_positions [2]);
    evaluate (__LINE__, __func__, {19}, filter_text.verses_text_note_positions [3]);
    evaluate (__LINE__, __func__, {}, filter_text.verses_text_note_positions [4]);
  }

  // Test incorrect \vp markup.
  {
    string usfm = R"(
\c 1
\p
\v 1 \vp A Jesus is King.
\v 2 \vp B Jesus is the son of God.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (filter_string_trim(usfm));
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    string standard = R"(
Unknown 1

A Jesus is King.  B Jesus is the son of God.
    )";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  // Test basic export to TBSX.
  {
    string usfm = R"(
\id GEN
\h Genesis
\c 1
\p
\v 1 I will sing to the LORD.
\c 2
\p
\v 2 Jesus came to save the people.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.tbsx_text = new Tbsx_Text ();
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    string output = filter_text.tbsx_text->get_document ();

  }

  // Test invalid UTF8 input text.
  {
    string path = filter_url_create_root_path ("unittests", "tests", "invalid-utf8-2.usfm");
    string invalid_utf8_usfm = filter_url_file_get_contents (path);
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (filter_string_trim(invalid_utf8_usfm));
    filter_text.run (styles_logic_standard_sheet ());
    filter_text.odf_text_standard->save (TextTestOdt);
    int ret = odf2txt (TextTestOdt, TextTestTxt);
    evaluate (__LINE__, __func__, 256, ret);
    string odt = filter_url_file_get_contents (TextTestTxt);
    bool invalid_token = odt.find ("not well-formed (invalid token)") != string::npos;
    evaluate (__LINE__, __func__, true, invalid_token);
  }

  filter_url_unlink (TextTestOdt);
  filter_url_unlink (TextTestHtml);
  filter_url_unlink (TextTestTxt);
  refresh_sandbox (true);
}
