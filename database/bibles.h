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


#pragma once

#include <config/libraries.h>

class Database_Bibles
{
public:
  std::vector <std::string> getBibles ();
  void createBible (std::string name);
  void deleteBible (std::string name);
  void storeChapter (std::string name, int book, int chapter_number, std::string chapter_text);
  void updateSearchFields (std::string name, int book, int chapter);
  std::vector <int> getBooks (std::string bible);
  void deleteBook (std::string bible, int book);
  std::vector <int> getChapters (std::string bible, int book);
  void deleteChapter (std::string bible, int book, int chapter);
  std::string getChapter (std::string bible, int book, int chapter);
  int getChapterId (std::string bible, int book, int chapter);
  int getChapterAge (std::string bible, int book, int chapter);
  void optimize ();
private:
  std::string mainFolder ();
public:
  std::string bibleFolder (std::string bible);
private:
  std::string bookFolder (std::string bible, int book);
  std::string chapterFolder (std::string bible, int book, int chapter);
};
