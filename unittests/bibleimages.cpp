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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <database/bibleimages.h>
#include <filter/url.h>


TEST (database, bible_images)
{
  refresh_sandbox (false);

  std::vector <std::string> images;
 
  constexpr const auto image_1_name = "bibleimage1.png";
  constexpr const auto image_2_name = "bibleimage1.jpeg";
  std::string image_1_path = filter_url_create_root_path ({"unittests", "tests", image_1_name});
  std::string image_2_path = filter_url_create_root_path ({"unittests", "tests", image_2_name});
  
  // Initially there's no images yet.
  images = database::bible_images::get ();
  EXPECT_EQ (0, images.size());

  // Store one image and check it's there.
  database::bible_images::store (image_1_path);
  images = database::bible_images::get ();
  EXPECT_EQ (1, images.size());
  if (!images.empty()) {
    EXPECT_EQ (image_1_name, images[0]);
  }

  // Store another image and check there are two of them now.
  database::bible_images::store (image_2_path);
  images = database::bible_images::get ();
  EXPECT_EQ (2, images.size());
  EXPECT_EQ ((std::vector<std::string>{image_2_name, image_1_name}), images);

  // Erase the first image and a non-existing one, and check the remaining image.
  database::bible_images::erase (image_1_name);
  database::bible_images::erase ("non-existing");
  images = database::bible_images::get ();
  EXPECT_EQ (1, images.size());
  if (!images.empty()) {
    EXPECT_EQ (image_2_name, images[0]);
  }

  // Get the contents of the second image.
  std::string standard_contents = filter_url_file_get_contents (image_2_path);
  std::string contents = database::bible_images::get (image_2_name);
  EXPECT_EQ (standard_contents, contents);

  // Cleanup.
  refresh_sandbox (true);
}

#endif
