/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#include <checks/space.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <locale/translate.h>


void Checks_Space::doubleSpaceUsfm (string bible, int book, int chapter, int verse, string data)
{
  size_t pos = data.find ("  ");
  if (pos != string::npos) {
    int start = pos - 10;
    if (start < 0) start = 0;
    string fragment = data.substr (start, 20);
    Database_Check database_check;
    database_check.recordOutput (bible, book, chapter, verse, translate ("Double space:") + " ... " + fragment + " ...");
  }
}


void Checks_Space::spaceBeforePunctuation (string bible, int book, int chapter, map <int, string> texts)
{
  Database_Check database_check;
  for (auto element : texts) {
    int verse = element.first;
    string text = element.second;
    if (text.find (" ,") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, translate ("Space before a comma"));
    }
    if (text.find (" ;") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, translate ("Space before a semicolon"));
    }
    if (text.find (" :") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, translate ("Space before a colon"));
    }
    if (text.find (" .") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, translate ("Space before a full stop"));
    }
    if (text.find (" ?") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, translate ("Space before a question mark"));
    }
    if (text.find (" !") != string::npos) {
      database_check.recordOutput (bible, book, chapter, verse, translate ("Space before an exclamation mark"));
    }
  }
}


void Checks_Space::spaceEndVerse (string bible, int book, int chapter, string usfm)
{
  Database_Check database_check;
  vector <int> verses = usfm_get_verse_numbers (usfm);
  for (auto verse : verses) {
    if (!verse) continue;
    string text = usfm_get_verse_text (usfm, verse);
    vector <string> items = usfm_get_markers_and_text (text);
    for (auto item : items) {
      if (usfm_is_usfm_marker (item)) {
        text = filter_string_str_replace (item, "", text);
      }
    }
    bool hit = false;
    if (!text.empty ()) {
      string trimmed = filter_string_trim (text);
      if (trimmed.empty ()) hit = true;
      char lastchar = text.back ();
      if (lastchar == ' ') hit = true;
    }
    if (hit) database_check.recordOutput (bible, book, chapter, verse, translate ("Space at the end of the verse"));
  }
}
