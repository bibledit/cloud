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


#include <unittests/image.h>
#include <unittests/utilities.h>
#include <filter/image.h>
#include <filter/url.h>


void test_image ()
{
  trace_unit_tests (__func__);
  
  // Test some functions of the image filter.
  {
    string image_2_name = "bibleimage2.png";
    string image_3_name = "bibleimage3.png";
    string image_2_path = filter_url_create_root_path_cpp17 ({"unittests", "tests", image_2_name});
    string image_3_path = filter_url_create_root_path_cpp17 ({"unittests", "tests", image_3_name});
    int width = 0, height = 0;
    filter_image_get_sizes (image_2_path, width, height);
    evaluate (__LINE__, __func__, 860, width);
    evaluate (__LINE__, __func__, 318, height);
    filter_image_get_sizes (image_3_path, width, height);
    evaluate (__LINE__, __func__, 427, width);
    evaluate (__LINE__, __func__, 304, height);
  }

}
