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
#include <editor/html2usfm.h>
#include <editor/usfm2html.h>
#include <styles/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <database/state.h>
#include <webserver/request.h>
#include <edit/logic.h>


class usfm_html : public ::testing::Test
{
public:
  usfm_html() = default;
  virtual ~usfm_html() = default;
  static void SetUpTestCase()
  {
    refresh_sandbox (false);
  }
  static void TearDownTestCase()
  {
    refresh_sandbox (false);
  }
  virtual void SetUp()
  {
    Database_State::create ();
  }
  virtual void TearDown()
  {
  }
};


TEST_F (usfm_html, one_unknown_opening_marker)
{
  std::string standard_usfm = R"(\abc)";
  std::string standard_html = R"(<p class="b-mono"><span>\abc </span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, filter::strings::trim (html));
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, filter::strings::trim (usfm));
}


TEST_F (usfm_html, two_unknown_opening_markers)
{
  std::string standard_usfm =
  R"(\abc)" "\n"
  R"(\abc)";
  std::string standard_html =
  R"(<p class="b-mono"><span>\abc </span></p>)"
  R"(<p class="b-mono"><span>\abc </span></p>)";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, one_unknown_closing_marker)
{
  std::string standard_usfm = R"(\abc text\abc*.)";
  std::string standard_html = R"(<p class="b-mono"><span>\abc </span><span>text</span><span>\abc*</span><span>.</span></p>)";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, two_unknown_closing_markers)
{
  std::string standard_usfm =
  R"(\abc text\abc*.)" "\n"
  R"(\abc text\abc*.)";
  std::string standard_html =
  R"(<p class="b-mono"><span>\abc </span><span>text</span><span>\abc*</span><span>.</span></p>)"
  R"(<p class="b-mono"><span>\abc </span><span>text</span><span>\abc*</span><span>.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, identifiers)
{
  std::string standard_usfm =
  R"(\id GEN)" "\n"
  R"(\h Header)" "\n"
  R"(\toc1 The Book of Genesis)" "\n"
  R"(\cl Chapter)" "\n"
  R"(\cp ②)" "\n"
  R"(\cp Ⅰ)";
  std::string standard_html =
  R"(<p class="b-mono"><span>\id </span><span>GEN</span></p>)"
  R"(<p class="b-mono"><span>\h </span><span>Header</span></p>)"
  R"(<p class="b-mono"><span>\toc1 </span><span>The Book of Genesis</span></p>)"
  R"(<p class="b-mono"><span>\cl </span><span>Chapter</span></p>)"
  R"(<p class="b-mono"><span>\cp </span><span>②</span></p>)"
  R"(<p class="b-mono"><span>\cp </span><span>Ⅰ</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, one_paragraph)
{
  std::string standard_usfm = R"(\p Paragraph text.)";
  std::string standard_html = R"(<p class="b-p"><span>Paragraph text.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, two_paragraphs)
{
  std::string standard_usfm =
  R"(\p Paragraph text.)" "\n"
  R"(\p Paragraph txt.)";
  std::string standard_html =
  R"(<p class="b-p"><span>Paragraph text.</span></p>)"
  R"(<p class="b-p"><span>Paragraph txt.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, inline_text)
{
  std::string standard_usfm = R"(\p Paragraph text plus \add added\add* text.)";
  std::string standard_html = R"(<p class="b-p"><span>Paragraph text plus </span><span class="i-add">added</span><span> text.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, inline_texts)
{
  std::string standard_usfm = R"(\p Paragraph text plus \add added\add* text plus \add added\add* text.)";
  std::string standard_html = R"(<p class="b-p"><span>Paragraph text plus </span><span class="i-add">added</span><span> text plus </span><span class="i-add">added</span><span> text.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, chapter)
{
  std::string standard_usfm =
  R"(\c 1)" "\n"
  R"(\p Paragraph.)";
  
  std::string standard_html =
  R"(<p class="b-c"><span>1</span></p>)"
  R"(<p class="b-p"><span>Paragraph.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, alternative_chapter)
{
  std::string standard_usfm =
  R"(\c 13)" "\n"
  R"(\ca 14\ca*)" "\n"
  R"(\p)" "\n"
  R"(\v 1 Text)";
  
  const std::string standard_html = R"(<p class="b-c"><span>13</span></p><p class="b-mono"><span>\ca </span><span>14</span><span>\ca*</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, verses)
{
  std::string standard_usfm =
  R"(\p)" "\n"
  R"(\v 1 One.)" "\n"
  R"(\v 2 Two.)";
  std::string standard_html =
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span>One.</span><span> </span><span class="i-v">2</span><span> </span><span>Two.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, published_verse_markers)
{
  std::string standard_usfm = R"(
\p
\v 1 \vp A\vp* One.
\v 2 \vp B\vp* Two.
)";
  standard_usfm = filter::strings::trim (standard_usfm);
  std::string standard_html = R"(
<p class="b-p"><span class="i-v">1</span><span> </span><span class="i-vp">A</span><span> One.</span><span> </span><span class="i-v">2</span><span> </span><span class="i-vp">B</span><span> Two.</span></p>
)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (filter::strings::trim (standard_html), html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, peripherals)
{
  std::string standard_usfm =
  "\\periph Title Page\n"
  "\\periph Publication Data";
  std::string standard_html =
  R"(<p class="b-mono"><span>\periph </span><span>Title Page</span></p>)"
  R"(<p class="b-mono"><span>\periph </span><span>Publication Data</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, picture)
{
  std::string standard_usfm =
  "\\p Text\n"
  "\\fig DESC|FILE|SIZE|LOC|COPY|CAP|REF\\fig*\n"
  "\\p Text";
  std::string standard_html =
  R"(<p class="b-p"><span>Text</span></p>)"
  R"(<p class="b-mono"><span>\fig </span><span>DESC|FILE|SIZE|LOC|COPY|CAP|REF</span><span>\fig*</span></p>)"
  R"(<p class="b-p"><span>Text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, table)
{
  std::string standard_usfm =
  "\\tr \\th1 Tribe \\th2 Leader \\thr3 Number\n"
  "\\tr \\tc1 Reuben \\tc2 Elizur son of Shedeur \\tcr3 46500\n"
  "\\tr \\tc1 Simeon \\tc2 Shelumiel son of Zurishaddai \\tcr3 59300\n"
  "\\tr \\tc1 Gad \\tc2 Eliasaph son of Reuel \\tcr3 45650\n"
  "\\tr \\tc1 \\tcr2 Total: \\tcr3 151450";
  std::string standard_html =
  R"(<p class="b-mono"><span>\tr </span><span class="i-th1">Tribe </span><span class="i-th2">Leader </span><span class="i-thr3">Number</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tc1">Reuben </span><span class="i-tc2">Elizur son of Shedeur </span><span class="i-tcr3">46500</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tc1">Simeon </span><span class="i-tc2">Shelumiel son of Zurishaddai </span><span class="i-tcr3">59300</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tc1">Gad </span><span class="i-tc2">Eliasaph son of Reuel </span><span class="i-tcr3">45650</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tcr2">Total: </span><span class="i-tcr3">151450</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  std::string standard_usfm2 = filter::strings::replace ("\\tc1 \\", "\\", standard_usfm);
  EXPECT_EQ (standard_usfm2, usfm);
}


TEST_F (usfm_html, word_list_entry)
{
  std::string standard_usfm = R"(\p A \ndx index\ndx* b \wh Hebrew\wh* c.)";
  std::string standard_html =
  R"(<p class="b-p"><span>A </span><span class="i-ndx">index</span><span> b </span><span class="i-wh">Hebrew</span><span> c.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, crossreference)
{
  std::string standard_usfm = R"(\p The elder\x + 2 Joh. 1.1\x* to the beloved Gaius.)";
  std::string standard_html =
  R"(<p class="b-p"><span>The elder</span><span class="i-notecall1">a</span><span> to the beloved Gaius.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-x"><span class="i-notebody1">a</span><span> </span><span>+ 2 Joh. 1.1</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, crossreferences)
{
  std::string standard_usfm =
  R"(\p The elder\x + 2 Joh. 1.1\x* to the beloved Gaius.)" "\n"
  R"(\v 1 The elders\x + 2 Joh. 2.2\x* to the beloved.)";
  std::string standard_html =
  R"(<p class="b-p"><span>The elder</span><span class="i-notecall1">a</span><span> to the beloved Gaius.</span><span> </span><span class="i-v">1</span><span> </span><span>The elders</span><span class="i-notecall2">b</span><span> to the beloved.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-x"><span class="i-notebody1">a</span><span> </span><span>+ 2 Joh. 1.1</span></p>)"
  R"(<p class="b-x"><span class="i-notebody2">b</span><span> </span><span>+ 2 Joh. 2.2</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, footnote)
{
  std::string standard_usfm =
  R"(\p The earth brought forth\f + \fk brought: \fl Heb. \fq explanation.\f*.)";
  std::string standard_html =
  R"(<p class="b-p"><span>The earth brought forth</span><span class="i-notecall1">1</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fk">brought: </span><span class="i-fl">Heb. </span><span class="i-fq">explanation.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, cycling_the_note_caller)
{
  std::string standard_usfm =
  "\\p Text\\f + note\\f*.\n"
  "\\p Text\\fe + note\\fe*.\n"
  "\\p Text\\x + note\\x*.\n"
  "\\p Text\\f + note\\f*.\n"
  "\\p Text\\fe + note\\fe*.\n"
  "\\p Text\\x + note\\x*.\n"
  "\\p Text\\f + note\\f*.\n"
  "\\p Text\\fe + note\\fe*.\n"
  "\\p Text\\x + note\\x*.\n"
  "\\p Text\\f + note\\f*.\n"
  "\\p Text\\fe + note\\fe*.\n"
  "\\p Text\\x + note\\x*.\n"
  "\\p Text\\f + note\\f*.\n"
  "\\p Text\\fe + note\\fe*.\n"
  "\\p Text\\x + note\\x*.\n"
  "\\p Text\\f + note\\f*.\n"
  "\\p Text\\fe + note\\fe*.\n"
  "\\p Text\\x + note\\x*.\n"
  "\\p Text\\f + note\\f*.\n"
  "\\p Text\\fe + note\\fe*.\n"
  "\\p Text\\x + note\\x*.";
  std::string standard_html = filter_url_file_get_contents (filter_url_create_root_path ({"unittests", "tests", "editor_roundtrip_1.txt"}));
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, endnote)
{
  std::string standard_usfm =
  R"(\p The earth brought forth\fe + \fk brought: \fl Heb. \fq explanation.\fe*.)";
  std::string standard_html =
  R"(<p class="b-p"><span>The earth brought forth</span><span class="i-notecall1">1</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-fe"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fk">brought: </span><span class="i-fl">Heb. </span><span class="i-fq">explanation.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, round_trip_from_real_life_1)
{
  std::string standard_usfm =
  "\\c 1\n"
  "\\s Ukufika kukaJesu Kristu kuyamenyezelwa nguJohane uMbhabhathizi\n"
  "\\p\n"
  "\\v 1 Ukuqala kwevangeli likaJesu Kristu, iNdodana kaNkulunkulu\\x + Mat. 14.33.\\x*;\n"
  "\\v 2 njengokulotshiweyo kubaprofethi\\x + Mat. 3.1-11. Luka 3.2-16.\\x* \\add ukuthi\\add*: Khangela, mina ngiyathuma isithunywa sami phambi kobuso bakho, esizalungisa indlela yakho phambi kwakho\\x + Mal. 3.1. Mat. 11.10. Luka 7.27.\\x*.\n"
  "\\v 3 Ilizwi lomemezayo enkangala, \\add lithi\\add*: Lungisani indlela yeNkosi; qondisani izindledlana zayo\\x + Isa. 40.3. Joha. 1.23.\\x*.\n"
  "\\v 4 Kweza uJohane ebhabhathiza enkangala, etshumayela ubhabhathizo lokuphenduka kukho ukuthethelelwa kwezono\\x + Mat. 26.28. Luka 1.77.\\x*,\n"
  "\\v 5 njalo kwaphumela kuye ilizwe lonke leJudiya, labo abeJerusalema, basebebhabhathizwa bonke nguye emfuleni iJordani, bevuma izono zabo.\n"
  "\\v 6 Futhi uJohane wayembethe uboya bekamela, elozwezwe lwesikhumba ekhalweni lwakhe, wayesidla intethe loluju lweganga.\n"
  "\\p\n"
  "\\v 7 Watshumayela, esithi: Olamandla kulami uyeza emva kwami, engingafanele ukukhothama ngithukulule umchilo wamanyathela akhe\\x + Gen. 14.23. Isa. 5.27. Luka 3.16. Joha. 1.27.\\x*.\n"
  "\\v 7 Watshumayela, esithi: Olamandla kulami uyeza emva kwami, engingafanele ukukhothama ngithukulule umchilo wamanyathela akhe\\x + Gen. 14.23. Isa. 5.27. Luka 3.16. Joha. 1.27.\\x*.\n"
  "\\v 8 Mina isibili ngilibhabhathize ngamanzi; kodwa yena uzalibhabhathiza ngoMoya oNgcwele.\n"
  "\\s Ukubhabhathizwa kukaJesu\n"
  "\\p\n"
  "\\v 9 Kwasekusithi ngalezonsuku\\x + Mat. 3.13-17. Luka 3.21,22.\\x* kwafika uJesu evela eNazaretha yeGalili\\x + Mat. 2.23.\\x*, wabhabhathizwa nguJohane eJordani.\n"
  "\\v 10 Wenela ukukhuphuka emanzini, wabona amazulu eqhekezeka, loMoya kungathi lijuba esehlela phezu kwakhe;\n"
  "\\v 11 kwasekusiza ilizwi livela emazulwini, \\add lathi\\add*: Wena uyiNdodana yami ethandekayo, engithokoza ngayo.";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, round_trip_from_real_life_2)
{
  std::string standard_usfm =
  "\\c 1\n"
  "\\s Ukudalwa komhlaba laye umuntu\n"
  "\\p\n"
  "\\v 1 Ekuqaleni\\x + Hlab. 33.6; 136.5; Joha. 1.1-3; Seb. 14.15; 17.24; Kol. 1.16,17; Heb. 1.8-10; 11.3.\\x* uNkulunkulu wadala amazulu lomhlaba.\n"
  "\\v 2 Njalo umhlaba wawungelasimo ungelalutho\\x + Isa. 34.11 Jer. 4.23.\\x*, lobumnyama babuphezu kokujula; loMoya kaNkulunkulu wayendiza phezu kwamanzi.\n"
  "\\v 3 UNkulunkulu wasesithi: Kakube khona ukukhanya\\x + 2 Kor. 4.6\\x*; kwasekusiba khona ukukhanya.\n"
  "\\v 4 UNkulunkulu wasebona ukukhanya ukuthi kuhle; uNkulunkulu wasesehlukanisa phakathi kokukhanya lobumnyama.\n"
  "\\v 5 UNkulunkulu wasebiza ukukhanya ngokuthi yimini, lobumnyama wabubiza ngokuthi yibusuku. Kwasekusiba yikuhlwa, njalo kwaba yikusa, usuku lokuqala.\n"
  "\\p\n"
  "\\v 6 UNkulunkulu wasesithi: Kakube khona umkhathi\\x + Jobe 37.18. Hlab. 136.5. Jer. 10.12; 51.15.\\x* phakathi kwamanzi; kawehlukanise phakathi kwamanzi lamanzi.\n"
  "\\v 7 UNkulunkulu wasesenza umkhathi, wasesehlukanisa phakathi kwamanzi angaphansi komkhathi lamanzi aphezu komkhathi\\x + Hlab. 148.4.\\x*; kwasekusiba njalo.\n"
  "\\v 8 UNkulunkulu wasebiza umkhathi ngokuthi ngamazulu. Kwasekusiba yikuhlwa, njalo kwaba yikusa, usuku lwesibili.\n"
  "\\p\n"
  "\\v 9 UNkulunkulu wasesithi: Kakuqoqelwe amanzi avela ngaphansi kwamazulu ndawonye\\x + Jobe 38.8. Hlab. 33.7. 2 Pet. 3.5.\\x*, njalo kakubonakale umhlabathi owomileyo; kwasekusiba njalo.\n"
  "\\v 10 UNkulunkulu wasebiza umhlabathi owomileyo ngokuthi ngumhlaba, lenhlangano yamanzi wayibiza ngokuthi zinlwandle. UNkulunkulu wasebona ukuthi kuhle.\n"
  "\\v 11 UNkulunkulu wasesithi: Umhlaba kawumilise utshani\\f + \\fk utshani: \\fl Heb. \\fq utshani obubuthakathaka.\\f*, imibhida ethela inhlanyelo, isihlahla sesithelo sisenza isithelo ngohlobo lwaso, onhlanyelo yaso ikuso phezu komhlaba. Kwasekusiba njalo.\n"
  "\\v 12 Umhlaba wasuveza utshani, imibhida ethela inhlanyelo ngohlobo lwayo, lesihlahla sisenza isithelo, esinhlanyelo yaso ikuso ngohlobo lwaso. UNkulunkulu wasebona ukuthi kuhle.\n"
  "\\v 13 Kwasekusiba yikuhlwa, njalo kwaba yikusa, usuku lwesithathu.\n"
  "\\p\n"
  "\\v 14 UNkulunkulu wasesithi: Kakube khona izikhanyiso emkhathini wamazulu, ukuze zehlukanise phakathi kwemini lobusuku\\f + \\fk phakathi kwemini lobusuku: \\fl Heb. \\fq phakathi kwemini laphakathi kobusuku.\\f*; njalo zibe yizibonakaliso lezikhathi ezimisiweyo, lensuku leminyaka.\n"
  "\\v 15 Njalo zibe yizikhanyiso emkhathini wamazulu, ukuze zikhanyise emhlabeni. Kwasekusiba njalo.\n"
  "\\v 16 UNkulunkulu wasesenza izikhanyiso ezimbili ezinkulu\\x + Dute. 4.19. Hlab. 136.7,8,9.\\x*: Isikhanyiso esikhulu ukuze sibuse imini, lesikhanyiso esincinyane ukuze sibuse\\f + \\fk sibuse: \\fl Heb. \\fq kube yikubusa kwe-.\\f* ubusuku, kanye lezinkanyezi.\n"
  "\\v 17 UNkulunkulu wasezimisa emkhathini wamazulu, ukuthi zikhanyise emhlabeni,\n"
  "\\v 18 lokuthi zibuse emini\\x + Jer. 31.35\\x* lebusuku, lokwehlukanisa phakathi kokukhanya lobumnyama. UNkulunkulu wasebona ukuthi kuhle.\n"
  "\\v 19 Kwasekusiba yikuhlwa, njalo kwaba yikusa, usuku lwesine.\n"
  "\\p\n"
  "\\v 20 UNkulunkulu wasesithi: Amanzi kawagcwale ukunyakazela kwemiphefumulo ephilayo, njalo kakuphaphe inyoni phezu komhlaba emkhathini\\f + \\fk emkhathini: \\fl Heb. \\fq ebusweni bomkhathi.\\f* wamazulu.\n"
  "\\v 21 UNkulunkulu wasedala oququmadevu bolwandle abakhulu, laso sonke isidalwa esiphilayo esibhinqikayo, amanzi agcwele sona, ngohlobo lwaso, layo yonke inyoni elempiko, ngohlobo lwayo. UNkulunkulu wasebona ukuthi kuhle.\n"
  "\\v 22 UNkulunkulu wasekubusisa, esithi: Zalani, lande\\x + 8.17; 9.1.\\x*, njalo ligcwalise amanzi enlwandle, lezinyoni kazande emhlabeni.\n"
  "\\v 23 Kwasekusiba yikuhlwa, njalo kwaba yikusa, usuku lwesihlanu.\n"
  "\\p\n"
  "\\v 24 UNkulunkulu wasesithi: Umhlaba kawuveze izidalwa eziphilayo ngohlobo lwazo, izifuyo lokuhuquzelayo lenyamazana zomhlaba ngenhlobo zazo. Kwasekusiba njalo.\n"
  "\\v 25 UNkulunkulu wasesenza izinyamazana zomhlaba ngenhlobo zazo, lezifuyo ngenhlobo zazo, lakho konke okuhuquzelayo komhlaba ngenhlobo zakho. UNkulunkulu wasebona ukuthi kuhle.\n"
  "\\p\n"
  "\\v 26 UNkulunkulu wasesithi: Asenze abantu ngomfanekiso wethu\\x + 5.1; 9.6. 1 Kor. 11.7. Efe. 4.24. Kol. 3.10. Jak. 3.9.\\x*, ngesimo sethu, njalo babuse phezu kwenhlanzi zolwandle, laphezu kwenyoni zamazulu, laphezu kwezifuyo, laphezu komhlaba wonke, laphezu kwakho konke okuhuquzelayo lokho okuhuquzela emhlabeni\\x + 9.2. Hlab. 8.6.\\x*.\n"
  "\\v 27 UNkulunkulu wasemdala umuntu ngomfanekiso wakhe; wamdala ngomfanekiso kaNkulunkulu; owesilisa lowesifazana wabadala\\x + 2.18; 5.2. Mal. 2.15. Mat. 19.4. Mark. 10.6.\\x*.\n"
  "\\v 28 UNkulunkulu wasebabusisa, uNkulunkulu wasesithi kibo: Zalani lande, ligcwalise umhlaba\\x + 9.1,7.\\x* njalo liwehlisele phansi; libuse phezu kwenhlanzi zolwandle, laphezu kwenyoni zamazulu, laphezu kwakho konke okuphilayo okuhuquzela emhlabeni.\n"
  "\\p\n"
  "\\v 29 UNkulunkulu wasesithi: Khangelani, ngilinike yonke imibhida ethela inhlanyelo, esebusweni bomhlaba wonke, laso sonke isihlahla, okukuso isithelo sesihlahla esithela\\f + \\fk esithela: \\fl Heb. \\fq esihlanyela.\\f* inhlanyelo; kini singesokudla\\x + 9.3. Hlab. 145.15,16.\\x*.\n"
  "\\v 30 Njalo kuyo yonke inyamazana yomhlaba, lakuyo yonke inyoni yamazulu, lakukho konke okuhuquzela emhlabeni, okukukho umoya\\f + \\fk umoya: \\fl Heb. \\fq umphefumulo.\\f* ophilayo, lonke uhlaza ngolokudla. Kwasekusiba njalo.\n"
  "\\v 31 UNkulunkulu wasebona konke akwenzileyo; khangela-ke kwakukuhle kakhulu\\x + Tshu. 7.29. 1 Tim. 4.4.\\x*. Kwasekusiba yikuhlwa, njalo kwaba yikusa, usuku lwesithupha.";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, nested_text_mark_1)
{
  std::string input_usfm =
  R"(\p)" "\n"
  R"(\v 2 \add add\+nd addnd\+nd*\add*.)";
  std::string output_usfm =
  R"(\p)" "\n"
  R"(\v 2 \add add\add*\add \+nd addnd\+nd*\add*.)";
  std::string html =
  R"(<p class="b-p"><span class="i-v">2</span><span> </span><span class="i-add">add</span><span class="i-add0nd">addnd</span><span>.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (input_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (output_usfm, output);
}


TEST_F (usfm_html, nested_text_mark_2)
{
  std::string input_usfm =
  R"(\p)" "\n"
  R"(\v 2 \add add\+nd addnd\add*.)";
  std::string output_usfm =
  R"(\p)" "\n"
  R"(\v 2 \add add\add*\add \+nd addnd\+nd*\add*.)";
  std::string html =
  R"(<p class="b-p"><span class="i-v">2</span><span> </span><span class="i-add">add</span><span class="i-add0nd">addnd</span><span>.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (input_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (output_usfm, output);
}


TEST_F (usfm_html, nested_text_mark_3)
{
  std::string input_usfm  = R"(\p The \add \+nd Lord God\+nd* is\add* calling you)";
  std::string output_usfm = R"(\p The \add \+nd Lord God\+nd*\add*\add  is\add* calling you)";
  std::string html =
  R"(<p class="b-p"><span>The </span><span class="i-add0nd">Lord God</span><span class="i-add"> is</span><span> calling you</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (input_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (output_usfm, output);
}


TEST_F (usfm_html, nested_note_markup_1)
{
  std::string standard_usfm =
  R"(\p)" "\n"
  R"(\v 2 text\f + \fk fk \+fdc key-fdc\+fdc*\fk* normal\f*.)";
  std::string html =
  R"(<p class="b-p"><span class="i-v">2</span><span> </span><span>text</span><span class="i-notecall1">1</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fk">fk </span><span class="i-fk0fdc">key-fdc</span><span> normal</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output);
}


TEST_F (usfm_html, nested_note_markup_2)
{
  std::string input_usfm =
  R"(\p)"
  R"(\v 2 text\f + \fk fk \+fdc key-fdc\fk* normal\f*.)";
  std::string output_usfm =
  R"(\p)" "\n"
  R"(\v 2 text\f + \fk fk \+fdc key-fdc\+fdc*\fk* normal\f*.)";
  std::string html =
  R"(<p class="b-p"><span class="i-v">2</span><span> </span><span>text</span><span class="i-notecall1">1</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fk">fk </span><span class="i-fk0fdc">key-fdc</span><span> normal</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (input_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (output_usfm, output);
}


TEST_F (usfm_html, fox_for_change_marker_ft_to_fk)
{
  // A bug was discovered in the Bible editor where "... \fk ... \ft ..." was changed to "... \fk ... \fk ...".
  // The bug was fixed.
  // This test would catch any regression.
  std::string usfm =
  R"(\c 1)" "\n"
  R"(\v 1 Canonical text\f + \fr 1:1 \fk Footnote fk style \ft Footnote ft style\f* canonical text.)";
  std::string html =
  R"(<p class="b-c"><span>1</span><span> </span><span class="i-v">1</span><span> </span><span>Canonical text</span><span class="i-notecall1">1</span><span> canonical text.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fr">1:1 </span><span class="i-fk">Footnote fk style </span><span class="i-ft">Footnote ft style</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (usfm, output);
}


TEST_F (usfm_html, blank_line)
{
  std::string standard_usfm =
  "\\p paragraph\n"
  "\\b\n"
  "\\p paragraph";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  std::string standard_html = R"(<p class="b-p"><span>paragraph</span></p><p class="b-b"><br/></p><p class="b-p"><span>paragraph</span></p>)";
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, test_the_marker_sd_and_sd2_semantic_divisions)
{
  std::string standard_usfm =
  "\\p paragraph\n"
  "\\sd\n"
  "\\p paragraph\n"
  "\\sd2"
  ;
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  std::string standard_html = R"(<p class="b-p"><span>paragraph</span></p><p class="b-sd"><br/></p><p class="b-p"><span>paragraph</span></p><p class="b-sd2"><br/></p>)";
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, test_marker_xo_and_xt)
{
  std::string standardusfm =
  "\\p\n\\v 1 The text\\x + \\xo 1 \\xt Passage\\x*.";
  std::string standardhtml =
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span>The text</span><span class="i-notecall1">a</span><span>.</span></p><p class="b-notes"> </p><p class="b-x"><span class="i-notebody1">a</span><span> </span><span>+ </span><span class="i-xo">1 </span><span class="i-xt">Passage</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standardusfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standardhtml, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standardhtml);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standardusfm, usfm);
}


TEST_F (usfm_html, unmatched_note_opener_and_xref_opener)
{
  std::string standard_usfm =
  R"(\c 117)" "\n"
  R"(\p)" "\n"
  R"(\v 1 Praise Yahweh\f all you nations!)" "\n"
  R"(\v 2 For his loving kindness\x is great toward us.)" "\n";
  std::string standard_html =
  R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span>\f </span><span>all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>For his loving kindness</span><span>\x </span><span>is great toward us.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (standard_usfm), filter::strings::trim (usfm));
}


TEST_F (usfm_html, inline_opener_without_matching_inline_closer)
{
  std::string standard_usfm =
  R"(\p The \add Lord God is calling you)";
  std::string standard_html =
  R"(<p class="b-p"><span>The </span><span>\add </span><span>Lord God is calling you</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (standard_usfm), filter::strings::trim (usfm));
}


// Inline opener without inline closer but with other inline markup.
TEST_F (usfm_html, inline_opener_wo_closer_w_other_inline_markup)
{
  std::string standard_usfm =
  R"(\p The \add Lord \nd God\nd* is calling you)" "\n"
  R"(\v 2 Verse text)";
  std::string standard_html =
  R"(<p class="b-p"><span>The </span><span>\add </span><span>Lord </span><span class="i-nd">God</span><span> is calling you</span><span> </span><span class="i-v">2</span><span> </span><span>Verse text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (standard_usfm), filter::strings::trim (usfm));
}


// Inline opener without a matching inline closer and with a paragraph after that.
TEST_F (usfm_html, inline_opener_wo_closer_w_paragraph_after)
{
  std::string standard_usfm =
  R"(\p The \add Lord God is calling you)" "\n"
  R"(\p Paragraph)";
  std::string standard_html =
  R"(<p class="b-p"><span>The </span><span>\add </span><span>Lord God is calling you</span></p><p class="b-p"><span>Paragraph</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (standard_usfm), filter::strings::trim (usfm));
}


// Testing editing one verse, which does not have a starting paragraph.
TEST_F (usfm_html, verse_wo_starting_paragraph)
{
  std::string usfm = "\\v 1 God created";
  std::string html = R"(<p><span class="i-v">1</span><span> </span><span>God created</span></p>)";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  output = editor_export_verse_quill (styles_logic_standard_sheet (), html);
  EXPECT_EQ (usfm, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string saved_usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (saved_usfm), filter::strings::trim ("\\p\n" + usfm));
}


// Testing verse editing one verse: The chapter number, or verse 0.
TEST_F (usfm_html, verse_ch1_vs0)
{
  std::string usfm = "\\c 1\n"
  "\\p";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  
  std::string html = R"(<p class="b-c"><span>1</span></p><p class="b-p"><br/></p>)";
  EXPECT_EQ (html, output);
  
  output = editor_export_verse_quill (styles_logic_standard_sheet (), html);
  EXPECT_EQ (usfm, output);
}


// One-verse editor, testing chapter 0 verse 0.
TEST_F (usfm_html, verse_editor_ch0_vs0)
{
  std::string usfm =
  "\\id GEN Genesis\n"
  "\\h Genesis\n"
  "\\toc1 The First Book of Moses, called Genesis\n"
  "\\mt1 The First Book of Moses, called Genesis";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  
  std::string html = R"(<p class="b-mono"><span>\id </span><span>GEN Genesis</span></p><p class="b-mono"><span>\h </span><span>Genesis</span></p><p class="b-mono"><span>\toc1 </span><span>The First Book of Moses, called Genesis</span></p><p class="b-mt1"><span>The First Book of Moses, called Genesis</span></p>)";
  EXPECT_EQ (html, output);
  
  output = editor_export_verse_quill (styles_logic_standard_sheet (), html);
  EXPECT_EQ (usfm, output);
}


// Testing one verse: a paragraph with content.
TEST_F (usfm_html, verse_editor_paragraph_w_content)
{
  std::string usfm = R"(\p And God called the firmament Heaven)";
  std::string html = R"(<p class="b-p"><span>And God called the firmament Heaven</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  output = editor_export_verse_quill (styles_logic_standard_sheet (), html);
  EXPECT_EQ (usfm, output);
}


// Testing \add ..\add* markup in a footnote.
TEST_F (usfm_html, add_in_footnote)
{
  std::string standard_usfm = R"(\p Praise Yahweh\f \add I\add* am\f*, all you nations!)";
  std::string standard_html = R"(<p class="b-p"><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span class="i-add">I</span><span> am</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output_html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, output_html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (output_html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output_usfm);
}


// Testing \xt in a footnote.
TEST_F (usfm_html, xt_in_footnote)
{
  std::string standard_usfm = R"(\p Praise Yahweh\f I am, see \xt Exod.6.3.\f*, all you nations!)";
  std::string standard_html = R"(<p class="b-p"><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>I am, see </span><span class="i-xt">Exod.6.3.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output_html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, output_html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (output_html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output_usfm);
}


// Testing \xt and \add markup in a footnote, in Romans 2.15, received from a user.
TEST_F (usfm_html, xt_add_in_footnote)
{
  std::string standard_usfm = R"(\p \f + \fr 2:15 \ft „tokie“ – t. „kurie“\f*tokie parodo savo širdyse įrašytą įstatymo \f + \fr 2:15 \ft „darbą“ – arba „poveikį“\f*darbą, jų sąžinei kartu \add tiems dalykams\add* paliudijant, ir \add jų\add* mintims \f + \fr 2:15 \ft „tuo tarpu \add juos\add* kaltinant arba net ginant“ – gr. „tarp savęs“; gal „tarpusavyje“, t. y. arba minčių tarpusavyje arba kitataučių tarpusavyje; gal „pakeičiant viena kitą \add juos\add* kaltindamos arba net gindamos“; žr. - \xt Mt 18:15, kur kalbama ne apie laiko tarpsnį, bet apie žodžių keitimąsi tarp du žmones\f*tuo tarpu \add juos\add* kaltinant arba net ginant) –)";
  std::string standard_html = R"(<p class="b-p"><span class="i-notecall1">1</span><span>tokie parodo savo širdyse įrašytą įstatymo </span><span class="i-notecall2">2</span><span>darbą, jų sąžinei kartu </span><span class="i-add">tiems dalykams</span><span> paliudijant, ir </span><span class="i-add">jų</span><span> mintims </span><span class="i-notecall3">3</span><span>tuo tarpu </span><span class="i-add">juos</span><span> kaltinant arba net ginant) –</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fr">2:15 </span><span class="i-ft">„tokie“ – t. „kurie“</span></p><p class="b-f"><span class="i-notebody2">2</span><span> </span><span>+ </span><span class="i-fr">2:15 </span><span class="i-ft">„darbą“ – arba „poveikį“</span></p><p class="b-f"><span class="i-notebody3">3</span><span> </span><span>+ </span><span class="i-fr">2:15 </span><span class="i-ft">„tuo tarpu </span><span class="i-add">juos</span><span> kaltinant arba net ginant“ – gr. „tarp savęs“; gal „tarpusavyje“, t. y. arba minčių tarpusavyje arba kitataučių tarpusavyje; gal „pakeičiant viena kitą </span><span class="i-add">juos</span><span> kaltindamos arba net gindamos“; žr. - </span><span class="i-xt">Mt 18:15, kur kalbama ne apie laiko tarpsnį, bet apie žodžių keitimąsi tarp du žmones</span></p>)";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string output_html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, output_html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (output_html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output_usfm);
}


// Regression testing for a situation that a user pastes some text into a note.
// https://github.com/bibledit/cloud/issues/353
TEST_F (usfm_html, regression_paste_text_in_note)
{
  std::string standard_usfm_long = R"(\p Verse text one\f + \fr 1:4 \ft Note text \ft one.\f* two.)";
  std::string standard_usfm_short = R"(\p Verse text one\f + \fr 1:4 \ft Note text one.\f* two.)";
  std::string standard_html = R"(<p class="b-p">Verse text one<span class="i-notecall1">1</span> two.</p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-fr">1:4 </span><span class="i-ft">Note </span>text&nbsp;<span class="i-ft">one.</span></p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm_short, output_usfm);
}


// Regression testing for a fixed bug where after entering a new line in a footnote,
// the part of the footnote text after the new line would disappear.
// https://github.com/bibledit/cloud/issues/444
TEST_F (usfm_html, regression_new_line_footnote)
{
  std::string standard_html = R"(<p class="b-p"><span class="i-v">1</span> One<span class="i-notecall1">1</span> two.</p><p class="b-notes">&nbsp;</p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-fr">117.3 </span><span class="i-fk">| key </span></p><p class="b-f"><span class="i-fk">word</span></p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
}


// Test text length for one verse.
TEST_F (usfm_html, text_length_one_verse)
{
  std::string usfm =
  "\\c 2\n"
  "\\p\n"
  "\\v 1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho\\x + Dute. 4.19. Hlab. 33.6.\\x*.\n";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (61, static_cast<int>(editor_usfm2html.m_text_tength));
  EXPECT_EQ ((std::map <int, int>{ std::pair (0, 0), std::pair (1, 2) }), editor_usfm2html.m_verse_start_offsets);
  EXPECT_EQ ("1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho.", editor_usfm2html.m_current_paragraph_content);
}


// Test text length for several verses.
TEST_F (usfm_html, text_length_verses)
{
  std::string usfm =
  "\\c 2\n"
  "\\p\n"
  "\\v 1 Kwasekuqediswa amazulu lomhlaba lalo lonke ibutho lakho\\x + Dute. 4.19. Hlab. 33.6.\\x*.\n"
  "\\v 2 UNkulunkulu wasewuqeda ngosuku lwesikhombisa umsebenzi wakhe abewenza. Waphumula ngosuku lwesikhombisa\\x + Eks. 20.11; 31.17. Dute. 5.14. Heb. 4.4.\\x* emsebenzini wakhe wonke abewenza.\n"
  "\\v 3 UNkulunkulu wasebusisa usuku lwesikhombisa, walungcwelisa; ngoba ngalo waphumula emsebenzini wakhe wonke, uNkulunkulu awudalayo wawenza\\f + \\fk wawenza: \\fl Heb. \\fq ukuwenza.\\f*.\n"
  "\\s Isivande seEdeni\n"
  "\\p\n"
  "\\v 4 Lezi yizizukulwana zamazulu lezomhlaba ekudalweni kwakho\\x + 1.1.\\x*, mhla iN\\nd kosi\\nd* uNkulunkulu isenza umhlaba lamazulu,\n"
  "\\v 5 laso sonke isihlahlakazana sensimu, singakabi khona emhlabeni, layo yonke imibhida yeganga\\x + 1.12.\\x*, ingakamili; ngoba iN\\nd kosi\\nd* uNkulunkulu yayinganisanga izulu emhlabeni, njalo kwakungelamuntu wokulima umhlabathi.\n"
  "\\v 6 Kodwa kwenyuka inkungu ivela emhlabathini, yasithelela ubuso bonke bomhlabathi.\n"
  "\\v 7 IN\\nd kosi\\nd* uNkulunkulu yasibumba umuntu ngothuli oluvela emhlabathini\\x + 3.19,23. Hlab. 103.14. Tshu. 12.7. 1 Kor. 15.47.\\x*, yaphefumulela emakhaleni akhe umoya wempilo; umuntu wasesiba ngumphefumulo ophilayo\\x + 7.22. Jobe 33.4. Isa. 2.22. 1 Kor. 15.45.\\x*.\n";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (913, static_cast<int>(editor_usfm2html.m_text_tength));
  EXPECT_EQ ((std::map <int, int>{
    std::pair (0, 0),
    std::pair (1, 2),
    std::pair (2, 62),
    std::pair (3, 202),
    std::pair (4, 359),
    std::pair (5, 469),
    std::pair (6, 676),
    std::pair (7, 758)
  }),
             editor_usfm2html.m_verse_start_offsets);
  EXPECT_EQ (550, static_cast<int>(editor_usfm2html.m_current_paragraph_content.size ()));
}


// Space after starting marker.
TEST_F (usfm_html, space_after_opener)
{
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 2 Text \\add of the \\add*1st\\add  second verse\\add*.\n";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  const std::string standard =
  R"(<p class="b-c"><span>1</span></p><p class="b-p"><span class="i-v">2</span><span> </span><span>Text </span><span class="i-add">of the </span><span>1st</span><span class="i-add"> second verse</span><span>.</span></p>)";
  EXPECT_EQ (standard, html);
}


// Apostrophy etc.
TEST_F (usfm_html, apostrophy_etc)
{
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 Judha muranda waJesu Kristu, uye munin'ina waJakobho ...\n";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  const std::string standard =
  R"(<p class="b-c"><span>1</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Judha muranda waJesu Kristu, uye munin'ina waJakobho ...</span></p>)";
  EXPECT_EQ (standard, html);
}


// Most recent paragraph style.
TEST_F (usfm_html, most_recent_paragraph_style)
{
  const std::string usfm =
  "\\c 2\n"
  "\\p\n"
  "\\v 1 One\n"
  "\\q2\n"
  "\\v 2 Two\n"
  "\\v 3 Three\n";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ ("q2", editor_usfm2html.m_current_paragraph_style);
  EXPECT_EQ ("2 Two 3 Three", editor_usfm2html.m_current_paragraph_content);
}


// Most recent paragraph style and length 0.
TEST_F (usfm_html, most_recent_p_style_length_0)
{
  const std::string usfm =
  "\\c 2\n"
  "\\p\n"
  "\\v 1 One\n"
  "\\q2\n"
  "\\v 2 Two\n"
  "\\v 3 Three\n"
  "\\q3\n";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ ("q3", editor_usfm2html.m_current_paragraph_style);
  EXPECT_EQ ("", editor_usfm2html.m_current_paragraph_content);
}


// Convert styles for Quill-based editor.
TEST_F (usfm_html, convert_styles_to_quill)
{
  const std::string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 2 Text \\add of the \\add*1st \\add second verse\\add*.\n";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  const std::string standard =
  R"(<p class="b-c"><span>1</span></p>)"
  R"(<p class="b-p"><span class="i-v">2</span><span> </span><span>Text </span><span class="i-add">of the </span><span>1st </span><span class="i-add">second verse</span><span>.</span></p>)";
  EXPECT_EQ (standard, html);
}


// Basic test.
TEST_F (usfm_html, basic)
{
  std::string html = "<p class=\"p\"><span>The earth brought forth.</span></p>";
  std::string standard = "\\p The earth brought forth.";
  {
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }
  // The Quill library uses <span> only when needed, so remove them for testing.
  html = filter::strings::replace ("<span>", "", html);
  html = filter::strings::replace ("</span>", "", html);
  {
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }
}


// Non-breaking spaces
TEST_F (usfm_html, non_breaking_spaces)
{
  std::string html = R"(<p class="p"><span>The&nbsp;earth &nbsp; brought&nbsp;&nbsp;forth.</span></p>)";
  const std::string standard = R"(\p The earth   brought  forth.)";
  // Test Quill-based editor.
  html = filter::strings::replace ("<span>", "", html);
  html = filter::strings::replace ("</span>", "", html);
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
}


// Test embedded <span> elements.
TEST_F (usfm_html, embedded_span_elements)
{
  std::string standard = R"(\p The \add \+nd Lord God\+nd* is calling\add* you.)";
  std::string html = R"(<p class="p">The <span class="add"><span class="nd">Lord God</span> is calling</span> you.</p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
}


TEST_F (usfm_html, basic_note)
{
  std::string standard = R"(\p The earth brought forth\x + 2 Joh. 1.1\x*.)";
  std::string html =
  R"(<p class="b-p"><span>The earth brought forth</span><span class="i-notecall1">x</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  "<br/>"
  "</p>"
  R"(<p class="b-x"><span class="i-notebody1">x</span><span> </span><span>+ 2 Joh. 1.1</span></p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
}


// Footnote with its body deleted.
TEST_F (usfm_html, footnote_body_deleted)
{
  std::string standard = R"(\p The earth brought forth.)";
  std::string html =
  R"(<p class="b-p"><span>The earth brought forth</span><span class="i-notecall1">f</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  R"(<p class="b-f"></p>)"
  " "
  "</p>";
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
  
  // Clear message from logbook.
  refresh_sandbox (false);
}


// Footnote with a deleted citation.
TEST_F (usfm_html, footnote_deleted_citation)
{
  const std::string standard = R"(\p The earth brought forth.)";
  const std::string html =
  R"(<p class="b-p"><span>The earth brought forth</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-f"><span class="i-notebody1">f</span><span> </span><span>+ </span><span class="i-fk">brought: </span><span class="i-fl">Heb. </span><span class="i-fq">explanation.</span></p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
}


// Two words with character markup in sequence.
// The converter used to take out the space between the two words.
// This tests that it does not do that.
TEST_F (usfm_html, word_sequence_with_character_markup)
{
  const std::string standard = R"(\p Praise \add Yahweh\add* \add all\add* you nations!)";
  const std::string html = R"(<p class="b-p"><span>Praise </span><span class="i-add">Yahweh</span><span> <span class="i-add">all</span> you nations!</span></p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
}


// Regression test for issue https://github.com/bibledit/cloud/issues/444.
// If a new line is given within the footnote,
// the footnote should still remain okay,
// when converted back to USFM.
TEST_F (usfm_html, new_line_in_footnote)
{
  {
    const std::string standard_usfm = R"(\p text\f + \ft footnote\f*)";
    const std::string standard_html = R"(<p class="b-p">text<span class="i-notecall1">1</span></p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">foot</span></p><p class="b-f"><span class="i-ft">note</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (standard_html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    const std::string output_usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, output_usfm);
  }
  {
    const std::string standard_usfm = R"(\p text1\f + \ft note1\f* text2\f + \fk keyword2\ft text2\f* text3\f + note3\f*)";
    const std::string standard_html = R"(<p class="b-p">text1<span class="i-notecall1">1</span> text2<span class="i-notecall2">2</span> text3<span class="i-notecall3">3</span></p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">note1</span></p><p class="b-f"><span class="i-notebody2">2</span> + <span class="i-fk">key</span></p><p class="b-f"><span class="i-fk">word2</span><span class="i-ft">text2</span></p><p class="b-f"><span class="i-notebody3">3</span> + <span class="i-ft">note3</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (standard_html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    const std::string output_usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, output_usfm);
  }
}


// Test that the converter to USFM removes the Quill caret class.
TEST_F (usfm_html, remove_quill_caret_class)
{
  std::string standard_html = R"(<p class="b-p">Text<span class="i-notecall1">1</span>.</p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">Text of </span><span class="i-ft"><span class="ql-cursor">﻿</span>the note</span></p>)";
  std::string standard_usfm = R"(\p Text\f + \ft Text of the note\f*.)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output_usfm);
}


// Text plus note but with some unexpected character before the notebody span.
// The user could have typed this unexpected character.
TEST_F (usfm_html, unexpected_char_before_note_body)
{
  std::string standard = R"(
\p
\v 1 Line\f  + \fr 117.1\fk  keyword\ft  Text.\f* one.
)";
  standard = filter::strings::ltrim(standard);
  std::string html = R"(
    <p class="b-p"><span class="i-v">1</span> Line<span class="i-notecall1">1</span> one.</p>
    <p class="b-f">&nbsp;<span class="i-notebody1">1</span> + <span class="i-fr">117.1</span><span class="i-fk"> keyword</span><span class="i-ft"> Text.</span></p>
    )";
  html = filter::strings::trim(html);
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
}


// Text plus note but with a deleted notes separator.
TEST_F (usfm_html, deleted_note_separator)
{
  std::string standard = R"(\p Body\x + \xo 117.1 \xt Note\x*)";
  standard.append ("\n");
  std::string html = R"(
    <p class="b-p">Body<span class="i-notecall33" /></p>
    <p class="b-x"><span class="i-notebody33">1</span> + <span class="i-xo">117.1</span> <span class="i-xt">Note</span></p>
    )";
  html = filter::strings::trim(html);
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard, usfm);
}


// Check that it only collapses three spaces into two.
// And that it does not collapse two spaces into one.
TEST_F (usfm_html, limited_spaces_collapse)
{
  std::string html = R"(<p class="p"><span>The   earth  brought    forth.</span></p>)";
  const std::string standard = R"(\p The   earth  brought    forth.)";
  {
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    const std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }
  // The Quill library uses <span> only when needed, so remove them for testing.
  html = filter::strings::replace ("<span>", "", html);
  html = filter::strings::replace ("</span>", "", html);
  {
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    const std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }
}


TEST_F (usfm_html, get_word_level_attributes_id)
{
  // Test that the word-level attributes ID never contains any "0" as part of the ID.
  constexpr const int id_count {1100};
  std::string usfm{};
  for (int i {0}; i < id_count; i++) {
    usfm.append(R"(\w A|B\w*)");
  }
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get();
  int count {};
  do {
    constexpr const std::string_view b_wla{"b-wla"};
    const size_t pos = html.find(b_wla);
    if (pos == std::string::npos)
      break;
    html.erase(0, pos + b_wla.size());
    count++;
    EXPECT_TRUE(html.substr(0, 10).find("0") == std::string::npos);
  }
  while (true);
  EXPECT_EQ(count, id_count);
}


TEST_F (usfm_html, word_level_attributes_basic)
{
  // This tests one basic word-level attribute.
  const std::string usfm = R"(\p A\w B|C="D"\w*)";
  const std::string html = R"(<p class="b-p"><span>A</span><span class="i-w0wla1">B</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">C="D"</p>)";

  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (usfm, editor_html2usfm.get());
}


TEST_F (usfm_html, word_level_attributes_strong)
{
  // This tests Strong attributes.
  // USFM 3 writes:
  // In cases where more than one value should be provided for an attribute key,
  // the author should provide a comma separated list within the value string.
  // Leading and trailing space characters adjacent to the comma separators are ignored.
  // Implementation comment: The extra spaces are ignored by Bibledit,
  // because Bibledit loads and saves the whole attributes string after the vertical bar as-is.
  const std::string usfm = R"(\p A \w B|strong="H1234"\w* \w C|strong="H2345"\w* D \w E|strong="H3456"\w* \w F|strong="H4567,H5678"\w* G)";
  const std::string html = R"(<p class="b-p"><span>A </span><span class="i-w0wla1">B</span><span> </span><span class="i-w0wla2">C</span><span> D </span><span class="i-w0wla3">E</span><span> </span><span class="i-w0wla4">F</span><span> G</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">strong="H1234"</p><p class="b-wla2">strong="H2345"</p><p class="b-wla3">strong="H3456"</p><p class="b-wla4">strong="H4567,H5678"</p>)";

  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (usfm, editor_html2usfm.get ());
}


TEST_F (usfm_html, word_level_attributes_ruby_gloss)
{
  // This tests Ruby glosses.
  // USFM 3 writes:
  // Following the syntax for word level attributes.
  // Use a colon : to separate multiple elements for glossing each of the characters within a base text phrase.
  // Parts of a phrase gloss may be left empty.
  // In the USFM below, one ruby gloss, the second and fourth base characters are unglossed.
  const std::string usfm = R"(\p A \rb BB|gloss="gg:gg"\rb* C \rb DD|"gg:gg"\rb* E \rb FFFF|g1::g3:\rb* One Han character with a single ruby gloss:\rb 哀|あい\rb*. Two Han characters with a single ruby phrase gloss: \rb 話賄|はなはなし\rb*. Phrase gloss broken down into individual pieces by adding colons between ruby characters: \rb 話賄|はな:はなし\rb*. A character sequence which includes non-Han characters (hiragana), which are not glossed: \rb 定ま|さだ:\rb*. An un-glossed character occurring between glossed characters in the “phrase”: \rb 神の子|かみ::こ\rb*.)";
  const std::string html = R"(<p class="b-p"><span>A </span><span class="i-rb0wla1">BB</span><span> C </span><span class="i-rb0wla2">DD</span><span> E </span><span class="i-rb0wla3">FFFF</span><span> One Han character with a single ruby gloss:</span><span class="i-rb0wla4">哀</span><span>. Two Han characters with a single ruby phrase gloss: </span><span class="i-rb0wla5">話賄</span><span>. Phrase gloss broken down into individual pieces by adding colons between ruby characters: </span><span class="i-rb0wla6">話賄</span><span>. A character sequence which includes non-Han characters (hiragana), which are not glossed: </span><span class="i-rb0wla7">定ま</span><span>. An un-glossed character occurring between glossed characters in the “phrase”: </span><span class="i-rb0wla8">神の子</span><span>.</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">gloss="gg:gg"</p><p class="b-wla2">"gg:gg"</p><p class="b-wla3">g1::g3:</p><p class="b-wla4">あい</p><p class="b-wla5">はなはなし</p><p class="b-wla6">はな:はなし</p><p class="b-wla7">さだ:</p><p class="b-wla8">かみ::こ</p>)";

  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (usfm, editor_html2usfm.get ());
}


TEST_F (usfm_html, word_level_attributes_default)
{
  // This tests default attributes.
  // USFM 3 writes:
  // The unidentified attribute is acceptable because lemma is defined as the default attribute for \w ...\w*.
  const std::string usfm = R"(\p A \w B|lemma="C"\w* D\w B|C\w* E.)";
  const std::string html = R"(<p class="b-p"><span>A </span><span class="i-w0wla1">B</span><span> D</span><span class="i-w0wla2">B</span><span> E.</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">lemma="C"</p><p class="b-wla2">C</p>)";

  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (usfm, editor_html2usfm.get ());
}


TEST_F (usfm_html, word_level_attributes_multiple)
{
  // This tests multiple attributes.
  // USFM 3 writes:
  // The unidentified attribute is acceptable because lemma is defined as the default attribute for \w ...\w*.
  const std::string usfm = R"(\p Text with multiple attributes: \w gracious|lemma="grace" x-myattr="metadata"\w*.)";
  const std::string html = R"(<p class="b-p"><span>Text with multiple attributes: </span><span class="i-w0wla1">gracious</span><span>.</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">lemma="grace" x-myattr="metadata"</p>)";

  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (usfm, editor_html2usfm.get ());
}


TEST_F (usfm_html, word_level_attributes_linking)
{
  // This tests the linking attributes.
  // USFM 3 writes:
  // The \xt ...\xt provides the linking attribute link-href as a default attribute.
  // Note that the word-level attribute "GEN 2:2" will not be separated from the cross reference,
  // as this was output as-is.
  const std::string usfm = R"(
\c 2
\cd \xt 1|GEN 2:1\xt* A \xt 8|GEN 2:8\xt* B \xt 18|GEN 2:18\xt* C \xt 21|GEN 2:21\xt* E
\p
\v 1 Verse one.
\v 2 Verse two, cross reference\x - \xt 1|GEN 2:2\xt*\x* with a link reference.
)";
  const std::string html = R"(<p class="b-c"><span>2</span></p><p class="b-cd"><span class="i-xt0wla1">1</span><span> A </span><span class="i-xt0wla2">8</span><span> B </span><span class="i-xt0wla3">18</span><span> C </span><span class="i-xt0wla4">21</span><span> E</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Verse one.</span><span> </span><span class="i-v">2</span><span> </span><span>Verse two, cross reference</span><span class="i-notecall1">a</span><span> with a link reference.</span></p><p class="b-notes"> </p><p class="b-x"><span class="i-notebody1">a</span><span> </span><span>- </span><span class="i-xt">1|GEN 2:2</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">GEN 2:1</p><p class="b-wla2">GEN 2:8</p><p class="b-wla3">GEN 2:18</p><p class="b-wla4">GEN 2:21</p>)";

  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (filter::strings::trim(usfm), editor_html2usfm.get ());
}


TEST_F (usfm_html, word_level_attributes_removed)
{
  // This tests a situation where the \w ... is removed from the text body,
  // but the word-level attributes themselves are still in the XML tree.
  const std::string html = R"(<p class="b-p"><span>A</span><span class="i-w0wla2">B</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">C="D"</p>)";
  const std::string usfm = R"(\p A\w B\w*)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (usfm, editor_html2usfm.get());
}


TEST_F (usfm_html, word_level_attributes_with_notes)
{
  // This tests the linking attributes.
  // USFM 3 writes:
  // The \xt ...\xt provides the linking attribute link-href as a default attribute.
  // Note that the word-level attribute "GEN 2:2" will not be separated from the cross reference,
  // as this was output as-is.
  const std::string usfm = R"(
\p
\v 2 And the \w Lord|strong="Strong"\w* said to me\x + \xo 1:2 \xt note text.\x*:
)";
  const std::string html = R"(<p class="b-p"><span class="i-v">2</span><span> </span><span>And the </span><span class="i-w0wla1">Lord</span><span> said to me</span><span class="i-notecall1">a</span><span>:</span></p><p class="b-notes"> </p><p class="b-x"><span class="i-notebody1">a</span><span> </span><span>+ </span><span class="i-xo">1:2 </span><span class="i-xt">note text.</span></p><p class="b-wordlevelattributes"> </p><p class="b-wla1">strong="Strong"</p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (filter::strings::trim(usfm), editor_html2usfm.get ());
}


#endif
