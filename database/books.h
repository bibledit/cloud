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


#pragma once

#include <config/libraries.h>

enum class book_type {
  unknown,
  old_testament,
  new_testament,
  front_back,
  other,
  apocryphal,
};

enum class book_id {
  _unknown = 0,
  _genesis = 1,
  _exodus = 2,
  _leviticus = 3,
  _numbers = 4,
  _deuteronomy = 5,
  _joshua = 6,
  _judges = 7,
  _ruth = 8,
  _1_samuel = 9,
  _2_samuel = 10,
  _1_kings = 11,
  _2_kings = 12,
  _1_chronicles = 13,
  _2_chronicles = 14,
  _ezra = 15,
  _nehemiah = 16,
  _esther = 17,
  _job = 18,
  _psalms = 19,
  _proverbs = 20,
  _ecclesiastes = 21,
  _song_of_solomon = 22,
  _isaiah = 23,
  _jeremiah = 24,
  _lamentations = 25,
  _ezekiel = 26,
  _daniel = 27,
  _hosea = 28,
  _joel = 29,
  _amos = 30,
  _obadiah = 31,
  _jonah = 32,
  _micah = 33,
  _nahum = 34,
  _habakkuk = 35,
  _zephaniah = 36,
  _haggai = 37,
  _zechariah = 38,
  _malachi = 39,
  _matthew = 40,
  _mark = 41,
  _luke = 42,
  _john = 43,
  _acts = 44,
  _romans = 45,
  _1_corinthians = 46,
  _2_corinthians = 47,
  _galatians = 48,
  _ephesians = 49,
  _philippians = 50,
  _colossians = 51,
  _1_thessalonians = 52,
  _2_thessalonians = 53,
  _1_timothy = 54,
  _2_timothy = 55,
  _titus = 56,
  _philemon = 57,
  _hebrews = 58,
  _james = 59,
  _1_peter = 60,
  _2_peter = 61,
  _1_john = 62,
  _2_john = 63,
  _3_john = 64,
  _jude = 65,
  _revelation = 66,
  _front_matter = 67,
  _back_matter = 68,
  _other_material = 69,
  _tobit = 70,
  _judith = 71,
  _esther_greek = 72,
  _wisdom_of_solomon = 73,
  _sirach = 74,
  _baruch = 75,
  _letter_of_jeremiah = 76,
  _song_of_the_three_children = 77,
  _susanna = 78,
  _bel_and_the_dragon = 79,
  _1_maccabees = 80,
  _2_maccabees = 81,
  _1_esdras_greek = 82,
  _prayer_of_manasses = 83,
  _psalm_151 = 84,
  _3_maccabees = 85,
  _2_esdras_latin = 86,
  _4_maccabees = 87,
  _daniel_greek = 88,
  _odes = 89,
  _psalms_of_solomon = 90,
  _ezra_apocalypse = 91,
  _5_ezra = 92,
  _6_ezra = 93,
  _psalms_152_155 = 94,
  _2_baruch_apocalypse = 95,
  _letter_of_baruch = 96,
  _jubilees = 97,
  _enoch = 98,
  _1_meqabyan_mekabis = 99,
  _2_meqabyan_mekabis = 100,
  _3_meqabyan_mekabis = 101,
  _reproof = 102,
  _4_baruch = 103,
  _letter_to_the_laodiceans = 104,
  _introduction_matter = 105,
  _concordance = 106,
  _glossary_wordlist = 107,
  _topical_index = 108,
  _names_index = 109,
};

namespace database::books {

std::vector <book_id> get_ids ();
book_id get_id_from_english (const std::string & english);
std::string get_english_from_id (book_id id);
std::string get_usfm_from_id (book_id id);
std::string get_bibleworks_from_id (book_id id);
std::string get_osis_from_id (book_id id);
book_id get_id_from_usfm (const std::string & usfm);
book_id get_id_from_osis (const std::string & osis);
book_id get_id_from_bibleworks (const std::string & bibleworks);
book_id get_id_like_text (const std::string & text);
book_id get_id_from_onlinebible (const std::string & onlinebible);
std::string get_onlinebible_from_id (book_id id);
short get_order_from_id (book_id id);
book_type get_type (book_id id);
std::string book_type_to_string (book_type type);

}
