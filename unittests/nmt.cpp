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


#include <unittests/nmt.h>
#include <unittests/utilities.h>
#include <filter/string.h>
#include <nmt/logic.h>


void test_nmt ()
{
  trace_unit_tests (__func__);

  string reference_text;
  string translating_text;
  vector <string> reference_bits, standard_reference_bits;
  vector <string> translating_bits, standard_translating_bits;

  reference_text = "Verse text English. Remainder.";
  standard_reference_bits = { "Verse text English", " Remainder" };
  translating_text = "Verse text Shona. Remainder.";
  standard_translating_bits = { "Verse text Shona", " Remainder" };
  nmt_logic_split (reference_text, translating_text, reference_bits, translating_bits);
  evaluate (__LINE__, __func__, standard_reference_bits, reference_bits);
  evaluate (__LINE__, __func__, standard_translating_bits, translating_bits);
  
  for (auto s : reference_bits) cout << s << endl; // Todo
  for (auto s : translating_bits) cout << s << endl; // Todo

  refresh_sandbox (true);
}
