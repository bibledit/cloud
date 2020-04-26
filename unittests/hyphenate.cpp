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


#include <unittests/hyphenate.h>
#include <unittests/utilities.h>
#include <manage/hyphenate.h>


void test_hyphenate ()
{
  trace_unit_tests (__func__);
  
  vector <string> firstset = {"a", "e", "i", "o", "u"};
  vector <string> secondset = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z"};
  string input =
  "\\s \\nd UNkulunkulu\\nd* u\\add ba\\add*xwayisa ngokulunga okungokwabantu 文字ab化け\n"
  "\\s Ukulunga okuku\\nd Kristu\\nd* אבabגד kuyinzuzo אבגד ab";
  string output = hyphenate_at_transition (firstset, secondset, input);
  string standard =
  "\\s \\nd UNku­lu­nku­lu\\nd* u\\add ba\\add*­xwa­yi­sa ngo­ku­lu­nga oku­ngo­kwa­ba­ntu 文字a­b化け\n"
  "\\s Uku­lu­nga oku­ku\\nd Kri­stu\\nd* אבa­bגד ku­yi­nzu­zo אבגד ab";
  evaluate (__LINE__, __func__, standard, output);
}
