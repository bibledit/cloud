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
#include <filter/passage.h>


struct Database_Kjv_Item
{
  std::string strong {};
  std::string english {};
};


class Database_Kjv
{
public:
  void create ();
  void optimize ();
  std::vector <Database_Kjv_Item> getVerse (int book, int chapter, int verse);
  std::vector <Passage> searchStrong (std::string strong);
  void store (int book, int chapter, int verse, std::string strong, std::string english);
  std::vector <int> rowids (int book, int chapter, int verse);
  std::string strong (int rowid);
  std::string english (int rowid);
private:
  const char * filename ();
  int get_id (const char * table_row, std::string item);
  std::string get_item (const char * item, int rowid);
};


