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


#include <codecvt>
#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/text.h>
#include <filter/image.h>
#include <editor/html2usfm.h>
#include <editor/html2format.h>
#include <styles/logic.h>
#include <database/state.h>
#include <database/login.h>
#include <database/users.h>
#include <database/bibleimages.h>
#include <webserver/request.h>
#include <user/logic.h>
#include <pugixml/pugixml.hpp>
#include <html/text.h>


using namespace pugi;


void test_dev () // Todo move into place.
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);

  // Test some functions of the image filter.
  {
    string image_2_name = "bibleimage2.png";
    string image_3_name = "bibleimage3.png";
    string image_2_path = filter_url_create_root_path ("unittests", "tests", image_2_name);
    string image_3_path = filter_url_create_root_path ("unittests", "tests", image_3_name);
    int width = 0, height = 0;
    filter_image_get_sizes (image_2_path, width, height);
    evaluate (__LINE__, __func__, 860, width);
    evaluate (__LINE__, __func__, 318, height);
    filter_image_get_sizes (image_3_path, width, height);
    evaluate (__LINE__, __func__, 427, width);
    evaluate (__LINE__, __func__, 304, height);
  }
  
  // Test extraction of all sorts of information from USFM code.
  // Test basic formatting into OpenDocument.
  {
    string image_2_name = "bibleimage2.png";
    string image_3_name = "bibleimage3.png";
    string image_2_path = filter_url_create_root_path ("unittests", "tests", image_2_name);
    string image_3_path = filter_url_create_root_path ("unittests", "tests", image_3_name);
    Database_BibleImages database_bibleimages;
    database_bibleimages.store(image_2_path);
    database_bibleimages.store(image_3_path);
    string bible = "bible";
    string usfm = R"(
\c 1
\p
\v 1 Verse one. \fig caption|src="bibleimage2.png" size="size" ref="reference"\fig*
\v 2 Verse two.
    )";
    string standard = R"(<p class="p"><span class="v">1</span><span> Verse one. </span></p><img alt="" src="bibleimage2.png" width="100%" /><p><span class="v">2</span><span> Verse two.</span></p>)";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.odf_text_standard = new Odf_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet());
    filter_text.odf_text_standard->save ("/Users/teus/Desktop/odt.odt");

//    evaluate (__LINE__, __func__, standard, html);
//    evaluate (__LINE__, __func__, {image_2_name}, filter_text.image_sources);
//    filter_text.html_text_standard->save ("/Users/teus/Desktop/html.html");
    for (auto src : filter_text.image_sources) {
      string contents = database_bibleimages.get(src);
      filter_url_file_put_contents("/Users/teus/Desktop/" + src, contents);
    }
  }
//  refresh_sandbox (true);
  exit (0); // Todo
}
