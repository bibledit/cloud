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
#include <database/styles.h>
#include <database/state.h>
#include <webserver/request.h>
#include <styles/css.h>
#include <styles/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/css.h>
#include <editor/styles.h>


class styles : public testing::Test {
protected:
  static void SetUpTestSuite() {
  }
  static void TearDownTestSuite() {
  }
  void SetUp() override {
    refresh_sandbox (false);
  }
  void TearDown() override {
    refresh_sandbox (true);
  }
};


// Sheet creation, deletion, getting names.
TEST_F (styles, sheets)
{
  using namespace database::styles;
  
  const std::vector<std::string> standard {styles_logic_standard_sheet ()};
  EXPECT_EQ (standard, get_sheets ());

  const std::string sheet1 {"sheet1"};
  const std::string sheet2 {"sheet2"};

  create_sheet (sheet1);
  const std::vector<std::string> standard_sheet1 {styles_logic_standard_sheet (), sheet1};
  EXPECT_EQ (standard_sheet1, get_sheets ());

  create_sheet (sheet2);
  const std::vector<std::string> standard_sheet1_sheet2 {styles_logic_standard_sheet (), sheet1, sheet2};
  EXPECT_EQ (standard_sheet1_sheet2, get_sheets ());
  
  delete_sheet (sheet1);
  const std::vector<std::string> standard_sheet2 {styles_logic_standard_sheet (), sheet2};
  EXPECT_EQ (standard_sheet2, get_sheets ());

  delete_sheet (sheet2);
  EXPECT_EQ (standard, get_sheets ());
  
  delete_sheet (styles_logic_standard_sheet ());
  EXPECT_EQ (standard, get_sheets ());
}


// Test creating CSS stylesheets.
TEST_F (styles, create_css)
{
  Webserver_Request webserver_request {};
  constexpr const char* testsheet {"testsheet"};
  database::styles::create_sheet (testsheet);

  // Create basic stylesheet.
  {
    Styles_Css styles_css (webserver_request, testsheet);
    styles_css.generate ();
    const std::string css = styles_css.css ();
    //filter_url_file_put_contents ("/tmp/css.css", css);
    const std::string standard = filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "basic.css"}));
    EXPECT_EQ (standard, css);
  }
  
  // Create stylesheet for export.
  {
    Styles_Css styles_css (webserver_request, testsheet);
    styles_css.exports ();
    styles_css.generate ();
    const std::string css = styles_css.css ();
    //filter_url_file_put_contents ("/tmp/css.css", css);
    const std::string standard = filter::strings::trim (filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "exports.css"})));
    EXPECT_EQ (standard, css);
  }
  
  // Stylesheet for the Bible editor.
  {
    Styles_Css styles_css (webserver_request, testsheet);
    styles_css.editor ();
    styles_css.generate ();
    const std::string css = styles_css.css ();
    //filter_url_file_put_contents ("/tmp/css.css", css);
    const std::string standard = filter::strings::trim (filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "editor.css"})));
    //filter_url_file_put_contents ("editor.css", css);
    EXPECT_EQ (standard, css);
  }
}


TEST_F (styles, css_properties)
{
  // Direction.
  {
    EXPECT_EQ ("checked", Filter_Css::directionUnspecified (100));
    EXPECT_EQ ("", Filter_Css::directionUnspecified (101));
    
    EXPECT_EQ ("checked", Filter_Css::directionLeftToRight (101));
    EXPECT_EQ ("", Filter_Css::directionLeftToRight (102));
    
    EXPECT_EQ ("checked", Filter_Css::directionRightToLeft (102));
    EXPECT_EQ ("", Filter_Css::directionRightToLeft (103));
    
    EXPECT_EQ (0, Filter_Css::directionValue (""));
    EXPECT_EQ (1, Filter_Css::directionValue ("ltr"));
    EXPECT_EQ (2, Filter_Css::directionValue ("rtl"));
  }

  // Writing Mode.
  {
    EXPECT_EQ ("checked", Filter_Css::writingModeUnspecified (102));
    EXPECT_EQ ("", Filter_Css::writingModeUnspecified (112));
    
    EXPECT_EQ ("checked", Filter_Css::writingModeTopBottomLeftRight (112));
    EXPECT_EQ ("", Filter_Css::writingModeTopBottomLeftRight (122));
    
    EXPECT_EQ ("checked", Filter_Css::writingModeTopBottomRightLeft (122));
    EXPECT_EQ ("", Filter_Css::writingModeTopBottomRightLeft (132));
    
    EXPECT_EQ ("checked", Filter_Css::writingModeBottomTopLeftRight (132));
    EXPECT_EQ ("", Filter_Css::writingModeBottomTopLeftRight (142));
    
    EXPECT_EQ ("checked", Filter_Css::writingModeBottomTopRightLeft (142));
    EXPECT_EQ ("", Filter_Css::writingModeBottomTopRightLeft (152));
  }
}


// Tests for getting the CSS.
TEST_F (styles, get_css)
{
  std::string css {};
  std::string standard {};
  
  css = Filter_Css::get_css ("class", std::string(), 0);
  standard =
  ".class\n"
  "{\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("class", std::string(), 101);
  standard =
  ".class\n"
  "{\n"
  "direction: ltr;\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("class", std::string(), 102);
  standard =
  ".class\n"
  "{\n"
  "direction: rtl;\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("class", std::string(), 110);
  standard =
  ".class\n"
  "{\n"
  "writing-mode: tb-lr;\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("CLass", std::string(), 130);
  standard =
  ".CLass\n"
  "{\n"
  "writing-mode: bt-lr;\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("CLass", std::string(), 1322);
  standard =
  ".CLass\n"
  "{\n"
  "direction: rtl;\n"
  "writing-mode: tb-rl;\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("Class", "sherif", 0);
  standard =
  ".Class\n"
  "{\n"
  "font-family: sherif;\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("Class", "sherif", 102);
  standard =
  ".Class\n"
  "{\n"
  "font-family: sherif;\n"
  "direction: rtl;\n"
  "}";
  EXPECT_EQ (standard, css);
  
  css = Filter_Css::get_css ("classs", "../font.ttf", 0);
  standard =
  "@font-face\n"
  "{\n"
  "font-family: classs;\n"
  "src: url(../font.ttf);\n"
  "}\n"
  ".classs\n"
  "{\n"
  "font-family: classs;\n"
  "}";
  EXPECT_EQ (standard, css);
}


// Getting the custom class.
TEST_F (styles, get_custom_class)
{
  const std::string custom_class = Filter_Css::getClass ("ആഈഘലറ");
  EXPECT_EQ ("customf86528", custom_class);
}


// Do a spot check on the markers and their associated parameters.
TEST_F (styles, spot_check_markers)
{
  constexpr const char* test_sheet {"testsheet"};
  
  database::styles::create_sheet (test_sheet);
  
  std::vector <std::string> markers {};
  
  markers = database::styles1::get_markers (styles_logic_standard_sheet ());
  EXPECT_EQ (205, static_cast<int>(markers.size ()));
  
  markers = database::styles1::get_markers (test_sheet);
  EXPECT_EQ (205, static_cast<int>(markers.size ()));
  
  std::string marker {"p"};
  if (find (markers.begin (), markers.end (), marker) == markers.end ())
    FAIL() << marker << " not found";
  marker = "add";
  if (find (markers.begin (), markers.end (), marker) == markers.end ())
    FAIL() << marker << " not found";
  
  std::map <std::string, std::string> markers_names = database::styles1::get_markers_and_names (test_sheet);
  EXPECT_EQ (205, markers_names.size());
  EXPECT_EQ ("Blank line", markers_names ["b"]);
  EXPECT_EQ ("Normal paragraph", markers_names ["p"]);
  EXPECT_EQ ("Translator’s addition", markers_names ["add"]);
  
  database::styles1::delete_marker (test_sheet, "p");
  markers = database::styles1::get_markers (test_sheet);
  marker = "p";
  if (find (markers.begin (), markers.end (), marker) != markers.end ())
    FAIL() << marker << " should not be there";
  marker = "add";
  if (find (markers.begin (), markers.end (), marker) == markers.end ())
    FAIL() << marker << " not found";
  
  markers_names = database::styles1::get_markers_and_names (test_sheet);
  EXPECT_EQ (std::string(), markers_names ["p"]);
  EXPECT_EQ ("Translator’s addition", markers_names ["add"]);
}


// More specific check on a marker.
TEST_F (styles, detail_check_markers)
{
  constexpr const char* test_sheet {"testsheet"};
  database::styles::create_sheet (test_sheet);
  database::styles1::Item data = database::styles1::get_marker_data (test_sheet, "add");
  EXPECT_EQ ("add", data.marker);
  EXPECT_EQ ("st", data.category);
}


// Updating a marker.
TEST_F (styles, update_marker)
{
  constexpr const char* test_sheet {"testsheet"};
  database::styles::create_sheet (test_sheet);
  database::styles1::update_name (test_sheet, "add", "Addition");
  database::styles1::Item data = database::styles1::get_marker_data (test_sheet, "add");
  EXPECT_EQ ("Addition", data.name);
  database::styles1::update_info (test_sheet, "p", "Paragraph");
  data = database::styles1::get_marker_data (test_sheet, "p");
  EXPECT_EQ ("Paragraph", data.info);
}


// Read and write access to the styles database.
TEST_F (styles, read_write_access_styles_db)
{
  constexpr const char* test_sheet {"testsheet"};
  database::styles::create_database ();
  database::styles::create_sheet (test_sheet);
  
  // A user does not have write access to the stylesheet.
  bool write = database::styles::has_write_access ("user", test_sheet);
  EXPECT_EQ (false, write);
  
  // Grant write access, and test it for this user, and for another user.
  database::styles::grant_write_access ("user", test_sheet);
  write = database::styles::has_write_access ("user", test_sheet);
  EXPECT_EQ (true, write);
  write = database::styles::has_write_access ("user2", test_sheet);
  EXPECT_EQ (false, write);
  write = database::styles::has_write_access ("user", "testsheet2");
  EXPECT_EQ (false, write);
  
  // Revoke write access for a user, test it in various ways.
  database::styles::revoke_write_access ("user2", test_sheet);
  write = database::styles::has_write_access ("user", test_sheet);
  EXPECT_EQ (true, write);
  database::styles::revoke_write_access ("user", test_sheet);
  write = database::styles::has_write_access ("user", test_sheet);
  EXPECT_EQ (false, write);
  
  // Revoking write access for all users.
  database::styles::grant_write_access ("user1", test_sheet);
  database::styles::grant_write_access ("user2", test_sheet);
  database::styles::revoke_write_access ("", test_sheet);
  write = database::styles::has_write_access ("user1", test_sheet);
  EXPECT_EQ (false, write);
}


// Adding a marker.
TEST_F (styles, add_marker)
{
  constexpr const char* test_sheet {"testsheet"};
  database::styles::create_database ();
  database::styles::create_sheet ("testsheet");
  
  // Get markers.
  std::vector <std::string> markers = database::styles1::get_markers (test_sheet);
  std::string marker {"zhq"};
  if (find (markers.begin (), markers.end (), marker) != markers.end ())
    FAIL() << marker << " should not be there";
  
  // Add marker.
  database::styles1::add_marker (test_sheet, marker);
  markers = database::styles1::get_markers (test_sheet);
  if (find (markers.begin (), markers.end (), marker) == markers.end ())
    FAIL() << marker << " should be there";
}


// Testing empty stylesheet.
TEST_F (styles, empty_stylesheet)
{
  database::styles::create_database ();
  std::vector <std::string> markers = database::styles1::get_markers ("");
  std::string marker {"zhq"};
  if (find (markers.begin (), markers.end (), marker) != markers.end ())
    FAIL() << marker << " should not be there";
}


// Testing the styles application in the editors.
TEST_F (styles, editors_application)
{
  Webserver_Request webserver_request;
  database::styles::create_database ();
  const std::vector<std::pair<const char*, const char*>> marker_action {
    { "add", "c"},
    { "addpn", "c"},
    { "b", "p"},
    { "bd", "c"},
    { "bdit", "c"},
    { "bk", "c"},
    { "c", "p"},
    { "ca", "m"},
    { "cat", "c"},
    { "cd", "p"},
    { "cl", "m"},
    { "cls", "p"},
    { "cp", "m"},
    { "d", "p"},
    { "dc", "c"},
    { "ef", "n"},
    { "em", "c"},
    { "esb", "p"},
    { "esbe", "p"},
    { "ex", "n"},
    { "f", "n"},
    { "fdc", "c"},
    { "fe", "n"},
    { "fig", "m"},
    { "fk", "c"},
    { "fl", "c"},
    { "fm", "c"},
    { "fp", "c"},
    { "fq", "c"},
    { "fqa", "c"},
    { "fr", "c"},
    { "ft", "c"},
    { "fv", "c"},
    { "h", "m"},
    { "h1", "m"},
    { "h2", "m"},
    { "h3", "m"},
    { "ib", "p"},
    { "id", "m"},
    { "ide", "u"},
    { "ie", "m"},
    { "iex", "p"},
    { "ili", "p"},
    { "ili1", "p"},
    { "ili2", "p"},
    { "im", "p"},
    { "imi", "p"},
    { "imq", "p"},
    { "imt", "p"},
    { "imt1", "p"},
    { "imt2", "p"},
    { "imt3", "p"},
    { "imt4", "p"},
    { "imte", "p"},
    { "imte1", "p"},
    { "imte2", "p"},
    { "io", "p"},
    { "io1", "p"},
    { "io2", "p"},
    { "io3", "p"},
    { "io4", "p"},
    { "ior", "c"},
    { "iot", "p"},
    { "ip", "p"},
    { "ipi", "p"},
    { "ipq", "p"},
    { "ipr", "p"},
    { "iq", "p"},
    { "iq1", "p"},
    { "iq2", "p"},
    { "iq3", "p"},
    { "iqt", "c"},
    { "is", "p"},
    { "is1", "p"},
    { "is2", "p"},
    { "it", "c"},
    { "jmp", "c"},
    { "k", "c"},
    { "lf", "p"},
    { "lh", "p"},
    { "li", "p"},
    { "li1", "p"},
    { "li2", "p"},
    { "li3", "p"},
    { "li4", "p"},
    { "lik", "c"},
    { "lim", "p"},
    { "lim1", "p"},
    { "lim2", "p"},
    { "lit", "p"},
    { "litl", "c"},
    { "liv", "c"},
    { "liv1", "c"},
    { "m", "p"},
    { "mi", "p"},
    { "mr", "p"},
    { "ms", "p"},
    { "ms1", "p"},
    { "ms2", "p"},
    { "mt", "p"},
    { "mt1", "p"},
    { "mt2", "p"},
    { "mt3", "p"},
    { "mt4", "p"},
    { "mte", "p"},
    { "mte1", "p"},
    { "mte2", "p"},
    { "nb", "p"},
    { "nd", "c"},
    { "ndx", "c"},
    { "no", "c"},
    { "ord", "c"},
    { "p", "p"},
    { "pb", "u"},
    { "pc", "p"},
    { "periph", "m"},
    { "pi", "p"},
    { "pi1", "p"},
    { "pi2", "p"},
    { "pi3", "p"},
    { "pm", "p"},
    { "pmc", "p"},
    { "pmo", "p"},
    { "pmr", "p"},
    { "pn", "c"},
    { "png", "c"},
    { "po", "p"},
    { "pro", "m"},
    { "q", "p"},
    { "q1", "p"},
    { "q2", "p"},
    { "q3", "p"},
    { "qa", "p"},
    { "qac", "c"},
    { "qc", "p"},
    { "qd", "p"},
    { "qm", "p"},
    { "qm1", "p"},
    { "qm2", "p"},
    { "qm3", "p"},
    { "qr", "p"},
    { "qs", "c"},
    { "qt", "c"},
    { "r", "p"},
    { "rb", "c"},
    { "rem", "m"},
    { "rq", "p"},
    { "rt", "c"},
    { "s", "p"},
    { "s1", "p"},
    { "s2", "p"},
    { "s3", "p"},
    { "s4", "p"},
    { "sc", "c"},
    { "sd", "p"},
    { "sd1", "p"},
    { "sd2", "p"},
    { "sd3", "p"},
    { "sd4", "p"},
    { "sig", "c"},
    { "sls", "c"},
    { "sp", "p"},
    { "sr", "p"},
    { "sts", "m"},
    { "sup", "c"},
    { "tc1", "m"},
    { "tc2", "m"},
    { "tc3", "m"},
    { "tc4", "m"},
    { "tcr1", "m"},
    { "tcr2", "m"},
    { "tcr3", "m"},
    { "tcr4", "m"},
    { "th1", "m"},
    { "th2", "m"},
    { "th3", "m"},
    { "th4", "m"},
    { "thr1", "m"},
    { "thr2", "m"},
    { "thr3", "m"},
    { "thr4", "m"},
    { "tl", "c"},
    { "toc", "m"},
    { "toc1", "m"},
    { "toc2", "m"},
    { "toc3", "m"},
    { "tr", "m"},
    { "v", "c"},
    { "va", "c"},
    { "vp", "c"},
    { "vp", "c"},
    { "w", "c"},
    { "wa", "c"},
    { "wg", "c"},
    { "wh", "c"},
    { "wj", "c"},
    { "x", "n"},
    { "xdc", "c"},
    { "xk", "c"},
    { "xnt", "c"},
    { "xo", "c"},
    { "xop", "c"},
    { "xot", "c"},
    { "xq", "c"},
    { "xt", "c"},
    { "xta", "c"},
  };
  for (const auto& pair : marker_action) {
    const std::string action = Editor_Styles::getAction (webserver_request, pair.first);
    if (action != pair.second)
      FAIL() << "Marker " << std::quoted(pair.first) << " got action " << std::quoted(action) << " but the expected action is " << std::quoted(pair.second);
  }
}


// Getting the list of styles v2.
TEST_F (styles, get_styles_v2)
{
  using namespace database::styles2;

  // Default stylesheet should have the hard-coded default styles.
  const std::list<stylesv2::Style>& default_styles = get_styles(styles_logic_standard_sheet ());
  EXPECT_EQ (default_styles.size(), stylesv2::styles.size());
  
  // Do a spot-check on markers.
  {
    constexpr const char * marker {"id"};
    const auto iter = std::find(default_styles.cbegin(), default_styles.cend(), marker);
    if (iter == default_styles.cend())
      FAIL() << "Should contain style " << std::quoted(marker);
    else {
      const stylesv2::Style& style = *iter;
      EXPECT_EQ (style.type, stylesv2::Type::book_id);
      EXPECT_EQ (style.name, "Identification");
      EXPECT_TRUE (style.properties.count(stylesv2::Property::starts_new_page));
    }
  }
  
  // A marker that is not in the stylesheet.
  {
    constexpr const char * marker {"abc"};
    const auto iter = std::find(default_styles.cbegin(), default_styles.cend(), marker);
    if (iter != default_styles.cend())
      FAIL() << "Should not contain style " << std::quoted(marker);
  }
}


// Test saving and loading style modification v2.
TEST_F (styles, save_load_styles_v2)
{
  using namespace database::styles2;
  using namespace stylesv2;

  // Create a stylesheet for testing.
  constexpr const char* sheet {"sheet"};
  database::styles::create_sheet (sheet);

  stylesv2::Style style;
  
  // Save and load the first default style, and check that the loaded style is the same as the saved one.
  style = stylesv2::styles.front();
  save_style(sheet, style);
  {
    const auto loaded_style = load_style(sheet, style.marker);
    EXPECT_TRUE(loaded_style);
    std::stringstream ss1{};
    ss1 << style;
    std::stringstream ss2{};
    ss2 << loaded_style.value();
    EXPECT_EQ(ss1.str(), ss2.str());
  }
  
  // Set a parameter to false, save and load, and compare again
  style.properties.at(Property::starts_new_page) = false;
  save_style(sheet, style);
  {
    const auto loaded_style = load_style(sheet, style.marker);
    EXPECT_TRUE(loaded_style);
    std::stringstream ss1{};
    ss1 << style;
    std::stringstream ss2{};
    ss2 << loaded_style.value();
    EXPECT_EQ(ss1.str(), ss2.str());
    EXPECT_FALSE(std::get<bool>(loaded_style.value().properties.at(Property::starts_new_page)));
  }

  // Set the same parameter back to true, save and load, and compare again
  style.properties.at(Property::starts_new_page) = true;
  save_style(sheet, style);
  {
    const auto loaded_style = load_style(sheet, style.marker);
    EXPECT_TRUE(loaded_style);
    std::stringstream ss1{};
    ss1 << style;
    std::stringstream ss2{};
    ss2 << loaded_style.value();
    EXPECT_EQ(ss1.str(), ss2.str());
    EXPECT_TRUE(std::get<bool>(loaded_style.value().properties.at(Property::starts_new_page)));
  }

  // Change the name, save, load, compare.
  constexpr const char* name {"name"};
  style.name = name;
  save_style(sheet, style);
  {
    const auto loaded_style = load_style(sheet, style.marker);
    EXPECT_TRUE(loaded_style);
    std::stringstream ss1{};
    ss1 << style;
    std::stringstream ss2{};
    ss2 << loaded_style.value();
    EXPECT_EQ(ss1.str(), ss2.str());
    EXPECT_EQ(loaded_style.value().name, name);
  }

  // Change and check the info field.
  constexpr const char* info {"info"};
  style.info = info;
  save_style(sheet, style);
  {
    const auto loaded_style = load_style(sheet, style.marker);
    EXPECT_TRUE(loaded_style);
    std::stringstream ss1{};
    ss1 << style;
    std::stringstream ss2{};
    ss2 << loaded_style.value();
    EXPECT_EQ(ss1.str(), ss2.str());
    EXPECT_EQ(loaded_style.value().info, info);
  }
  
  // Update the style marker.
  constexpr const char* marker {"marker"};
  style.marker = marker;
  save_style(sheet, style);
  {
    auto loaded_style = load_style(sheet, style.marker);
    EXPECT_TRUE(loaded_style);
    loaded_style.value().implemented = true;
    std::stringstream ss1{};
    ss1 << style;
    std::stringstream ss2{};
    ss2 << loaded_style.value();
    EXPECT_EQ(ss1.str(), ss2.str());
    EXPECT_EQ(loaded_style.value().marker, marker);
  }
}


TEST_F (styles, get_styles_etc_v2)
{
  using namespace database::styles2;

  constexpr const char* sheet {"sheet"};
  database::styles::create_sheet (sheet);
  
  // The default stylesheet has the default number of styles.
  {
    const std::list<stylesv2::Style>& styles = get_styles(styles_logic_standard_sheet());
    EXPECT_EQ (styles.size(), stylesv2::styles.size());
    const std::map <std::string, std::string> markers_names = get_markers_and_names (styles_logic_standard_sheet());
    EXPECT_EQ (markers_names.size(), stylesv2::styles.size());
  }

  // A non-default stylesheet should be created on the fly if it does not exist, and return default styles.
  {
    const std::list<stylesv2::Style>& styles = get_styles(sheet);
    EXPECT_EQ (styles.size(), stylesv2::styles.size());
    const std::map <std::string, std::string> markers_names = get_markers_and_names (sheet);
    EXPECT_EQ (markers_names.size(), stylesv2::styles.size());
  }

  // Add a style. Check the increased styles count.
  constexpr const char* marker {"marker"};
  {
    add_marker(sheet, marker);
    const std::list<stylesv2::Style>& styles = get_styles(sheet);
    EXPECT_EQ (styles.size(), stylesv2::styles.size() + 1);
    const std::map <std::string, std::string> markers_names = get_markers_and_names (sheet);
    EXPECT_EQ (markers_names.size(), stylesv2::styles.size() + 1);
  }
  
  // Check getting the style data.
  {
    const stylesv2::Style* style {get_marker_data(sheet, marker)};
    EXPECT_EQ(marker, style->marker);
    EXPECT_EQ("name", style->name);
    EXPECT_EQ("info", style->info);
  }

  // Check that getting marker data for a style not in the stylesheet returns a null optional.
  {
    const stylesv2::Style* style {get_marker_data(sheet, "unknown")};
    EXPECT_FALSE(style);
  }
  
  // Check for the available markers.
  // It should have the default ones, plus the added one(s).
  {
    const std::vector<std::string> markers {get_markers(sheet)};
    const auto test = [&markers](const std::string& markerv2) {
      const auto iter = std::find(markers.cbegin(), markers.cend(), markerv2);
      if (iter == markers.cend()) {
        FAIL() << "The marker " << std::quoted(markerv2) << " should have been found but was not";
      }
    };
    for (const stylesv2::Style& style : stylesv2::styles) {
      test(style.marker);
    }
    test(marker);
  }
  
  // Delete the added marker and check it's gone.
  delete_marker(sheet, marker);
  {
    const std::vector<std::string> markers = get_markers(sheet);
    const auto iter = std::find(markers.cbegin(), markers.cend(), marker);
    if (iter != markers.cend()) {
      FAIL() << "The marker " << std::quoted(marker) << " should have been deleted but it is still in the list";
    }
  }
  
  // Delete a standard marker, and check it's gone.
  constexpr const char* id_marker {"id"};
  delete_marker(sheet, id_marker);
  {
    const std::vector<std::string> markers = get_markers(sheet);
    const auto iter = std::find(markers.cbegin(), markers.cend(), id_marker);
    if (iter != markers.cend()) {
      FAIL() << "The marker " << std::quoted(id_marker) << " should have been deleted but it is still in the list";
    }
  }
  
  // Add a marker again, check it's there, reset it, check it's gone.
  reset_marker(sheet, id_marker);
  add_marker(sheet, marker);
  {
    const std::list<stylesv2::Style>& styles = get_styles(sheet);
    EXPECT_EQ (styles.size(), stylesv2::styles.size() + 1);
  }
  reset_marker(sheet, marker);
  {
    const std::list<stylesv2::Style>& styles = get_styles(sheet);
    EXPECT_EQ (styles.size(), stylesv2::styles.size());
  }
}


// Test that the default styles are in the correct order of category.
TEST_F (styles, styles_order)
{
  auto previous_category {stylesv2::Category::unknown};
  for (const stylesv2::Style& style : stylesv2::styles) {
    if (style.category == stylesv2::Category::unknown)
      FAIL() << "Marker " << std::quoted(style.marker) << " is not yet categorized properly";
    if (style.category < previous_category) {
      std::stringstream cat{};
      cat << style.category;
      FAIL() << "Marker " << std::quoted(style.marker) << " in category " << std::quoted(cat.str()) << " is listed out of order";
    }
    previous_category = style.category;
  }
}


#endif
