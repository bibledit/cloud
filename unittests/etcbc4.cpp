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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/etcbc4.h>
#include <unittests/utilities.h>
#include <database/etcbc4.h>
using namespace std;


TEST (database, etcbc4)
{
  Database_Etcbc4 database;
  
  vector <int> rowids = database.rowids (1, 1, 1);
  EXPECT_EQ ((vector <int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}), rowids);
  
  rowids = database.rowids (2, 3, 4);
  EXPECT_EQ ((vector <int>{
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
  }), rowids);
  
  string result;
  
  result = database.word (2);
  EXPECT_EQ ("רֵאשִׁ֖ית", result);
  
  result = database.word (1001);
  EXPECT_EQ ("טֹ֛וב", result);
  
  result = database.vocalized_lexeme (2);
  EXPECT_EQ ("רֵאשִׁית", result);
  
  result = database.vocalized_lexeme (1001);
  EXPECT_EQ ("טֹוב", result);
  
  result = database.consonantal_lexeme (2);
  EXPECT_EQ ("ראשׁית", result);
  
  result = database.consonantal_lexeme (1001);
  EXPECT_EQ ("טוב", result);
  
  result = database.gloss (2);
  EXPECT_EQ ("beginning", result);
  
  result = database.gloss (1001);
  EXPECT_EQ ("good", result);
  
  result = database.pos (2);
  EXPECT_EQ ("subs", result);
  
  result = database.pos (1001);
  EXPECT_EQ ("adjv", result);
  
  result = database.subpos (2);
  EXPECT_EQ ("none", result);
  
  result = database.subpos (1001);
  EXPECT_EQ ("none", result);
  
  result = database.gender (2);
  EXPECT_EQ ("f", result);
  
  result = database.gender (1001);
  EXPECT_EQ ("m", result);
  
  result = database.number (4);
  EXPECT_EQ ("pl", result);
  
  result = database.number (1001);
  EXPECT_EQ ("sg", result);
  
  result = database.person (3);
  EXPECT_EQ ("p3", result);
  
  result = database.person (1001);
  EXPECT_EQ ("NA", result);
  
  result = database.state (2);
  EXPECT_EQ ("a", result);
  
  result = database.state (1001);
  EXPECT_EQ ("a", result);
  
  result = database.tense (3);
  EXPECT_EQ ("perf", result);
  
  result = database.tense (1001);
  EXPECT_EQ ("NA", result);
  
  result = database.stem (3);
  EXPECT_EQ ("qal", result);
  
  result = database.stem (1001);
  EXPECT_EQ ("NA", result);
  
  result = database.phrase_function (2);
  EXPECT_EQ ("Time", result);
  
  result = database.phrase_function (1001);
  EXPECT_EQ ("PreC", result);
  
  result = database.phrase_type (2);
  EXPECT_EQ ("PP", result);
  
  result = database.phrase_type (1001);
  EXPECT_EQ ("AdjP", result);
  
  result = database.phrase_relation (2);
  EXPECT_EQ ("NA", result);
  
  result = database.phrase_relation (1001);
  EXPECT_EQ ("NA", result);
  
  result = database.phrase_a_relation (2);
  EXPECT_EQ ("NA", result);
  
  result = database.phrase_a_relation (1001);
  EXPECT_EQ ("NA", result);
  
  result = database.clause_text_type (2);
  EXPECT_EQ ("?", result);
  
  result = database.clause_text_type (1001);
  EXPECT_EQ ("NQ", result);
  
  result = database.clause_type (2);
  EXPECT_EQ ("xQtX", result);
  
  result = database.clause_type (1001);
  EXPECT_EQ ("AjCl", result);
  
  result = database.clause_relation (2);
  EXPECT_EQ ("NA", result);
  
  result = database.clause_relation (1001);
  EXPECT_EQ ("NA", result);
}

#endif

