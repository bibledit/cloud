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


#pragma once

#include <config/libraries.h>
#include <filter/passage.h>

class Database_Versifications
{
public:
  void create ();
  void defaults ();
  void optimize ();
  void input (const std::string& contents, const std::string& name);
  std::string output (const std::string& name);
  void erase (const std::string& name);
  int getID (const std::string& name);
  int createSystem (const std::string& name);
  std::vector <std::string> getSystems ();
  std::vector <Passage> getBooksChaptersVerses (const std::string& name);
  std::vector <int> getBooks (const std::string& name);
  std::vector <int> getChapters (const std::string& name, int book, bool include0 = false);
  std::vector <int> getVerses (const std::string& name, int book, int chapter);
  std::vector <int> getMaximumBooks ();
  std::vector <int> getMaximumChapters (int book);
  std::vector <int> getMaximumVerses (int book, int chapter);
private:
  bool creating_defaults = false;
};
