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
#include <flate/flate.h>
#include <filter/url.h>


static const std::string tests_folder() {
  return filter_url_create_root_path ({"unittests", "tests"});
  
}

const std::string template_file(const int number) {
  return filter_url_create_path ({tests_folder(), "flate" + std::to_string(number) + ".html"});
}


// Test for the flate template engine.
TEST (flate, basic)
{
  {
    Flate flate {};
    const std::string desired =
    "line 1\n"
    "line 6\n"
    "LocalizeOne\n"
    "LocalizeTwo\n";
    const std::string actual = flate.render (template_file(1));
    EXPECT_EQ (desired, actual);
  }
  
  {
    Flate flate {};
    flate.enable_zone ("one");
    flate.enable_zone ("two");
    const std::string desired =
    "line 1\n"
    "line 2\n"
    "line 3\n"
    "line 4\n"
    "line 6\n"
    "LocalizeOne\n"
    "LocalizeTwo\n";
    const std::string actual = flate.render (template_file(1));
    EXPECT_EQ (desired, actual);
  }
  
  {
    Flate flate;
    flate.enable_zone ("one");
    flate.enable_zone ("three");
    flate.set_variable ("three", "THREE");
    const std::string desired =
    "line 1\n"
    "line 2\n"
    "line 4\n"
    "THREE\n"
    "line 5\n"
    "line 6\n"
    "LocalizeOne\n"
    "LocalizeTwo\n";
    const std::string actual = flate.render (template_file(1));
    EXPECT_EQ (desired, actual);
  }
  
  // Test that a variable containing dashes (#) works OK.
  {
    Flate flate {};
    flate.set_variable ("one", "one##one");
    flate.set_variable ("two", "two####two");
    flate.set_variable ("three", "three######three");
    const std::string desired =
    "one##one\n"
    "two####two\n"
    "three######three\n";
    const std::string actual = flate.render (template_file(2));
    EXPECT_EQ (desired, actual);
  }
  
  // Test iterations.
  {
    Flate flate {};
    flate.add_iteration ("users", {
      std::pair ("one", "RenderingOne"),
      std::pair ("two", "RenderingTwo")
    });
    flate.add_iteration ("users", {
      std::pair ("one", "Translation1"),
      std::pair ("two", "Translation2")
    });
    const std::string actual = flate.render (template_file(3));
    const std::string desired =
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
    EXPECT_EQ (desired, actual);
  }
}

  
#endif
  
