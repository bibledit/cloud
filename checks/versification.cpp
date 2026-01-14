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


#include <checks/versification.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <database/versifications.h>
#include <database/config/bible.h>
#include <database/check.h>
#include <locale/translate.h>
#include <checks/issues.h>


void checks_versification::books (const std::string& bible, const std::vector <int> & books)
{
  Database_Versifications database_versifications {};
  std::string versification = database::config::bible::get_versification_system (bible);
  if (versification.empty ()) versification = filter::string::english ();
  const std::vector <int> standardBooks = database_versifications.getBooks (versification);
  const std::vector <int> absentBooks = filter::string::array_diff (standardBooks, books);
  const std::vector <int> extraBooks = filter::string::array_diff (books, standardBooks);
  for (auto book : absentBooks) {
    database::check::record_output (bible, book, 1, 1, checks::issues::text(checks::issues::issue::this_book_is_absent_from_the_bible));
  }
  for (auto book : extraBooks) {
    database::check::record_output (bible, book, 1, 1, checks::issues::text(checks::issues::issue::this_book_is_extra_in_the_bible));
  }
}


void checks_versification::chapters (const std::string& bible, int book, const std::vector <int> & chapters)
{
  Database_Versifications database_versifications {};
  std::string versification = database::config::bible::get_versification_system (bible);
  if (versification.empty ()) versification = filter::string::english ();
  const std::vector <int> standardChapters = database_versifications.getChapters (versification, book, true);
  const std::vector <int> absentChapters = filter::string::array_diff (standardChapters, chapters);
  const std::vector <int> extraChapters = filter::string::array_diff (chapters, standardChapters);
  for (auto chapter : absentChapters) {
    database::check::record_output (bible, book, chapter, 1, checks::issues::text(checks::issues::issue::this_chapter_is_missing));
  }
  for (auto chapter : extraChapters) {
    database::check::record_output (bible, book, chapter, 1, checks::issues::text(checks::issues::issue::this_chapter_is_extra));
  }
}


void checks_versification::verses (const std::string& bible, int book, int chapter, const std::vector <int> & verses)
{
  // Get verses in this chapter according to the versification system for the Bible.
  Database_Versifications database_versifications {};
  std::string versification = database::config::bible::get_versification_system (bible);
  if (versification.empty ()) versification = filter::string::english ();
  const std::vector <int> standardVerses = database_versifications.getVerses (versification, book, chapter);
  // Look for missing and extra verses.
  const std::vector <int> absentVerses = filter::string::array_diff (standardVerses, verses);
  const std::vector <int> extraVerses = filter::string::array_diff (verses, standardVerses);
  for (auto verse : absentVerses) {
    database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::this_verse_is_missing_according_to_the_versification_system));
  }
  for (auto verse : extraVerses) {
    //if ((chapter == 0) && (verse == 0)) continue;
    database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::this_verse_is_extra_according_to_the_versification_system));
  }
  // Look for verses out of order.
  int previousVerse {0};
  for (size_t i = 0; i < verses.size(); i++) {
    int verse = verses[i];
    if (i > 0) {
      if (verse != (previousVerse + 1)) {
        database::check::record_output (bible, book, chapter, verse, checks::issues::text(checks::issues::issue::the_verse_is_out_of_sequence));
      }
    }
    previousVerse = verse;
  }
}
