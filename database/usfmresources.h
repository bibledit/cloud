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


#ifndef INCLUDED_DATABASE_USFMRESOURCES_H
#define INCLUDED_DATABASE_USFMRESOURCES_H


#include <config/libraries.h>


class Database_UsfmResources
{
public:
  vector <string> getResources ();
  void deleteResource (const string& name);
  void deleteBook (const string& name, int book);
  void deleteChapter (const string& name, int book, int chapter);
  void storeChapter (const string& name, int book, int chapter, const string& usfm);
  vector <int> getBooks (const string& name);
  vector <int> getChapters (const string& name, int book);
  string getUsfm (const string& name, int book, int chapter);
  int getSize (const string& name, int book, int chapter);
private:
  string mainFolder ();
  string resourceFolder (const string& name);
  string bookFolder (const string& name, int book);
  string chapterFile (const string& name, int book, int chapter);
};


#endif
