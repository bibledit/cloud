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
#include <filter/diff.h>
#include <filter/quill.h>
#include <database/state.h>
#include <webserver/request.h>
#include <edit/logic.h>
#include "usfm.h"


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
  virtual void SetUp() override
  {
    Database_State::create ();
  }
  virtual void TearDown() override
  {
  }
};


TEST_F (usfm_html, one_unknown_opening_marker)
{
  constexpr const auto standard_usfm = R"(\abc)";
  constexpr const auto standard_html = R"(<p class="b-mono"><span>\abc </span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, filter::strings::trim (html));
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, filter::strings::trim (usfm));
}


TEST_F (usfm_html, two_unknown_opening_markers)
{
  constexpr const auto standard_usfm =
  R"(\abc)" "\n"
  R"(\abc)";
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\abc </span></p>)"
  R"(<p class="b-mono"><span>\abc </span></p>)";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, one_unknown_closing_marker)
{
  constexpr const auto standard_usfm = R"(\abc text\abc*.)";
  constexpr const auto standard_html = R"(<p class="b-mono"><span>\abc </span><span>text</span><span>\abc*</span><span>.</span></p>)";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, two_unknown_closing_markers)
{
  constexpr const auto standard_usfm =
  R"(\abc text\abc*.)" "\n"
  R"(\abc text\abc*.)";
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\abc </span><span>text</span><span>\abc*</span><span>.</span></p>)"
  R"(<p class="b-mono"><span>\abc </span><span>text</span><span>\abc*</span><span>.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, identifiers)
{
  constexpr const auto standard_usfm =
  R"(\id GEN)" "\n"
  R"(\h Header)" "\n"
  R"(\toc1 The Book of Genesis)" "\n"
  R"(\cl Chapter)" "\n"
  R"(\cp ②)" "\n"
  R"(\cp Ⅰ)";
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\id </span><span>GEN</span></p>)"
  R"(<p class="b-mono"><span>\h </span><span>Header</span></p>)"
  R"(<p class="b-mono"><span>\toc1 </span><span>The Book of Genesis</span></p>)"
  R"(<p class="b-mono"><span>\cl </span><span>Chapter</span></p>)"
  R"(<p class="b-mono"><span>\cp </span><span>②</span></p>)"
  R"(<p class="b-mono"><span>\cp </span><span>Ⅰ</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, one_paragraph)
{
  constexpr const auto standard_usfm = R"(\p Paragraph text.)";
  constexpr const auto standard_html = R"(<p class="b-p"><span>Paragraph text.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, two_paragraphs)
{
  constexpr const auto standard_usfm =
  R"(\p Paragraph text.)" "\n"
  R"(\p Paragraph txt.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>Paragraph text.</span></p>)"
  R"(<p class="b-p"><span>Paragraph txt.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, inline_text)
{
  constexpr const auto standard_usfm = R"(\p Paragraph text plus \add added\add* text.)";
  constexpr const auto standard_html = R"(<p class="b-p"><span>Paragraph text plus </span><span class="i-add">added</span><span> text.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, inline_texts)
{
  constexpr const auto standard_usfm = R"(\p Paragraph text plus \add added\add* text plus \add added\add* text.)";
  constexpr const auto standard_html = R"(<p class="b-p"><span>Paragraph text plus </span><span class="i-add">added</span><span> text plus </span><span class="i-add">added</span><span> text.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, chapter)
{
  constexpr const auto standard_usfm =
  R"(\c 1)" "\n"
  R"(\p Paragraph.)";
  
  constexpr const auto standard_html =
  R"(<p class="b-c"><span>1</span></p>)"
  R"(<p class="b-p"><span>Paragraph.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, alternative_chapter)
{
  constexpr const auto standard_usfm =
  R"(\c 13)" "\n"
  R"(\ca 14\ca*)" "\n"
  R"(\p)" "\n"
  R"(\v 1 Text)";
  
  constexpr const auto standard_html = R"(<p class="b-c"><span>13</span></p><p class="b-mono"><span>\ca </span><span>14</span><span>\ca*</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, verses)
{
  constexpr const auto standard_usfm =
  R"(\p)" "\n"
  R"(\v 1 One.)" "\n"
  R"(\v 2 Two.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span>One.</span><span> </span><span class="i-v">2</span><span> </span><span>Two.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  constexpr const auto standard_html = R"(
<p class="b-p"><span class="i-v">1</span><span> </span><span class="i-vp">A</span><span> One.</span><span> </span><span class="i-v">2</span><span> </span><span class="i-vp">B</span><span> Two.</span></p>
)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (filter::strings::trim (standard_html), html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, peripherals)
{
  constexpr const auto standard_usfm =
  "\\periph Title Page\n"
  "\\periph Publication Data";
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\periph </span><span>Title Page</span></p>)"
  R"(<p class="b-mono"><span>\periph </span><span>Publication Data</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, picture)
{
  constexpr const auto standard_usfm =
  "\\p Text\n"
  "\\fig DESC|FILE|SIZE|LOC|COPY|CAP|REF\\fig*\n"
  "\\p Text";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>Text</span></p>)"
  R"(<p class="b-mono"><span>\fig </span><span>DESC|FILE|SIZE|LOC|COPY|CAP|REF</span><span>\fig*</span></p>)"
  R"(<p class="b-p"><span>Text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, table)
{
  constexpr const auto standard_usfm =
  "\\tr \\th1 Tribe \\th2 Leader \\thr3 Number\n"
  "\\tr \\tc1 Reuben \\tc2 Elizur son of Shedeur \\tcr3 46500\n"
  "\\tr \\tc1 Simeon \\tc2 Shelumiel son of Zurishaddai \\tcr3 59300\n"
  "\\tr \\tc1 Gad \\tc2 Eliasaph son of Reuel \\tcr3 45650\n"
  "\\tr \\tc1 \\tcr2 Total: \\tcr3 151450";
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\tr </span><span class="i-th1">Tribe </span><span class="i-th2">Leader </span><span class="i-thr3">Number</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tc1">Reuben </span><span class="i-tc2">Elizur son of Shedeur </span><span class="i-tcr3">46500</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tc1">Simeon </span><span class="i-tc2">Shelumiel son of Zurishaddai </span><span class="i-tcr3">59300</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tc1">Gad </span><span class="i-tc2">Eliasaph son of Reuel </span><span class="i-tcr3">45650</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tcr2">Total: </span><span class="i-tcr3">151450</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  const auto standard_usfm2 = filter::strings::replace ("\\tc1 \\", "\\", standard_usfm);
  EXPECT_EQ (standard_usfm2, usfm);
}


TEST_F (usfm_html, word_list_entry)
{
  constexpr const auto standard_usfm = R"(\p A \ndx index\ndx* b \wh Hebrew\wh* c.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>A </span><span class="i-ndx">index</span><span> b </span><span class="i-wh">Hebrew</span><span> c.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, crossreference)
{
  constexpr const auto standard_usfm = R"(\p The elder\x + 2 Joh. 1.1\x* to the beloved Gaius.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>The elder</span><span class="i-notecall1">a</span><span> to the beloved Gaius.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-x"><span class="i-notebody1">a</span><span> </span><span>+ 2 Joh. 1.1</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, crossreferences)
{
  constexpr const auto standard_usfm =
  R"(\p The elder\x + 2 Joh. 1.1\x* to the beloved Gaius.)" "\n"
  R"(\v 1 The elders\x + 2 Joh. 2.2\x* to the beloved.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>The elder</span><span class="i-notecall1">a</span><span> to the beloved Gaius.</span><span> </span><span class="i-v">1</span><span> </span><span>The elders</span><span class="i-notecall2">b</span><span> to the beloved.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-x"><span class="i-notebody1">a</span><span> </span><span>+ 2 Joh. 1.1</span></p>)"
  R"(<p class="b-x"><span class="i-notebody2">b</span><span> </span><span>+ 2 Joh. 2.2</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, footnote)
{
  constexpr const auto standard_usfm =
  R"(\p The earth brought forth\f + \fk brought: \fl Heb. \fq explanation.\f*.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>The earth brought forth</span><span class="i-notecall1">1</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fk">brought: </span><span class="i-fl">Heb. </span><span class="i-fq">explanation.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, cycling_the_note_caller)
{
  constexpr const auto standard_usfm =
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
  const auto standard_html = filter_url_file_get_contents (filter_url_create_root_path ({"unittests", "tests", "editor_roundtrip_1.txt"}));
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, endnote)
{
  constexpr const auto standard_usfm =
  R"(\p The earth brought forth\fe + \fk brought: \fl Heb. \fq explanation.\fe*.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>The earth brought forth</span><span class="i-notecall1">1</span><span>.</span></p>)"
  R"(<p class="b-notes">)"
  " "
  "</p>"
  R"(<p class="b-fe"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fk">brought: </span><span class="i-fl">Heb. </span><span class="i-fq">explanation.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, round_trip_from_real_life_1)
{
  constexpr const auto standard_usfm =
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, round_trip_from_real_life_2)
{
  constexpr const auto standard_usfm =
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (output_usfm, output);
}


TEST_F (usfm_html, nested_note_markup_1)
{
  constexpr const auto standard_usfm =
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  output = editor_html2usfm.get ();
  EXPECT_EQ (usfm, output);
}


TEST_F (usfm_html, blank_line)
{
  constexpr const auto standard_usfm =
  "\\p paragraph\n"
  "\\b\n"
  "\\p paragraph";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  constexpr const auto standard_html = R"(<p class="b-p"><span>paragraph</span></p><p class="b-b"><br/></p><p class="b-p"><span>paragraph</span></p>)";
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, test_the_marker_sd_and_sd2_semantic_divisions)
{
  constexpr const auto standard_usfm =
  "\\p paragraph\n"
  "\\sd\n"
  "\\p paragraph\n"
  "\\sd2"
  ;
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  constexpr const auto standard_html = R"(<p class="b-p"><span>paragraph</span></p><p class="b-sd"><br/></p><p class="b-p"><span>paragraph</span></p><p class="b-sd2"><br/></p>)";
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standardhtml, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standardhtml);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standardusfm, usfm);
}


TEST_F (usfm_html, unmatched_note_opener_and_xref_opener)
{
  constexpr const auto standard_usfm =
  R"(\c 117)" "\n"
  R"(\p)" "\n"
  R"(\v 1 Praise Yahweh\f all you nations!)" "\n"
  R"(\v 2 For his loving kindness\x is great toward us.)" "\n";
  constexpr const auto standard_html =
  R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span>\f </span><span>all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>For his loving kindness</span><span>\x </span><span>is great toward us.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (standard_usfm), filter::strings::trim (usfm));
}


TEST_F (usfm_html, inline_opener_without_matching_inline_closer)
{
  constexpr const auto standard_usfm =
  R"(\p The \add Lord God is calling you)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>The </span><span>\add </span><span>Lord God is calling you</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (standard_usfm), filter::strings::trim (usfm));
}


// Inline opener without inline closer but with other inline markup.
TEST_F (usfm_html, inline_opener_wo_closer_w_other_inline_markup)
{
  constexpr const auto standard_usfm =
  R"(\p The \add Lord \nd God\nd* is calling you)" "\n"
  R"(\v 2 Verse text)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>The </span><span>\add </span><span>Lord </span><span class="i-nd">God</span><span> is calling you</span><span> </span><span class="i-v">2</span><span> </span><span>Verse text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (filter::strings::trim (standard_usfm), filter::strings::trim (usfm));
}


// Inline opener without a matching inline closer and with a paragraph after that.
TEST_F (usfm_html, inline_opener_wo_closer_w_paragraph_after)
{
  constexpr const auto standard_usfm =
  R"(\p The \add Lord God is calling you)" "\n"
  R"(\p Paragraph)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>The </span><span>\add </span><span>Lord God is calling you</span></p><p class="b-p"><span>Paragraph</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  output = editor_export_verse_quill (stylesv2::standard_sheet (), html);
  EXPECT_EQ (usfm, output);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  
  std::string html = R"(<p class="b-c"><span>1</span></p><p class="b-p"><br/></p>)";
  EXPECT_EQ (html, output);
  
  output = editor_export_verse_quill (stylesv2::standard_sheet (), html);
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  
  std::string html = R"(<p class="b-mono"><span>\id </span><span>GEN Genesis</span></p><p class="b-mono"><span>\h </span><span>Genesis</span></p><p class="b-mono"><span>\toc1 </span><span>The First Book of Moses, called Genesis</span></p><p class="b-mt1"><span>The First Book of Moses, called Genesis</span></p>)";
  EXPECT_EQ (html, output);
  
  output = editor_export_verse_quill (stylesv2::standard_sheet (), html);
  EXPECT_EQ (usfm, output);
}


// Testing one verse: a paragraph with content.
TEST_F (usfm_html, verse_editor_paragraph_w_content)
{
  std::string usfm = R"(\p And God called the firmament Heaven)";
  std::string html = R"(<p class="b-p"><span>And God called the firmament Heaven</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output = editor_usfm2html.get ();
  EXPECT_EQ (html, output);
  
  output = editor_export_verse_quill (stylesv2::standard_sheet (), html);
  EXPECT_EQ (usfm, output);
}


// Testing \add ..\add* markup in a footnote.
TEST_F (usfm_html, add_in_footnote)
{
  constexpr const auto standard_usfm = R"(\p Praise Yahweh\f \add I\add* am\f*, all you nations!)";
  constexpr const auto standard_html = R"(<p class="b-p"><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span class="i-add">I</span><span> am</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output_html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, output_html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (output_html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output_usfm);
}


// Testing \xt in a footnote.
TEST_F (usfm_html, xt_in_footnote)
{
  constexpr const auto standard_usfm = R"(\p Praise Yahweh\f I am, see \xt Exod.6.3.\f*, all you nations!)";
  constexpr const auto standard_html = R"(<p class="b-p"><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>I am, see </span><span class="i-xt">Exod.6.3.</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output_html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, output_html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (output_html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output_usfm);
}


// Testing \xt and \add markup in a footnote, in Romans 2.15, received from a user.
TEST_F (usfm_html, xt_add_in_footnote)
{
  constexpr const auto standard_usfm = R"(\p \f + \fr 2:15 \ft „tokie“ – t. „kurie“\f*tokie parodo savo širdyse įrašytą įstatymo \f + \fr 2:15 \ft „darbą“ – arba „poveikį“\f*darbą, jų sąžinei kartu \add tiems dalykams\add* paliudijant, ir \add jų\add* mintims \f + \fr 2:15 \ft „tuo tarpu \add juos\add* kaltinant arba net ginant“ – gr. „tarp savęs“; gal „tarpusavyje“, t. y. arba minčių tarpusavyje arba kitataučių tarpusavyje; gal „pakeičiant viena kitą \add juos\add* kaltindamos arba net gindamos“; žr. - \xt Mt 18:15, kur kalbama ne apie laiko tarpsnį, bet apie žodžių keitimąsi tarp du žmones\f*tuo tarpu \add juos\add* kaltinant arba net ginant) –)";
  constexpr const auto standard_html = R"(<p class="b-p"><span class="i-notecall1">1</span><span>tokie parodo savo širdyse įrašytą įstatymo </span><span class="i-notecall2">2</span><span>darbą, jų sąžinei kartu </span><span class="i-add">tiems dalykams</span><span> paliudijant, ir </span><span class="i-add">jų</span><span> mintims </span><span class="i-notecall3">3</span><span>tuo tarpu </span><span class="i-add">juos</span><span> kaltinant arba net ginant) –</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fr">2:15 </span><span class="i-ft">„tokie“ – t. „kurie“</span></p><p class="b-f"><span class="i-notebody2">2</span><span> </span><span>+ </span><span class="i-fr">2:15 </span><span class="i-ft">„darbą“ – arba „poveikį“</span></p><p class="b-f"><span class="i-notebody3">3</span><span> </span><span>+ </span><span class="i-fr">2:15 </span><span class="i-ft">„tuo tarpu </span><span class="i-add">juos</span><span> kaltinant arba net ginant“ – gr. „tarp savęs“; gal „tarpusavyje“, t. y. arba minčių tarpusavyje arba kitataučių tarpusavyje; gal „pakeičiant viena kitą </span><span class="i-add">juos</span><span> kaltindamos arba net gindamos“; žr. - </span><span class="i-xt">Mt 18:15, kur kalbama ne apie laiko tarpsnį, bet apie žodžių keitimąsi tarp du žmones</span></p>)";
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  std::string output_html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, output_html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (output_html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, output_usfm);
}


// Regression testing for a situation that a user pastes some text into a note.
// https://github.com/bibledit/cloud/issues/353
TEST_F (usfm_html, regression_paste_text_in_note)
{
  [[maybe_unused]] constexpr const auto standard_usfm_long = R"(\p Verse text one\f + \fr 1:4 \ft Note text \ft one.\f* two.)";
  constexpr const auto standard_usfm_short = R"(\p Verse text one\f + \fr 1:4 \ft Note text one.\f* two.)";
  constexpr const auto standard_html = R"(<p class="b-p">Verse text one<span class="i-notecall1">1</span> two.</p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-fr">1:4 </span><span class="i-ft">Note </span>text&nbsp;<span class="i-ft">one.</span></p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  std::string output_usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm_short, output_usfm);
}


// Regression testing for a fixed bug where after entering a new line in a footnote,
// the part of the footnote text after the new line would disappear.
// https://github.com/bibledit/cloud/issues/444
TEST_F (usfm_html, regression_new_line_footnote)
{
  constexpr const auto standard_html = R"(<p class="b-p"><span class="i-v">1</span> One<span class="i-notecall1">1</span> two.</p><p class="b-notes">&nbsp;</p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-fr">117.3 </span><span class="i-fk">| key </span></p><p class="b-f"><span class="i-fk">word</span></p>)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
    constexpr const auto standard_usfm = R"(\p text\f + \ft footnote\f*)";
    constexpr const auto standard_html = R"(<p class="b-p">text<span class="i-notecall1">1</span></p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">foot</span></p><p class="b-f"><span class="i-ft">note</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (standard_html);
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
    editor_html2usfm.run ();
    const std::string output_usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, output_usfm);
  }
  {
    constexpr const auto standard_usfm = R"(\p text1\f + \ft note1\f* text2\f + \fk keyword2\ft text2\f* text3\f + note3\f*)";
    constexpr const auto standard_html = R"(<p class="b-p">text1<span class="i-notecall1">1</span> text2<span class="i-notecall2">2</span> text3<span class="i-notecall3">3</span></p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">note1</span></p><p class="b-f"><span class="i-notebody2">2</span> + <span class="i-fk">key</span></p><p class="b-f"><span class="i-fk">word2</span><span class="i-ft">text2</span></p><p class="b-f"><span class="i-notebody3">3</span> + <span class="i-ft">note3</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (standard_html);
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
    editor_html2usfm.run ();
    const std::string output_usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, output_usfm);
  }
}


// Test that the converter to USFM removes the Quill caret class.
TEST_F (usfm_html, remove_quill_caret_class)
{
  constexpr const auto standard_html = R"(<p class="b-p">Text<span class="i-notecall1">1</span>.</p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">Text of </span><span class="i-ft"><span class="ql-cursor">﻿</span>the note</span></p>)";
  constexpr const auto standard_usfm = R"(\p Text\f + \ft Text of the note\f*.)";
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (standard_html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
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
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  EXPECT_EQ (html, editor_usfm2html.get());
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  EXPECT_EQ (filter::strings::trim(usfm), editor_html2usfm.get ());
}


TEST_F (usfm_html, introduction_end)
{
  constexpr const auto standard_usfm =
  R"(\c 13)" "\n"
  R"(\ie)" "\n"
  R"(\p)" "\n"
  R"(\v 1 Text)";
  
  constexpr const auto standard_html = R"(<p class="b-c"><span>13</span></p><p class="b-mono"><span>\ie </span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, pronunciation)
{
  constexpr const auto standard_usfm =
  R"(\c 1)" "\n"
  R"(\p)" "\n"
  R"(\v 1 Name \pro pronunciation\pro* and text)";
  
  constexpr const auto standard_html = R"(<p class="b-c"><span>1</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Name </span><span class="i-pro">pronunciation</span><span> and text</span></p>)";
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, inline_quotation_reference)
{
  constexpr const auto standard_usfm =
  R"(\p)" "\n"
  R"(\v 1 verse 1)" "\n"
  R"(\rq reference\rq*)" "\n"
  R"(\p)" "\n"
  R"(\v 2 verse 2)";
  
  constexpr const auto standard_html =
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span>verse 1</span></p>)"
  R"(<p class="b-rq"><span>reference</span></p>)"
  R"(<p class="b-p"><span class="i-v">2</span><span> </span><span>verse 2</span></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, footnote_paragraph)
{
  constexpr const auto standard_usfm = R"(\p \f + \ft text \fp paragraph\f*)";
  
  constexpr const auto standard_html =
  R"(<p class="b-p"><span class="i-notecall1">1</span><br/></p>)"
  R"(<p class="b-notes"> </p>)"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-ft">text </span><span class="i-fp">paragraph</span></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, footnote_verse)
{
  constexpr const auto standard_usfm = R"(\p \f + \ft text \fv 3\fv*\f*)";
  
  constexpr const auto standard_html =
  R"(<p class="b-p"><span class="i-notecall1">1</span><br/></p>)"
  R"(<p class="b-notes"> </p>)"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-ft">text </span><span class="i-fv">3</span></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, footnote_deuterocanonical)
{
  constexpr const auto standard_usfm = R"(\p \f + \ft text \fdc dc\fdc*\f*)";
  
  constexpr const auto standard_html =
  R"(<p class="b-p"><span class="i-notecall1">1</span><br/></p>)"
  R"(<p class="b-notes"> </p>)"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-ft">text </span><span class="i-fdc">dc</span></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, road_is_clear)
{
  const std::string stylesheet {stylesv2::standard_sheet ()};
  
  // The testing data.
  // The first element of the pair contains the markup.
  // The second element says whether the road ahead should be clear.
  std::vector<std::pair<std::vector<std::string>,bool>> testing_usfm {
    
    // If the current item is text, not a marker, the road ahead is clear.
    { { "text" }, true},
    // If the current item is an unknown marker, the road ahead is clear.
    { { R"(\unknown )" }, true},
    // If the current item is a footnote or endnote opener, and ahead is a note closer, the road is clear.
    { { R"(\f )", "+", "note", R"(\f*)" }, true},
    { { R"(\fe )", "+", "note", R"(\fe*)" }, true},
    // If the current item is a footnote or endnote opener, and ahead is another note opener, the road is blocked.
    { { R"(\f )", "+", "note", R"(\f )" }, false},
    { { R"(\fe )", "+", "note", R"(\fe )" }, false},
    // If starting from footnote / endnote opener, and ahead is a verse number, the road is blocked.
    { { R"(\f )", "+", "note", R"(\v 1)" }, false},
    { { R"(\fe )", "+", "note", R"(\v 1)" }, false},
    // If a footnote/endnote encounters a cross reference opener or closer: blocker.
    { { R"(\f )", "+", "note", R"(\x )" }, false},
    { { R"(\f )", "+", "note", R"(\x*)" }, false},
    { { R"(\fe )", "+", "note", R"(\x )" }, false},
    { { R"(\fe )", "+", "note", R"(\x*)" }, false},
    // If xref encounters closer: road is clear.
    { { R"(\x )", "note", R"(\x*)" }, true},
    // If xref opener encounters another xref opener: blocker.
    { { R"(\x )", "note", R"(\x )" }, false},
    // If xref opener encounters verse, this is blocker.
    { { R"(\x )", "note", R"(\v 1)" }, false},
    // If xref opener encounters footnote, this is blocker.
    { { R"(\x )", "note", R"(\f)" }, false},
    { { R"(\x )", "note", R"(\fe)" }, false},
    // Embedded character style opener: Road is clear.
    { { R"(\+add )" }, true},
    // Inline text opener, non-embedded, followed by embedded inline marker: Road ahead is clear.
    { { R"(\add )", "text", R"(\+add )" }, true},
    { { R"(\add )", "text", R"(\+add*)" }, true},
    // Inline text opener, non-embedded, followed by non-embedded inline opening marker: Blocked.
    { { R"(\add )", "text", R"(\add )" }, false},
    { { R"(\pro )", "text", R"(\pro )" }, false},
    // Inline text opener, non-embedded, followed by matching closer: OK.
    { { R"(\add )", "text", R"(\add*)" }, true},
    { { R"(\pro )", "text", R"(\pro*)" }, true},
    // Inline text opener, non-embedded, followed by verse: Block.
    { { R"(\add )", "text", R"(\v 1)" }, false},
    // Inline text opener, non-embedded, followed by paragraph: Block.
    { { R"(\add )", "text", R"(\p)" }, false},
    // Inline text opener, non-embedded, at end of chapter: Blocker.
    { { R"(\add )", "text" }, false},
  };
  for (const auto& [usfm, clear] : testing_usfm) {
    if (clear != road_is_clear(usfm, 0, stylesheet)) {
      std::stringstream ss {};
      for (const auto& bit : usfm) {
        if (!ss.str().empty())
          ss << " ";
        ss << bit;
      }
      ADD_FAILURE() << "The sequence " << std::quoted(ss.str()) << " is expected to result in a " << (clear?"clear":"blocked") << " road ahead but it actually led to the opposite";
    }
  }
}


TEST_F (usfm_html, category)
{
  constexpr auto standard_usfm =
  //R"(\esb \cat category\cat*)" "\n"
  R"(\esb)" "\n"
  R"(\p text\ef - \cat category\cat* \ft note\ef*)" "\n"
  R"(\esbe)"
  ;
  
  constexpr auto standard_html =
  R"(<p class="b-mono"><span>\esb </span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall1">1</span></p>)"
  R"(<p class="b-mono"><span>\esbe </span></p>)"
  R"(<p class="b-notes"> </p>)"
  R"(<p class="b-ef"><span class="i-notebody1">1</span><span> </span><span>- </span><span class="i-cat">category</span><span> </span><span class="i-ft">note</span></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);

  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, usfm_with_all_markers)
{
  constexpr const auto standard_usfm {usfm_with_all_markers};
  
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\id </span><span>GEN</span></p>)"
  R"(<p class="b-mono"><span>\ide </span><span>UTF-8</span></p>)"
  R"(<p class="b-mono"><span>\sts </span><span>Released</span></p>)"
  R"(<p class="b-mono"><span>\usfm </span><span>3.0</span></p>)"
  R"(<p class="b-mono"><span>\rem </span><span>Remark</span></p>)"
  R"(<p class="b-mono"><span>\h </span><span>Genesis</span></p>)"
  R"(<p class="b-mono"><span>\h1 </span><span>Genesis</span></p>)"
  R"(<p class="b-mono"><span>\h2 </span><span>Left</span></p>)"
  R"(<p class="b-mono"><span>\h3 </span><span>Right</span></p>)"
  R"(<p class="b-mono"><span>\toc1 </span><span>The book of Genesis</span></p>)"
  R"(<p class="b-mono"><span>\toc2 </span><span>Genesis</span></p>)"
  R"(<p class="b-mono"><span>\toc3 </span><span>Gen.</span></p>)"
  R"(<p class="b-mt"><span>Major title</span></p>)"
  R"(<p class="b-mt1"><span>Major title 1</span></p>)"
  R"(<p class="b-mt2"><span>Major title 2</span></p>)"
  R"(<p class="b-mt3"><span>Major title 3</span></p>)"
  R"(<p class="b-mt4"><span>Major title 4</span></p>)"
  R"(<p class="b-mte"><span>Major title at ending</span></p>)"
  R"(<p class="b-mte1"><span>Major title at ending 1</span></p>)"
  R"(<p class="b-mte2"><span>Major title at ending 2</span></p>)"
  R"(<p class="b-ms"><span>Major section heading</span></p>)"
  R"(<p class="b-ms1"><span>Major section heading 1</span></p>)"
  R"(<p class="b-ms2"><span>Major section heading 2</span></p>)"
  R"(<p class="b-mr"><span>Major section references</span></p>)"
  R"(<p class="b-s"><span>Section heading</span></p>)"
  R"(<p class="b-s1"><span>Section heading 1</span></p>)"
  R"(<p class="b-s2"><span>Section heading 2</span></p>)"
  R"(<p class="b-s3"><span>Section heading 3</span></p>)"
  R"(<p class="b-s4"><span>Section heading 4</span></p>)"
  R"(<p class="b-sr"><span>Section references</span></p>)"
  R"(<p class="b-r"><span>Parallel passage references</span></p>)"
  R"(<p class="b-rq"><span>Inline quotation references</span></p>)"
  R"(<p class="b-d"><span>Descriptive title</span></p>)"
  R"(<p class="b-sp"><span>Speaker</span></p>)"
  R"(<p class="b-sd"><span>Semantic division</span></p>)"
  R"(<p class="b-sd1"><span>Semantic division 1</span></p>)"
  R"(<p class="b-sd2"><span>Semantic division 2</span></p>)"
  R"(<p class="b-sd3"><span>Semantic division 3</span></p>)"
  R"(<p class="b-sd4"><span>Semantic division 4</span></p>)"
  R"(<p class="b-imt"><span>Main Title</span></p>)"
  R"(<p class="b-imt1"><span>Main Title 1</span></p>)"
  R"(<p class="b-imt2"><span>Main Title 2</span></p>)"
  R"(<p class="b-imt3"><span>Main Title 3</span></p>)"
  R"(<p class="b-imt4"><span>Main Title 4</span></p>)"
  R"(<p class="b-is"><span>Section</span></p>)"
  R"(<p class="b-is1"><span>Section 1</span></p>)"
  R"(<p class="b-is2"><span>Section 2</span></p>)"
  R"(<p class="b-ip"><span>Introduction paragraph</span></p>)"
  R"(<p class="b-ipi"><span>Introduction paragraph indented</span></p>)"
  R"(<p class="b-im"><span>Introduction margin paragraph</span></p>)"
  R"(<p class="b-imi"><span>Introduction margin paragraph indented</span></p>)"
  R"(<p class="b-ipq"><span>Introduction paragraph quote</span></p>)"
  R"(<p class="b-imq"><span>Introduction margin paragraph quote</span></p>)"
  R"(<p class="b-ipr"><span>Introduction paragraph right</span></p>)"
  R"(<p class="b-iq"><span>Introduction poetry</span></p>)"
  R"(<p class="b-iq1"><span>Introduction poetry 1</span></p>)"
  R"(<p class="b-iq2"><span>Introduction poetry 2</span></p>)"
  R"(<p class="b-iq3"><span>Introduction poetry 3</span></p>)"
  R"(<p class="b-ib"><br/></p>)"
  R"(<p class="b-ili"><span>1 Introduction list item</span></p>)"
  R"(<p class="b-ili1"><span>1 Introduction list item 1</span></p>)"
  R"(<p class="b-ili2"><span>1 Introduction list item 2</span></p>)"
  R"(<p class="b-iot"><span>Introduction Outline Title</span></p>)"
  R"(<p class="b-io"><span>Introduction outline</span></p>)"
  R"(<p class="b-io1"><span>Introduction outline 1</span></p>)"
  R"(<p class="b-io2"><span>Introduction outline 2</span></p>)"
  R"(<p class="b-io3"><span>Introduction outline 3 </span><span class="i-ior">references</span></p>)"
  R"(<p class="b-io4"><span>Introduction outline 4 </span><span class="i-iqt">quotation</span></p>)"
  R"(<p class="b-iex"><span>Introduction explanatory text</span></p>)"
  R"(<p class="b-imte"><span>Introduction main title ending</span></p>)"
  R"(<p class="b-imte1"><span>Introduction main title ending 1</span></p>)"
  R"(<p class="b-imte2"><span>Introduction main title ending 2</span></p>)"
  R"(<p class="b-mono"><span>\ie </span></p>)"
  R"(<p class="b-c"><span>1</span></p>)"
  R"(<p class="b-mono"><span>\ca </span><span>2</span><span>\ca*</span></p>)"
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span class="i-vp">1b</span><span> Text name</span><span class="i-pro">pronunciation</span><span>.</span></p>)"
  R"(<p class="b-p"><span class="i-v">2</span><span> </span><span class="i-va">3</span><span> text</span></p>)"
  R"(<p class="b-c"><span>2</span></p>)"
  R"(<p class="b-mono"><span>\cl </span><span>Psalm</span></p>)"
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span>Normal </span><span class="i-add">added</span><span> and </span><span class="i-addpn">AddPn</span></p>)"
  R"(<p class="b-c"><span>3</span></p>)"
  R"(<p class="b-mono"><span>\cp </span><span>א</span></p>)"
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span>Verse text</span></p>)"
  R"(<p class="b-c"><span>4</span></p>)"
  R"(<p class="b-cd"><span>Chapter description</span></p>)"
  R"(<p class="b-m"><span>Continuation (margin) paragraph.</span></p>)"
  R"(<p class="b-po"><span>Paragraph opening an epistle.</span></p>)"
  R"(<p class="b-pr"><span>Right aligned paragraph.</span></p>)"
  R"(<p class="b-cls"><span>Closure of letter.</span></p>)"
  R"(<p class="b-pmo"><span>Embedded text opening.</span></p>)"
  R"(<p class="b-pm"><span>Embedded text paragraph.</span></p>)"
  R"(<p class="b-pmc"><span>Embedded text closing.</span></p>)"
  R"(<p class="b-pmr"><span>Embedded text refrain.</span></p>)"
  R"(<p class="b-pi"><span>Indented paragraph.</span></p>)"
  R"(<p class="b-pi1"><span>Indented paragraph 1.</span></p>)"
  R"(<p class="b-pi2"><span>Indented paragraph 2.</span></p>)"
  R"(<p class="b-pi3"><span>Indented paragraph 3.</span></p>)"
  R"(<p class="b-mi"><span>Margin indented.</span></p>)"
  R"(<p class="b-nb"><span>No break.</span></p>)"
  R"(<p class="b-pc"><span>Centred paragraph.</span></p>)"
  R"(<p class="b-ph"><span>Hanging paragraph.</span></p>)"
  R"(<p class="b-ph1"><span>Hanging paragraph 1.</span></p>)"
  R"(<p class="b-ph2"><span>Hanging paragraph 2.</span></p>)"
  R"(<p class="b-ph3"><span>Hanging paragraph 3.</span></p>)"
  R"(<p class="b-ph4"><span>Hanging paragraph 4.</span></p>)"
  R"(<p class="b-b"><br/></p>)"
  R"(<p class="b-q"><span>Poetic line.</span></p>)"
  R"(<p class="b-q1"><span>Poetic line 1.</span></p>)"
  R"(<p class="b-q2"><span>Poetic line 2.</span></p>)"
  R"(<p class="b-q3"><span>Poetic line 3.</span></p>)"
  R"(<p class="b-qr"><span>Right-aligned poetic line</span></p>)"
  R"(<p class="b-qc"><span>Centered poetic line</span></p>)"
  R"(<p class="b-q"><span>Poetry </span><span class="i-qs">Selah</span></p>)"
  R"(<p class="b-qa"><span>Aleph</span></p>)"
  R"(<p class="b-p"><span class="i-qac">A</span><span>rostic</span></p>)"
  R"(<p class="b-qm"><span>Embedded poetic line</span></p>)"
  R"(<p class="b-qm1"><span>Embedded poetic line 1</span></p>)"
  R"(<p class="b-qm2"><span>Embedded poetic line 2</span></p>)"
  R"(<p class="b-qm3"><span>Embedded poetic line 3</span></p>)"
  R"(<p class="b-qd"><span>Hebrew note</span></p>)"
  R"(<p class="b-lh"><span>List header</span></p>)"
  R"(<p class="b-li"><span>List item</span></p>)"
  R"(<p class="b-li1"><span>List item 1</span></p>)"
  R"(<p class="b-li2"><span>List item 2</span></p>)"
  R"(<p class="b-li3"><span>List item 3</span></p>)"
  R"(<p class="b-li4"><span>List item 4</span></p>)"
  R"(<p class="b-lf"><span>List footer</span></p>)"
  R"(<p class="b-lim"><span>Embedded list item</span></p>)"
  R"(<p class="b-lim1"><span>Embedded list item 1</span></p>)"
  R"(<p class="b-lim2"><span>Embedded list item 2</span></p>)"
  R"(<p class="b-p"><span>List </span><span class="i-litl">total</span></p>)"
  R"(<p class="b-li"><span class="i-lik">Key</span><span> </span><span class="i-liv">Value</span><span> </span><span class="i-liv1">Value 1</span><span> </span><span class="i-liv2">Value 2</span></p>)"
  R"(<p class="b-c"><span>5</span></p>)"
  R"(<p class="b-p"><span class="i-v">1</span><span> </span><span>The table:</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-th1">Column1</span><span class="i-th2">Column2</span><span class="i-th3">Column3</span><span class="i-th4">Column4</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-thr1">R-Column1</span><span class="i-thr2">R-Column2</span><span class="i-thr3">R-Column3</span><span class="i-thr4">R-Column4</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tc1">Cell1</span><span class="i-tc2">Cell2</span><span class="i-tc3">Cell3</span><span class="i-tc4">Cell4</span></p>)"
  R"(<p class="b-mono"><span>\tr </span><span class="i-tcr1">R-Cell1</span><span class="i-tcr2">R-Cell2</span><span class="i-tcr3">R-Cell3</span><span class="i-tcr4">R-Cell4</span></p>)"
  R"(<p class="b-c"><span>6</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall1">1</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall2">1</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall3">1</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall4">2</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall5">a</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall6">a</span></p>)"

  R"(<p class="b-p"><span>The </span><span class="i-bk">Book</span><span> name</span></p>)"
  R"(<p class="b-p"><span>Proto </span><span class="i-dc">Deutero</span><span> text.</span></p>)"
  R"(<p class="b-p"><span>This is a </span><span class="i-k">keyword</span></p>)"
  R"(<p class="b-p"><span class="i-nd">Yahweh</span></p>)"
  R"(<p class="b-p"><span>The 1</span><span class="i-ord">st</span><span>.</span></p>)"
  R"(<p class="b-p"><span>It is about </span><span class="i-pn">Jesus</span><span>.</span></p>)"
  R"(<p class="b-p"><span>The capital is </span><span class="i-png">Amsterdam</span><span>.</span></p>)"
  R"(<p class="b-p"><span>This is a </span><span class="i-qt">quotation</span><span>.</span></p>)"
  R"(<p class="b-p"><span class="i-sig">Kind regards from Paul</span><span>.</span></p>)"
  R"(<p class="b-p"><span>Hebrew and </span><span class="i-sls">Aramaic</span><span> source.</span></p>)"
  R"(<p class="b-p"><span>Jesus: </span><span class="i-tl">Eli, Eli, lema sabachthani?</span></p>)"
  R"(<p class="b-p"><span>Jesus: </span><span class="i-wj">I am</span><span>.</span></p>)"
  R"(<p class="b-p"><span>This is </span><span class="i-em">emphasized</span><span> text.</span></p>)"
  R"(<p class="b-p"><span>This is </span><span class="i-bd">bold</span><span> text.</span></p>)"
  R"(<p class="b-p"><span>This is </span><span class="i-it">italic</span><span> text.</span></p>)"
  R"(<p class="b-p"><span>This is </span><span class="i-bdit">bold/italic</span><span> text.</span></p>)"
  R"(<p class="b-s"><span>Section heading with </span><span class="i-no">normal</span><span> text</span></p>)"
  R"(<p class="b-p"><span>This is </span><span class="i-sc">small cap</span><span> text.</span></p>)"
  R"(<p class="b-p"><span>This is </span><span class="i-sup">superscript</span><span> text.</span></p>)"
  R"(<p class="b-lit"><span>Liturgical note</span></p>)"
  R"(<p class="b-mono"><span>\pb </span></p>)"
  R"(<p class="b-c"><span>7</span></p>)"
  R"(<p class="b-p"><span>This chapter is on a new page</span></p>)"
  R"(<p class="b-c"><span>8</span></p>)"
  R"(<p class="b-p"><span>Text</span></p>)"
  R"(<p class="b-mono"><span>\fig </span><span>caption|src="image.png" size="size" ref="reference"</span><span>\fig*</span></p>)"
  R"(<p class="b-p"><span>Text</span></p>)"
  R"(<p class="b-mono"><span>\fig </span><span>DESC|FILE|SIZE|LOC|COPY|CAP|REF</span><span>\fig*</span></p>)"
  R"(<p class="b-p"><span class="i-ndx">index</span><span> </span><span class="i-w">word</span><span> </span><span class="i-wg">greek</span><span> </span><span class="i-wh">hebrew</span><span> </span><span class="i-wa">aramaic</span></p>)"
  R"(<p class="b-p"><span>Text</span><span class="i-rb0wla1">wd</span></p>)"
  R"(<p class="b-mono"><span>\esb </span></p>)"
  R"(<p class="b-p"><span>Sidebar</span></p>)"
  R"(<p class="b-mono"><span>\esbe </span></p>)"
  R"(<p class="b-mono"><span>\esb </span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-notecall7">2</span></p>)"
  R"(<p class="b-mono"><span>\esbe </span></p>)"
  R"(<p class="b-mono"><span>\periph </span><span>Title Page|id="title"</span></p>)"
  R"(<p class="b-p"><span>paragraph </span><span class="i-jmp0wla2">text</span></p>)"
  R"(<p class="b-c"><span>9</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-qt_s0mls1">🏁</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-qt1_s0mls2">🏁</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-qt2_s0mls3">🏁</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-qt_e0mls4">🏁</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-qt1_e0mls5">🏁</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-qt2_e0mls6">🏁</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-ts_s0mls7">🏁</span></p>)"
  R"(<p class="b-p"><span>text</span><span class="i-ts_e0mls8">🏁</span></p>)"
  
  R"(<p class="b-notes"> </p>)"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fr">ref </span><span class="i-ft">note </span><span class="i-fq">quote </span><span class="i-fv">3</span><span> </span><span class="i-fqa">alternate quote </span><span class="i-fk">keyword </span><span class="i-fl">label </span><span class="i-fw">witness </span><span class="i-fdc">deuterocanonical</span><span> </span><span class="i-fm">mark</span></p>)"
  R"(<p class="b-fe"><span class="i-notebody2">1</span><span> </span><span>+ </span><span class="i-fr">ref</span><span class="i-ft">note</span></p>)"
  R"(<p class="b-ef"><span class="i-notebody3">1</span><span> </span><span>+ </span><span class="i-fr">ref</span><span class="i-ft">note</span></p>)"
  R"(<p class="b-f"><span class="i-notebody4">2</span><span> </span><span>+ </span><span class="i-fp">paragraph</span></p>)"
  R"(<p class="b-x"><span class="i-notebody5">a</span><span> </span><span>+ </span><span class="i-xo">origin </span><span class="i-xk">keyword </span><span class="i-xq">quotation </span><span class="i-xt">targets </span><span class="i-xta">added </span><span class="i-xop">published</span><span> </span><span class="i-xot">old testament</span><span> </span><span class="i-xnt">new testament</span><span> </span><span class="i-xdc">deuterocanonical</span></p>)"
  R"(<p class="b-ex"><span class="i-notebody6">a</span><span> </span><span>+ </span><span class="i-xt">targets</span></p>)"
  R"(<p class="b-ef"><span class="i-notebody7">2</span><span> </span><span>- </span><span class="i-cat">category</span><span> </span><span class="i-ft">note</span></p>)"

  R"(<p class="b-wordlevelattributes"> </p>)"
  R"(<p class="b-wla1">"gloss:gloss"</p>)"
  R"(<p class="b-wla2">link-href="href"</p>)"

  R"(<p class="b-milestoneattributes"> </p>)"
  R"(<p class="b-mls1">attribute-qt-s="value-qt-s"</p>)"
  R"(<p class="b-mls2">attribute-qt1-s="value-qt1-s"</p>)"
  R"(<p class="b-mls3">attribute-qt2-s="value-qt2-s"</p>)"
  R"(<p class="b-mls4">attribute-qt-e="value-qt-e"</p>)"
  R"(<p class="b-mls5">attribute-qt1-e="value-qt1-e"</p>)"
  R"(<p class="b-mls6">attribute-qt2-e="value-qt2-e"</p>)"
  R"(<p class="b-mls7">attribute-ts-s="value-ts-s"</p>)"
  R"(<p class="b-mls8">attribute-ts-e="value-ts-e"</p>)"
  
  ;

  const auto make_readable = [] (const auto& html) {
    return filter::strings::replace ("</p>", "</p>\n", html);
  };

  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  if (html != standard_html) {
    ADD_FAILURE() << "The generated html differs from the reference html";
    std::cout << "Generated html:" << std::endl;
    std::cout << make_readable(html) << std::endl;
    std::vector<std::string> removals;
    std::vector<std::string> additions;
    filter_diff_diff (standard_html, html, &removals, &additions);
    if (!removals.empty())
      std::cout << "First item that the generated html does not have: " << removals.front() << std::endl;
    if (!additions.empty())
      std::cout <<"First item that the generated html has which is not in the standard: " << additions.front() << std::endl;
  }
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  if (filter::strings::trim(standard_usfm) != filter::strings::trim(usfm)) {
    ADD_FAILURE() << "The generated USFM differs from the reference USFM";
    std::cout << "Generated USFM:" << std::endl;
    std::cout << usfm << std::endl;
    std::vector<std::string> removals;
    std::vector<std::string> additions;
    filter_diff_diff (filter::strings::trim(standard_usfm), filter::strings::trim(usfm), &removals, &additions);
    if (!removals.empty())
      std::cout << "First item that the generated USFM does not have: " << removals.front() << std::endl;
    if (!additions.empty())
      std::cout << "First item that the generated USFM has which is not in the standard: " << additions.front() << std::endl;
    const std::string tmpfile = "/tmp/usfm.txt";
    filter_url_file_put_contents (tmpfile, usfm);
    std::cout << "The produced USFM was written to " << tmpfile << std::endl;
  }
}


TEST_F (usfm_html, quill_hyphen_underscore)
{
  EXPECT_EQ ("",       quill::hyphen_to_underscore(""));
  EXPECT_EQ ("",       quill::underscore_to_hyphen(""));
  EXPECT_EQ ("i-qt_s", quill::hyphen_to_underscore("i-qt-s"));
  EXPECT_EQ ("i-qt-s", quill::underscore_to_hyphen("i-qt_s"));
}


TEST_F (usfm_html, milestones)
{
  // Test the standard milestone, meaning, one with attributes.
  {
    constexpr auto standard_usfm = R"(\p text\qt-s |sid="sid" who="who"\*)";
    
    constexpr auto standard_html =
    R"(<p class="b-p"><span>text</span><span class="i-qt_s0mls1">🏁</span></p>)"
    R"(<p class="b-milestoneattributes"> </p>)"
    R"(<p class="b-mls1">sid="sid" who="who"</p>)"
    ;
    
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (standard_usfm);
    editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
    editor_usfm2html.run ();
    const std::string html = editor_usfm2html.get ();
    EXPECT_EQ (standard_html, html);
    
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
    editor_html2usfm.run ();
    const std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, usfm);
  }

  // Test milestones without content, which means that the ending marker ( \*) follows straight on the opening marker.
  {
    constexpr auto standard_usfm = R"(\p text1\qt-e\* text2\qt-e \*)";
    const auto replaced_usfm = filter::strings::replace (R"( \*)", R"(\*)", standard_usfm);
    
    constexpr auto standard_html =
    R"(<p class="b-p">)"
    R"(<span>text1</span><span class="i-qt_e">🏁</span>)"
    R"(<span> text2</span><span class="i-qt_e">🏁</span>)"
    R"(</p>)"
    ;
    
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (standard_usfm);
    editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
    editor_usfm2html.run ();
    const std::string html = editor_usfm2html.get ();
    EXPECT_EQ (standard_html, html);
    
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
    editor_html2usfm.run ();
    const std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (replaced_usfm, usfm);
  }

  // Test milestone that is malformed, it lacks a closing marker.
  {
    constexpr auto standard_usfm = R"(\p text1\qt-e text2)";
    
    constexpr auto standard_html =
    R"(<p class="b-p">)"
    R"(<span>text1</span><span>\qt-e </span><span>text2</span>)"
    R"(</p>)"
    ;
    
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (standard_usfm);
    editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
    editor_usfm2html.run ();
    const std::string html = editor_usfm2html.get ();
    EXPECT_EQ (standard_html, html);
    
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
    editor_html2usfm.run ();
    const std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, usfm);
  }
  
  // Test single milestone closing marker, without a milestone opener.
  {
    constexpr auto standard_usfm = R"(\p text1\*text2)";
    
    constexpr auto standard_html =
    R"(<p class="b-p">)"
    R"(<span>text1</span><span>\*</span><span>text2</span>)"
    R"(</p>)"
    ;
    
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (standard_usfm);
    editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
    editor_usfm2html.run ();
    const std::string html = editor_usfm2html.get ();
    EXPECT_EQ (standard_html, html);
    
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
    editor_html2usfm.run ();
    const std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, usfm);
  }
}


// The Bible editor enter the vertical bar | when entering a note to the text through the styles menu.
// There has been a regression that this vertical bar was seen as an indicator for linking attributes.
// Hence it would produce endmarkers for \fr and for \ft, among others.
// This is now fixed.
// This test keeps an eye on any regression in this area.
TEST_F (usfm_html, note_entered_via_bible_editor)
{
  constexpr const auto standard_usfm = R"(\p Text\f + \fr 2:0 \fk |keyword \ft |text.\f*.)";
  constexpr const auto standard_html =
  R"(<p class="b-p"><span>Text</span><span class="i-notecall1">1</span><span>.</span></p>)"
  R"(<p class="b-notes"> </p>)"
  R"(<p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ </span><span class="i-fr">2:0 </span><span class="i-fk">|keyword </span><span class="i-ft">|text.</span></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, empty_paragraph)
{
  constexpr const auto standard_usfm =
  R"(\id GEN)" "\n"
  R"(\p)"
  ;
  
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\id </span><span>GEN</span></p>)"
  R"(<p class="b-p"><br/></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


TEST_F (usfm_html, backslash_only)
{
  constexpr const auto standard_usfm =
  R"(\id GEN)" "\n"
  R"(\)"
  ;
  
  constexpr const auto standard_html =
  R"(<p class="b-mono"><span>\id </span><span>GEN</span></p>)"
  R"(<p class="b-mono"><span>\ </span></p>)"
  ;
  
  Editor_Usfm2Html editor_usfm2html;
  editor_usfm2html.load (standard_usfm);
  editor_usfm2html.stylesheet (stylesv2::standard_sheet ());
  editor_usfm2html.run ();
  const std::string html = editor_usfm2html.get ();
  EXPECT_EQ (standard_html, html);
  
  Editor_Html2Usfm editor_html2usfm;
  editor_html2usfm.load (html);
  editor_html2usfm.stylesheet (stylesv2::standard_sheet ());
  editor_html2usfm.run ();
  const std::string usfm = editor_html2usfm.get ();
  EXPECT_EQ (standard_usfm, usfm);
}


#endif
