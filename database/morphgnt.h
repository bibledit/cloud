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

class Database_MorphGnt
{
public:
  void create ();
  void optimize ();
  void store (int book, int chapter, int verse,
              std::string pos, std::string parsing, std::string word, std::string lemma);
  std::vector <int> rowids (int book, int chapter, int verse);
  std::string pos (int rowid);
  std::string parsing (int rowid);
  std::string word (int rowid);
  std::string lemma (int rowid);
private:
  const char * filename ();
  int get_id (const char * table_row, std::string item);
  std::string get_item (const char * item, int rowid);
};

