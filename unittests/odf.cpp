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
#include <odf/text.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/config/bible.h>
using namespace std;


TEST (filter, odf)
{
  refresh_sandbox (false);
  std::string odf_text_test_odt = "/tmp/test.odt";
  std::string Odt2TxtOutput = "/tmp/test.txt";
  std::string bible = "testbible";
  std::string fontname = "Marker Felt";
  Database_Config_Bible::setExportFont (bible, fontname);

  // Test converting paragraphs.
  {
    odf_text odf_text (bible);
    odf_text.create_page_break_style ();
    odf_text.new_paragraph ();
    EXPECT_EQ (styles_logic_standard_sheet (), odf_text.m_current_paragraph_style);
    odf_text.add_text ("Paragraph One");
    EXPECT_EQ ("Paragraph One", odf_text.m_current_paragraph_content);
    odf_text.new_paragraph ();
    EXPECT_EQ ("", odf_text.m_current_paragraph_content);
    odf_text.add_text ("Paragraph Two");
    EXPECT_EQ ("Paragraph Two", odf_text.m_current_paragraph_content);
    odf_text.new_heading1 ("Heading One");
    EXPECT_EQ ("", odf_text.m_current_paragraph_content);
    odf_text.new_page_break ();
    odf_text.new_paragraph ();
    odf_text.add_text ("Paragraph Three");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = ""
    "Paragraph One\n"
    "\n"
    "Paragraph Two\n"
    "\n"
    "Heading One\n"
    "\n"
    "\n"
    "\n"
    "Paragraph Three\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
  filter_url_unlink (Odt2TxtOutput);

  // Test automatic paragraph.
  {
    odf_text odf_text (bible);
    odf_text.add_text ("Should create new paragraph automatically");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = ""
    "Should create new paragraph automatically\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
  filter_url_unlink (Odt2TxtOutput);

  // Test basic note.
  {
    odf_text odf_text (bible);
    odf_text.new_paragraph ();
    odf_text.add_text ("Text");
    odf_text.add_note ("†", "");
    odf_text.add_note_text ("Note");
    odf_text.add_text (".");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = ""
    "Text†Note\n"
    "\n"
    ".\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
  filter_url_unlink (Odt2TxtOutput);

  // Test basic formatted text.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item add = database_styles.getMarkerData (styles_logic_standard_sheet (), "add");
    odf_text odf_text (bible);
    odf_text.new_paragraph ();
    odf_text.add_text ("text");
    odf_text.open_text_style (add, false, false);
    odf_text.add_text ("add");
    odf_text.close_text_style (false, false);
    odf_text.add_text ("normal");
    odf_text.add_text (".");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = "textaddnormal.";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
  filter_url_unlink (Odt2TxtOutput);

  // Test basic formatted note.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item add = database_styles.getMarkerData (styles_logic_standard_sheet (), "add");
    odf_text odf_text (bible);
    odf_text.new_paragraph ();
    odf_text.add_text ("Text");
    odf_text.add_note ("𐌰", "f");
    odf_text.open_text_style (add, true, false);
    odf_text.add_note_text ("Add");
    odf_text.close_text_style (true, false);
    odf_text.add_note_text ("normal");
    odf_text.add_text (".");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = ""
    "Text𐌰Addnormal\n"
    "\n"
    ".\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
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
    odf_text odf_text (bible);
    odf_text.new_paragraph ();
    odf_text.add_text ("text");
    odf_text.open_text_style (add, false, false);
    odf_text.add_text ("add");
    odf_text.open_text_style (nd, false, true);
    odf_text.add_text ("nd");
    odf_text.close_text_style (false, false);
    odf_text.add_text ("normal");
    odf_text.add_text (".");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = "textaddndnormal.";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
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
    odf_text odf_text (bible);
    odf_text.new_paragraph ();
    odf_text.add_text ("text");
    odf_text.add_note ("𐌰", "f");
    odf_text.open_text_style (add, true, false);
    odf_text.add_note_text ("add");
    odf_text.open_text_style (nd, true, true);
    odf_text.add_note_text ("nd");
    odf_text.close_text_style (true, false);
    odf_text.add_note_text ("normal");
    odf_text.add_text (".");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = ""
    "text𐌰addndnormal\n"
    "\n"
    ".\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
  filter_url_unlink (Odt2TxtOutput);

  // Test paragraph formatting.
  {
    Database_Styles database_styles;
    database_styles.create ();
    Database_Styles_Item d = database_styles.getMarkerData (styles_logic_standard_sheet (), "d");
    odf_text odf_text (bible);
    odf_text.create_paragraph_style (d.marker, fontname, d.fontsize, d.italic, d.bold, d.underline, d.smallcaps, d.justification, d.spacebefore, d.spaceafter, d.leftmargin, d.rightmargin, d.firstlineindent, true, false);
    odf_text.new_paragraph ("d");
    odf_text.add_text ("Paragraph with d style");
    odf_text.new_paragraph ("d");
    odf_text.add_text ("Paragraph with d style at first, then Standard");
    odf_text.update_current_paragraph_style (styles_logic_standard_sheet ());
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = ""
    "Paragraph with d style\n"
    "\n"
    "Paragraph with d style at first, then Standard\n"
    "";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  filter_url_unlink (odf_text_test_odt);
  filter_url_unlink (Odt2TxtOutput);
  
  // Test converting apostrophy.
  {
    odf_text odf_text (bible);
    odf_text.new_paragraph ();
    EXPECT_EQ (styles_logic_standard_sheet (), odf_text.m_current_paragraph_style);
    odf_text.add_text ("One apostrophy ' and two more ''.");
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = "One apostrophy ' and two more ''.";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
  }
  
  // Test a tab.
  {
    odf_text odf_text (bible);
    odf_text.new_paragraph ();
    odf_text.add_tab ();
    odf_text.save (odf_text_test_odt);
    int ret = odf2txt (odf_text_test_odt, Odt2TxtOutput);
    EXPECT_EQ (0, ret);
    std::string odt = filter_url_file_get_contents (Odt2TxtOutput);
    std::string standard = "\t\n\n";
    EXPECT_EQ (standard, odt);
  }

  filter_url_unlink (odf_text_test_odt);
  filter_url_unlink (Odt2TxtOutput);
  
  refresh_sandbox (true);
}


#endif

