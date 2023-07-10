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
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <filter/string.h>
#include <database/books.h>
using namespace std;


TEST (studylight, logic)
{
  string resource {};
  int book {};
  string text {};

  resource = "Albert Barnes' Notes on the Whole Bible (studylight-eng/bnb)";

  vector <book_id> book_ids = database::books::get_ids ();
  for (auto book_id : book_ids) {
    continue;
    std::cout << static_cast <int>(book_id) << std::endl;
    book_type type = database::books::get_type (book_id);
    if ((type != book_type::old_testament) && (type != book_type::new_testament)) continue;
    int verse {1};
    if (book_id == book_id::_2_chronicles) verse = 2;
    int book_num = static_cast <int> (book_id);
    text = resource_logic_study_light_get (resource, book_num, 1, 1);
    if (text.empty ()) {
      EXPECT_EQ (database::books::get_english_from_id (book_id) + " should not be empty - book " + to_string(static_cast<int>(book_id)), string());
    }
  }

  book = 23; // Isaiah.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (361, text.find("parent bird"));

  book = 27; // Daniel.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (293, text.find("For yet the vision is for many days"));

  book = 52; // 1 Thessalonians.
  text = resource_logic_study_light_get (resource, book, 1, 4);
  text = filter::strings::html2text (text);
  EXPECT_EQ (83, text.find("beloved of God, your election"));

  book = 53; // 2 Thessalonians.
  text = resource_logic_study_light_get (resource, book, 1, 4);
  text = filter::strings::html2text (text);
  EXPECT_EQ (354, text.find("You have shownunwavering confidence in God in your afflictions"));

  book = 58; // Hebrews.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (2917, text.find("τους ἁγιαζομενους"));
  EXPECT_EQ (3678, text.find("By one offering Christ hath forever justifiedsuch as are purged or cleansed by it"));

  resource = "Expository Notes of Dr. Thomas Constable (studylight-eng/dcc)";
  book = 58; // Hebrews.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (2011, text.find("accomplishment of our high priest"));
  EXPECT_EQ (2485, text.find("distinctive features of the high priestly office of the Son"));
  EXPECT_EQ (3151, text.find("The one sacrifice ofChrist"));
  EXPECT_EQ (3471, text.find("the finality of Jesus Christ’s offering"));
  EXPECT_EQ (4232, text.find("whom Jesus Christ has"));
}

#endif

