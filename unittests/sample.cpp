/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <unittests/sample.h>
#include <unittests/utilities.h>
#include <database/sample.h>
#include <filter/string.h>


void test_database_sample ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  Database_Sample::create ();
  
  // Store samples.
  for (unsigned int i = 1; i <= 5; i++) {
    int file = (1000 * i);
    int data = (10000 * i);
    Database_Sample::store (convert_to_string (file), convert_to_string (data));
  }

  // Check amount of stored sample.
  vector <int> rowids = Database_Sample::get ();
  evaluate (__LINE__, __func__, { 1, 2, 3, 4, 5 }, rowids);
  
  // Retrieve and check the samples.
  for (unsigned int i = 1; i <= 5; i++) {
    string standard_file = convert_to_string (int (1000 * i));
    string standard_data = convert_to_string (int (10000 * i));
    string file, data;
    Database_Sample::get (i, file, data);
    evaluate (__LINE__, __func__, standard_file, file);
    evaluate (__LINE__, __func__, standard_data, data);
  }
}
