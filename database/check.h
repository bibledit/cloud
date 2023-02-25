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
#include <filter/passage.h>


class Database_Check_Hit
{
public:
  int rowid {0};
  std::string bible {};
  int book {0};
  int chapter {0};
  int verse {0};
  std::string data {};
};


class Database_Check
{
public:
  void create ();
  void optimize ();
  void truncateOutput (std::string bible);
  void recordOutput (std::string bible, int book, int chapter, int verse, std::string data);
  std::vector <Database_Check_Hit> getHits ();
  void approve (int id);
  void erase (int id);
  Passage getPassage (int id);
  std::vector <Database_Check_Hit> getSuppressions ();
  void release (int id);
private:
  const char * filename ();
};

