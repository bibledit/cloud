/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


void Checks_Verses::missingPunctuationAtEnd (string bible, int book, int chapter, map <int, string> verses,
                                             string center_marks, string end_marks, string disregards)
{
  vector <string> centermarks = filter_string_explode (center_marks, ' ');
  vector <string> endmarks = filter_string_explode (end_marks, ' ');
  vector <string> ignores = filter_string_explode (disregards, ' ');
  Database_Check database_check;
  for (auto element : verses) {
    int verse = element.first;
    string text = element.second;
    if (verse == 0) continue;
    if (text.empty ()) continue;
    for (auto ignore : ignores) {
      text = filter_string_str_replace (ignore, "", text);
    }
    size_t text_length = unicode_string_length (text);
    string lastCharacter = unicode_string_substr (text, text_length - 1, 1);
    if (in_array (lastCharacter, centermarks)) continue;
    if (in_array (lastCharacter, endmarks)) continue;
    database_check.recordOutput (bible, book, chapter, verse, translate ("No punctuation at end of verse:") + " " + lastCharacter);
  }
}


void Checks_Verses::patterns (string bible, int book, int chapter, map <int, string> verses, vector <string> patterns)
{
  Database_Check database_check;
  for (auto element : verses) {
    int verse = element.first;
    string text = element.second;
    for (auto pattern : patterns) {
      if (pattern.empty ()) continue;
      if (text.find (pattern) != string::npos) {
        database_check.recordOutput (bible, book, chapter, verse, translate ("Pattern found in text:") + " " + pattern);
      }
    }
  }
}
