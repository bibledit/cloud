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
#include <database/bibleimages.h>
#include <filter/url.h>
using namespace std;


TEST (database, bible_images)
{
  refresh_sandbox (false);

  Database_BibleImages database_bibleimages;
  std::vector <std::string> images;
  string image_1_name = "bibleimage1.png";
  string image_2_name = "bibleimage1.jpeg";
  string image_1_path = filter_url_create_root_path ({"unittests", "tests", image_1_name});
  string image_2_path = filter_url_create_root_path ({"unittests", "tests", image_2_name});
  
  // Initially there's no images yet.
  images = database_bibleimages.get ();
  EXPECT_EQ (0, images.size());

  // Store one image and check it's there.
  database_bibleimages.store (image_1_path);
  images = database_bibleimages.get ();
  EXPECT_EQ (1, images.size());
  if (!images.empty()) {
    EXPECT_EQ (image_1_name, images[0]);
  }

  // Store another image and check there are two of them now.
  database_bibleimages.store (image_2_path);
  images = database_bibleimages.get ();
  EXPECT_EQ (2, images.size());
  EXPECT_EQ ((vector<string>{image_2_name, image_1_name}), images);

  // Erase the first image and a non-existing one, and check the remaining image.
  database_bibleimages.erase (image_1_name);
  database_bibleimages.erase ("non-existing");
  images = database_bibleimages.get ();
  EXPECT_EQ (1, images.size());
  if (!images.empty()) {
    EXPECT_EQ (image_2_name, images[0]);
  }

  // Get the contents of the second image.
  string standard_contents = filter_url_file_get_contents (image_2_path);
  string contents = database_bibleimages.get (image_2_name);
  EXPECT_EQ (standard_contents, contents);

  // Cleanup.
  refresh_sandbox (true);
}

#endif

