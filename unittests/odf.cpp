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


#include <unittests/odf.h>
#include <unittests/utilities.h>
#include <odf/text.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/config/bible.h>


void test_odf ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  string OdfTextTestDotOdt = "/tmp/OdfTextTest.odt";
  string Odt2TxtOutput = "/tmp/Odt2TxtOutput.txt";
  string bible = "phpunit";
  string fontname = "Marker Felt";
  Database_Config_Bible::setExportFont (bible, fontname);

  // Test converting paragraphs.
  {
    Odf_Text odf_text (bible);
    odf_text.createPageBreakStyle ();
    odf_text.newParagraph ();
    evaluate (__LINE__, __func__, styles_logic_standard_sheet (), odf_text.current_paragraph_style);
    odf_text.addText ("Paragraph One");
    evaluate (__LINE__, __func__, "Paragraph One", odf_text.current_paragraph_content);
    odf_text.newParagraph ();
    evaluate (__LINE__, __func__, "", odf_text.current_paragraph_content);
    odf_text.addText ("Paragraph Two");
    evaluate (__LINE__, __func__, "Paragraph Two", odf_text.current_paragraph_content);
    odf_text.newHeading1 ("Heading One");
    evaluate (__LINE__, __func__, "", odf_text.current_paragraph_content);
    odf_text.newPageBreak ();
    odf_text.newParagraph ();
    odf_text.addText ("Paragraph Three");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = ""
    "Paragraph One\n"
    "\n"
    "Paragraph Two\n"
    "\n"
    "Heading One\n"
    "\n"
    "\n"
    "\n"
    "Paragraph Three\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);

  // Test automatic paragraph.
  {
    Odf_Text odf_text (bible);
    odf_text.addText ("Should create new paragraph automatically");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = ""
    "Should create new paragraph automatically\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);

  // Test basic note.
  {
    Odf_Text odf_text (bible);
    odf_text.newParagraph ();
    odf_text.addText ("Text");
    odf_text.addNote ("†", "");
    odf_text.addNoteText ("Note");
    odf_text.addText (".");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = ""
    "Text†Note\n"
    "\n"
    ".\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);

  // Test basic formatted text.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item add = database_styles.getMarkerData (styles_logic_standard_sheet (), "add");
    Odf_Text odf_text (bible);
    odf_text.newParagraph ();
    odf_text.addText ("text");
    odf_text.openTextStyle (add, false, false);
    odf_text.addText ("add");
    odf_text.closeTextStyle (false, false);
    odf_text.addText ("normal");
    odf_text.addText (".");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = "textaddnormal.";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);

  // Test basic formatted note.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item add = database_styles.getMarkerData (styles_logic_standard_sheet (), "add");
    Odf_Text odf_text (bible);
    odf_text.newParagraph ();
    odf_text.addText ("Text");
    odf_text.addNote ("𐌰", "f");
    odf_text.openTextStyle (add, true, false);
    odf_text.addNoteText ("Add");
    odf_text.closeTextStyle (true, false);
    odf_text.addNoteText ("normal");
    odf_text.addText (".");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = ""
    "Text𐌰Addnormal\n"
    "\n"
    ".\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);

  // Test embedded formatted text.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item add = database_styles.getMarkerData (styles_logic_standard_sheet (), "add");
    add.italic = ooitOn;
    add.bold = 0;
    add.underline = 0;
    add.smallcaps = 0;
    add.superscript = false;
    add.color = "#000000";
    Database_Styles_Item nd = database_styles.getMarkerData (styles_logic_standard_sheet (), "nd");
    nd.italic = 0;
    nd.bold = 0;
    nd.underline = 0;
    nd.smallcaps = ooitOn;
    nd.superscript = false;
    nd.color = "#000000";
    Odf_Text odf_text (bible);
    odf_text.newParagraph ();
    odf_text.addText ("text");
    odf_text.openTextStyle (add, false, false);
    odf_text.addText ("add");
    odf_text.openTextStyle (nd, false, true);
    odf_text.addText ("nd");
    odf_text.closeTextStyle (false, false);
    odf_text.addText ("normal");
    odf_text.addText (".");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = "textaddndnormal.";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);

  // Test embedded formatted note.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item add = database_styles.getMarkerData (styles_logic_standard_sheet (), "add");
    add.italic = ooitOn;
    add.bold = 0;
    add.underline = 0;
    add.smallcaps = 0;
    add.superscript = false;
    add.color = "#000000";
    Database_Styles_Item nd = database_styles.getMarkerData (styles_logic_standard_sheet (), "nd");
    nd.italic = 0;
    nd.bold = 0;
    nd.underline = 0;
    nd.smallcaps = ooitOn;
    nd.superscript = false;
    nd.color = "#000000";
    Odf_Text odf_text (bible);
    odf_text.newParagraph ();
    odf_text.addText ("text");
    odf_text.addNote ("𐌰", "f");
    odf_text.openTextStyle (add, true, false);
    odf_text.addNoteText ("add");
    odf_text.openTextStyle (nd, true, true);
    odf_text.addNoteText ("nd");
    odf_text.closeTextStyle (true, false);
    odf_text.addNoteText ("normal");
    odf_text.addText (".");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = ""
    "text𐌰addndnormal\n"
    "\n"
    ".\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);

  // Test paragraph formatting.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item d = database_styles.getMarkerData (styles_logic_standard_sheet (), "d");
    Odf_Text odf_text (bible);
    odf_text.createParagraphStyle (d.marker, fontname, d.fontsize, d.italic, d.bold, d.underline, d.smallcaps, d.justification, d.spacebefore, d.spaceafter, d.leftmargin, d.rightmargin, d.firstlineindent, true, false);
    odf_text.newParagraph ("d");
    odf_text.addText ("Paragraph with d style");
    odf_text.newParagraph ("d");
    odf_text.addText ("Paragraph with d style at first, then Standard");
    odf_text.updateCurrentParagraphStyle (styles_logic_standard_sheet ());
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = ""
    "Paragraph with d style\n"
    "\n"
    "Paragraph with d style at first, then Standard\n"
    "";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);
  
  // Test converting apostrophy.
  {
    Odf_Text odf_text (bible);
    odf_text.newParagraph ();
    evaluate (__LINE__, __func__, styles_logic_standard_sheet (), odf_text.current_paragraph_style);
    odf_text.addText ("One apostrophy ' and two more ''.");
    odf_text.save (OdfTextTestDotOdt);
    int ret = odf2txt (OdfTextTestDotOdt, Odt2TxtOutput);
    evaluate (__LINE__, __func__, 0, ret);
    string odt = filter_url_file_get_contents (Odt2TxtOutput);
    string standard = "One apostrophy ' and two more ''.";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (odt));
  }
  
  filter_url_unlink (OdfTextTestDotOdt);
  filter_url_unlink (Odt2TxtOutput);
  
  refresh_sandbox (true);
}


