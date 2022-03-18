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


vector <int> Database_Books::getIDs ()
{
  vector <int> ids;
  for (unsigned int i = 0; i < data_count (); i++) {
    int id = books_table[i].id;
    ids.push_back (id);
  }
  return ids;
}


int Database_Books::getIdFromEnglish (string english)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (english == books_table[i].english) {
      return books_table[i].id;
    }
  }
  return 0;  
}


string Database_Books::getEnglishFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].english;
    }
  }
  return translate ("Unknown");
}


string Database_Books::getUsfmFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].usfm;
    }
  }
  return "XXX";
}


string Database_Books::getBibleworksFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].bibleworks;
    }
  }
  return "Xxx";
}


string Database_Books::getOsisFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].osis;
    }
  }
  return translate ("Unknown");
}


int Database_Books::getIdFromUsfm (string usfm)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (usfm == books_table[i].usfm) {
      return books_table[i].id;
    }
  }
  return 0;
}


int Database_Books::getIdFromOsis (string osis)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (osis == books_table[i].osis) {
      return books_table[i].id;
    }
  }
  return 0;
}


int Database_Books::getIdFromBibleworks (string bibleworks)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (bibleworks == books_table[i].bibleworks) {
      return books_table[i].id;
    }
  }
  return 0;
}


/*
Tries to interprete $text as the name of a Bible book.
Returns the book's identifier if it succeeds.
If it fails, it returns 0.
*/
int Database_Books::getIdLikeText (string text)
{
  // Go through all known book names and abbreviations.
  // Note how much the $text differs from the known names.
  // Then return the best match.
  vector <int> ids;
  vector <int> similarities;
  for (unsigned int i = 0; i < data_count(); i++) {
    int id = books_table[i].id;
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
  quick_sort (similarities, ids, 0, (int)ids.size());
  return ids.back ();
}


int Database_Books::getIdFromOnlinebible (string onlinebible)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (onlinebible == books_table[i].onlinebible) {
      return books_table[i].id;
    }
  }
  return 0;
}


string Database_Books::getOnlinebibleFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].onlinebible;
    }
  }
  return "";
}


int Database_Books::getOrderFromId (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].order;
    }
  }
  return 0;
}


string Database_Books::getType (int id)
{
  for (unsigned int i = 0; i < data_count(); i++) {
    if (id == books_table[i].id) {
      return books_table[i].type;
    }
  }
  return string();
}


unsigned int Database_Books::data_count ()
{
  return sizeof (books_table) / sizeof (*books_table);
}
