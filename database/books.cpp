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


#include <database/books.h>
#include <config/globals.h>
#include <database/sqlite.h>
#include <filter/string.h>
#include <filter/diff.h>
#include <locale/translate.h>
#include <database/booksdata.h>
using namespace std;


namespace database::books {


// Internal function for the number of data elements.
constexpr size_t data_count = sizeof (books_table) / sizeof (*books_table);


std::vector <book_id> get_ids ()
{
  vector <book_id> ids;
  for (unsigned int i = 0; i < data_count; i++) {
    book_id id = books_table[i].id;
    ids.push_back (id);
  }
  return ids;
}


book_id get_id_from_english (const string & english)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (english == books_table[i].english) {
      return books_table[i].id;
    }
  }
  return book_id::_unknown;
}


string get_english_from_id (book_id id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].english;
    }
  }
  return translate ("Unknown");
}


string get_usfm_from_id (book_id id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].usfm;
    }
  }
  return "XXX";
}


string get_bibleworks_from_id (book_id id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].bibleworks;
    }
  }
  return "Xxx";
}


string get_osis_from_id (book_id id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].osis;
    }
  }
  return translate ("Unknown");
}


book_id get_id_from_usfm (const string & usfm)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (usfm == books_table[i].usfm) {
      return books_table[i].id;
    }
  }
  return book_id::_unknown;
}


book_id get_id_from_osis (const string & osis)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (osis == books_table[i].osis) {
      return books_table[i].id;
    }
  }
  return book_id::_unknown;
}


book_id get_id_from_bibleworks (const string & bibleworks)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (bibleworks == books_table[i].bibleworks) {
      return books_table[i].id;
    }
  }
  return book_id::_unknown;
}


// Tries to interprete $text as the name of a Bible book.
// Returns the book's identifier if it succeeds.
// If it fails, it returns 0.
book_id get_id_like_text (const string & text)
{
  // Go through all known book names and abbreviations.
  // Note how much the $text differs from the known names.
  // Then return the best match.
  vector <int> ids {};
  vector <int> similarities {};
  for (unsigned int i = 0; i < data_count; i++) {
    int id {static_cast<int>(books_table[i].id)};
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, filter::strings::unicode_string_casefold(books_table[i].english)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, filter::strings::unicode_string_casefold(books_table[i].osis)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].usfm));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, filter::strings::unicode_string_casefold(books_table[i].bibleworks)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, filter::strings::unicode_string_casefold(books_table[i].onlinebible)));
  }
  filter::strings::quick_sort (similarities, ids, 0, static_cast<unsigned>(ids.size()));
  int id = ids.back ();
  return static_cast<book_id>(id);
}


book_id get_id_from_onlinebible (const string & onlinebible)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (onlinebible == books_table[i].onlinebible) {
      return books_table[i].id;
    }
  }
  return book_id::_unknown;
}


string get_onlinebible_from_id (book_id id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].onlinebible;
    }
  }
  return string();
}


short get_order_from_id (book_id id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].order;
    }
  }
  return 0;
}


book_type get_type (book_id id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].type;
    }
  }
  return book_type::unknown;
}


std::string book_type_to_string (book_type type)
{
  switch (type) {
    case book_type::unknown: return string();
    case book_type::old_testament: return "ot";
    case book_type::new_testament: return "nt";
    case book_type::front_back: return "frontback";
    case book_type::other: return "other";
    case book_type::apocryphal: return "ap";
    default: return string();
  }
  return string();
}


} // End of namespace.
