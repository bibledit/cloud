/*
Copyright (©) 2003-2026 Teus Benschop.

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
#include <database/books.h>


// Tests for the Database_Books object.
TEST (database, books)
{
  refresh_sandbox (false);
  EXPECT_EQ (109, database::books::get_ids ().size());
  EXPECT_EQ (static_cast<int>(book_id::_exodus), static_cast<int>(database::books::get_id_from_english ("Exodus")));
  EXPECT_EQ (static_cast<int>(book_id::_unknown), static_cast<int>(database::books::get_id_from_english ("exodus")));
  EXPECT_EQ ("Leviticus", database::books::get_english_from_id (book_id::_leviticus));
  EXPECT_EQ ("NUM", database::books::get_usfm_from_id (book_id::_numbers));
  EXPECT_EQ ("Deu", database::books::get_bibleworks_from_id (book_id::_deuteronomy));
  EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(database::books::get_id_from_usfm ("SNG")));
  EXPECT_EQ (static_cast<int>(book_id::_1_chronicles), static_cast<int>(database::books::get_id_from_osis ("1Chr")));
  EXPECT_EQ (static_cast<int>(book_id::_2_kings), static_cast<int>(database::books::get_id_from_bibleworks ("2Ki")));
  EXPECT_EQ (static_cast<int>(book_id::_2_kings), static_cast<int>(database::books::get_id_like_text ("2Ki")));
  EXPECT_EQ (static_cast<int>(book_id::_2_chronicles), static_cast<int>(database::books::get_id_like_text ("2Chron")));
  EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(database::books::get_id_like_text ("Genes")));
  EXPECT_EQ (static_cast<int>(book_id::_2_kings), static_cast<int>(database::books::get_id_from_onlinebible ("2Ki")));
  EXPECT_EQ ("De", database::books::get_onlinebible_from_id (book_id::_deuteronomy));
  EXPECT_EQ (7, database::books::get_order_from_id (book_id::_deuteronomy));
  EXPECT_EQ (static_cast <int> (book_type::new_testament), static_cast <int> (database::books::get_type (book_id::_matthew)));
  EXPECT_EQ (static_cast <int> (book_type::old_testament), static_cast <int> (database::books::get_type (book_id::_malachi)));
  EXPECT_EQ (static_cast <int> (book_type::unknown), static_cast <int> (database::books::get_type (book_id::_unknown)));
  EXPECT_EQ (static_cast <int> (book_id::_introduction_matter), static_cast <int> (database::books::get_id_from_usfm ("INT")));
}


#endif
