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


#include <unittests/html2format.h>
#include <unittests/utilities.h>
#include <database/state.h>
#include <editor/html2format.h>
#include <styles/logic.h>
#include <filter/string.h>


void test_html2format ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_State::create ();

  // Basic test.
  {
    string html = "<p class=\"p\"><span>The earth brought forth.</span></p>";
    vector<string> texts = {"\n", "The earth brought forth."};
    vector<string> formats = {"p", ""};
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
    // The Quill library uses <span> only when needed, so remove them for testing.
    html = filter_string_str_replace ("<span>", "", html);
    html = filter_string_str_replace ("</span>", "", html);
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
  }

  // Non-breaking spaces.
  {
    string html = "<p class=\"p\"><span>The&nbsp;earth &nbsp; brought&nbsp;&nbsp;forth.</span></p>";
    vector<string> texts = {"\n", "The earth  brought  forth."};
    vector<string> formats = {"p", ""};
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
    // Test Quill-based editor.
    html = filter_string_str_replace ("<span>", "", html);
    html = filter_string_str_replace ("</span>", "", html);
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
  }

  // Test embedded <span> elements.
  {
    string html = "<p class=\"p\"><span>The </span><span class=\"add0nd\">Lord God</span> is calling you<span>.</span></p>";
    vector<string> texts = {"\n", "The ", "Lord God", " is calling you", "."};
    vector<string> formats = {"p", "", "add0nd", "", ""};
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
    html = "<p class=\"p\">The <span class=\"add\"><span class=\"add0nd\">Lord God</span> is calling you</span>.</p>";
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
  }
  
  // Basic note.
  {
    string html =
    "<p class=\"b-p\"><span>The earth brought forth</span><span class=\"i-notecall1\">x</span><span>.</span></p>"
    "<p class=\"b-notes\">"
    " "
    "</p>"
    "<p class=\"b-x\"><span class=\"i-notebody1\">x</span><span> </span><span>+ 2 Joh. 1.1</span></p>";
    vector<string> texts = {
      "\n",
      "The earth brought forth",
      "x",
      ".",
      "\n",
      " ",
      "\n",
      "x",
      " ",
      "+ 2 Joh. 1.1"
    };
    vector<string> formats = {
      "p",
      "",
      "notecall1",
      "",
      "notes",
      "",
      "x",
      "notebody1",
      "",
      ""
    };
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
  }
  
  // Footnote with its body deleted.
  {
    string html =
    "<p class=\"b-p\"><span>The earth brought forth</span><span class=\"i-notecall1\">f</span><span>.</span></p>"
    "<p class=\"b-notes\">"
    " "
    "<p class=\"b-f\"></p>"
    "</p>";
    vector<string> texts = {
      "\n",
      "The earth brought forth",
      "f",
      ".",
      "\n",
      " ",
      "\n"
    };
    vector<string> formats = {
      "p",
      "",
      "notecall1",
      "",
      "notes",
      "",
      "f"
    };
    Editor_Html2Format editor_html2format;
    editor_html2format.load (html);
    editor_html2format.run ();
    evaluate (__LINE__, __func__, texts, editor_html2format.texts);
    evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    // Clear message from logbook.
    refresh_sandbox (false);
  }
  
  // Footnote with a deleted citation.
  {
    {
      string html =
      "<p class=\"b-p\"><span>The earth brought forth</span><span>.</span></p>"
      "<p class=\"b-notes\">"
      " "
      "</p>"
      "<p class=\"b-f\"><span class=\"i-notebody1\">f</span><span> </span><span>+ </span><span class=\"i-fk\">brought: </span><span class=\"i-fl\">Heb. </span><span class=\"i-fq\">explanation.</span></p>";
      vector<string> texts = {
        "\n",
        "The earth brought forth",
        ".",
        "\n",
        " ",
        "\n",
        "f",
        " ",
        "+ ",
        "brought: ",
        "Heb. ",
        "explanation."
      };
      vector<string> formats = {
        "p",
        "",
        "",
        "notes",
        "",
        "f",
        "notebody1",
        "",
        "",
        "fk",
        "fl",
        "fq"
      };
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
  }
  
  // Two words with character markup in sequence.
  // The converter used to take out the space between the two words.
  // This tests that it does not do that.
  {
    {
      string html = "<p class=\"p\"><span>Praise </span><span class=\"add\">Yahweh</span><span> <span class=\"add\">all</span> you nations!</span></p>";
      vector<string> texts = {
        "\n",
        "Praise ",
        "Yahweh",
        " ",
        "all",
        " you nations!"
      };
      vector<string> formats = {
        "p",
        "",
        "add",
        "",
        "add",
        ""
      };
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
    {
      string html = "<p class=\"b-p\"><span>Praise </span><span class=\"i-add\">Yahweh</span><span> <span class=\"i-add\">all</span> you nations!</span></p>";
      vector<string> texts = {
        "\n",
        "Praise ",
        "Yahweh",
        " ",
        "all",
        " you nations!"
      };
      vector<string> formats = {
        "p",
        "",
        "add",
        "",
        "add",
        ""
      };
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
  }
  
  // Test that the converter to formatting removes the Quill caret class.
  {
    string html = R"(<p class="b-p"><span>Praise </span><span class="i-add">Yahweh</span><span> <span class="i-add">all</span> you na</span><span class="ql-cursor">﻿</span><span>tions!</span></p>)";
    vector<string> texts = {
      "\n",
      "Praise ",
      "Yahweh",
      " ",
      "all",
      " you na",
      "tions!"
    };
    vector<string> formats = {
      "p",
      "",
      "add",
      "",
      "add",
      "",
      ""
    };
    Editor_Html2Format editor_html2format;
    editor_html2format.load (html);
    editor_html2format.run ();
    evaluate (__LINE__, __func__, texts, editor_html2format.texts);
    evaluate (__LINE__, __func__, formats, editor_html2format.formats);
  }

  // Test that it changes three or more spaces in sequence to two spaces.
  {
    string html = "<p class=\"p\"><span>The   earth    brought forth.</span></p>";
    vector<string> texts = {"\n", "The  earth  brought forth."};
    vector<string> formats = {"p", ""};
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
    // The Quill library uses <span> only when needed, so remove them for testing.
    html = filter_string_str_replace ("<span>", "", html);
    html = filter_string_str_replace ("</span>", "", html);
    {
      Editor_Html2Format editor_html2format;
      editor_html2format.load (html);
      editor_html2format.run ();
      evaluate (__LINE__, __func__, texts, editor_html2format.texts);
      evaluate (__LINE__, __func__, formats, editor_html2format.formats);
    }
  }

  refresh_sandbox (true);
}
