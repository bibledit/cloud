/*
Copyright (©) 2003-2020 Teus Benschop.

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


#include <unittests/etcbc4.h>
#include <unittests/utilities.h>
#include <database/etcbc4.h>


void test_database_etcbc4 ()
{
  trace_unit_tests (__func__);
  
  Database_Etcbc4 database;
  
  vector <int> rowids = database.rowids (1, 1, 1);
  evaluate (__LINE__, __func__, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, rowids);
  
  rowids = database.rowids (2, 3, 4);
  evaluate (__LINE__, __func__, {
    29690,
    29691,
    29692,
    29693,
    29694,
    29695,
    29696,
    29697,
    29698,
    29699,
    29700,
    29701,
    29702,
    29703,
    29704,
    29705,
    29706,
    29707,
    29708,
    29709,
    29710,
    29711
  }, rowids);
  
  string result;
  
  result = database.word (2);
  evaluate (__LINE__, __func__, "רֵאשִׁ֖ית", result);
  
  result = database.word (1001);
  evaluate (__LINE__, __func__, "טֹ֛וב", result);
  
  result = database.vocalized_lexeme (2);
  evaluate (__LINE__, __func__, "רֵאשִׁית", result);
  
  result = database.vocalized_lexeme (1001);
  evaluate (__LINE__, __func__, "טֹוב", result);
  
  result = database.consonantal_lexeme (2);
  evaluate (__LINE__, __func__, "ראשׁית", result);
  
  result = database.consonantal_lexeme (1001);
  evaluate (__LINE__, __func__, "טוב", result);
  
  result = database.gloss (2);
  evaluate (__LINE__, __func__, "beginning", result);
  
  result = database.gloss (1001);
  evaluate (__LINE__, __func__, "good", result);
  
  result = database.pos (2);
  evaluate (__LINE__, __func__, "subs", result);
  
  result = database.pos (1001);
  evaluate (__LINE__, __func__, "adjv", result);
  
  result = database.subpos (2);
  evaluate (__LINE__, __func__, "none", result);
  
  result = database.subpos (1001);
  evaluate (__LINE__, __func__, "none", result);
  
  result = database.gender (2);
  evaluate (__LINE__, __func__, "f", result);
  
  result = database.gender (1001);
  evaluate (__LINE__, __func__, "m", result);
  
  result = database.number (4);
  evaluate (__LINE__, __func__, "pl", result);
  
  result = database.number (1001);
  evaluate (__LINE__, __func__, "sg", result);
  
  result = database.person (3);
  evaluate (__LINE__, __func__, "p3", result);
  
  result = database.person (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.state (2);
  evaluate (__LINE__, __func__, "a", result);
  
  result = database.state (1001);
  evaluate (__LINE__, __func__, "a", result);
  
  result = database.tense (3);
  evaluate (__LINE__, __func__, "perf", result);
  
  result = database.tense (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.stem (3);
  evaluate (__LINE__, __func__, "qal", result);
  
  result = database.stem (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_function (2);
  evaluate (__LINE__, __func__, "Time", result);
  
  result = database.phrase_function (1001);
  evaluate (__LINE__, __func__, "PreC", result);
  
  result = database.phrase_type (2);
  evaluate (__LINE__, __func__, "PP", result);
  
  result = database.phrase_type (1001);
  evaluate (__LINE__, __func__, "AdjP", result);
  
  result = database.phrase_relation (2);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_relation (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_a_relation (2);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.phrase_a_relation (1001);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.clause_text_type (2);
  evaluate (__LINE__, __func__, "?", result);
  
  result = database.clause_text_type (1001);
  evaluate (__LINE__, __func__, "NQ", result);
  
  result = database.clause_type (2);
  evaluate (__LINE__, __func__, "xQtX", result);
  
  result = database.clause_type (1001);
  evaluate (__LINE__, __func__, "AjCl", result);
  
  result = database.clause_relation (2);
  evaluate (__LINE__, __func__, "NA", result);
  
  result = database.clause_relation (1001);
  evaluate (__LINE__, __func__, "NA", result);
}
