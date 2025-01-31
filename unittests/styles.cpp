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
  Database_State database_state;
  database_state.create ();
  std::string action = Editor_Styles::getAction (webserver_request, "add");
  EXPECT_EQ ("c", action);
  action = Editor_Styles::getAction (webserver_request, "vp");
  EXPECT_EQ ("c", action);
  action = Editor_Styles::getAction (webserver_request, "id");
  EXPECT_EQ ("m", action);
  refresh_sandbox (true, {"5 Creating sample Bible", "5 Sample Bible was created"});
}


// Getting the list of styles v2.
TEST_F (styles, get_styles_v2)
{
  using namespace database::styles2;

  // As the styles are being built, this indicates the number of styles already available now.
  constexpr const int available_styles {1};
  
  // Default stylesheet should have the hard-coded default styles.
  const std::list<stylesv2::Style>& default_styles = get_styles(styles_logic_standard_sheet ());
  EXPECT_EQ (default_styles.size(), available_styles);
  
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
      EXPECT_EQ (style.capabilities.at(0), stylesv2::Capability::starts_new_page);
    }
  }
  
  // A marker that is not in the stylesheet.
  {
    constexpr const char * marker {"abc"};
    const auto iter = std::find(default_styles.cbegin(), default_styles.cend(), marker);
    if (iter != default_styles.cend())
      FAIL() << "Should not contain style " << std::quoted(marker);
  }
  
  // A non-default stylesheet should be created on the fly if it does not exist, and return default styles.
  const std::list<stylesv2::Style> user_styles = get_styles("sheet");
  EXPECT_EQ (user_styles.size(), available_styles);
}


// Test saving and loading style modification v2.
TEST_F (styles, save_load_styles_v2)
{
  using namespace database::styles2;
  
  // Create a stylesheet for testing.
  constexpr const char* sheet {"sheet"};
  database::styles::create_sheet (sheet);

  // Initially there should not be any updated markers.
  EXPECT_EQ (std::vector<std::string>(), get_updated_markers (sheet));

  // Save the first default style.
  // Since there's no update made on the saved style,
  // the save operation won't write anything to file.
  {
    stylesv2::Style style {stylesv2::styles.front()};
    save_style(sheet, style);
    EXPECT_EQ (std::vector<std::string>(), get_updated_markers (sheet));
  }

  // Make a couple of updates on the default style, and save it, and load it, and check on that.
  {
    stylesv2::Style style {stylesv2::styles.front()};

    constexpr const char* marker {"marker"};
    
    const auto check_marker_count = [&style]() {
      std::vector<std::string> markers = get_updated_markers (sheet);
      constexpr const int marker_count_1 {1};
      EXPECT_EQ (marker_count_1, markers.size());
      if (markers.size() == marker_count_1) {
        EXPECT_EQ(style.marker, markers.at(0));
      }
    };

    const auto check_marker = [marker, &style]() {
      const auto loaded_style = load_style(sheet, marker);
      EXPECT_NE (loaded_style, std::nullopt);
      EXPECT_EQ (loaded_style.value().marker, marker);
    };

    const auto check_type = [marker, &style]() {
      const auto loaded_style = load_style(sheet, marker);
      EXPECT_EQ (static_cast<int>(loaded_style.value().type), static_cast<int>(style.type));
    };

    const auto check_name = [marker, &style]() {
      const auto loaded_style = load_style(sheet, marker);
      EXPECT_EQ (loaded_style.value().name, style.name);
    };

    const auto check_info = [marker, &style]() {
      const auto loaded_style = load_style(sheet, marker);
      EXPECT_EQ (loaded_style.value().info, style.info);
    };

    style.marker = marker;
    save_style(sheet, style);
    check_marker_count();
    check_marker();
    check_type();

    constexpr const char* name {"name"};
    style.name = name;
    save_style(sheet, style);
    check_marker_count();
    check_marker();
    check_type();
    check_name();

    constexpr const char* info {"info"};
    style.info = info;
    save_style(sheet, style);
    check_marker_count();
    check_marker();
    check_type();
    check_name();
    check_info();

    
  }
  
  // Test updating something.
  
  // Test saving default again, it should remove the file.
  
  
  
}


#endif
