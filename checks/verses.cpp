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


#include <checks/verses.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <database/styles.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <styles/logic.h>
#include <locale/translate.h>
using namespace std;


void checks_verses::missing_punctuation_at_end (const string & bible, int book, int chapter,
                                                const map <int, string> & verses,
                                                const string & center_marks, const string & end_marks,
                                                const string & disregards)
{
  const vector <string> centermarks = filter_string_explode (center_marks, ' ');
  const vector <string> endmarks = filter_string_explode (end_marks, ' ');
  const vector <string> ignores = filter_string_explode (disregards, ' ');
  Database_Check database_check {};
  for (const auto & element : verses) {
    int verse = element.first;
    string text = element.second;
    if (verse == 0) continue;
    if (text.empty ()) continue;
    for (const auto & ignore_text : ignores) {
      text = filter_string_str_replace (ignore_text, string(), text);
    }
    const size_t text_length = unicode_string_length (text);
    const string lastCharacter = unicode_string_substr (text, text_length - 1, 1);
    if (in_array (lastCharacter, centermarks)) continue;
    if (in_array (lastCharacter, endmarks)) continue;
    database_check.recordOutput (bible, book, chapter, verse, translate ("No punctuation at end of verse:") + " " + lastCharacter);
  }
}


void checks_verses::patterns (const string & bible, int book, int chapter,
                              const map <int, string> & verses, const vector <string> & patterns)
{
  Database_Check database_check {};
  for (const auto & element : verses) {
    const int verse = element.first;
    const string text = element.second;
    for (const auto & pattern : patterns) {
      if (pattern.empty ()) continue;
      if (text.find (pattern) != string::npos) {
        database_check.recordOutput (bible, book, chapter, verse, translate ("Pattern found in text:") + " " + pattern);
      }
    }
  }
}
