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

class Database_Navigation
{
public:
  void create ();
  void upgrade ();
  void downgrade ();
  void trim ();
  void record (const int time, const std::string& user,
               const int book, const int chapter, const int verse,
               const int focus_group);
  bool previous_exists (const std::string& user, const int focus_group);
  bool next_exists (const std::string& user, const int focus_group);
  Passage get_previous (const std::string& user, const int focus_group);
  Passage get_next (const std::string& user, const int focus_group);
  std::vector <Passage> get_history (const std::string& user, const int direction, const int focus_group);
private:
  int get_previous_id (const std::string& user, const int focus_group);
  int get_next_id (const std::string& user, const int focus_group);
};
