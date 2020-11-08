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


#include <unittests/related.h>
#include <unittests/utilities.h>
#include <filter/passage.h>
#include <related/logic.h>


// Test the related verses feature.
void test_related ()
{
  trace_unit_tests (__func__);

  {
    // Test situation of one input passage that has several output passages.
    vector <Passage> output = related_logic_get_verses ({ Passage ("", 1, 10, "6") });
    int size = 6;
    evaluate (__LINE__, __func__, size, output.size ());
    if ((int)output.size () == size) {
      evaluate (__LINE__, __func__, true, Passage ("", 1, 10, "6").equal (output[0]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 10, "7").equal (output[1]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 10, "8").equal (output[2]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "8").equal (output[3]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "9").equal (output[4]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "10").equal (output[5]));
    }
  }

  {
    // Test that an input passage that finds no parallel passages in the XML files results in output equal to the input.
    vector <Passage> output = related_logic_get_verses ({ Passage ("", 2, 3, "4") });
    evaluate (__LINE__, __func__, 0, output.size ());
  }

  {
    // Test input passages that match two entries in the XML files.
    vector <Passage> output = related_logic_get_verses ({ Passage ("", 1, 10, "29"), Passage ("", 1, 11, "12") });
    int size = 12;
    evaluate (__LINE__, __func__, size, output.size ());
    if ((int)output.size () == size) {
      evaluate (__LINE__, __func__, true, Passage ("", 1, 10, "26").equal (output[0]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 10, "27").equal (output[1]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 10, "28").equal (output[2]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 10, "29").equal (output[3]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 11, "12").equal (output[4]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 11, "13").equal (output[5]));
      evaluate (__LINE__, __func__, true, Passage ("", 1, 11, "14").equal (output[6]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "18").equal (output[7]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "20").equal (output[8]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "21").equal (output[9]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "22").equal (output[10]));
      evaluate (__LINE__, __func__, true, Passage ("", 13, 1, "23").equal (output[11]));
    }
  }

  {
    // Test third passage of synoptic parallels whether it also gets the first two passages, and the fourth.
    // At the same time it takes data from the quotations XML.
    vector <Passage> output = related_logic_get_verses ({ Passage ("", 42, 3, "4") });
    int size = 10;
    evaluate (__LINE__, __func__, size, output.size ());
    if ((int)output.size () == size) {
      evaluate (__LINE__, __func__, true, Passage ("", 23, 40, "3").equal (output[0]));
      evaluate (__LINE__, __func__, true, Passage ("", 23, 40, "4").equal (output[1]));
      evaluate (__LINE__, __func__, true, Passage ("", 23, 40, "5").equal (output[2]));
      evaluate (__LINE__, __func__, true, Passage ("", 40, 3, "3").equal (output[3]));
      evaluate (__LINE__, __func__, true, Passage ("", 41, 1, "2").equal (output[4]));
      evaluate (__LINE__, __func__, true, Passage ("", 41, 1, "3").equal (output[5]));
      evaluate (__LINE__, __func__, true, Passage ("", 42, 3, "4").equal (output[6]));
      evaluate (__LINE__, __func__, true, Passage ("", 42, 3, "5").equal (output[7]));
      evaluate (__LINE__, __func__, true, Passage ("", 42, 3, "6").equal (output[8]));
      evaluate (__LINE__, __func__, true, Passage ("", 43, 1, "23").equal (output[9]));
    }
  }
}
