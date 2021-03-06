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


#ifndef INCLUDED_DATABASE_BIBLEACTIONS_H
#define INCLUDED_DATABASE_BIBLEACTIONS_H


#include <config/libraries.h>


class Database_BibleActions
{
public:
  void create ();
  void clear ();
  void optimize ();
  void record (string bible, int book, int chapter, string usfm);
  vector <string> getBibles ();
  vector <int> getBooks (string bible);
  vector <int> getChapters (string bible, int book);
  string getUsfm (string bible, int book, int chapter);
  void erase (string bible, int book, int chapter);
private:
  const char * filename ();
};


#endif
