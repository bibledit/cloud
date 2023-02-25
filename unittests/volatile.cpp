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


#include <unittests/volatile.h>
#include <unittests/utilities.h>
#include <database/volatile.h>
using namespace std;


void test_database_volatile ()
{
  trace_unit_tests (__func__);
  
  // No value initially.
  string value = Database_Volatile::getValue (1, "key");
  evaluate (__LINE__, __func__, "", value);
  
  // Store value and retrieve it.
  Database_Volatile::setValue (2, "key2", "value2");
  value = Database_Volatile::getValue (2, "key2");
  evaluate (__LINE__, __func__, "value2", value);
  
  // Another key should retrieve nothing.
  value = Database_Volatile::getValue (2, "key1");
  evaluate (__LINE__, __func__, "", value);
}


