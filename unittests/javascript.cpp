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
#include <filter/shell.h>


TEST (javascript, basic)
{
  // In some older browsers it gives this warning or error in the console:
  // -file- is being assigned a //# sourceMappingURL, but already has one.
  // A solution to this is to ensure that minified JavaScript does not assign the source map.
  // This unit test check on regressions in this area.
  {
    std::string out_err;
    int exitcode = filter::shell::run ("grep '//# sourceMappingURL' `find . -name '*.js'`", out_err);
    EXPECT_EQ (0, exitcode);
    EXPECT_EQ (59, out_err.size());
  }
}

#endif

