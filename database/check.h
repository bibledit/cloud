/*
Copyright (©) 2003-2024 Teus Benschop.

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

class Passage;

namespace database::check {

struct Hit
{
  int rowid {0};
  std::string bible {};
  int book {0};
  int chapter {0};
  int verse {0};
  std::string data {};
};


void create ();
void optimize ();
void truncate_output (const std::string& bible);
void record_output (const std::string& bible, int book, int chapter, int verse, std::string data);
std::vector <Hit> get_hits ();
void approve (int id);
void erase (int id);
Passage get_passage (int id);
std::vector <Hit> get_suppressions ();
void release (int id);


}
