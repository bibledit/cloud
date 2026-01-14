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


#include <checks/space.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <locale/translate.h>
#include <checks/issues.h>


namespace checks::space {


void double_space_usfm (const std::string& bible, int book, int chapter, int verse, const std::string& data)
{
  const size_t pos = data.find ("  ");
  if (pos != std::string::npos) {
    int start = static_cast<int>(pos) - 10;
    if (start < 0) start = 0;
    const std::string fragment = data.substr (static_cast <size_t> (start), 20);
    database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::double_space) + ": ... " + fragment + " ...");
  }
}


void space_before_punctuation (const std::string& bible, int book, int chapter, const std::map <int, std::string> & texts)
{
  for (const auto & element : texts) {
    const int verse = element.first;
    const std::string text = element.second;
    if (text.find (" ,") != std::string::npos) {
      database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_a_comma));
    }
    if (text.find (" ;") != std::string::npos) {
      database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_a_semicolon));
    }
    if (text.find (" :") != std::string::npos) {
      database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_a_colon));
    }
    if (text.find (" .") != std::string::npos) {
      database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_a_full_stop));
    }
    if (text.find (" ?") != std::string::npos) {
      database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_a_question_mark));
    }
    if (text.find (" !") != std::string::npos) {
      database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_an_exclamation_mark));
    }
  }
}


void space_end_verse (const std::string& bible, int book, int chapter, const std::string& usfm)
{
  std::vector <int> verses = filter::usfm::get_verse_numbers (usfm);
  for (auto verse : verses) {
    if (!verse) continue;
    std::string text = filter::usfm::get_verse_text (usfm, verse);
    const std::vector <std::string> items = filter::usfm::get_markers_and_text (text);
    for (const auto & item : items) {
      if (filter::usfm::is_usfm_marker (item)) {
        text = filter::string::replace (item, "", text);
      }
    }
    bool hit {false};
    if (!text.empty ()) {
      const std::string trimmed = filter::string::trim (text);
      if (trimmed.empty ()) hit = true;
      const char lastchar = text.back ();
      if (lastchar == ' ') hit = true;
    }
    if (hit) database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_at_the_end_of_the_verse));
  }
}


bool transpose_note_space (std::string & usfm)
{
  // Samples of footnote and cross reference markers that have spacing to be transposed.
  // \v 1 Verse\f + \fr 3.1\fk  keyword\ft  Text.\f* one.
  // \v 2 Verse\x + \xo 3.2\xt  Text.\x* two.

  bool transposed {false};
  const size_t pos = usfm.find("  ");
  if (pos != std::string::npos) {
    std::map <std::string, std::string> data = {
      std::pair (R"(\fk  )", R"( \fk )"),
      std::pair (R"(\ft  )", R"( \ft )"),
      std::pair (R"(\xt  )", R"( \xt )")
    };
    for (const auto & search_replace : data) {
      int count {0};
      usfm = filter::string::replace (search_replace.first, search_replace.second, usfm, &count);
      if (count) transposed = true;
    }
  }
  return transposed;
}


void space_before_final_note_markup (const std::string& bible, int book, int chapter, int verse, const std::string& data)
{
  if (data.find (R"( \f*)") != std::string::npos) {
    database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_final_note_markup));
  }
  if (data.find (R"( \fe*)") != std::string::npos) {
    database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_final_note_markup));
  }
  if (data.find (R"( \x*)") != std::string::npos) {
    database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::space_before_final_cross_reference_markup));
  }
}


}
