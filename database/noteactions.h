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
#include <filter/passage.h>

struct Database_Note_Action
{
  int rowid {0};
  std::string username {};
  int timestamp {0};
  int action {0};
  std::string content {};
};

class Database_NoteActions
{
public:
  void create ();
  void clear ();
  void optimize ();
  void record (const std::string& username, int note, int action, const std::string& content);
  std::vector <int> getNotes ();
  std::vector <Database_Note_Action> getNoteData (int note);
  void updateNotes (int oldId, int newId);
  void erase (int rowid);
  bool exists (int note);
};
