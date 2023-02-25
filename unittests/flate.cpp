/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <unittests/flate.h>
#include <unittests/utilities.h>
#include <flate/flate.h>
#include <filter/url.h>
using namespace std;


void test_flate ()
{
  trace_unit_tests (__func__);
  
  // Test for the flate2 template engine.
  string folder = filter_url_create_root_path ({"unittests", "tests"});
  string tpl1 = filter_url_create_path ({folder, "flate1.html"});
  string tpl2 = filter_url_create_path ({folder, "flate2.html"});
  string tpl3 = filter_url_create_path ({folder, "flate3.html"});
  
  {
    Flate flate;
    string desired =
    "line 1\n"
    "line 6\n"
    "LocalizeOne\n"
    "LocalizeTwo\n";
    string actual = flate.render (tpl1);
    evaluate (__LINE__, __func__, desired, actual);
    
  }

  {
    Flate flate;
    flate.enable_zone ("one");
    flate.enable_zone ("two");
    string desired =
    "line 1\n"
    "line 2\n"
    "line 3\n"
    "line 4\n"
    "line 6\n"
    "LocalizeOne\n"
    "LocalizeTwo\n";
    string actual = flate.render (tpl1);
    evaluate (__LINE__, __func__, desired, actual);
  }

  {
    Flate flate;
    flate.enable_zone ("one");
    flate.enable_zone ("three");
    flate.set_variable ("three", "THREE");
    string desired =
    "line 1\n"
    "line 2\n"
    "line 4\n"
    "THREE\n"
    "line 5\n"
    "line 6\n"
    "LocalizeOne\n"
    "LocalizeTwo\n";
    string actual = flate.render (tpl1);
    evaluate (__LINE__, __func__, desired, actual);
  }
  
  // Test that a variable containing dashes (#) works OK.
  {
    Flate flate;
    flate.set_variable ("one", "one##one");
    flate.set_variable ("two", "two####two");
    flate.set_variable ("three", "three######three");
    string desired =
    "one##one\n"
    "two####two\n"
    "three######three\n";
    string actual = flate.render (tpl2);
    evaluate (__LINE__, __func__, desired, actual);
  }
  
  // Test iterations.
  {
    Flate flate;
    flate.add_iteration ("users", { pair ("one", "RenderingOne"), pair ("two", "RenderingTwo") });
    flate.add_iteration ("users", { pair ("one", "Translation1"), pair ("two", "Translation2") });
    string actual = flate.render (tpl3);
    string desired =
    "line 1\n"
    "line 2\n"
    "RenderingOne\n"
    "RenderingTwo\n"
    "line 3\n"
    "line 2\n"
    "Translation1\n"
    "Translation2\n"
    "line 3\n"
    "line 4\n";
    evaluate (__LINE__, __func__, desired, actual);
  }
}
