/*
Copyright (©) 2003-2025 Teus Benschop.

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
#include <database/config/bible.h>
#include <database/bibleimages.h>
#include <filter/text.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/diff.h>
#include "usfm.h"

constexpr const char* bible {"unittest"};

constexpr const char* text_odt {"/tmp/test.odt"};
constexpr const char*  text_txt {"/tmp/test.txt"};


class filter_text : public testing::Test {
protected:
  static void SetUpTestSuite() { }
  static void TearDownTestSuite() { }
  void SetUp() override {
    refresh_sandbox (false);
    // The unittests depend on known settings.
    database::config::bible::set_export_chapter_drop_caps_frames (bible, true);
    database::config::bible::set_odt_space_after_verse (bible, " ");
  }
  void TearDown() override {
  }
};


// Test extraction of all sorts of information from USFM code.
// Test basic formatting into OpenDocument.
TEST_F (filter_text, extract)
{
  const std::string usfm = R"(
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
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  // Check that it finds the running headers.
  const int desiredRunningHeaders = 5;
  const int actualRunningHeaders = static_cast<int>(filter_text.runningHeaders.size());
  EXPECT_EQ (desiredRunningHeaders, actualRunningHeaders);
  if (actualRunningHeaders == desiredRunningHeaders) {
    EXPECT_EQ (1, filter_text.runningHeaders[0].m_book);
    EXPECT_EQ (0, filter_text.runningHeaders[0].m_chapter);
    EXPECT_EQ ("0", filter_text.runningHeaders[0].m_verse);
    EXPECT_EQ ("h", filter_text.runningHeaders[0].m_marker);
    EXPECT_EQ ("Header", filter_text.runningHeaders[0].m_value);
    EXPECT_EQ (1, filter_text.runningHeaders[1].m_book);
    EXPECT_EQ (0, filter_text.runningHeaders[1].m_chapter);
    EXPECT_EQ ("0", filter_text.runningHeaders[1].m_verse);
    EXPECT_EQ ("h1", filter_text.runningHeaders[1].m_marker);
    EXPECT_EQ ("Header1", filter_text.runningHeaders[1].m_value);
    EXPECT_EQ (1, filter_text.runningHeaders[2].m_book);
    EXPECT_EQ (0, filter_text.runningHeaders[2].m_chapter);
    EXPECT_EQ ("0", filter_text.runningHeaders[2].m_verse);
    EXPECT_EQ ("h2", filter_text.runningHeaders[2].m_marker);
    EXPECT_EQ ("Header2", filter_text.runningHeaders[2].m_value);
    EXPECT_EQ (1, filter_text.runningHeaders[3].m_book);
    EXPECT_EQ (0, filter_text.runningHeaders[3].m_chapter);
    EXPECT_EQ ("0", filter_text.runningHeaders[3].m_verse);
    EXPECT_EQ ("h3", filter_text.runningHeaders[3].m_marker);
    EXPECT_EQ ("Header3", filter_text.runningHeaders[3].m_value);
    EXPECT_EQ (1, filter_text.runningHeaders[4].m_book);
    EXPECT_EQ (2, filter_text.runningHeaders[4].m_chapter);
    EXPECT_EQ ("0", filter_text.runningHeaders[4].m_verse);
    EXPECT_EQ ("h", filter_text.runningHeaders[4].m_marker);
    EXPECT_EQ ("Header4", filter_text.runningHeaders[4].m_value);
  }
  
  // Check table of contents items.
  const int desiredlongTOCs = 1;
  const size_t actuallongTOCs = filter_text.longTOCs.size();
  EXPECT_EQ (desiredlongTOCs, actuallongTOCs);
  if (desiredlongTOCs == static_cast<int>(actuallongTOCs)) {
    EXPECT_EQ (1, filter_text.longTOCs[0].m_book);
    EXPECT_EQ (0, filter_text.longTOCs[0].m_chapter);
    EXPECT_EQ ("0", filter_text.longTOCs[0].m_verse);
    EXPECT_EQ ("toc1", filter_text.longTOCs[0].m_marker);
    EXPECT_EQ ("The Book of Genesis", filter_text.longTOCs[0].m_value);
  }
  const int desiredshortTOCs = 1;
  const size_t actualshortTOCs = filter_text.shortTOCs.size();
  EXPECT_EQ (desiredshortTOCs, actualshortTOCs);
  if (desiredlongTOCs == static_cast<int>(actuallongTOCs)) {
    EXPECT_EQ (1, filter_text.shortTOCs[0].m_book);
    EXPECT_EQ (0, filter_text.shortTOCs[0].m_chapter);
    EXPECT_EQ ("0", filter_text.shortTOCs[0].m_verse);
    EXPECT_EQ ("toc2", filter_text.shortTOCs[0].m_marker);
    EXPECT_EQ ("Genesis", filter_text.shortTOCs[0].m_value);
  }
  
  // Check book abbreviation.
  const int desiredbookAbbreviations = 1;
  const size_t actualbookAbbreviations = filter_text.bookAbbreviations.size();
  EXPECT_EQ (desiredbookAbbreviations, actualbookAbbreviations);
  if (desiredbookAbbreviations == static_cast<int>(actualbookAbbreviations)) {
    EXPECT_EQ (1, filter_text.bookAbbreviations[0].m_book);
    EXPECT_EQ (0, filter_text.bookAbbreviations[0].m_chapter);
    EXPECT_EQ ("0", filter_text.bookAbbreviations[0].m_verse);
    EXPECT_EQ ("toc3", filter_text.bookAbbreviations[0].m_marker);
    EXPECT_EQ ("Gen", filter_text.bookAbbreviations[0].m_value);
  }
  
  // Check published chapter markers.
  const int desiredpublished_chapter_markers = 2;
  const size_t actualpublished_chapter_markers = filter_text.published_chapter_markers.size();
  EXPECT_EQ (desiredpublished_chapter_markers, actualpublished_chapter_markers);
  if (desiredpublished_chapter_markers == static_cast<int>(actualpublished_chapter_markers)) {
    EXPECT_EQ (1, filter_text.published_chapter_markers[0].m_book);
    EXPECT_EQ (1, filter_text.published_chapter_markers[0].m_chapter);
    EXPECT_EQ ("0", filter_text.published_chapter_markers[0].m_verse);
    EXPECT_EQ ("cp", filter_text.published_chapter_markers[0].m_marker);
    EXPECT_EQ ("Ⅰ", filter_text.published_chapter_markers[0].m_value);
    EXPECT_EQ (1, filter_text.published_chapter_markers[1].m_book);
    EXPECT_EQ (2, filter_text.published_chapter_markers[1].m_chapter);
    EXPECT_EQ ("0", filter_text.published_chapter_markers[1].m_verse);
    EXPECT_EQ ("cp", filter_text.published_chapter_markers[1].m_marker);
    EXPECT_EQ ("②", filter_text.published_chapter_markers[1].m_value);
  }
  
  // OpenDocument output.
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  const std::string standard = R"(
Header4

Header4 Ⅰ

Ⅰ

This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1. This is the text of chapter 1, verse 1.

Header4 ②

②

This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2. This is the text of chapter 2, verse 2.
)";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// There are two books here.
// This normally gives one new page between these two books.
// Test that basic USFM code gets transformed correctly.
TEST_F (filter_text, new_page_between_books)
{
  std::string usfm =
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
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  std::string standard = ""
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
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  EXPECT_EQ ((std::vector<std::string>{"Matthew 2:0 Unknown marker \\xxx, formatting error: Unknown markup"}), filter_text.fallout);
  EXPECT_EQ ((std::vector<std::string>{"Genesis 0:0 Text encoding: \\ide XYZ", "Matthew 2:0 Comment: \\rem Comment"}), filter_text.info);
}


// Test multiple books in one OpenDocument file.
// The headers of each new book should be correct.
TEST_F (filter_text, books_odt_headers)
{
  std::string directory = filter_url_create_root_path ({"unittests", "tests"});
  std::string usfm_ruth = filter_url_file_get_contents (filter_url_create_path ({directory, "08-Ruth.usfm"}));
  std::string usfm_1_peter = filter_url_file_get_contents (filter_url_create_path ({directory, "60-1Peter.usfm"}));
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm_ruth);
  filter_text.add_usfm_code (usfm_1_peter);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  const std::string standard = ""
  "Ruth\n"
  "\n"
  "The Book of\n"
  "\n"
  "Ruth\n"
  "\n"
  "Ruth 1\n"
  "\n"
  "1\n"
  "\n"
  "In the days when the judges judged, there was a famine in the land. A certain man of Bethlehem Judah went to live in the country of Moab with his wife and his two sons.\n"
  "\n"
  "\n"
  "\n"
  "1 Peter\n"
  "\n"
  "Peter’s First Letter\n"
  "\n"
  "1 Peter 1\n"
  "\n"
  "1\n"
  "\n"
  "Peter, an apostle of Jesus Christ, to the chosen ones who are living as foreigners in the Dispersion in Pontus, Galatia, Cappadocia, Asia, and Bithynia,\n"
  "\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test transformation of verse numbers and text following.
TEST_F (filter_text, transform_verse_numbers)
{
  const std::string usfm =
  "\\id GEN\n"
  "\\v 1 Verse One.\n"
  "\\p Paragraph One.\n"
  "\\v 2 Verse Two.\n"
  "\\p\n"
  "\\v 3 Verse Three.\n"
  "\\v 4 Verse Four.\n"
  "\\v 5 Verse Five.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  std::string standard = ""
  "Genesis\n"
  "\n"
  "1 Verse One.\n"
  "\n"
  "Paragraph One. 2 Verse Two.\n"
  "\n"
  "3 Verse Three. 4 Verse Four. 5 Verse Five.\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test footnotes and cross references.
TEST_F (filter_text, footnotes_xrefs_1)
{
  const std::string usfm =
  R"(\id GEN)" "\n"
  R"(\v 1 Text 1\x + \xt Isa. 1.1.\x*\x - \xt Isa. 2.2.\x*\x + \xt Isa. 3.3.\x*, text 2\f + \fk Word1: \fl Heb. \fq Explanation1.\f*\f + \fk Word2: \fl Heb. \fq Explanation2.\f*, text3.\f + \fk Test: \fl Heb. \fq Note at the very end.\f*)" "\n"
  ;
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  const std::string standard = ""
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
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test footnotes and cross references and their behaviour in new chapters.
TEST_F (filter_text, footnotes_xrefs_new_chapters)
{
  std::string usfm = R"(
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
  usfm = filter::strings::trim (usfm);
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  const std::string standard = R"(
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
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test transformation of published verse numbers.
TEST_F (filter_text, transform_published_verse_numbers)
{
  std::string usfm = R"(
\id GEN
\c 1
\p
\v 1 \vp ၁။\vp* ကိုယ်တော်သည် တမန်တော် တစ်ဆယ့်နှစ်ပါးတို့ကို အတူတကွခေါ်ပြီးလျှင် နတ်ဆိုးအပေါင်းတို့ကို နှင်ထုတ်နိုင်ရန်နှင့် အနာရောဂါများကို ပျောက်ကင်းစေနိုင်ရန် တန်ခိုးအာဏာတို့ကို သူတို့အား ပေးတော်မူ၏။-
\v 2 \vp ၂။\vp* ထို့နောက် ကိုယ်တော်သည် ဘုရားသခင်၏ နိုင်ငံတော်အကြောင်းကို ဟောပြောရန်နှင့် ဖျားနာသူများကို ကုသ ပျောက်ကင်းစေရန် သူတို့ကို စေလွှတ်တော်မူ၏။-
\v 3 \vp ၃။\vp* ကိုယ်တော်က သင်တို့သွားရမည့်လမ်းခရီးအတွက် မည်သည့်အရာကိုမျှ ယူဆောင်မသွားကြလေနှင့်။ တောင်ဝှေး၊ လွယ်အိတ်၊  စားစရာနှင့် ငွေကြေးတို့ကို သယ်ဆောင်မသွားကြနှင့်။ ဝတ်ရုံကိုလည်း နှစ်ထည်ယူမသွားလေနှင့်။-
\v 4 \vp ၄။\vp* မည်သည့်အိမ်ကိုမဆို သင်တို့ဝင်ကြလျှင် ထိုအိမ်၌နေထိုင်၍ ထိုနေရာမှပင် ပြန်လည်ထွက်ခွာကြလော့။-
)";
  usfm = filter::strings::trim (usfm);
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (text_txt);
  const std::string standard = R"(
Genesis

Genesis 1

1

ကိုယ်တော်သည် တမန်တော် တစ်ဆယ့်နှစ်ပါးတို့ကို အတူတကွခေါ်ပြီးလျှင် နတ်ဆိုးအပေါင်းတို့ကို နှင်ထုတ်နိုင်ရန်နှင့် အနာရောဂါများကို ပျောက်ကင်းစေနိုင်ရန် တန်ခိုးအာဏာတို့ကို သူတို့အား ပေးတော်မူ၏။- ၂။ ထို့နောက် ကိုယ်တော်သည် ဘုရားသခင်၏ နိုင်ငံတော်အကြောင်းကို ဟောပြောရန်နှင့် ဖျားနာသူများကို ကုသ ပျောက်ကင်းစေရန် သူတို့ကို စေလွှတ်တော်မူ၏။- ၃။ ကိုယ်တော်က သင်တို့သွားရမည့်လမ်းခရီးအတွက် မည်သည့်အရာကိုမျှ ယူဆောင်မသွားကြလေနှင့်။ တောင်ဝှေး၊ လွယ်အိတ်၊  စားစရာနှင့် ငွေကြေးတို့ကို သယ်ဆောင်မသွားကြနှင့်။ ဝတ်ရုံကိုလည်း နှစ်ထည်ယူမသွားလေနှင့်။- ၄။ မည်သည့်အိမ်ကိုမဆို သင်တို့ဝင်ကြလျှင် ထိုအိမ်၌နေထိုင်၍ ထိုနေရာမှပင် ပြန်လည်ထွက်ခွာကြလော့။-
)";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test that the \vp... markup does not introduce an extra space after the \v marker.
TEST_F (filter_text, vp_no_space_after_v)
{
  const std::string usfm = R"(
\id MAT
\c 1
\p
\v 1 \vp A\vp* Verse text.
)";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.text_text = new Text_Text ();
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::string output = filter_text.text_text->get ();
  const std::string standard = R"(
1
A Verse text.
)";
  EXPECT_EQ (filter::strings::trim (standard), output);
}


// Test clear text export.
TEST_F (filter_text, clear_text_export_1)
{
  const std::string usfm =
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
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::string output = filter_text.text_text->get ();
  const std::string notes = filter_text.text_text->getnote ();
  const std::string standard =
  "The book of\n"
  "Genesis\n"
  "1\n"
  "1 In the beginning, God created the heavens and the earth.";
  EXPECT_EQ (standard, output);
  const std::string standardnotes =
  "Isa. 1.1.\n"
  "Isa. 2.2.\n"
  "Word1: Heb. Explanation1.\n"
  "Word2: Heb. Explanation2.\n"
  "Test: Heb. Note at the very end.";
  EXPECT_EQ (standardnotes, notes);
}


// Test clear text export.
TEST_F (filter_text, clear_text_export_2)
{
  const std::string usfm =
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
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::string output = filter_text.text_text->get ();
  const std::string notes = filter_text.text_text->getnote ();
  const std::string standard =
  "1\n"
  "1 Chapter 1, verse one. 2 Verse two.\n"
  "2\n"
  "1 Chapter 2, verse one. 2 Verse two.\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (output));
  EXPECT_EQ (std::string(), notes);
}


// Test verses headings.
TEST_F (filter_text, verse_headings_1)
{
  const std::string usfm =
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
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::map <int, std::string> output = filter_text.verses_headings;
  const std::map <int, std::string> standard = { {0, "Heading three"}, {2, "Heading one"}, {3, "Heading two"} };
  EXPECT_EQ (standard, output);
}


// Test verses headings.
TEST_F (filter_text, verse_headings_2)
{
  const std::string usfm =
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
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::map <int, std::string> output = filter_text.verses_headings;
  const std::map <int, std::string> standard = { {1, "Usuku lweNkosi luyeza masinyane"}, {2, "Heading two"} };
  EXPECT_EQ (standard, output);
}


// Test verses text.
TEST_F (filter_text, verses_text_1)
{
  const std::string usfm =
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
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::map <int, std::string> output = filter_text.getVersesText ();
  const std::map <int, std::string> standard = {
    {1, "Verse one."},
    {2, "Verse two."},
    {3, "Verse three."},
    {4, "Verse four."},
    {5, "Verse five."},
    {6, "Verse six."},
  };
  EXPECT_EQ (standard, output);
}


// Test verses text.
TEST_F (filter_text, verses_text_2)
{
  const std::string usfm =
  "\\c 15\n"
  "\\s Heading\n"
  "\\p\n"
  "\\v 1 He said:\n"
  "\\p I will sing to the Lord.\n"
  "\\v 2 The Lord is my strength.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.initializeHeadingsAndTextPerVerse (false);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::map <int, std::string> output = filter_text.getVersesText ();
  const std::map <int, std::string> standard = {
    {1, "He said: I will sing to the Lord."},
    {2, "The Lord is my strength." }
  };
  EXPECT_EQ (standard, output);
}


// Test paragraph starting markers.
TEST_F (filter_text, paragraph_starting_markers)
{
  const std::string usfm =
  "\\c 1\n"
  "\\s Heading\n"
  "\\p\n"
  "\\v 1 He said:\n"
  "\\q1 I will sing to the Lord.\n"
  "\\v 2 The Lord is my strength.\n"
  "\\q2 I trust in Him.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.initializeHeadingsAndTextPerVerse (false);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  EXPECT_EQ ((std::vector<std::string>{"p", "q1", "q2"}), filter_text.paragraph_starting_markers);
}


// Test improved paragraph detection.
TEST_F (filter_text, improved_paragraph_detection)
{
  const std::string path = filter_url_create_root_path ({"unittests", "tests", "ShonaNumbers23.usfm"});
  const std::string usfm = filter_url_file_get_contents (path);
  Filter_Text filter_text = Filter_Text (std::string());
  filter_text.initializeHeadingsAndTextPerVerse (false);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  for (size_t i = 0; i < filter_text.verses_paragraphs.size (); i++) {
    const std::map <int, std::string> verses_paragraph = filter_text.verses_paragraphs [i];
    std::map <int, std::string> standard;
    if (i == 0) {
      standard = {
        std::pair (1, "Zvino Bharamu wakati kuna Bharaki: Ndivakire pano aritari nomwe, ugondigadzirire pano nzombe nomwe nemakondohwe manomwe."),
        std::pair (2, "Bharaki ndokuita Bharamu sezvakataura; Bharaki naBharamu ndokupa nzombe diki negondohwe paaritari imwe neimwe."),
        std::pair (3, "Zvino Bharamu wakati kuna Bharaki: Mira pachipo chako chinopiswa, asi ini ndichaenda; zvimwe Jehovha achauya kuzosangana neni; zvino chinhu chaanondiratidza ndichakuudza. Zvino wakaenda pakakwirira."),
        std::pair (4, "Zvino Mwari wakasangana naBharamu, iye ndokuti kwaari: Ndagadzira aritari nomwe uye ndapa nzombe diki negondohwe paaritari imwe neimwe. "),
        std::pair (5, "Jehovha ndokuisa shoko mumuromo waBharamu, ndokuti: Dzokera kuna Bharaki ugotaura seizvi."),
        std::pair (6, "Zvino wakadzokera kwaari, uye tarira, amire pachipo chake chinopiswa, iye nemachinda ese aMoabhu."),
        std::pair (7, "Zvino wakasimudza mufananidzo wake ndokuti: Bharaki mambo waMoabhu wakandivinga kubva Siriya, pamakomo ekumabvazuva achiti: Uya nditukire Jakove, uye uya unyombe Israeri."),
        std::pair (8, "Ini ndingatuka sei Mwari waasina kutuka; uye ndinganyomba sei Jehovha waasina kunyomba?"),
        std::pair (9, "Nokuti kubva panhongonya yemabwe ndinomuona, uye kubva pazvikomo ndinomutarisa; tarira, vanhu vachagara vega, uye havangaverengwi pakati pendudzi."),
        std::pair (10, "Ndiani angaverenga guruva raJakove, nekuverenga chechina chaIsraeri? Mweya wangu ngaufe rufu rwevakarurama, uye magumo angu ngaave seake!"),
        std::pair (11, "Zvino Bharaki wakati kuna Bharamu: Wandiitei? Ndakutora kuti utuke vavengi vangu; zvino tarira, wavaropafadza nekuvaropafadza."),
        std::pair (12, "Zvino wakapindura ndokuti: Handifaniri kuchenjerera kutaura izvo Jehovha zvaanoisa mumuromo mangu here?"),
        std::pair (13, "Zvino Bharaki wakati kwaari: Uya hako neni kune imwe nzvimbo paungavaona kubvapo; uchaona kuguma kwavo chete, uye haungavaoni vese; ugonditukira ivo kubva ipapo.")
      };
    }
    if (i == 1) {
      standard = {
        std::pair (14, "Zvino wakamutora akamuisa mumunda weZofimu, panhongonya yePisiga, ndokuvaka aritari nomwe, ndokupa nzombe diki negondohwe paaritari imwe neimwe."),
        std::pair (15, "Zvino wakati kuna Bharaki: Mira pano pachipo chako chinopiswa, zvino ini ndichasangana naJehovha uko."),
        std::pair (16, "Zvino Jehovha wakasangana naBharamu, ndokuisa shoko mumuromo make, ndokuti: Dzokera kuna Bharaki ugotaura seizvi."),
        std::pair (17, "Zvino asvika kwaari, tarira, amire pachipo chake chinopiswa, nemachinda aMoabhu anaye. Bharaki ndokuti kwaari: Jehovha wataurei?"),
        std::pair (18, "Zvino wakasimudza mufananidzo wake ndokuti: Simuka Bharaki, ugonzwa; urereke nzeve kwandiri, iwe mwanakomana waZipori:"),
        std::pair (19, "Mwari haasi munhu kuti areve nhema, kana mwanakomana wemunhu kuti ashanduke. Iye wakareva, haangaiti kanhi? Kana wakareva, haangasimbisi kanhi?"),
        std::pair (20, "Tarira, ndagamuchira kuti ndiropafadze; zvino waropafadza, uye handigoni kuzvidzosa."),
        std::pair (21, "Haana kutarisira chakaipa kuna Jakove; uye haana kuona kutsauka kuna Israeri; Jehovha Mwari wake anaye, nekudanidzira kwamambo kuri pakati pavo."),
        std::pair (22, "Mwari wakavabudisa Egipita, ane sesimba renyati."),
        std::pair (23, "Zvirokwazvo hakuna un'anga hunopikisa Jakove, kana kuvuka kunopikisa Israeri. Ikozvino zvicharehwa pamusoro paJakove nepamusoro paIsraeri kuti Mwari wakazviita."),
        std::pair (24, "Tarirai, vanhu vachasimuka seshumbakadzi, vazvisimudze seshumba. Havangarari pasi kusvika vadya chakajimbiwa, uye vanwa ropa revakaurawa.")
      };
    }
    if (i == 2) {
      standard = {
        std::pair (25, "Zvino Bharaki wakati kuna Bharamu: Zvino usatongovatuka kana kutongovaropafadza."),
        std::pair (26, "Asi Bharamu wakapindura, ndokuti kuna Bharaki: Handina kukuudza here ndichiti: Zvese Jehovha zvaanotaura, ndizvo zvandichaita?")
      };
    }
    if (i == 3) {
      standard = {
        std::pair (27, "Zvino Bharaki wakati kuna Bharamu: Uya hako, ndichakuendesa kune imwe nzvimbo; zvimwe zvingarurama pameso aMwari kuti unditukire ivo kubva ipapo."),
        std::pair (28, "Zvino Bharaki wakatora ndokuisa Bharamu panhongonya yePeori, pakatarisa pasi pachiso cherenje."),
        std::pair (29, "Zvino Bharamu wakati kuna Bharaki: Ndivakire pano aritari nomwe, undigadzirire pano nzombe diki nomwe nemakondohwe manomwe."),
        std::pair (30, "Bharaki ndokuita Bharamu sezvaakataura, ndokupa nzombe diki negondohwe paaritari imwe neimwe.")
      };
    }
    EXPECT_EQ (standard, verses_paragraph);
  }
  EXPECT_EQ ((std::vector<std::string>{"p", "p", "p", "p"}), filter_text.paragraph_starting_markers);
}


// Test embedded character styles to text output.
TEST_F (filter_text, embedded_character_styles_to_text)
{
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 He said: I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n"
  "\\v 2 The \\nd Lord\\nd* is my strength.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.initializeHeadingsAndTextPerVerse (false);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::map <int, std::string> output = filter_text.getVersesText ();
  const std::map <int, std::string> standard = {
    {1, "He said: I will sing to the Lord."},
    {2, "The Lord is my strength."}
  };
  EXPECT_EQ (standard, output);
}


// Test embedded character styles to html output.
TEST_F (filter_text, embedded_character_styles_to_html_1)
{
  // Open character style, and embedded character style, and close both normally.
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new HtmlText (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  std::string html = filter_text.html_text_standard->get_inner_html ();
  const std::string standard =
  R"(<p class="p">)"
  R"(<span class="dropcaps">1</span>)"
  "<span>I will sing </span>"
  R"(<span class="add">to the </span>)"
  R"(<span class="add nd">Lord</span>)"
  "<span>.</span>"
  "</p>\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (html));
}


// Test embedded character styles to html output.
TEST_F (filter_text, embedded_character_styles_to_html_2)
{
  // Open character style, open embedded character style, close embedded one, then close the outer one.
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 I will sing \\add to the \\+nd Lord\\+nd* God\\add*.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new HtmlText (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::string html = filter_text.html_text_standard->get_inner_html ();
  const std::string standard = R"(<p class="p"><span class="dropcaps">1</span><span>I will sing </span><span class="add">to the </span><span class="add nd">Lord</span><span class="add"> God</span><span>.</span></p>)";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (html));
}


// Test embedded character styles to html output.
TEST_F (filter_text, embedded_character_styles_to_html_3)
{
  // Open character style, open embedded character style,
  // then closing the outer one closes the embedded one also.
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 I will sing \\add to the \\+nd Lord\\add*.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new HtmlText (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  std::string html = filter_text.html_text_standard->get_inner_html ();
  std::string standard = R"(<p class="p"><span class="dropcaps">1</span><span>I will sing </span><span class="add">to the </span><span class="add nd">Lord</span><span>.</span></p>)";
  EXPECT_EQ (standard, html);
}


// Test embedded character styles to OpenDocument output.
TEST_F (filter_text, embedded_character_styles_to_odt_1)
{
  // Open character style, and embedded character style, and close both normally.
  std::string usfm =
  "\\id GEN\n"
  "\\c 1\n"
  "\\p\n"
  "\\v 1 I will sing \\add to the \\+nd Lord\\+nd*\\add*.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  std::string standard = ""
  "Genesis\n"
  "\n"
  "Genesis 1\n"
  "\n"
  "1\n"
  "\n"
  "I will sing to the Lord.\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Exercise bits in document to generate text and note citations.
TEST_F (filter_text, generate_text_note_citations)
{
  std::string usfm =
  "\\id GEN\n"
  "\\v 1 Text 1\\x + \\xt Isa. 1.1.\\x* text\\f + \\fk Word: \\fl Heb. \\fq Explanation1.\\f* text\\fe + \\fk Word: \\fl Heb. \\fq Explanation1.\\fe*.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_text_and_note_citations = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_text_and_note_citations->save (text_odt);
  int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  std::string standard = ""
  "Genesis\n"
  "\n"
  "1 Text 1a text1 text1.";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test embedded character styles to OpenDocument output.
TEST_F (filter_text, embedded_character_styles_to_odt_2)
{
  // Open character style, open embedded character style, close embedded one, then close the outer one.
  const std::string usfm =
  "\\id GEN\n"
  "\\c 1\n"
  "\\p\n"
  "\\v 1 I will sing \\add to the \\+nd Lord\\+nd* God\\add*.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  const std::string standard =
  "Genesis\n"
  "\n"
  "Genesis 1\n"
  "\n"
  "1\n"
  "\n"
  "I will sing to the Lord God.\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test embedded character styles to OpenDocument output.
TEST_F (filter_text, embedded_character_styles_to_odt_3)
{
  // Open character style, open embedded character style, then closing the outer one closes the embedded one also.
  std::string usfm =
  "\\id GEN\n"
  "\\c 1\n"
  "\\p\n"
  "\\v 1 I will sing \\add to the \\+nd Lord\\add*.\n";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  std::string standard =
  "Genesis\n"
  "\n"
  "Genesis 1\n"
  "\n"
  "1\n"
  "\n"
  "I will sing to the Lord.\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test the behaviour of a chapter label put in chapter zero.
TEST_F (filter_text, chapter_label_in_chapter_zero)
{
  // The following USFM has the \cl - chapter label - before the first chapter.
  // It means that the \cl represents the text for "chapter" to be used throughout the book.
  // So it will output:
  // "Chapter 1"
  // ...
  // "Chapter 2"
  // ... and so on.
  const std::string usfm = R"(
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
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  
  // Check chapter labels.
  const int desiredchapter_labels = 1;
  const size_t actualchapter_labels = filter_text.chapter_labels.size();
  EXPECT_EQ (desiredchapter_labels, actualchapter_labels);
  if (desiredchapter_labels == static_cast<int>(actualchapter_labels)) {
    EXPECT_EQ (1, filter_text.chapter_labels[0].m_book);
    EXPECT_EQ (0, filter_text.chapter_labels[0].m_chapter);
    EXPECT_EQ ("0", filter_text.chapter_labels[0].m_verse);
    EXPECT_EQ ("cl", filter_text.chapter_labels[0].m_marker);
    EXPECT_EQ ("Chapter", filter_text.chapter_labels[0].m_value);
  }
  
  // OpenDocument output.
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", "", odt);
  const std::string standard = R"(
Genesis

Genesis 1

Chapter 1

1 I will sing to the LORD.

Genesis 2

Chapter 2

2 Jesus came to save the people.
    )";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test the behaviour of a chapter label put in each separate chapter.
TEST_F (filter_text, chapter_label_in_chapters)
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
  const std::string usfm = R"(
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
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  
  // Check chapter labels.
  const int desiredchapter_labels = 2;
  const size_t actualchapter_labels = filter_text.chapter_labels.size();
  EXPECT_EQ (desiredchapter_labels, actualchapter_labels);
  if (desiredchapter_labels == static_cast<int>(actualchapter_labels)) {
    EXPECT_EQ (1, filter_text.chapter_labels[0].m_book);
    EXPECT_EQ (1, filter_text.chapter_labels[0].m_chapter);
    EXPECT_EQ ("0", filter_text.chapter_labels[0].m_verse);
    EXPECT_EQ ("cl", filter_text.chapter_labels[0].m_marker);
    EXPECT_EQ ("Chapter One", filter_text.chapter_labels[0].m_value);
    EXPECT_EQ (1, filter_text.chapter_labels[1].m_book);
    EXPECT_EQ (2, filter_text.chapter_labels[1].m_chapter);
    EXPECT_EQ ("0", filter_text.chapter_labels[1].m_verse);
    EXPECT_EQ ("cl", filter_text.chapter_labels[1].m_marker);
    EXPECT_EQ ("Chapter Two", filter_text.chapter_labels[1].m_value);
  }
  
  // OpenDocument output.
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  odt = filter::strings::replace ("  ", std::string(), odt);
  const std::string standard = R"(
Genesis

Genesis 1

Chapter One

1 I will sing to the LORD.

Genesis 2

Chapter Two

2 Jesus came to save the people.
    )";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test footnotes and cross references in plain text.
TEST_F (filter_text, footnotes_xrefs_plain_text)
{
  const std::string usfm = R"(
\id GEN
\c 1
\p
\v 1 This is verse one\x + Xref 1\x*.
\v 2 This is verse two\f + Note 2\f*.
\v 3 This is verse three\fe + Endnote 3\fe*.
)";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  constexpr size_t n {3};
  const size_t size = filter_text.notes_plain_text.size();
  EXPECT_EQ (n, size);
  if (size == n) {
    EXPECT_EQ ("1", filter_text.notes_plain_text[0].first);
    EXPECT_EQ ("2", filter_text.notes_plain_text[1].first);
    EXPECT_EQ ("3", filter_text.notes_plain_text[2].first);
    EXPECT_EQ ("Xref 1", filter_text.notes_plain_text[0].second);
    EXPECT_EQ ("Note 2", filter_text.notes_plain_text[1].second);
    EXPECT_EQ ("Endnote 3", filter_text.notes_plain_text[2].second);
  }
}


// Test plain text and notes for export.
TEST_F (filter_text, plain_text_notes_export)
{
  const std::string usfm = R"(
\id GEN
\c 1
\p
\v 1 This is verse one\x + Xref 1\x*\f + Note 1\f*.
\v 2 This is verse two\f + Note 2\f*\x + Xref 2\x*.
\v 3 This is verse three\fe + Endnote 3\fe*.
)";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.initializeHeadingsAndTextPerVerse (false);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  
  const std::map <int, std::string> output = filter_text.getVersesText ();
  const std::map <int, std::string> standard = {
    {1, "This is verse one."},
    {2, "This is verse two."},
    {3, "This is verse three."},
  };
  EXPECT_EQ (standard, output);
  
  constexpr size_t n = 5;
  const size_t size = filter_text.notes_plain_text.size();
  EXPECT_EQ (n, size);
  if (size == n) {
    EXPECT_EQ ("1", filter_text.notes_plain_text[0].first);
    EXPECT_EQ ("1", filter_text.notes_plain_text[1].first);
    EXPECT_EQ ("2", filter_text.notes_plain_text[2].first);
    EXPECT_EQ ("2", filter_text.notes_plain_text[3].first);
    EXPECT_EQ ("3", filter_text.notes_plain_text[4].first);
    EXPECT_EQ ("Xref 1", filter_text.notes_plain_text[0].second);
    EXPECT_EQ ("Note 1", filter_text.notes_plain_text[1].second);
    EXPECT_EQ ("Note 2", filter_text.notes_plain_text[2].second);
    EXPECT_EQ ("Xref 2", filter_text.notes_plain_text[3].second);
    EXPECT_EQ ("Endnote 3", filter_text.notes_plain_text[4].second);
  }
  
  EXPECT_EQ (3, filter_text.verses_text_note_positions.size());
  EXPECT_EQ (std::vector <int>{}, filter_text.verses_text_note_positions [0]);
  EXPECT_EQ ((std::vector <int>{17, 17}), filter_text.verses_text_note_positions [1]);
  EXPECT_EQ ((std::vector <int>{17, 17}), filter_text.verses_text_note_positions [2]);
  EXPECT_EQ (std::vector <int>{19}, filter_text.verses_text_note_positions [3]);
  EXPECT_EQ (std::vector <int>{}, filter_text.verses_text_note_positions [4]);
}


// Test incorrect \vp markup.
TEST_F (filter_text, incorrect_vp_markup)
{
  const std::string usfm = R"(
\c 1
\p
\v 1 \vp A Jesus is King.
\v 2 \vp B Jesus is the son of God.
)";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (filter::strings::trim(usfm));
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (text_txt);
  // What happens is that due to the incorrect markup, the entire first verse \vp is omitted from the output.
  const std::string standard = R"(
Unknown 1

1

B Jesus is the son of God.
)";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


// Test invalid UTF8 input text.
TEST_F (filter_text, invalid_utf8_input)
{
  refresh_sandbox (false);
  std::string path = filter_url_create_root_path ({"unittests", "tests", "invalid-utf8-2.usfm"});
  std::string invalid_utf8_usfm = filter_url_file_get_contents (path);
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (filter::strings::trim(invalid_utf8_usfm));
  filter_text.run (stylesv2::standard_sheet ());
  filter_text.odf_text_standard->save (text_odt);
  int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (256, ret);
  std::string odt = filter_url_file_get_contents (text_txt);
  bool invalid_token = odt.find ("not well-formed (invalid token)") != std::string::npos;
  EXPECT_EQ (true, invalid_token);
  // The above test produces logbook entries.
  // Clear them so they don't disturb the output in the terminal.
  refresh_sandbox (false);
}


// Test converting USFM with an image to other formats.
TEST_F (filter_text, convert_image_to_format) 
{
  // Store images in the database that keeps the Bible images.
  const std::string image_2_name = "bibleimage2.png";
  const std::string image_3_name = "bibleimage3.png";
  const std::string image_2_path = filter_url_create_root_path ({"unittests", "tests", image_2_name});
  const std::string image_3_path = filter_url_create_root_path ({"unittests", "tests", image_3_name});
  database::bible_images::store(image_2_path);
  database::bible_images::store(image_3_path);
  const std::string usfm = R"(
\c 1
\p
\v 1 Verse one. \fig caption|src="bibleimage2.png" size="size" ref="reference"\fig*
\v 2 Verse two.
)";
  // Test converting the USFM with an image to html.
  {
    const std::string standard_html = R"(<p class="p"><span class="dropcaps">1</span><span>Verse one. </span></p><img alt="" src="bibleimage2.png" width="100%"/><p class="fig"><span>caption</span></p><p><span class="v">2</span><span> </span><span>Verse two.</span></p>)";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.html_text_standard = new HtmlText (bible);
    filter_text.add_usfm_code (usfm);
    filter_text.run (stylesv2::standard_sheet());
    const std::string html = filter_text.html_text_standard->get_inner_html();
    EXPECT_EQ (standard_html, html);
    EXPECT_EQ (std::vector<std::string>{image_2_name}, filter_text.image_sources);
    for (const auto& src : filter_text.image_sources) {
      const std::string contents = database::bible_images::get(src);
      const std::string standard = filter_url_file_get_contents(image_2_path);
      EXPECT_EQ (standard, contents);
    }
  }
  // Test converting the USFM with an image to OpenDocument.
  {
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new odf_text (bible);
    filter_text.add_usfm_code (usfm);
    filter_text.run (stylesv2::standard_sheet());
    filter_text.odf_text_standard->save (text_odt);
    const int ret = odf2txt (text_odt, text_txt);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (text_txt);
    odt = filter::strings::replace ("  ", "", odt);
    const std::string standard = 
    "Unknown 1"
    "\n"
    "\n"
    "1"
    "\n"
    "\n"
    "Verse one. "
    "\n"
    "\n"
    "\n"
    "\n"
    "caption"
    "\n"
    "\n"
    "2 Verse two.";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
}


// Test export word-level attributes.
// https://ubsicap.github.io/usfm/attributes/index.html
TEST_F (filter_text, export_no_word_level_attributes)
{
  const std::string usfm = R"(
\p
\v 1 This is verse one.
\v 2 And the \nd \+w Lord|strong="H3068"\+w*\nd* \w said|strong="H0559"\w* unto \w Cain|strong="H7014"\w*:
\v 3 This is verse three.
\v 4 Text with ruby glosses: \rb One|gloss="gg:gg"\rb* and \rb two|"gg:gg"\rb*.
\v 5 Text with default attribute: \w gracious|strong="H1234,G5485"\w*.
\v 6 Text with multiple attributes: \w gracious|lemma="grace" x-myattr="metadata"\w*.
\v 7 Text with \x - \xt 1|GEN 2:1\xt*\x*link reference.
  )";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.initializeHeadingsAndTextPerVerse (false);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet ());
  const std::map <int, std::string> verses_text = filter_text.getVersesText ();
  const std::map <int, std::string> standard = {
    {1, "This is verse one."},
    {2, "And the Lord said unto Cain:"},
    {3, "This is verse three."},
    {4, "Text with ruby glosses: One and two."},
    {5, "Text with default attribute: gracious."},
    {6, "Text with multiple attributes: gracious."},
    {7, "Text with link reference."},
  };
  EXPECT_EQ (standard, verses_text);
}


TEST_F (filter_text, alternate_chapter_number)
{
  const std::string usfm = R"(
  \c 13
  \ca 14\ca*
  \p
  \v 1 Verse one.
  )";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (text_txt);
  const std::string standard =
  "Unknown 13    (14)\n"
  "\n"
  "13    (14)\n"
  "\n"
  " \n"
  "\n   1 Verse one.\n\n"
  ;
  EXPECT_EQ (standard, odt);
}


TEST_F (filter_text, introduction_main_title)
{
  const std::string usfm = R"(
  \c 1
  \imt Title
  )";
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (text_txt);
  const std::string standard =
  "Unknown 1   \n"
  "\n"
  "Title\n"
  "\n";
  EXPECT_EQ (standard, odt);
}


TEST_F (filter_text, sidebar)
{
  const std::string usfm =
  R"(\p before)" "\n"
  R"(\esb)" "\n"
  R"(\p sidebar)" "\n"
  R"(\esbe)" "\n"
  R"(\p after)" "\n"
  ;
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (text_txt);
  const std::string standard =
  "before\n"
  "\n"
  "sidebar\n"
  "\n"
  "after\n"
  "\n"
  ;
  EXPECT_EQ (standard, odt);
}


TEST_F (filter_text, usfm_with_all_markers)
{
  const std::string usfm = usfm_with_all_markers;
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.html_text_standard = new HtmlText (bible);
  filter_text.text_text = new Text_Text ();
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (text_txt);
  const std::string text = filter_text.text_text->get ();
  const std::string html = filter_text.html_text_standard->get_inner_html();

  const std::string odt2_txt = filter_url_create_root_path ({"unittests", "tests", "odt2.txt"});
  const std::string standard_odt = filter_url_file_get_contents (odt2_txt);
  if (odt != standard_odt) {
    const std::string tmpfile = "/tmp/odt2.txt";
    filter_url_file_put_contents (tmpfile, odt);
    ADD_FAILURE() << "The produced odt differs from the reference odt in file " << odt2_txt << std::endl
    << "The produced odt was written to " << tmpfile;
  }
  
  const std::string text2_txt = filter_url_create_root_path ({"unittests", "tests", "text2.txt"});
  const std::string standard_text = filter_url_file_get_contents (text2_txt);
  if (text != standard_text) {
    const std::string tmpfile = "/tmp/text2.txt";
    filter_url_file_put_contents (tmpfile, text);
    ADD_FAILURE() << "The produced text differs from the reference text in file " << text2_txt << std::endl
    << "The produced text was written to " << tmpfile;
    std::vector<std::string> removals;
    std::vector<std::string> additions;
    filter_diff_diff (filter::strings::trim(standard_text), filter::strings::trim(text), &removals, &additions);
    if (!removals.empty())
      ADD_FAILURE() << "First item that the generated text does not have: " << removals.front();
    if (!additions.empty())
      ADD_FAILURE() << "First item that the generated text has which is not in the standard: " << additions.front();
  }

  const std::string standard_html =
  R"(<p class="mt"><span>Major title</span></p>)"
  R"(<p class="mt1"><span>Major title 1</span></p>)"
  R"(<p class="mt2"><span>Major title 2</span></p>)"
  R"(<p class="mt3"><span>Major title 3</span></p>)"
  R"(<p class="mt4"><span>Major title 4</span></p>)"
  R"(<p class="mte"><span>Major title at ending</span></p>)"
  R"(<p class="mte1"><span>Major title at ending 1</span></p>)"
  R"(<p class="mte2"><span>Major title at ending 2</span></p>)"
  R"(<p class="ms"><span>Major section heading</span></p>)"
  R"(<p class="ms1"><span>Major section heading 1</span></p>)"
  R"(<p class="ms2"><span>Major section heading 2</span></p>)"
  R"(<p class="mr"><span>Major section references</span></p>)"
  R"(<p class="s"><span>Section heading</span></p>)"
  R"(<p class="s1"><span>Section heading 1</span></p>)"
  R"(<p class="s2"><span>Section heading 2</span></p>)"
  R"(<p class="s3"><span>Section heading 3</span></p>)"
  R"(<p class="s4"><span>Section heading 4</span></p>)"
  R"(<p class="sr"><span>Section references</span></p>)"
  R"(<p class="r"><span>Parallel passage references</span></p>)"
  R"(<p class="rq"><span>Inline quotation references</span></p>)"
  R"(<p class="d"><span>Descriptive title</span></p>)"
  R"(<p class="sp"><span>Speaker</span></p>)"
  R"(<p class="sd"><span>Semantic division</span></p>)"
  R"(<p class="sd1"><span>Semantic division 1</span></p>)"
  R"(<p class="sd2"><span>Semantic division 2</span></p>)"
  R"(<p class="sd3"><span>Semantic division 3</span></p>)"
  R"(<p class="sd4"><span>Semantic division 4</span></p>)"
  R"(<p class="imt"><span>Main Title</span></p>)"
  R"(<p class="imt1"><span>Main Title 1</span></p>)"
  R"(<p class="imt2"><span>Main Title 2</span></p>)"
  R"(<p class="imt3"><span>Main Title 3</span></p>)"
  R"(<p class="imt4"><span>Main Title 4</span></p>)"
  R"(<p class="is"><span>Section</span></p>)"
  R"(<p class="is1"><span>Section 1</span></p>)"
  R"(<p class="is2"><span>Section 2</span></p>)"
  R"(<p class="ip"><span>Introduction paragraph</span></p>)"
  R"(<p class="ipi"><span>Introduction paragraph indented</span></p>)"
  R"(<p class="im"><span>Introduction margin paragraph</span></p>)"
  R"(<p class="imi"><span>Introduction margin paragraph indented</span></p>)"
  R"(<p class="ipq"><span>Introduction paragraph quote</span></p>)"
  R"(<p class="imq"><span>Introduction margin paragraph quote</span></p>)"
  R"(<p class="ipr"><span>Introduction paragraph right</span></p>)"
  R"(<p class="iq"><span>Introduction poetry</span></p>)"
  R"(<p class="iq1"><span>Introduction poetry 1</span></p>)"
  R"(<p class="iq2"><span>Introduction poetry 2</span></p>)"
  R"(<p class="iq3"><span>Introduction poetry 3</span></p>)"
  R"(<p class="ib"><br/></p>)"
  R"(<p class="ili"><span>1 Introduction list item</span></p>)"
  R"(<p class="ili1"><span>1 Introduction list item 1</span></p>)"
  R"(<p class="ili2"><span>1 Introduction list item 2</span></p>)"
  R"(<p class="iot"><span>Introduction Outline Title</span></p>)"
  R"(<p class="io"><span>Introduction outline</span></p>)"
  R"(<p class="io1"><span>Introduction outline 1</span></p>)"
  R"(<p class="io2"><span>Introduction outline 2</span></p>)"
  R"(<p class="io3"><span>Introduction outline 3 </span><span class="ior">references</span></p>)"
  R"(<p class="io4"><span>Introduction outline 4 </span><span class="iqt">quotation</span></p>)"
  R"(<p class="iex"><span>Introduction explanatory text</span></p>)"
  R"(<p class="imte"><span>Introduction main title ending</span></p>)"
  R"(<p class="imte1"><span>Introduction main title ending 1</span></p>)"
  R"(<p class="imte2"><span>Introduction main title ending 2</span></p>)"
  R"(<p class="c"><span>1 (2)</span><span> </span><span> </span><span> </span><span> </span></p>)"
  R"(<p class="p"><span class="v">1b</span><span> </span><span>Text name</span><span class="pro">pronunciation</span><span>.</span></p>)"
  R"(<p class="p"><span class="v">2</span><span> </span><span> text</span></p>)"
  R"(<p class="c"><span>Psalm</span></p>)"
  R"(<p class="p"><span class="v">1</span><span> </span><span>Normal </span><span class="add">added</span><span> and </span><span class="addpn">AddPn</span></p>)"
  R"(<p class="c"><span>א</span></p>)"
  R"(<p class="p"><span class="v">1</span><span> </span><span>Verse text</span></p>)"
  R"(<p class="c"><span>4</span></p>)"
  R"(<p class="cd"><span>Chapter description</span></p>)"
  R"(<p class="m"><span>Continuation (margin) paragraph.</span></p>)"
  R"(<p class="po"><span>Paragraph opening an epistle.</span></p>)"
  R"(<p class="pr"><span>Right aligned paragraph.</span></p>)"
  R"(<p class="cls"><span>Closure of letter.</span></p>)"
  R"(<p class="pmo"><span>Embedded text opening.</span></p>)"
  R"(<p class="pm"><span>Embedded text paragraph.</span></p>)"
  R"(<p class="pmc"><span>Embedded text closing.</span></p>)"
  R"(<p class="pmr"><span>Embedded text refrain.</span></p>)"
  R"(<p class="pi"><span>Indented paragraph.</span></p>)"
  R"(<p class="pi1"><span>Indented paragraph 1.</span></p>)"
  R"(<p class="pi2"><span>Indented paragraph 2.</span></p>)"
  R"(<p class="pi3"><span>Indented paragraph 3.</span></p>)"
  R"(<p class="mi"><span>Margin indented.</span></p>)"
  R"(<p class="nb"><span>No break.</span></p>)"
  R"(<p class="pc"><span>Centred paragraph.</span></p>)"
  R"(<p class="ph"><span>Hanging paragraph.</span></p>)"
  R"(<p class="ph1"><span>Hanging paragraph 1.</span></p>)"
  R"(<p class="ph2"><span>Hanging paragraph 2.</span></p>)"
  R"(<p class="ph3"><span>Hanging paragraph 3.</span></p>)"
  R"(<p class="ph4"><span>Hanging paragraph 4.</span></p>)"
  R"(<p class="b"><br/></p>)"
  R"(<p class="q"><span>Poetic line.</span></p>)"
  R"(<p class="q1"><span>Poetic line 1.</span></p>)"
  R"(<p class="q2"><span>Poetic line 2.</span></p>)"
  R"(<p class="q3"><span>Poetic line 3.</span></p>)"
  R"(<p class="qr"><span>Right-aligned poetic line</span></p>)"
  R"(<p class="qc"><span>Centered poetic line</span></p>)"
  R"(<p class="q"><span>Poetry </span><span class="qs">Selah</span></p>)"
  R"(<p class="qa"><span>Aleph</span></p>)"
  R"(<p class="p"><span class="qac">A</span><span>rostic</span></p>)"
  R"(<p class="qm"><span>Embedded poetic line</span></p>)"
  R"(<p class="qm1"><span>Embedded poetic line 1</span></p>)"
  R"(<p class="qm2"><span>Embedded poetic line 2</span></p>)"
  R"(<p class="qm3"><span>Embedded poetic line 3</span></p>)"
  R"(<p class="qd"><span>Hebrew note</span></p>)"
  R"(<p class="lh"><span>List header</span></p>)"
  R"(<p class="li"><span>List item</span></p>)"
  R"(<p class="li1"><span>List item 1</span></p>)"
  R"(<p class="li2"><span>List item 2</span></p>)"
  R"(<p class="li3"><span>List item 3</span></p>)"
  R"(<p class="li4"><span>List item 4</span></p>)"
  R"(<p class="lf"><span>List footer</span></p>)"
  R"(<p class="lim"><span>Embedded list item</span></p>)"
  R"(<p class="lim1"><span>Embedded list item 1</span></p>)"
  R"(<p class="lim2"><span>Embedded list item 2</span></p>)"
  R"(<p class="p"><span>List </span><span class="litl">total</span></p>)"
  R"(<p class="li"><span class="lik">Key</span><span> </span><span class="liv">Value</span><span> </span><span class="liv1">Value 1</span><span> </span><span class="liv2">Value 2</span></p>)"
  R"(<p class="c"><span>5</span></p>)"
  R"(<p class="p"><span class="v">1</span><span> </span><span>The table:</span></p>)"
  R"(<p class="th1"><span>Column1</span></p>)"
  R"(<p class="th2"><span>Column2</span></p>)"
  R"(<p class="th3"><span>Column3</span></p>)"
  R"(<p class="th4"><span>Column4</span></p>)"
  R"(<p class="thr1"><span>R-Column1</span></p>)"
  R"(<p class="thr2"><span>R-Column2</span></p>)"
  R"(<p class="thr3"><span>R-Column3</span></p>)"
  R"(<p class="thr4"><span>R-Column4</span></p>)"
  R"(<p class="tc1"><span>Cell1</span></p>)"
  R"(<p class="tc2"><span>Cell2</span></p>)"
  R"(<p class="tc3"><span>Cell3</span></p>)"
  R"(<p class="tc4"><span>Cell4</span></p>)"
  R"(<p class="tcr1"><span>R-Cell1</span></p>)"
  R"(<p class="tcr2"><span>R-Cell2</span></p>)"
  R"(<p class="tcr3"><span>R-Cell3</span></p>)"
  R"(<p class="tcr4"><span>R-Cell4</span></p>)"
  R"(<p class="c"><span>6</span></p>)"
  R"(<p class="p"><span>text</span><a href="#note1" id="citation1" class="superscript">1</a></p>)"
  R"(<p class="p"><span>text</span><a href="#note2" id="citation2" class="superscript">1</a></p>)"
  R"(<p class="p"><span>text</span><a href="#note3" id="citation3" class="superscript">1</a></p>)"
  R"(<p class="p"><span>text</span><a href="#note4" id="citation4" class="superscript">2</a></p>)"
  R"(<p class="p"><span>text</span><a href="#note5" id="citation5" class="superscript">a</a></p>)"
  R"(<p class="p"><span>text</span><a href="#note6" id="citation6" class="superscript">a</a></p>)"
  
  
  R"(<p class="p"><span>The </span><span class="bk">Book</span><span> name</span></p>)"
  R"(<p class="p"><span>Proto </span><span class="dc">Deutero</span><span> text.</span></p>)"
  R"(<p class="p"><span>This is a </span><span class="k">keyword</span></p>)"
  R"(<p class="p"><span class="nd">Yahweh</span></p>)"
  R"(<p class="p"><span>The 1</span><span class="ord">st</span><span>.</span></p>)"
  R"(<p class="p"><span>It is about </span><span class="pn">Jesus</span><span>.</span></p>)"
  R"(<p class="p"><span>The capital is </span><span class="png">Amsterdam</span><span>.</span></p>)"
  R"(<p class="p"><span>This is a </span><span class="qt">quotation</span><span>.</span></p>)"
  R"(<p class="p"><span class="sig">Kind regards from Paul</span><span>.</span></p>)"
  R"(<p class="p"><span>Hebrew and </span><span class="sls">Aramaic</span><span> source.</span></p>)"
  R"(<p class="p"><span>Jesus: </span><span class="tl">Eli, Eli, lema sabachthani?</span></p>)"
  R"(<p class="p"><span>Jesus: </span><span class="wj">I am</span><span>.</span></p>)"
  R"(<p class="p"><span>This is </span><span class="em">emphasized</span><span> text.</span></p>)"
  R"(<p class="p"><span>This is </span><span class="bd">bold</span><span> text.</span></p>)"
  R"(<p class="p"><span>This is </span><span class="it">italic</span><span> text.</span></p>)"
  R"(<p class="p"><span>This is </span><span class="bdit">bold/italic</span><span> text.</span></p>)"
  R"(<p class="s"><span>Section heading with </span><span class="no">normal</span><span> text</span></p>)"
  R"(<p class="p"><span>This is </span><span class="sc">small cap</span><span> text.</span></p>)"
  R"(<p class="p"><span>This is </span><span class="sup">superscript</span><span> text.</span></p>)"
  R"(<p class="lit"><span>Liturgical note</span></p>)"
  R"(<p class="break"/>)"
  R"(<p class="c"><span>7</span></p>)"
  R"(<p class="p"><span>This chapter is on a new page</span></p>)"
  R"(<p class="c"><span>8</span></p>)"
  R"(<p class="p"><span>Text</span></p>)"
  R"(<img alt="" src="image.png" width="100%"/>)"
  R"(<p class="fig"><span>caption</span></p>)"
  R"(<p class="p"><span>Text</span></p>)"
  R"(<img alt="DESC" src="FILE" width="100%"/>)"
  R"(<p class="fig"><span>CAP</span></p>)"
  R"(<p class="p"><span>index</span><span> </span><span>word</span><span> </span><span>greek</span><span> </span><span>hebrew</span><span> </span><span>aramaic</span></p>)"
  R"(<p class="p"><span>Text</span><span class="rb">wd</span></p>)"
  R"(<p class="p"><span>Sidebar</span></p>)"
  R"(<p class="p"><span>text</span><a href="#note7" id="citation7" class="superscript"></a></p>)"
  R"(<p class="break"/>)"
  R"(<p class="p"><span>paragraph </span><span class="jmp">text|link-href="href"</span></p>)"
  
  R"(<p class="c"><span>9</span></p>)"
  R"(<p class="p"><span>text</span></p>)"
  R"(<p class="p"><span>text</span></p>)"
  R"(<p class="p"><span>text</span></p>)"
  R"(<p class="p"><span>text</span></p>)"
  R"(<p class="p"><span>text</span></p>)"
  R"(<p class="p"><span>text</span></p>)"
  R"(<p class="p"><span>text</span></p>)"
  R"(<p class="p"><span>text</span></p>)"

  R"(<div>)"
  R"(<p class="ft"><a href="#citation1" id="note1">1</a><span> </span><span class="fr">ref </span><span>note </span><span class="fq">quote </span><span class="fv">3</span><span> </span><span class="fqa">alternate quote </span><span class="fk">keyword </span><span class="fl">label </span><span class="fw">witness </span><span class="fdc">deuterocanonical</span><span> </span><span class="fm">mark</span></p>)"
  R"(<p class="ft"><a href="#citation2" id="note2">1</a><span> </span><span class="fr">ref</span><span>note</span></p>)"
  R"(<p class="ft"><a href="#citation3" id="note3">1</a><span> </span><span class="fr">ref</span><span>note</span></p>)"
  R"(<p class="ft"><a href="#citation4" id="note4">2</a><span> </span><span class="fp">paragraph</span></p>)"
  R"(<p class="xt"><a href="#citation5" id="note5">a</a><span> </span><span class="xo">origin </span><span class="xk">keyword </span><span class="xq">quotation </span><span>targets </span><span class="xta">added </span><span class="xop">published</span><span> </span><span class="xot">old testament</span><span> </span><span class="xnt">new testament</span><span> </span><span class="xdc">deuterocanonical</span></p>)"
  R"(<p class="xt"><a href="#citation6" id="note6">a</a><span> </span><span>targets</span></p>)"
  R"(<p class="ft"><a href="#citation7" id="note7"></a><span> </span><span>category</span><span> </span><span>note</span></p>)"
  R"(</div>)"
  ;

  const auto make_readable = [] (const auto& html) {
    return filter::strings::replace ("</p>", "</p>\n", html);
  };

  if (html != standard_html) {
    ADD_FAILURE() << "The produced html differs from the reference html";
    std::cout << "Generated html:" << std::endl;
    std::cout << make_readable(html) << std::endl;
    std::vector<std::string> removals;
    std::vector<std::string> additions;
    filter_diff_diff (standard_html, html, &removals, &additions);
    if (!removals.empty())
      ADD_FAILURE() << "The first item that the generated html lacks is this: " << removals.front();
    if (!additions.empty())
      ADD_FAILURE() << "The first item that the generated html has extra related to the standard html is this: " << additions.front();
  }
}


TEST_F (filter_text, milestones)
{
  const std::string usfm =
  R"(\p text1\qt-s |attribute1="value1"\*)" "\n"
  R"(\p text2\qt-e |attribute2="value2"\*)" "\n"
  ;
  
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.odf_text_standard = new odf_text (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesv2::standard_sheet());
  filter_text.odf_text_standard->save (text_odt);
  const int ret = odf2txt (text_odt, text_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (text_txt);
  const std::string standard =
  "text1\n"
  "\n"
  "text2\n"
  "\n"
  ;
  EXPECT_EQ (standard, odt);
}


#endif

