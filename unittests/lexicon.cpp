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


#include <unittests/lexicon.h>
#include <unittests/utilities.h>
#include <database/hebrewlexicon.h>
#include <lexicon/logic.h>
using namespace std;


void test_lexicons ()
{
  trace_unit_tests (__func__);
  
  {
    Database_HebrewLexicon database;
    string result = database.getaug ("1");
    evaluate (__LINE__, __func__, "aac", result);
  }

  {
    Database_HebrewLexicon database;
    string result = database.getaug ("10");
    evaluate (__LINE__, __func__, "aai", result);
  }

  {
    Database_HebrewLexicon database;
    string result = database.getbdb ("a.aa.aa");
    evaluate (__LINE__, __func__, 160, result.length ());
  }

  {
    Database_HebrewLexicon database;
    string result = database.getbdb ("a.ac.ac");
    evaluate (__LINE__, __func__, 424, result.length ());
  }

  {
    Database_HebrewLexicon database;
    string result = database.getmap ("aaa");
    evaluate (__LINE__, __func__, "a.aa.aa", result);
  }

  {
    Database_HebrewLexicon database;
    string result = database.getmap ("aaj");
    evaluate (__LINE__, __func__, "a.ac.af", result);
  }

  {
    Database_HebrewLexicon database;
    string result = database.getpos ("a");
    evaluate (__LINE__, __func__, "adjective", result);
  }

  {
    Database_HebrewLexicon database;
    string result = database.getpos ("x");
    evaluate (__LINE__, __func__, "indefinite pronoun", result);
  }

  {
    Database_HebrewLexicon database;
    string result = database.getstrong ("H0");
    evaluate (__LINE__, __func__, "", result);
  }

  {
    Database_HebrewLexicon database;
    string result = database.getstrong ("H1");
    evaluate (__LINE__, __func__, 303, result.length ());
  }
  
  {
    Database_HebrewLexicon database;
    string result = database.getstrong ("H2");
    evaluate (__LINE__, __func__, 149, result.length ());
  }
  
  {
    string result = lexicon_logic_render_strongs_definition ("G5590");
    string standard = R"(Strong's 5590 ψυχή psychḗ psoo-khay' from <a href="G5594">5594</a> ; breath, i.e. (by implication) spirit, abstractly or concretely (the animal sentient principle only; thus distinguished on the one hand from <a href="G4151">4151</a> , which is the rational and immortal soul; and on the other from <a href="G2222">2222</a> , which is mere vitality, even of plants: these terms thus exactly correspond respectively to the Hebrew <a href="H05315">05315</a> , <a href="H07307">07307</a> and <a href="H02416">02416</a> ); usage in King James Bible:--heart (+ -ily), life, mind, soul, + us, + you.)";
    evaluate (__LINE__, __func__, standard, result);
  }
}
