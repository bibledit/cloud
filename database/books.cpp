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


vector <int> get_ids_v1 () // Todo goes out.
{
  vector <int> ids;
  for (unsigned int i = 0; i < data_count; i++) {
    int id = books_table[i].id_v1;
    ids.push_back (id);
  }
  return ids;
}


std::vector <book_id> get_ids_v2 () // Todo use this.
{
  vector <book_id> ids;
  for (unsigned int i = 0; i < data_count; i++) {
    book_id id = books_table[i].id_v2;
    ids.push_back (id);
  }
  return ids;
}


int get_id_from_english_v1 (const string & english) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (english == books_table[i].english) {
      return books_table[i].id_v1;
    }
  }
  return 0;  
}


book_id get_id_from_english_v2 (const string & english) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (english == books_table[i].english) {
      return books_table[i].id_v2;
    }
  }
  return book_id::_unknown;
}


string get_english_from_id_v1 (int id) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].english;
    }
  }
  return translate ("Unknown");
}


string get_english_from_id_v2 (book_id id) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v2) {
      return books_table[i].english;
    }
  }
  return translate ("Unknown");
}


string get_usfm_from_id_v1 (int id) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].usfm;
    }
  }
  return "XXX";
}


string get_usfm_from_id_v2 (book_id id) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v2) {
      return books_table[i].usfm;
    }
  }
  return "XXX";
}


string get_bibleworks_from_id_v1 (int id) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].bibleworks;
    }
  }
  return "Xxx";
}


string get_bibleworks_from_id_v2 (book_id id) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v2) {
      return books_table[i].bibleworks;
    }
  }
  return "Xxx";
}


string get_osis_from_id_v1 (int id) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].osis;
    }
  }
  return translate ("Unknown");
}


int get_id_from_usfm_v1 (const string & usfm) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (usfm == books_table[i].usfm) {
      return books_table[i].id_v1;
    }
  }
  return 0;
}


book_id get_id_from_usfm_v2 (const string & usfm) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (usfm == books_table[i].usfm) {
      return books_table[i].id_v2;
    }
  }
  return book_id::_unknown;
}


int get_id_from_osis_v1 (const string & osis) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (osis == books_table[i].osis) {
      return books_table[i].id_v1;
    }
  }
  return 0;
}


book_id get_id_from_osis_v2 (const string & osis) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (osis == books_table[i].osis) {
      return books_table[i].id_v2;
    }
  }
  return book_id::_unknown;
}


int get_id_from_bibleworks_v1 (const string & bibleworks) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (bibleworks == books_table[i].bibleworks) {
      return books_table[i].id_v1;
    }
  }
  return 0;
}


book_id get_id_from_bibleworks_v2 (const string & bibleworks) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (bibleworks == books_table[i].bibleworks) {
      return books_table[i].id_v2;
    }
  }
  return book_id::_unknown;
}


// Tries to interprete $text as the name of a Bible book.
// Returns the book's identifier if it succeeds.
// If it fails, it returns 0.
int get_id_like_text_v1 (const string & text) // Todo use _v2
{
  // Go through all known book names and abbreviations.
  // Note how much the $text differs from the known names.
  // Then return the best match.
  vector <int> ids {};
  vector <int> similarities {};
  for (unsigned int i = 0; i < data_count; i++) {
    int id {books_table[i].id_v1};
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].english)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].osis)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].usfm));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].bibleworks)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].onlinebible)));
  }
  quick_sort (similarities, ids, 0, static_cast<unsigned>(ids.size()));
  return ids.back ();
}


// Tries to interprete $text as the name of a Bible book.
// Returns the book's identifier if it succeeds.
// If it fails, it returns 0.
book_id get_id_like_text_v2 (const string & text) // Todo use this
{
  // Go through all known book names and abbreviations.
  // Note how much the $text differs from the known names.
  // Then return the best match.
  vector <int> ids {};
  vector <int> similarities {};
  for (unsigned int i = 0; i < data_count; i++) {
    int id {static_cast<int>(books_table[i].id_v2)};
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].english)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].osis)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].usfm));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].bibleworks)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].onlinebible)));
  }
  quick_sort (similarities, ids, 0, static_cast<unsigned>(ids.size()));
  int id = ids.back ();
  return static_cast<book_id>(id);
}


int get_id_from_onlinebible_v1 (const string & onlinebible) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (onlinebible == books_table[i].onlinebible) {
      return books_table[i].id_v1;
    }
  }
  return 0;
}


book_id get_id_from_onlinebible_v2 (const string & onlinebible) // Todo use todo
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (onlinebible == books_table[i].onlinebible) {
      return books_table[i].id_v2;
    }
  }
  return book_id::_unknown;
}


string get_onlinebible_from_id_v1 (int id) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].onlinebible;
    }
  }
  return string() ;
}


string get_onlinebible_from_id_v2 (book_id id) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v2) {
      return books_table[i].onlinebible;
    }
  }
  return string();
}


int get_order_from_id_v1 (int id) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].order;
    }
  }
  return 0;
}


int get_order_from_id_v2 (book_id id) // Todo use this one.
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v2) {
      return books_table[i].order;
    }
  }
  return 0;
}


book_type get_type_v1 (int id) // Todo use _v2
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].type_v2;
    }
  }
  return book_type::unknown;
}


book_type get_type_v2 (book_id id) // Todo use this
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v2) {
      return books_table[i].type_v2;
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


book_id get_id_v2_from_id_v1 (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id_v1) {
      return books_table[i].id_v2;
    }
  }
  return book_id::_unknown;

}


} // End of namespace.
