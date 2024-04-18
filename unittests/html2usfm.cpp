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
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <database/state.h>
#include <editor/html2usfm.h>
#include <styles/logic.h>
#include <filter/string.h>
using namespace std;


TEST (editor, html2usfm)
{
  refresh_sandbox (false);
  Database_State::create ();

  // Basic test.
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
  {
    std::string html = R"(<p class="p"><span>The&nbsp;earth &nbsp; brought&nbsp;&nbsp;forth.</span></p>)";
    std::string standard = R"(\p The earth   brought  forth.)";
    // Test Quill-based editor.
    html = filter::strings::replace ("<span>", "", html);
    html = filter::strings::replace ("</span>", "", html);
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }

  // Test embedded <span> elements.
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

  // Basic note
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
  {
    std::string standard = R"(\p The earth brought forth.)";
    std::string html =
    R"(<p class="b-p"><span>The earth brought forth</span><span>.</span></p>)"
    R"(<p class="b-notes">)"
    " "
    "</p>"
    R"(<p class="b-f"><span class="i-notebody1">f</span><span> </span><span>+ </span><span class="i-fk">brought: </span><span class="i-fl">Heb. </span><span class="i-fq">explanation.</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }

  // Two words with character markup in sequence.
  // The converter used to take out the space between the two words.
  // This tests that it does not do that.
  {
    std::string standard = R"(\p Praise \add Yahweh\add* \add all\add* you nations!)";
    std::string html = R"(<p class="b-p"><span>Praise </span><span class="i-add">Yahweh</span><span> <span class="i-add">all</span> you nations!</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }

  // Regression test for issue https://github.com/bibledit/cloud/issues/444.
  // If a new line is given within the footnote,
  // the footnote should still remain okay,
  // when converted back to USFM.
  {
    std::string standard_usfm = R"(\p text\f + \ft footnote\f*)";
    std::string standard_html = R"(<p class="b-p">text<span class="i-notecall1">1</span></p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">foot</span></p><p class="b-f"><span class="i-ft">note</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (standard_html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    std::string output_usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, output_usfm);
  }
  {
    std::string standard_usfm = R"(\p text1\f + \ft note1\f* text2\f + \fk keyword2\ft text2\f* text3\f + note3\f*)";
    std::string standard_html = R"(<p class="b-p">text1<span class="i-notecall1">1</span> text2<span class="i-notecall2">2</span> text3<span class="i-notecall3">3</span></p><p class="b-notes"></p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">note1</span></p><p class="b-f"><span class="i-notebody2">2</span> + <span class="i-fk">key</span></p><p class="b-f"><span class="i-fk">word2</span><span class="i-ft">text2</span></p><p class="b-f"><span class="i-notebody3">3</span> + <span class="i-ft">note3</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (standard_html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    std::string output_usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard_usfm, output_usfm);
  }
  
  // Test that the converter to USFM removes the Quill caret class.
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
    std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }
  
  // Text plus note but with a deleted notes separator.
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
    std::string usfm = editor_html2usfm.get ();
    EXPECT_EQ (standard, usfm);
  }

  // Check that it collapses only three spaces into two.
  // And that it does not collapse two spaces into one.
  {
    std::string html = R"(<p class="p"><span>The   earth  brought    forth.</span></p>)";
    std::string standard = R"(\p The   earth  brought    forth.)";
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

  refresh_sandbox (true);
}

#endif

