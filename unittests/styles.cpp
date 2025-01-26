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


TEST (styles, css)
{
  Webserver_Request webserver_request {};

  // Create basic stylesheet.
  {
    refresh_sandbox (true);
    database::styles::create_sheet ("testsheet");
    Styles_Css styles_css (webserver_request, "testsheet");
    styles_css.generate ();
    std::string css = styles_css.css ();
    //filter_url_file_put_contents ("/tmp/css.css", css);
    std::string standard = filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "basic.css"}));
    EXPECT_EQ (standard, css);
  }
  
  // Create stylesheet for export.
  {
    refresh_sandbox (true);
    database::styles::create_sheet ("testsheet");
    Styles_Css styles_css (webserver_request, "testsheet");
    styles_css.exports ();
    styles_css.generate ();
    std::string css = styles_css.css ();
    //filter_url_file_put_contents ("/tmp/css.css", css);
    std::string standard = filter::strings::trim (filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "exports.css"})));
    EXPECT_EQ (standard, css);
  }
  
  // Stylesheet for the Bible editor.
  {
    refresh_sandbox (true);
    database::styles::create_sheet ("testsheet");
    Styles_Css styles_css (webserver_request, "testsheet");
    styles_css.editor ();
    styles_css.generate ();
    std::string css = styles_css.css ();
    //filter_url_file_put_contents ("/tmp/css.css", css);
    std::string standard = filter::strings::trim (filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "editor.css"})));
    //filter_url_file_put_contents ("editor.css", css);
    EXPECT_EQ (standard, css);
  }
  
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

  // Writing Mode
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

  // CSS.
  {
    std::string css = Filter_Css::get_css ("class", std::string(), 0);
    
    std::string standard =
    ".class\n"
    "{\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
    css = Filter_Css::get_css ("class", std::string(), 101);
    standard =
    ".class\n"
    "{\n"
    "direction: ltr;\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
    css = Filter_Css::get_css ("class", std::string(), 102);
    standard =
    ".class\n"
    "{\n"
    "direction: rtl;\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
    css = Filter_Css::get_css ("class", std::string(), 110);
    standard =
    ".class\n"
    "{\n"
    "writing-mode: tb-lr;\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
    css = Filter_Css::get_css ("CLass", std::string(), 130);
    standard =
    ".CLass\n"
    "{\n"
    "writing-mode: bt-lr;\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
    css = Filter_Css::get_css ("CLass", std::string(), 1322);
    standard =
    ".CLass\n"
    "{\n"
    "direction: rtl;\n"
    "writing-mode: tb-rl;\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
    css = Filter_Css::get_css ("Class", "sherif", 0);
    standard =
    ".Class\n"
    "{\n"
    "font-family: sherif;\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
    css = Filter_Css::get_css ("Class", "sherif", 102);
    standard =
    ".Class\n"
    "{\n"
    "font-family: sherif;\n"
    "direction: rtl;\n"
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
    
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
    "}\n";
    EXPECT_EQ (filter::strings::trim (standard), filter::strings::trim (css));
  }

  // Class.
  {
    std::string clss = Filter_Css::getClass ("ആഈഘലറ");
    EXPECT_EQ ("customf86528", clss);
  }
  
  // Creating and deleting stylesheets.
  {
    refresh_sandbox (true);
    std::vector <std::string> standard {};
    
    std::vector <std::string> sheets = database::styles::get_sheets ();
    standard = { styles_logic_standard_sheet () };
    EXPECT_EQ (standard, sheets);
    
    database::styles::create_sheet ("testsheet");
    sheets = database::styles::get_sheets ();
    standard = { styles_logic_standard_sheet (), "testsheet" };
    EXPECT_EQ (standard, sheets);
    
    database::styles::delete_sheet ("testsheet");
    sheets = database::styles::get_sheets ();
    standard = { styles_logic_standard_sheet () };
    EXPECT_EQ (standard, sheets);
    
    database::styles::delete_sheet (styles_logic_standard_sheet ());
    sheets = database::styles::get_sheets ();
    standard = { styles_logic_standard_sheet () };
    EXPECT_EQ (standard, sheets);
  }
  
  // Do a spot check on the markers and their associated parameters.
  {
    refresh_sandbox (true);
    database::styles::create_sheet ("testsheet");
    
    std::vector <std::string> markers {};
    
    markers = database::styles1::get_markers (styles_logic_standard_sheet ());
    EXPECT_EQ (205, static_cast<int>(markers.size ()));
    
    markers = database::styles1::get_markers ("testsheet");
    EXPECT_EQ (205, static_cast<int>(markers.size ()));
    
    std::string marker {"p"};
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) EXPECT_EQ (marker, "not found");
    marker = "add";
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) EXPECT_EQ (marker, "not found");
    
    std::map <std::string, std::string> markers_names = database::styles1::get_markers_and_names ("testsheet");
    EXPECT_EQ (205, static_cast<int>(markers_names.size()));
    EXPECT_EQ ("Blank line", markers_names ["b"]);
    EXPECT_EQ ("Normal paragraph", markers_names ["p"]);
    EXPECT_EQ ("Translator’s addition", markers_names ["add"]);
    
    database::styles1::delete_marker ("testsheet", "p");
    markers = database::styles1::get_markers ("testsheet");
    marker = "p";
    if (find (markers.begin (), markers.end (), marker) != markers.end ()) EXPECT_EQ (marker, "should not be there");
    marker = "add";
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) EXPECT_EQ (marker, "not found");
    
    markers_names = database::styles1::get_markers_and_names ("testsheet");
    EXPECT_EQ ("", markers_names ["p"]);
    EXPECT_EQ ("Translator’s addition", markers_names ["add"]);
  }
  
  // More specific check on a marker.
  {
    refresh_sandbox (true);
    database::styles::create_sheet ("testsheet");
    database::styles1::Item data = database::styles1::get_marker_data ("testsheet", "add");
    EXPECT_EQ ("add", data.marker);
    EXPECT_EQ ("st", data.category);
  }
  
  // Updating a marker.
  {
    refresh_sandbox (true);
    database::styles::create_sheet ("testsheet");
    database::styles1::update_name ("testsheet", "add", "Addition");
    database::styles1::Item data = database::styles1::get_marker_data ("testsheet", "add");
    EXPECT_EQ ("Addition", data.name);
    database::styles1::update_info ("testsheet", "p", "Paragraph");
    data = database::styles1::get_marker_data ("testsheet", "p");
    EXPECT_EQ ("Paragraph", data.info);
  }
  
  // Read and write access to the styles database.
  {
    refresh_sandbox (true);
    database::styles::create_database ();
    database::styles::create_sheet ("testsheet");
    
    // A user does not have write access to the stylesheet.
    bool write = database::styles::has_write_access ("user", "testsheet");
    EXPECT_EQ (false, write);
    
    // Grant write access, and test it for this user, and for another user.
    database::styles::grant_write_access ("user", "testsheet");
    write = database::styles::has_write_access ("user", "testsheet");
    EXPECT_EQ (true, write);
    write = database::styles::has_write_access ("user2", "testsheet");
    EXPECT_EQ (false, write);
    write = database::styles::has_write_access ("user", "phpunit2");
    EXPECT_EQ (false, write);
    
    // Revoke write access for a user, test it in various ways.
    database::styles::revoke_write_access ("user2", "testsheet");
    write = database::styles::has_write_access ("user", "testsheet");
    EXPECT_EQ (true, write);
    database::styles::revoke_write_access ("user", "testsheet");
    write = database::styles::has_write_access ("user", "testsheet");
    EXPECT_EQ (false, write);
    
    // Revoking write access for all users.
    database::styles::grant_write_access ("user1", "testsheet");
    database::styles::grant_write_access ("user2", "testsheet");
    database::styles::revoke_write_access ("", "testsheet");
    write = database::styles::has_write_access ("user1", "testsheet");
    EXPECT_EQ (false, write);
  }
  
  // Adding a marker.
  {
    refresh_sandbox (true);
    database::styles::create_database ();
    database::styles::create_sheet ("testsheet");
    
    // Get markers.
    std::vector <std::string> markers = database::styles1::get_markers ("testsheet");
    std::string marker {"zhq"};
    if (find (markers.begin (), markers.end (), marker) != markers.end ()) EXPECT_EQ (marker, "should not be there");
    
    // Add marker.
    database::styles1::add_marker ("testsheet", marker);
    markers = database::styles1::get_markers ("testsheet");
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) EXPECT_EQ (marker, "should be there");
  }

  // Testing empty stylesheet.
  {
    refresh_sandbox (true);
    database::styles::create_database ();
    std::vector <std::string> markers = database::styles1::get_markers ("");
    std::string marker {"zhq"};
    if (find (markers.begin (), markers.end (), marker) != markers.end ()) EXPECT_EQ (marker, "should not be there");
  }
  
  // Testing the styles application in the editors.
  {
    refresh_sandbox (true);
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
  
  // Done.
  refresh_sandbox (true);
}


#endif

