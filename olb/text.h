/*
Copyright (©) 2003-2020 Teus Benschop.

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


#ifndef INCLUDED_ONLINEBIBLE_TEXT_H
#define INCLUDED_ONLINEBIBLE_TEXT_H


#include <config/libraries.h>


class OnlineBible_Text
{
public:
  OnlineBible_Text ();
  void newVerse (int bookIdentifier, int chapterNumber, int verseNumber);
  void addText (string text);
  void addNote ();
  void closeCurrentNote ();
  void save (string name);
  void storeData ();
private:
  string currentLine;
  bool lineLoaded = false;
  vector <string> output;
};


#endif

