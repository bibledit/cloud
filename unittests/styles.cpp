/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/styles.h>
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
using namespace std;


void test_styles ()
{
  trace_unit_tests (__func__);
  Webserver_Request request {};

  // Create basic stylesheet.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.createSheet ("testsheet");
    Styles_Css styles_css = Styles_Css (&request, "testsheet");
    styles_css.generate ();
    string css = styles_css.css ();
    string standard = filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "basic.css"}));
    evaluate (__LINE__, __func__, standard, css);
  }
  
  // Create stylesheet for export.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.createSheet ("testsheet");
    Styles_Css styles_css = Styles_Css (&request, "testsheet");
    styles_css.exports ();
    styles_css.generate ();
    string css = styles_css.css ();
    string standard = filter_string_trim (filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "exports.css"})));
    evaluate (__LINE__, __func__, standard, css);
  }
  
  // Stylesheet for the Bible editor.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.createSheet ("testsheet");
    Styles_Css styles_css = Styles_Css (&request, "testsheet");
    styles_css.editor ();
    styles_css.generate ();
    string css = styles_css.css ();
    string standard = filter_string_trim (filter_url_file_get_contents (filter_url_create_path ({"unittests", "tests", "editor.css"})));
    //filter_url_file_put_contents ("editor.css", css);
    evaluate (__LINE__, __func__, standard, css);
  }
  
  // Direction.
  {
    evaluate (__LINE__, __func__, "checked", Filter_Css::directionUnspecified (100));
    evaluate (__LINE__, __func__, "", Filter_Css::directionUnspecified (101));
    
    evaluate (__LINE__, __func__, "checked", Filter_Css::directionLeftToRight (101));
    evaluate (__LINE__, __func__, "", Filter_Css::directionLeftToRight (102));
    
    evaluate (__LINE__, __func__, "checked", Filter_Css::directionRightToLeft (102));
    evaluate (__LINE__, __func__, "", Filter_Css::directionRightToLeft (103));
    
    evaluate (__LINE__, __func__, 0, Filter_Css::directionValue (""));
    evaluate (__LINE__, __func__, 1, Filter_Css::directionValue ("ltr"));
    evaluate (__LINE__, __func__, 2, Filter_Css::directionValue ("rtl"));
  }

  // Writing Mode
  {
    evaluate (__LINE__, __func__, "checked", Filter_Css::writingModeUnspecified (102));
    evaluate (__LINE__, __func__, "", Filter_Css::writingModeUnspecified (112));
    
    evaluate (__LINE__, __func__, "checked", Filter_Css::writingModeTopBottomLeftRight (112));
    evaluate (__LINE__, __func__, "", Filter_Css::writingModeTopBottomLeftRight (122));
    
    evaluate (__LINE__, __func__, "checked", Filter_Css::writingModeTopBottomRightLeft (122));
    evaluate (__LINE__, __func__, "", Filter_Css::writingModeTopBottomRightLeft (132));
    
    evaluate (__LINE__, __func__, "checked", Filter_Css::writingModeBottomTopLeftRight (132));
    evaluate (__LINE__, __func__, "", Filter_Css::writingModeBottomTopLeftRight (142));
    
    evaluate (__LINE__, __func__, "checked", Filter_Css::writingModeBottomTopRightLeft (142));
    evaluate (__LINE__, __func__, "", Filter_Css::writingModeBottomTopRightLeft (152));
  }

  // CSS.
  {
    string css = Filter_Css::get_css ("class", string(), 0);
    
    string standard =
    ".class\n"
    "{\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
    css = Filter_Css::get_css ("class", string(), 101);
    standard =
    ".class\n"
    "{\n"
    "direction: ltr;\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
    css = Filter_Css::get_css ("class", string(), 102);
    standard =
    ".class\n"
    "{\n"
    "direction: rtl;\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
    css = Filter_Css::get_css ("class", string(), 110);
    standard =
    ".class\n"
    "{\n"
    "writing-mode: tb-lr;\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
    css = Filter_Css::get_css ("CLass", string(), 130);
    standard =
    ".CLass\n"
    "{\n"
    "writing-mode: bt-lr;\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
    css = Filter_Css::get_css ("CLass", string(), 1322);
    standard =
    ".CLass\n"
    "{\n"
    "direction: rtl;\n"
    "writing-mode: tb-rl;\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
    css = Filter_Css::get_css ("Class", "sherif", 0);
    standard =
    ".Class\n"
    "{\n"
    "font-family: sherif;\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
    css = Filter_Css::get_css ("Class", "sherif", 102);
    standard =
    ".Class\n"
    "{\n"
    "font-family: sherif;\n"
    "direction: rtl;\n"
    "}\n";
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
    
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
    evaluate (__LINE__, __func__, filter_string_trim (standard), filter_string_trim (css));
  }

  // Class.
  {
    string clss = Filter_Css::getClass ("ആഈഘലറ");
    evaluate (__LINE__, __func__, "customf86528", clss);
  }
  
  // Creating and deleting stylesheets.
  {
    refresh_sandbox (true);
    Database_Styles database_styles;
    
    vector <string> sheets = database_styles.getSheets ();
    evaluate (__LINE__, __func__, { styles_logic_standard_sheet () }, sheets);
    
    database_styles.createSheet ("testsheet");
    sheets = database_styles.getSheets ();
    evaluate (__LINE__, __func__, { styles_logic_standard_sheet (), "testsheet" }, sheets);
    
    database_styles.deleteSheet ("testsheet");
    sheets = database_styles.getSheets ();
    evaluate (__LINE__, __func__, { styles_logic_standard_sheet () }, sheets);
    
    database_styles.deleteSheet (styles_logic_standard_sheet ());
    sheets = database_styles.getSheets ();
    evaluate (__LINE__, __func__, { styles_logic_standard_sheet () }, sheets);
  }
  
  // Do a spot check on the markers and their associated parameters.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.createSheet ("testsheet");
    
    vector <string> markers {};
    
    markers = database_styles.getMarkers (styles_logic_standard_sheet ());
    evaluate (__LINE__, __func__, 204, static_cast<int>(markers.size ()));
    
    markers = database_styles.getMarkers ("testsheet");
    evaluate (__LINE__, __func__, 204, static_cast<int>(markers.size ()));
    
    string marker {"p"};
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) evaluate (__LINE__, __func__, marker, "not found");
    marker = "add";
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) evaluate (__LINE__, __func__, marker, "not found");
    
    map <string, string> markers_names = database_styles.getMarkersAndNames ("testsheet");
    evaluate (__LINE__, __func__, 204, static_cast<int>(markers_names.size()));
    evaluate (__LINE__, __func__, "Blank line", markers_names ["b"]);
    evaluate (__LINE__, __func__, "Normal paragraph", markers_names ["p"]);
    evaluate (__LINE__, __func__, "Translator’s addition", markers_names ["add"]);
    
    database_styles.deleteMarker ("testsheet", "p");
    markers = database_styles.getMarkers ("testsheet");
    marker = "p";
    if (find (markers.begin (), markers.end (), marker) != markers.end ()) evaluate (__LINE__, __func__, marker, "should not be there");
    marker = "add";
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) evaluate (__LINE__, __func__, marker, "not found");
    
    markers_names = database_styles.getMarkersAndNames ("testsheet");
    evaluate (__LINE__, __func__, "", markers_names ["p"]);
    evaluate (__LINE__, __func__, "Translator’s addition", markers_names ["add"]);
  }
  
  // More specific check on a marker.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.createSheet ("testsheet");
    Database_Styles_Item data = database_styles.getMarkerData ("testsheet", "add");
    evaluate (__LINE__, __func__, "add", data.marker);
    evaluate (__LINE__, __func__, "st", data.category);
  }
  
  // Updating a marker.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.createSheet ("testsheet");
    database_styles.updateName ("testsheet", "add", "Addition");
    Database_Styles_Item data = database_styles.getMarkerData ("testsheet", "add");
    evaluate (__LINE__, __func__, "Addition", data.name);
    database_styles.updateInfo ("testsheet", "p", "Paragraph");
    data = database_styles.getMarkerData ("testsheet", "p");
    evaluate (__LINE__, __func__, "Paragraph", data.info);
  }
  
  // Read and write access to the styles database.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.create ();
    database_styles.createSheet ("testsheet");
    
    // A user does not have write access to the stylesheet.
    bool write = database_styles.hasWriteAccess ("user", "testsheet");
    evaluate (__LINE__, __func__, false, write);
    
    // Grant write access, and test it for this user, and for another user.
    database_styles.grantWriteAccess ("user", "testsheet");
    write = database_styles.hasWriteAccess ("user", "testsheet");
    evaluate (__LINE__, __func__, true, write);
    write = database_styles.hasWriteAccess ("user2", "testsheet");
    evaluate (__LINE__, __func__, false, write);
    write = database_styles.hasWriteAccess ("user", "phpunit2");
    evaluate (__LINE__, __func__, false, write);
    
    // Revoke write access for a user, test it in various ways.
    database_styles.revokeWriteAccess ("user2", "testsheet");
    write = database_styles.hasWriteAccess ("user", "testsheet");
    evaluate (__LINE__, __func__, true, write);
    database_styles.revokeWriteAccess ("user", "testsheet");
    write = database_styles.hasWriteAccess ("user", "testsheet");
    evaluate (__LINE__, __func__, false, write);
    
    // Revoking write access for all users.
    database_styles.grantWriteAccess ("user1", "testsheet");
    database_styles.grantWriteAccess ("user2", "testsheet");
    database_styles.revokeWriteAccess ("", "testsheet");
    write = database_styles.hasWriteAccess ("user1", "testsheet");
    evaluate (__LINE__, __func__, false, write);
  }
  
  // Adding a marker.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.create ();
    database_styles.createSheet ("testsheet");
    
    // Get markers.
    vector <string> markers = database_styles.getMarkers ("testsheet");
    string marker {"zhq"};
    if (find (markers.begin (), markers.end (), marker) != markers.end ()) evaluate (__LINE__, __func__, marker, "should not be there");
    
    // Add marker.
    database_styles.addMarker ("testsheet", marker);
    markers = database_styles.getMarkers ("testsheet");
    if (find (markers.begin (), markers.end (), marker) == markers.end ()) evaluate (__LINE__, __func__, marker, "should be there");
  }

  // Testing empty stylesheet.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.create ();
    vector <string> markers = database_styles.getMarkers ("");
    string marker {"zhq"};
    if (find (markers.begin (), markers.end (), marker) != markers.end ()) evaluate (__LINE__, __func__, marker, "should not be there");
  }
  
  // Testing the styles application in the editors.
  {
    refresh_sandbox (true);
    Database_Styles database_styles {};
    database_styles.create ();
    Database_State database_state;
    database_state.create ();
    string action = Editor_Styles::getAction (&request, "add");
    evaluate (__LINE__, __func__, "c", action);
    action = Editor_Styles::getAction (&request, "vp");
    evaluate (__LINE__, __func__, "c", action);
    action = Editor_Styles::getAction (&request, "id");
    evaluate (__LINE__, __func__, "m", action);
    refresh_sandbox (true, {"5 Creating sample Bible", "5 Sample Bible was created"});
  }
  
  // Done.
  refresh_sandbox (true);
}
