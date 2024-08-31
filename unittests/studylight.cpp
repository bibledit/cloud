/*
Copyright (©) 2003-2024 Teus Benschop.

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
#include <resource/logic.h>
#include <filter/string.h>
#include <database/books.h>


TEST (studylight, albert_barnet)
{
  constexpr auto resource {"Albert Barnes' Notes on the Whole Bible (studylight-eng/bnb)"};
  std::string text {};

  std::vector <book_id> book_ids = database::books::get_ids ();
  for (auto book_id : book_ids) {
    continue;
    std::cout << static_cast <int>(book_id) << std::endl;
    book_type type = database::books::get_type (book_id);
    if ((type != book_type::old_testament) && (type != book_type::new_testament)) continue;
    [[maybe_unused]]int verse {1};
    if (book_id == book_id::_2_chronicles) verse = 2;
    int book_num = static_cast <int> (book_id);
    text = resource_logic_study_light_get (resource, book_num, 1, 1);
    if (text.empty ()) {
      EXPECT_EQ (database::books::get_english_from_id (book_id) + " should not be empty - book " + std::to_string(static_cast<int>(book_id)), std::string());
    }
  }

  constexpr auto isaiah {23};
  text = resource_logic_study_light_get (resource, isaiah, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (361, text.find("parent bird"));

  constexpr auto daniel {27};
  text = resource_logic_study_light_get (resource, daniel, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (293, text.find("For yet the vision is for many days"));

  constexpr auto first_thessalonians {52};
  text = resource_logic_study_light_get (resource, first_thessalonians, 1, 4);
  text = filter::strings::html2text (text);
  EXPECT_EQ (83, text.find("beloved of God, your election"));

  constexpr auto second_thessalonians {53};
  text = resource_logic_study_light_get (resource, second_thessalonians, 1, 4);
  text = filter::strings::html2text (text);
  EXPECT_EQ (354, text.find("You have shownunwavering confidence in God in your afflictions"));

  constexpr auto hebrews {58};
  text = resource_logic_study_light_get (resource, hebrews, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (2917, text.find("τους ἁγιαζομενους"));
  EXPECT_EQ (3678, text.find("By one offering Christ hath forever justifiedsuch as are purged or cleansed by it"));
}


TEST (studylight, thomas_constable)
{
  constexpr auto resource = "Expository Notes of Dr. Thomas Constable (studylight-eng/dcc)";
  constexpr auto hebrews {58};
  std::string text {};
  text = resource_logic_study_light_get (resource, hebrews, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (2011, text.find("accomplishment of our high priest"));
  EXPECT_EQ (2485, text.find("distinctive features of the high priestly office of the Son"));
  EXPECT_EQ (3151, text.find("The one sacrifice of Christ"));
  EXPECT_EQ (3471, text.find("the finality of Jesus Christ’s offering"));
  EXPECT_EQ (4232, text.find("whom Jesus Christ has"));
}


#endif

