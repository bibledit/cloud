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


#include <unittests/studylight.h>
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <resource/external.h>
#include <filter/string.h>


void test_gbs () // Todo
{
  trace_unit_tests (__func__);

  string resource;
  int book, chapter, verse;
  string text;
  
  resource = "Statenbijbel GBS";
  book = 58; // Hebrews.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 10, 14);


  cout << text << endl; // Todo
 
//  resource = "Albert Barnes' Notes on the Whole Bible (studylight-eng/bnb)";
//  text = filter_string_html2text (text);
//  evaluate (__LINE__, __func__, 2973, text.find("τους ἁγιαζομενους"));
//  evaluate (__LINE__, __func__, 3750, text.find("By one offering Christ hath forever justified such as are purged or cleansed by it"));
//
//  resource = "Expository Notes of Dr. Thomas Constable (studylight-eng/dcc)";
//  book = 58; // Hebrews.
//  text = resource_logic_study_light_get (resource, book, 10, 14);
//  text = filter_string_html2text (text);
//  evaluate (__LINE__, __func__, 17, text.find("The accomplishment of our high priest"));
//  evaluate (__LINE__, __func__, 509, text.find("distinctive features of the high priestly office of the Son"));
//  evaluate (__LINE__, __func__, 1183, text.find("The one sacrifice of Christ"));
//  evaluate (__LINE__, __func__, 1490, text.find("the finality of Jesus Christ\"s offering"));
//  evaluate (__LINE__, __func__, 2254, text.find("those whom Jesus Christ has perfected"));
}
