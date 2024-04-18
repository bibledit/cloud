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
#include <filter/image.h>
#include <filter/url.h>
using namespace std;


TEST (filter, image)
{
  // Test some functions of the image filter.
  {
    std::string image_2_name = "bibleimage2.png";
    std::string image_3_name = "bibleimage3.png";
    std::string image_2_path = filter_url_create_root_path ({"unittests", "tests", image_2_name});
    std::string image_3_path = filter_url_create_root_path ({"unittests", "tests", image_3_name});
    int width = 0, height = 0;
    filter_image_get_sizes (image_2_path, width, height);
    EXPECT_EQ (860, width);
    EXPECT_EQ (318, height);
    filter_image_get_sizes (image_3_path, width, height);
    EXPECT_EQ (427, width);
    EXPECT_EQ (304, height);
  }

}

#endif

