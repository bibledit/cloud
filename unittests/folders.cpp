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
#include <filter/url.h>


TEST (folders, basic)
{
  // There should be no empty folders in the library, because git won't include them.
  // If there were such empty folders, they would not be included in the git.
  // Apart from any empty folders in the ./git folder itself,
  // and apart from the Xcode project.
  const int result = system ("find . -type d -empty -not -path './.git/*' -not -path './xcode.xcodeproj/*' > /tmp/bibledittest.txt");
  EXPECT_EQ (0, result);
  const std::string contents = filter_url_file_get_contents ("/tmp/bibledittest.txt");
  EXPECT_EQ (std::string(), contents);
  filter_url_unlink ("/tmp/bibledittest.txt");
}

#endif

