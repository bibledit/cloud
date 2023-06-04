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


#include <checks/space.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <locale/translate.h>
using namespace std;


namespace checks::space {


void double_space_usfm (const string & bible, int book, int chapter, int verse, const string & data)
{
  const size_t pos = data.find ("  ");
  if (pos != string::npos) {
    int start = static_cast<int>(pos) - 10;
    if (start < 0) start = 0;
    string fragment = data.substr (static_cast <size_t> (start), 20);
    Database_Check database_check {};
    database_check.recordOutput (bible, book, chapter, verse, translate ("Double space:") + " ... " + fragment + " ...");
  }
}


void space_before_punctuation (const string & bible, int book, int chapter, const map <int, string> & texts)
{
  Database_Check database_check {};
  for (const auto & element : texts) {
    const int verse = element.first;
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


void space_end_verse (const string & bible, int book, int chapter, const string & usfm)
{
  Database_Check database_check {};
  vector <int> verses = filter::usfm::get_verse_numbers (usfm);
  for (auto verse : verses) {
    if (!verse) continue;
    string text = filter::usfm::get_verse_text (usfm, verse);
    vector <string> items = filter::usfm::get_markers_and_text (text);
    for (const auto & item : items) {
      if (filter::usfm::is_usfm_marker (item)) {
        text = filter::strings::replace (item, "", text);
      }
    }
    bool hit {false};
    if (!text.empty ()) {
      string trimmed = filter::strings::trim (text);
      if (trimmed.empty ()) hit = true;
      char lastchar = text.back ();
      if (lastchar == ' ') hit = true;
    }
    if (hit) database_check.recordOutput (bible, book, chapter, verse, translate ("Space at the end of the verse"));
  }
}


bool transpose_note_space (string & usfm)
{
  // Samples of footnote and cross reference markers that have spacing to be transposed.
  // \v 1 Verse\f + \fr 3.1\fk  keyword\ft  Text.\f* one.
  // \v 2 Verse\x + \xo 3.2\xt  Text.\x* two.

  bool transposed {false};
  const size_t pos = usfm.find("  ");
  if (pos != string::npos) {
    map <string, string> data = {
      pair (R"(\fk  )", R"( \fk )"),
      pair (R"(\ft  )", R"( \ft )"),
      pair (R"(\xt  )", R"( \xt )")
    };
    for (const auto & search_replace : data) {
      int count {0};
      usfm = filter::strings::replace (search_replace.first, search_replace.second, usfm, &count);
      if (count) transposed = true;
    }
  }
  return transposed;
}


void space_before_final_note_markup (const string & bible, int book, int chapter, int verse, const string & data)
{
  Database_Check database_check {};
  if (data.find (R"( \f*)") != string::npos) {
    database_check.recordOutput (bible, book, chapter, verse, translate ("Space before final note markup"));
  }
  if (data.find (R"( \fe*)") != string::npos) {
    database_check.recordOutput (bible, book, chapter, verse, translate ("Space before final note markup"));
  }
  if (data.find (R"( \x*)") != string::npos) {
    database_check.recordOutput (bible, book, chapter, verse, translate ("Space before final cross reference markup"));
  }
}


}
