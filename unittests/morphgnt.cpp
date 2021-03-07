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


#include <unittests/morphgnt.h>
#include <unittests/utilities.h>
#include <database/morphgnt.h>


void test_database_morphgnt ()
{
  trace_unit_tests (__func__);
  
  Database_MorphGnt database;
  
  vector <int> results;
  
  results = database.rowids (0, 1, 2);
  evaluate (__LINE__, __func__, 0, results.size ());
  
  results = database.rowids (20, 3, 4);
  evaluate (__LINE__, __func__, 0, results.size ());
  
  results = database.rowids (40, 5, 6);
  evaluate (__LINE__, __func__, 10, results.size ());
  
  results = database.rowids (66, 7, 8);
  evaluate (__LINE__, __func__, 16, results.size ());
}
