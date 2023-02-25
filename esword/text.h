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

class Esword_Text
{
public:
  Esword_Text (std::string bible);
  void flushCache ();
  void newBook (int book);
  void newChapter (int chapter);
  void newVerse (int verse);
  void addText (std::string text);
  void finalize ();
  void createModule (std::string filename);
  std::vector <std::string> get_sql ();
private:
  int currentBook {0};
  int currentChapter {0};
  int currentVerse {0};
  std::string currentText {};
  std::vector <std::string> sql {}; // Contains the generated SQL.
};
