/*
Copyright (©) 2003-2025 Teus Benschop.

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

class Database_UsfmResources
{
public:
  std::vector <std::string> getResources ();
  void deleteResource (const std::string& name);
  void deleteBook (const std::string& name, int book);
  void deleteChapter (const std::string& name, int book, int chapter);
  void storeChapter (const std::string& name, int book, int chapter, const std::string& usfm);
  std::vector <int> getBooks (const std::string& name);
  std::vector <int> getChapters (const std::string& name, int book);
  std::string getUsfm (const std::string& name, int book, int chapter);
  int getSize (const std::string& name, int book, int chapter);
private:
  std::string mainFolder ();
  std::string resourceFolder (const std::string& name);
  std::string bookFolder (const std::string& name, int book);
  std::string chapterFile (const std::string& name, int book, int chapter);
};
