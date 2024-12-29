/*
Copyright (©) 2003-2025 Teus Benschop.

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <manage/hyphenate.h>


TEST (hyphenate, basic)
{
  std::vector <std::string> firstset = {"a", "e", "i", "o", "u"};
  std::vector <std::string> secondset = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z"};
  std::string input =
  "\\s \\nd UNkulunkulu\\nd* u\\add ba\\add*xwayisa ngokulunga okungokwabantu 文字ab化け\n"
  "\\s Ukulunga okuku\\nd Kristu\\nd* אבabגד kuyinzuzo אבגד ab";
  std::string output = hyphenate_at_transition (firstset, secondset, input);
  std::string standard =
  "\\s \\nd UNku­lu­nku­lu\\nd* u\\add ba\\add*­xwa­yi­sa ngo­ku­lu­nga oku­ngo­kwa­ba­ntu 文字a­b化け\n"
  "\\s Uku­lu­nga oku­ku\\nd Kri­stu\\nd* אבa­bגד ku­yi­nzu­zo אבגד ab";
  EXPECT_EQ (standard, output);
}


#endif

