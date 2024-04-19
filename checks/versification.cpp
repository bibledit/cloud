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


#include <checks/versification.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <database/versifications.h>
#include <database/config/bible.h>
#include <locale/translate.h>


void checks_versification::books (const std::string& bible, const std::vector <int> & books)
{
  Database_Versifications database_versifications {};
  std::string versification = Database_Config_Bible::getVersificationSystem (bible);
  if (versification.empty ()) versification = filter::strings::english ();
  const std::vector <int> standardBooks = database_versifications.getBooks (versification);
  const std::vector <int> absentBooks = filter::strings::array_diff (standardBooks, books);
  const std::vector <int> extraBooks = filter::strings::array_diff (books, standardBooks);
  Database_Check database_check {};
  for (auto book : absentBooks) {
    database_check.recordOutput (bible, book, 1, 1, translate ("This book is absent from the Bible"));
  }
  for (auto book : extraBooks) {
    database_check.recordOutput (bible, book, 1, 1, translate ("This book is extra in the Bible"));
  }
}


void checks_versification::chapters (const std::string& bible, int book, const std::vector <int> & chapters)
{
  Database_Versifications database_versifications {};
  std::string versification = Database_Config_Bible::getVersificationSystem (bible);
  if (versification.empty ()) versification = filter::strings::english ();
  const std::vector <int> standardChapters = database_versifications.getChapters (versification, book, true);
  const std::vector <int> absentChapters = filter::strings::array_diff (standardChapters, chapters);
  const std::vector <int> extraChapters = filter::strings::array_diff (chapters, standardChapters);
  Database_Check database_check {};
  for (auto chapter : absentChapters) {
    database_check.recordOutput (bible, book, chapter, 1, translate ("This chapter is missing"));
  }
  for (auto chapter : extraChapters) {
    database_check.recordOutput (bible, book, chapter, 1, translate ("This chapter is extra"));
  }
}


void checks_versification::verses (const std::string& bible, int book, int chapter, const std::vector <int> & verses)
{
  // Get verses in this chapter according to the versification system for the Bible.
  Database_Versifications database_versifications {};
  std::string versification = Database_Config_Bible::getVersificationSystem (bible);
  if (versification.empty ()) versification = filter::strings::english ();
  const std::vector <int> standardVerses = database_versifications.getVerses (versification, book, chapter);
  // Look for missing and extra verses.
  const std::vector <int> absentVerses = filter::strings::array_diff (standardVerses, verses);
  const std::vector <int> extraVerses = filter::strings::array_diff (verses, standardVerses);
  Database_Check database_check {};
  for (auto verse : absentVerses) {
    database_check.recordOutput (bible, book, chapter, verse, translate ("This verse is missing according to the versification system"));
  }
  for (auto verse : extraVerses) {
    //if ((chapter == 0) && (verse == 0)) continue;
    database_check.recordOutput (bible, book, chapter, verse, translate ("This verse is extra according to the versification system"));
  }
  // Look for verses out of order.
  int previousVerse {0};
  for (size_t i = 0; i < verses.size(); i++) {
    int verse = verses[i];
    if (i > 0) {
      if (verse != (previousVerse + 1)) {
        database_check.recordOutput (bible, book, chapter, verse, translate ("The verse is out of sequence"));
      }
    }
    previousVerse = verse;
  }
}
