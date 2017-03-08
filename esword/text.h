/*
Copyright (©) 2003-2017 Teus Benschop.

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


#ifndef INCLUDED_ESWORD_TEXT_H
#define INCLUDED_ESWORD_TEXT_H


#include <config/libraries.h>


class Esword_Text
{
public:
  Esword_Text (string bible);
  void flushCache ();
  void newBook (int book);
  void newChapter (int chapter);
  void newVerse (int verse);
  void addText (string text);
  void finalize ();
  void createModule (string filename);
  vector <string> get_sql ();
private:
  int currentBook;
  int currentChapter;
  int currentVerse;
  string currentText;
  vector <string> sql; // Contains the generated SQL.
};


#endif

