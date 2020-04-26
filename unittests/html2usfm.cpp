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


#include <unittests/html2usfm.h>
#include <unittests/utilities.h>
#include <database/state.h>
#include <editor/html2usfm.h>
#include <styles/logic.h>
#include <filter/string.h>


void test_html2usfm ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_State::create ();
  // Basic test.
  {
    string html = "<p class=\"p\"><span>The earth brought forth.</span></p>";
    string standard = "\\p The earth brought forth.";
    // DOM-based editor test.
    {
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    // Quill-based editor test.
    {
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    // The Quill library uses <span> only when needed, so remove them for testing.
    html = filter_string_str_replace ("<span>", "", html);
    html = filter_string_str_replace ("</span>", "", html);
    {
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
  }
  // Non-breaking spaces
  {
    string html = "<p class=\"p\"><span>The&nbsp;earth &nbsp; brought&nbsp;&nbsp;forth.</span></p>";
    string standard = "\\p The earth brought forth.";
    // Test DOM-based editor.
    {
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    // Test Quill-based editor.
    html = filter_string_str_replace ("<span>", "", html);
    html = filter_string_str_replace ("</span>", "", html);
    {
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
  }
  // Test embedded <span> elements.
  {
    string html = "<p class=\"p\"><span>The <span class=\"add\"><span class=\"nd\">Lord God</span> is calling</span> you</span><span>.</span></p>";
    string standard = "\\p The \\add \\+nd Lord God\\+nd* is calling\\add* you.";
    // Test DOM-based editor.
    {
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    // Test Quill-based editor.
    html = "<p class=\"p\">The <span class=\"add\"><span class=\"nd\">Lord God</span> is calling</span> you.</p>";
    {
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
  }
  // Basic note
  {
    string standard = "\\p The earth brought forth\\x + 2 Joh. 1.1\\x*.";
    {
      // DOM-based editor.
      string html =
      "<p class=\"p\"><span>The earth brought forth</span><a href=\"#note1\" id=\"citation1\" class=\"superscript\">x</a><span>.</span></p>"
      "<div id=\"notes\">"
      "<hr/>"
      "<p class=\"x\"><a href=\"#citation1\" id=\"note1\">x</a><span> </span><span>+ 2 Joh. 1.1</span></p>"
      "<br/>"
      "</div>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    {
      // Quill-based editor.
      string html =
      "<p class=\"b-p\"><span>The earth brought forth</span><span class=\"i-notecall1\">x</span><span>.</span></p>"
      "<p class=\"b-notes\">"
      "<br/>"
      "</p>"
      "<p class=\"b-x\"><span class=\"i-notebody1\">x</span><span> </span><span>+ 2 Joh. 1.1</span></p>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
  }
  // Footnote with its body deleted.
  {
    string standard = "\\p The earth brought forth.";
    {
      // DOM-based editor.
      string html =
      "<p class=\"p\"><span>The earth brought forth</span><a href=\"#note1\" id=\"citation1\" class=\"superscript\">f</a><span>.</span></p>"
      "<div id=\"notes\">"
      "<hr/>"
      "<p class=\"f\"></p>"
      "<br/>"
      "</div>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    {
      // Quill-based editor.
      string html =
      "<p class=\"b-p\"><span>The earth brought forth</span><span class=\"i-notecall1\">f</span><span>.</span></p>"
      "<p class=\"b-notes\">"
      "<br/>"
      "<p class=\"b-f\"></p>"
      "<br/>"
      "</p>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    // Clear message from logbook.
    refresh_sandbox (false);
  }
  // Footnote with a deleted citation.
  {
    string standard = "\\p The earth brought forth.";
    {
      // DOM-based editor.
      string html =
      "<p class=\"p\"><span>The earth brought forth</span><span>.</span></p>"
      "<div id=\"notes\">"
      "<hr />"
      "<p class=\"f\"><a href=\"#citation1\" id=\"note1\">f</a><span> </span><span>+ </span><span class=\"fk\">brought: </span><span class=\"fl\">Heb. </span><span class=\"fq\">explanation.</span></p>"
      "</div>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    {
      // Quill-based editor.
      string html =
      "<p class=\"b-p\"><span>The earth brought forth</span><span>.</span></p>"
      "<p class=\"b-notes\">"
      "<br />"
      "</p>"
      "<p class=\"b-f\"><span class=\"i-notebody1\">f</span><span> </span><span>+ </span><span class=\"i-fk\">brought: </span><span class=\"i-fl\">Heb. </span><span class=\"i-fq\">explanation.</span></p>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
  }
  // Two words with character markup in sequence.
  // The converter used to take out the space between the two words.
  // This tests that it does not do that.
  {
    string standard = "\\p Praise \\add Yahweh\\add* \\add all\\add* you nations!";
    {
      // DOM-based editor.
      string html = "<p class=\"p\"><span>Praise </span><span class=\"add\">Yahweh</span><span> <span class=\"add\">all</span> you nations!</span></p>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
    {
      // Quill-based editor.
      string html = "<p class=\"b-p\"><span>Praise </span><span class=\"i-add\">Yahweh</span><span> <span class=\"i-add\">all</span> you nations!</span></p>";
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard, usfm);
    }
  }
  refresh_sandbox (true);
}
