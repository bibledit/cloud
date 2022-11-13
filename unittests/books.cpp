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


#include <unittests/archive.h>
#include <unittests/utilities.h>
#include <database/books.h>
using namespace std;


// Tests for the Database_Books object.
void test_database_books ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  evaluate (__LINE__, __func__, 109, database::books::get_ids_v1 ().size());
  evaluate (__LINE__, __func__, 109, database::books::get_ids_v2 ().size());
  evaluate (__LINE__, __func__, 2, database::books::get_id_from_english_v1 ("Exodus"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_exodus), static_cast<int>(database::books::get_id_from_english_v2 ("Exodus")));
  evaluate (__LINE__, __func__, 0, database::books::get_id_from_english_v1 ("exodus"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_unknown), static_cast<int>(database::books::get_id_from_english_v2 ("exodus")));
  evaluate (__LINE__, __func__, "Leviticus", database::books::get_english_from_id_v1 (3));
  evaluate (__LINE__, __func__, "Leviticus", database::books::get_english_from_id_v2 (book_id::_leviticus));
  evaluate (__LINE__, __func__, "NUM", database::books::get_usfm_from_id_v1 (4));
  evaluate (__LINE__, __func__, "NUM", database::books::get_usfm_from_id_v2 (book_id::_numbers));
  evaluate (__LINE__, __func__, "Deu", database::books::get_bibleworks_from_id_v1 (5));
  evaluate (__LINE__, __func__, "Deu", database::books::get_bibleworks_from_id_v2 (book_id::_deuteronomy));
  evaluate (__LINE__, __func__, 22, database::books::get_id_from_usfm_v1 ("SNG"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_song_of_solomon), static_cast<int>(database::books::get_id_from_usfm_v2 ("SNG")));
  evaluate (__LINE__, __func__, 13, database::books::get_id_from_osis_v1 ("1Chr"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_chronicles), static_cast<int>(database::books::get_id_from_osis_v2 ("1Chr")));
  evaluate (__LINE__, __func__, 12, database::books::get_id_from_bibleworks_v1 ("2Ki"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_kings), static_cast<int>(database::books::get_id_from_bibleworks_v2 ("2Ki")));
  evaluate (__LINE__, __func__, 12, database::books::get_id_like_text_v1 ("2Ki"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_kings), static_cast<int>(database::books::get_id_like_text_v2 ("2Ki")));
  evaluate (__LINE__, __func__, 14, database::books::get_id_like_text_v1 ("2Chron"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_chronicles), static_cast<int>(database::books::get_id_like_text_v2 ("2Chron")));
  evaluate (__LINE__, __func__, 1, database::books::get_id_like_text_v1 ("Genes"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_genesis), static_cast<int>(database::books::get_id_like_text_v2 ("Genes")));
  evaluate (__LINE__, __func__, 12, database::books::get_id_from_onlinebible_v1 ("2Ki"));
  evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_kings), static_cast<int>(database::books::get_id_from_onlinebible_v2 ("2Ki")));
  evaluate (__LINE__, __func__, "De", database::books::get_onlinebible_from_id_v1 (5));
  evaluate (__LINE__, __func__, "De", database::books::get_onlinebible_from_id_v2 (book_id::_deuteronomy));
  evaluate (__LINE__, __func__, 7, database::books::get_order_from_id_v2 (book_id::_deuteronomy));
  evaluate (__LINE__, __func__, static_cast <int> (book_type::new_testament), static_cast <int> (database::books::get_type_v2 (book_id::_matthew)));
  evaluate (__LINE__, __func__, static_cast <int> (book_type::old_testament), static_cast <int> (database::books::get_type_v2 (book_id::_malachi)));
  evaluate (__LINE__, __func__, static_cast <int> (book_type::unknown), static_cast <int> (database::books::get_type_v2 (book_id::_unknown)));
  evaluate (__LINE__, __func__, 105, database::books::get_id_from_usfm_v1 ("INT"));
  evaluate (__LINE__, __func__, static_cast <int> (book_id::_introduction_matter), static_cast <int> (database::books::get_id_from_usfm_v2 ("INT")));
}


