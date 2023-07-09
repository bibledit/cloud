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
#include <unittests/nmt.h>
#include <unittests/utilities.h>
#include <filter/string.h>
#include <nmt/logic.h>
using namespace std;


TEST (nmt, logic)
{
  string reference_text;
  string translating_text;
  vector <string> reference_bits, standard_reference_bits;
  vector <string> translating_bits, standard_translating_bits;

  reference_text = "Reference text one. Ref two.";
  standard_reference_bits = { "Reference text one", "Ref two" };
  translating_text = "Translating text one. Translate two.";
  standard_translating_bits = { "Translating text one", "Translate two" };
  nmt_logic_split (reference_text, translating_text, reference_bits, translating_bits);
  EXPECT_EQ (standard_reference_bits, reference_bits);
  EXPECT_EQ (standard_translating_bits, translating_bits);

  reference_text = "Reference text one! Ref two.";
  standard_reference_bits = { "Reference text one", "Ref two" };
  translating_text = "Translating text one? Translate two.";
  standard_translating_bits = { "Translating text one", "Translate two" };
  nmt_logic_split (reference_text, translating_text, reference_bits, translating_bits);
  EXPECT_EQ (standard_reference_bits, reference_bits);
  EXPECT_EQ (standard_translating_bits, translating_bits);

  reference_text = "ref text, one; ref two";
  standard_reference_bits = { "ref text, one", "ref two" };
  translating_text = "translating text, one? translate two";
  standard_translating_bits = { "translating text, one", "translate two" };
  nmt_logic_split (reference_text, translating_text, reference_bits, translating_bits);
  EXPECT_EQ (standard_reference_bits, reference_bits);
  EXPECT_EQ (standard_translating_bits, translating_bits);

  refresh_sandbox (true);
}

#endif

