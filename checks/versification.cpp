/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


void Checks_Versification::books (string bible, vector <int> books)
{
  Database_Versifications database_versifications;
  string versification = Database_Config_Bible::getVersificationSystem (bible);
  if (versification.empty ()) versification = english ();
  vector <int> standardBooks = database_versifications.getBooks (versification);
  vector <int> absentBooks = filter_string_array_diff (standardBooks, books);
  vector <int> extraBooks = filter_string_array_diff (books, standardBooks);
  Database_Check database_check;
  for (auto book : absentBooks) {
    database_check.recordOutput (bible, book, 1, 1, "This book is absent from the Bible");
  }
  for (auto book : extraBooks) {
    database_check.recordOutput (bible, book, 1, 1, "This book is extra in the Bible");
  }
}


void Checks_Versification::chapters (string bible, int book, vector <int> chapters)
{
  Database_Versifications database_versifications;
  string versification = Database_Config_Bible::getVersificationSystem (bible);
  if (versification.empty ()) versification = english ();
  vector <int> standardChapters = database_versifications.getChapters (versification, book, true);
  vector <int> absentChapters = filter_string_array_diff (standardChapters, chapters);
  vector <int> extraChapters = filter_string_array_diff (chapters, standardChapters);
  Database_Check database_check;
  for (auto chapter : absentChapters) {
    database_check.recordOutput (bible, book, chapter, 1, "This chapter is missing");
  }
  for (auto chapter : extraChapters) {
    database_check.recordOutput (bible, book, chapter, 1, "This chapter is extra");
  }
}


void Checks_Versification::verses (string bible, int book, int chapter, vector <int> verses)
{
  // Get verses in this chapter according to the versification system for the Bible.
  Database_Versifications database_versifications;
  string versification = Database_Config_Bible::getVersificationSystem (bible);
  if (versification.empty ()) versification = english ();
  vector <int> standardVerses = database_versifications.getVerses (versification, book, chapter);
  // Look for missing and extra verses.
  vector <int> absentVerses = filter_string_array_diff (standardVerses, verses);
  vector <int> extraVerses = filter_string_array_diff (verses, standardVerses);
  Database_Check database_check;
  for (auto verse : absentVerses) {
    database_check.recordOutput (bible, book, chapter, verse, "This verse is missing according to the versification system");
  }
  for (auto verse : extraVerses) {
    //if ((chapter == 0) && (verse == 0)) continue;
    database_check.recordOutput (bible, book, chapter, verse, "This verse is extra according to the versification system");
  }
  // Look for verses out of order.
  int previousVerse = 0;
  for (unsigned int i = 0; i < verses.size(); i++) {
    int verse = verses[i];
    if (i > 0) {
      if (verse != (previousVerse + 1)) {
        database_check.recordOutput (bible, book, chapter, verse, "The verse is out of sequence");
      }
    }
    previousVerse = verse;
  }
}
