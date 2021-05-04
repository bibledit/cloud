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


#include <unittests/html.h>
#include <unittests/utilities.h>
#include <html/text.h>


void test_html () // Todo
{
  trace_unit_tests (__func__);

  // Test paragraphs.
  {
    Html_Text html_text ("TestOne");
    html_text.new_paragraph ();
    evaluate (__LINE__, __func__, "", html_text.current_paragraph_style);
    html_text.add_text ("Paragraph One");
    evaluate (__LINE__, __func__, "Paragraph One", html_text.current_paragraph_content);
    html_text.new_paragraph ();
    evaluate (__LINE__, __func__, "", html_text.current_paragraph_content);
    html_text.add_text ("Paragraph Two");
    evaluate (__LINE__, __func__, "Paragraph Two", html_text.current_paragraph_content);
    html_text.new_heading1 ("Heading One");
    evaluate (__LINE__, __func__, "", html_text.current_paragraph_content);
    html_text.new_paragraph ();
    html_text.add_text ("Paragraph Three");
    string html = html_text.get_inner_html ();
    string standard =
    "<p><span>Paragraph One</span></p>"
    "<p><span>Paragraph Two</span></p>"
    "<h1>Heading One</h1>"
    "<p><span>Paragraph Three</span></p>";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test automatic paragraph.
  {
    Html_Text html_text ("TestTwo");
    html_text.add_text ("Should create new paragraph automatically");
    string html = html_text.get_inner_html ();
    string standard =
    "<p><span>Should create new paragraph automatically</span></p>";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test basic note.
  {
    Html_Text html_text ({});
    html_text.new_paragraph ();
    html_text.add_text ("Text1");
    html_text.add_note ("†", "");
    html_text.add_note_text ("Note1.");
    html_text.add_text (".");
    string html = html_text.get_inner_html ();
    string standard = R"(<p><span>Text1</span><a href="#note1" id="citation1" class="superscript">†</a><span>.</span></p><div><p class=""><a href="#citation1" id="note1">†</a><span> </span><span>Note1.</span></p></div>)";
    evaluate (__LINE__, __func__, standard, html);
  }
  {
    Html_Text html_text ({});
    html_text.have_popup_notes(); // Todo
    html_text.new_paragraph ();
    html_text.add_text ("Text1");
    html_text.add_note ("†", "");
    html_text.add_note_text ("Note1.");
    html_text.add_text (".");
    string html = html_text.get_inner_html ();
    string standard = R"(<p><span>Text1</span><a href="#note1" id="citation1" class="superscript">†<span class="popup"><span> </span><span>Note1.</span></span></a><span>.</span></p><div><p class=""><a href="#citation1" id="note1">†</a><span> </span><span>Note1.</span></p></div>)";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test getInnerHtml ()
  {
    Html_Text html_text ("test");
    html_text.new_paragraph ();
    html_text.add_text ("Paragraph One");
    html_text.new_paragraph ();
    html_text.add_text ("Paragraph Two");
    string html = html_text.get_inner_html ();
    string standard = "<p><span>Paragraph One</span></p><p><span>Paragraph Two</span></p>";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test basic formatted note. Todo add counterpart with popup notes.
  {
    Database_Styles_Item style;
    Html_Text html_text ("");
    html_text.new_paragraph ();
    html_text.add_text ("Text");
    html_text.add_note ("𐌰", "f");
    style.marker = "add";
    html_text.open_text_style (style, true, false);
    html_text.add_note_text ("Add");
    html_text.close_text_style (true, false);
    html_text.add_note_text ("normal");
    html_text.add_text (".");
    string html = html_text.get_inner_html ();
    string standard = R"(<p><span>Text</span><a href="#note1" id="citation1" class="superscript">𐌰</a><span>.</span></p><div><p class="f"><a href="#citation1" id="note1">𐌰</a><span> </span><span class="add">Add</span><span>normal</span></p></div>)";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test embedded formatted note. Todo add counterpart with popup notes.
  {
    Database_Styles_Item style;
    Html_Text html_text ("");
    html_text.new_paragraph ();
    html_text.add_text ("text");
    html_text.add_note ("𐌰", "f");
    style.marker = "add";
    html_text.open_text_style (style, true, false);
    html_text.add_note_text ("add");
    style.marker = "nd";
    html_text.open_text_style (style, true, true);
    html_text.add_note_text ("nd");
    html_text.close_text_style (true, false);
    html_text.add_note_text ("normal");
    html_text.add_text (".");
    string html = html_text.get_inner_html ();
    string standard =
    "<p>"
    "<span>text</span>"
    "<a href=\"#note1\" id=\"citation1\" class=\"superscript\">𐌰</a>"
    "<span>.</span>"
    "</p>"
    "<div>"
    "<p class=\"f\">"
    "<a href=\"#citation1\" id=\"note1\">𐌰</a>"
    "<span> </span>"
    "<span class=\"add\">add</span>"
    "<span class=\"add nd\">nd</span>"
    "<span>normal</span>"
    "</p>"
    "</div>";
    evaluate (__LINE__, __func__, standard, html);
  }

}
