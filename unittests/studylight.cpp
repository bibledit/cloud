/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/studylight.h>
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <filter/string.h>
#include <database/books.h>
using namespace std;


void test_studylight ()
{
  trace_unit_tests (__func__);
  
  string resource {};
  int book {};
  string text {};

  resource = "Albert Barnes' Notes on the Whole Bible (studylight-eng/bnb)";

  vector <book_id> book_ids = database::books::get_ids_v2 ();
  for (auto book_id : book_ids) {
    continue;
    book_type type = database::books::get_type (book_id);
    if ((type != book_type::old_testament) && (type != book_type::new_testament)) continue;
    int verse {1};
    if (book_id == book_id::_2_chronicles) verse = 2;
    int book_num = static_cast <int> (book_id);
    text = resource_logic_study_light_get (resource, book_num, 1, 1);
    if (text.empty ()) {
      evaluate (__LINE__, __func__, database::books::get_english_from_id_v2 (book_id) + " should not be empty - book " + to_string(static_cast<int>(book_id)), string());
    }
  }

  book = 23; // Isaiah.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 417, text.find("eggs that are left of the parent bird"));

  book = 27; // Daniel.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 296, text.find("For yet the vision is for many days"));

  book = 52; // 1 Thessalonians.
  text = resource_logic_study_light_get (resource, book, 1, 4);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 84, text.find("beloved of God, your election"));

  book = 53; // 2 Thessalonians.
  text = resource_logic_study_light_get (resource, book, 1, 4);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 357, text.find("You have shown unwavering confidence in God in your afflictions"));

  book = 58; // Hebrews.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 2990, text.find("τους ἁγιαζομενους"));
  evaluate (__LINE__, __func__, 3759, text.find("By one offering Christ hath forever justified such as are purged or cleansed by it"));

  resource = "Expository Notes of Dr. Thomas Constable (studylight-eng/dcc)";
  book = 58; // Hebrews.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 1326, text.find("The accomplishment of our high priest"));
  evaluate (__LINE__, __func__, 1822, text.find("distinctive features of the high priestly office of the Son"));
  evaluate (__LINE__, __func__, 2515, text.find("The one sacrifice of Christ"));
  evaluate (__LINE__, __func__, 2827, text.find("the finality of Jesus Christ’s offering"));
  evaluate (__LINE__, __func__, 3607, text.find("those whom Jesus Christ has perfected"));
}
