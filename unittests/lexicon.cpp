/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <unittests/lexicon.h>
#include <unittests/utilities.h>
#include <database/hebrewlexicon.h>


void test_database_hebrewlexicon ()
{
  trace_unit_tests (__func__);
  
  Database_HebrewLexicon database;
  string result;
  
  result = database.getaug ("1");
  evaluate (__LINE__, __func__, "aac", result);
  
  result = database.getaug ("10");
  evaluate (__LINE__, __func__, "aai", result);
  
  result = database.getbdb ("a.aa.aa");
  evaluate (__LINE__, __func__, 160, result.length ());
  
  result = database.getbdb ("a.ac.ac");
  evaluate (__LINE__, __func__, 424, result.length ());
  
  result = database.getmap ("aaa");
  evaluate (__LINE__, __func__, "a.aa.aa", result);
  
  result = database.getmap ("aaj");
  evaluate (__LINE__, __func__, "a.ac.af", result);
  
  result = database.getpos ("a");
  evaluate (__LINE__, __func__, "adjective", result);
  
  result = database.getpos ("x");
  evaluate (__LINE__, __func__, "indefinite pronoun", result);
  
  result = database.getstrong ("H0");
  evaluate (__LINE__, __func__, "", result);
  
  result = database.getstrong ("H1");
  evaluate (__LINE__, __func__, 303, result.length ());
  
  result = database.getstrong ("H2");
  evaluate (__LINE__, __func__, 149, result.length ());
}
