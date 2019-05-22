/*
Copyright (©) 2003-2019 Teus Benschop.

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


#include <unittests/md5.h>
#include <unittests/utilities.h>
#include <filter/md5.h>
#include <mbedtls/md5.h>


void test_md5 ()
{
  trace_unit_tests (__func__);

  string apple = "apple";
  string hexits = "1f3870be274f6c49b3e31a0c6728957f";
  
  // The md5 from mbedtls library.
  evaluate (__LINE__, __func__, hexits, md5 (apple));

  // The md5 digest hash of an empty string.
  evaluate (__LINE__, __func__, "d41d8cd98f00b204e9800998ecf8427e", md5 (""));
}
