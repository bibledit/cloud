/*
Copyright (©) 2003-2018 Teus Benschop.

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


#include <unittests/javascript.h>
#include <unittests/utilities.h>
#include <filter/shell.h>


void test_javascript ()
{
  trace_unit_tests (__func__);
  
  // In some older browsers it gives this warning or error in the console:
  // -file- is being assigned a //# sourceMappingURL, but already has one.
  // A solution to this is to ensure that minified JavaScript does not assign the source map.
  // This unit test check on regressions in this area.
  {
    string out_err;
    int exitcode = filter_shell_run ("grep sourceMappingURL `find . -name '*.js'`", out_err);
    evaluate (__LINE__, __func__, 256, exitcode);
    evaluate (__LINE__, __func__, "", out_err);
  }
}
