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
#include <odf/text.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/config/bible.h>


constexpr const char* tmp_test_odt {"/tmp/test.odt"};
constexpr const char* tmp_test_txt {"/tmp/test.txt"};
constexpr const char* bible {"bible"};
constexpr const char* fontname {"Marker Felt"};


class opendocument : public testing::Test {
protected:
  static void SetUpTestSuite() {
  }
  static void TearDownTestSuite() {
  }
  void SetUp() override {
    refresh_sandbox (false);
    database::config::bible::set_export_font (bible, fontname);
    filter_url_unlink (tmp_test_odt);
    filter_url_unlink (tmp_test_txt);
    database::styles::create_database ();
  }
  void TearDown() override {
    refresh_sandbox (true);
  }
};


TEST_F (opendocument, converting_paragraphs)
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
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard =
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


TEST_F (opendocument, automatic_paragraph)
{
  odf_text odf_text (bible);
  odf_text.add_text ("Should create new paragraph automatically");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = ""
  "Should create new paragraph automatically\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, basic_note)
{
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_text ("Text");
  odf_text.add_note ("†", "");
  odf_text.add_note_text ("Note");
  odf_text.add_text (".");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = ""
  "Text†Note\n"
  "\n"
  ".\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, basic_formatted_test_v1)
{
  const database::styles1::Item add = database::styles1::get_marker_data (styles_logic_standard_sheet (), "add");
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_text ("text");
  odf_text.open_text_style (&add, nullptr, false, false);
  odf_text.add_text ("add");
  odf_text.close_text_style (false, false);
  odf_text.add_text ("normal");
  odf_text.add_text (".");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = "textaddnormal.";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, basic_formatted_test_v2)
{
  const stylesv2::Style* pro = database::styles2::get_marker_data (styles_logic_standard_sheet (), "pro");
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_text ("text");
  odf_text.open_text_style (nullptr, pro, false, false);
  odf_text.add_text ("pronunciation");
  odf_text.close_text_style (false, false);
  odf_text.add_text ("normal");
  odf_text.add_text (".");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = "textpronunciationnormal.";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, basic_formatted_note_v1)
{
  const database::styles1::Item add = database::styles1::get_marker_data (styles_logic_standard_sheet (), "add");
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_text ("Text");
  odf_text.add_note ("𐌰", "f");
  odf_text.open_text_style (&add, nullptr, true, false);
  odf_text.add_note_text ("Add");
  odf_text.close_text_style (true, false);
  odf_text.add_note_text ("normal");
  odf_text.add_text (".");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = ""
  "Text𐌰Addnormal\n"
  "\n"
  ".\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, basic_formatted_note_v2)
{
  const stylesv2::Style* pro = database::styles2::get_marker_data (styles_logic_standard_sheet (), "pro");
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_text ("Text");
  odf_text.add_note ("𐌰", "f");
  odf_text.open_text_style (nullptr, pro, true, false);
  odf_text.add_note_text ("Pronunciation");
  odf_text.close_text_style (true, false);
  odf_text.add_note_text ("Normal");
  odf_text.add_text (".");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = ""
  "Text𐌰PronunciationNormal\n"
  "\n"
  ".\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, embedded_formatted_text)
{
  const stylesv2::Style* add = database::styles2::get_marker_data (styles_logic_standard_sheet (), "add");
  stylesv2::Style add2 = *add;
  add2.character.value().italic = stylesv2::FourState::on;
  add2.character.value().bold = stylesv2::FourState::off;
  add2.character.value().underline = stylesv2::FourState::off;
  add2.character.value().smallcaps = stylesv2::FourState::off;
  add2.character.value().superscript = stylesv2::TwoState::off;
  add2.character.value().foreground_color = "#000000";
  const stylesv2::Style* pro = database::styles2::get_marker_data (styles_logic_standard_sheet (), "pro");
  stylesv2::Style pro2 = *pro;
  pro2.character.value().italic = stylesv2::FourState::off;
  pro2.character.value().smallcaps = stylesv2::FourState::on;
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_text ("text");
  odf_text.open_text_style (nullptr, &add2, false, false);
  odf_text.add_text ("add");
  odf_text.open_text_style (nullptr, &pro2, false, true);
  odf_text.add_text ("pro");
  odf_text.close_text_style (false, false);
  odf_text.add_text ("normal");
  odf_text.add_text (".");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  const std::string odt = filter_url_file_get_contents (tmp_test_txt);
  const std::string standard = "textaddpronormal.";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, embedded_formatted_note)
{
  const stylesv2::Style* add = database::styles2::get_marker_data (styles_logic_standard_sheet (), "add");
  stylesv2::Style add2 = *add;
  add2.character.value().italic = stylesv2::FourState::on;
  add2.character.value().bold = stylesv2::FourState::off;
  add2.character.value().underline = stylesv2::FourState::off;
  add2.character.value().smallcaps = stylesv2::FourState::off;
  add2.character.value().superscript = stylesv2::TwoState::off;
  add2.character.value().foreground_color = "#000000";
  const stylesv2::Style* pro = database::styles2::get_marker_data (styles_logic_standard_sheet (), "pro");
  stylesv2::Style pro2 = *pro;
  pro2.character.value().italic = stylesv2::FourState::off;
  pro2.character.value().smallcaps = stylesv2::FourState::on;
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_text ("text");
  odf_text.add_note ("𐌰", "f");
  odf_text.open_text_style (nullptr, &add2, true, false);
  odf_text.add_note_text ("add");
  odf_text.open_text_style (nullptr, &pro2, true, true);
  odf_text.add_note_text ("pro");
  odf_text.close_text_style (true, false);
  odf_text.add_note_text ("normal");
  odf_text.add_text (".");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = ""
  "text𐌰addpronormal\n"
  "\n"
  ".\n";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, paragraph_formatting)
{
  database::styles1::Item d = database::styles1::get_marker_data (styles_logic_standard_sheet (), "d");
  odf_text odf_text (bible);
  // Todo odf_text.create_paragraph_style (d.marker, fontname, d.fontsize, d.italic, d.bold, d.underline, d.smallcaps, d.justification, d.spacebefore, d.spaceafter, d.leftmargin, d.rightmargin, d.firstlineindent, true, false);
  odf_text.new_paragraph ("d");
  odf_text.add_text ("Paragraph with d style");
  odf_text.new_paragraph ("d");
  odf_text.add_text ("Paragraph with d style at first, then Standard");
  odf_text.update_current_paragraph_style (styles_logic_standard_sheet ());
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = ""
  "Paragraph with d style\n"
  "\n"
  "Paragraph with d style at first, then Standard\n"
  "";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, converting_apostrophy)
{
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  EXPECT_EQ (styles_logic_standard_sheet (), odf_text.m_current_paragraph_style);
  odf_text.add_text ("One apostrophy ' and two more ''.");
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = "One apostrophy ' and two more ''.";
  EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (odt));
}


TEST_F (opendocument, tab)
{
  odf_text odf_text (bible);
  odf_text.new_paragraph ();
  odf_text.add_tab ();
  odf_text.save (tmp_test_odt);
  int ret = odf2txt (tmp_test_odt, tmp_test_txt);
  EXPECT_EQ (0, ret);
  std::string odt = filter_url_file_get_contents (tmp_test_txt);
  std::string standard = "\t\n\n";
  EXPECT_EQ (standard, odt);
}

  
#endif

