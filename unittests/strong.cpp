/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <unittests/strong.h>
#include <unittests/utilities.h>
#include <database/strong.h>


void test_database_strong ()
{
  trace_unit_tests (__func__);
  
  Database_Strong database;
  
  string result = database.definition ("G0");
  evaluate (__LINE__, __func__, "", result);
  
  result = database.definition ("G1");
  int length_h = result.length ();
  
  evaluate (__LINE__, __func__, true, length_h > 100);
  
  vector <string> results = database.strong ("χρηστοσ");
  evaluate (__LINE__, __func__, 1, results.size ());
  if (!results.empty ()) {
    evaluate (__LINE__, __func__, "G5543", results[0]);
  }
}
