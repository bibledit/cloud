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


#include <unittests/html.h>
#include <unittests/utilities.h>
#include <html/text.h>


void test_html ()
{
  trace_unit_tests (__func__);

  // Test paragraphs.
  {
    Html_Text html_text ("TestOne");
    html_text.newParagraph ();
    evaluate (__LINE__, __func__, "", html_text.currentParagraphStyle);
    html_text.addText ("Paragraph One");
    evaluate (__LINE__, __func__, "Paragraph One", html_text.currentParagraphContent);
    html_text.newParagraph ();
    evaluate (__LINE__, __func__, "", html_text.currentParagraphContent);
    html_text.addText ("Paragraph Two");
    evaluate (__LINE__, __func__, "Paragraph Two", html_text.currentParagraphContent);
    html_text.newHeading1 ("Heading One");
    evaluate (__LINE__, __func__, "", html_text.currentParagraphContent);
    html_text.newParagraph ();
    html_text.addText ("Paragraph Three");
    string html = html_text.getInnerHtml ();
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
    html_text.addText ("Should create new paragraph automatically");
    string html = html_text.getInnerHtml ();
    string standard =
    "<p><span>Should create new paragraph automatically</span></p>";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test basic note
  {
    Html_Text html_text ("TestThree");
    html_text.newParagraph ();
    html_text.addText ("Text1");
    html_text.addNote ("†", "");
    html_text.addNoteText ("Note1.");
    html_text.addText (".");
    string html = html_text.getInnerHtml ();
    string standard =
    "<p><span>Text1</span><a href=\"#note1\" id=\"citation1\" class=\"superscript\">†</a><span>.</span></p>"
    "<div>"
    "<p class=\"\"><a href=\"#citation1\" id=\"note1\">†</a><span> </span><span>Note1.</span></p>"
    "</div>";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test getInnerHtml ()
  {
    Html_Text html_text ("test");
    html_text.newParagraph ();
    html_text.addText ("Paragraph One");
    html_text.newParagraph ();
    html_text.addText ("Paragraph Two");
    string html = html_text.getInnerHtml ();
    string standard =
    "<p><span>Paragraph One</span></p>"
    "<p><span>Paragraph Two</span></p>";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test basic formatted note.
  {
    Database_Styles_Item style;
    Html_Text html_text ("");
    html_text.newParagraph ();
    html_text.addText ("Text");
    html_text.addNote ("𐌰", "f");
    style.marker = "add";
    html_text.openTextStyle (style, true, false);
    html_text.addNoteText ("Add");
    html_text.closeTextStyle (true, false);
    html_text.addNoteText ("normal");
    html_text.addText (".");
    string html = html_text.getInnerHtml ();
    string standard =
    "<p>"
    "<span>Text</span>"
    "<a href=\"#note1\" id=\"citation1\" class=\"superscript\">𐌰</a>"
    "<span>.</span>"
    "</p>"
    "<div>"
    "<p class=\"f\">"
    "<a href=\"#citation1\" id=\"note1\">𐌰</a>"
    "<span> </span>"
    "<span class=\"add\">Add</span>"
    "<span>normal</span>"
    "</p>"
    "</div>";
    evaluate (__LINE__, __func__, standard, html);
  }
  
  // Test embedded formatted note
  {
    Database_Styles_Item style;
    Html_Text html_text ("");
    html_text.newParagraph ();
    html_text.addText ("text");
    html_text.addNote ("𐌰", "f");
    style.marker = "add";
    html_text.openTextStyle (style, true, false);
    html_text.addNoteText ("add");
    style.marker = "nd";
    html_text.openTextStyle (style, true, true);
    html_text.addNoteText ("nd");
    html_text.closeTextStyle (true, false);
    html_text.addNoteText ("normal");
    html_text.addText (".");
    string html = html_text.getInnerHtml ();
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
