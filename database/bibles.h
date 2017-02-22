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


#ifndef INCLUDED_DATABASE_BIBLES_H
#define INCLUDED_DATABASE_BIBLES_H


#include <config/libraries.h>


class Database_Bibles
{
public:
  vector <string> getBibles ();
  void createBible (string name);
  void deleteBible (string name);
  void storeChapter (string name, int book, int chapter_number, string chapter_text);
  void updateSearchFields (string name, int book, int chapter);
  vector <int> getBooks (string bible);
  void deleteBook (string bible, int book);
  vector <int> getChapters (string bible, int book);
  void deleteChapter (string bible, int book, int chapter);
  string getChapter (string bible, int book, int chapter);
  int getChapterId (string bible, int book, int chapter);
  void optimize ();
private:
  string mainFolder ();
public:
  string bibleFolder (string bible);
private:
  string bookFolder (string bible, int book);
  string chapterFolder (string bible, int book, int chapter);
};


#endif
