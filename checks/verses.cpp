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


#include <checks/verses.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <database/styles.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <styles/logic.h>
#include <locale/translate.h>


void checks_verses::missing_punctuation_at_end (const std::string& bible, int book, int chapter,
                                                const std::map <int, std::string> & verses,
                                                const std::string& center_marks, const std::string& end_marks,
                                                const std::string& disregards)
{
  const std::vector <std::string> centermarks = filter::strings::explode (center_marks, ' ');
  const std::vector <std::string> endmarks = filter::strings::explode (end_marks, ' ');
  const std::vector <std::string> ignores = filter::strings::explode (disregards, ' ');
  Database_Check database_check {};
  for (const auto & element : verses) {
    int verse = element.first;
    std::string text = element.second;
    if (verse == 0) continue;
    if (text.empty ()) continue;
    for (const auto & ignore_text : ignores) {
      text = filter::strings::replace (ignore_text, std::string(), text);
    }
    const size_t text_length = filter::strings::unicode_string_length (text);
    const std::string lastCharacter = filter::strings::unicode_string_substr (text, text_length - 1, 1);
    if (in_array (lastCharacter, centermarks)) continue;
    if (in_array (lastCharacter, endmarks)) continue;
    database_check.recordOutput (bible, book, chapter, verse, translate ("No punctuation at end of verse:") + " " + lastCharacter);
  }
}


void checks_verses::patterns (const std::string& bible, int book, int chapter,
                              const std::map <int, std::string> & verses, const std::vector <std::string> & patterns)
{
  Database_Check database_check {};
  for (const auto & element : verses) {
    const int verse = element.first;
    const std::string text = element.second;
    for (const auto & pattern : patterns) {
      if (pattern.empty ()) continue;
      if (text.find (pattern) != std::string::npos) {
        database_check.recordOutput (bible, book, chapter, verse, translate ("Pattern found in text:") + " " + pattern);
      }
    }
  }
}
