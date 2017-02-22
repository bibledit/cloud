/*
Copyright (©) 2003-2016 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_NOTEACTIONS_H
#define INCLUDED_DATABASE_NOTEACTIONS_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Note_Action
{
public:
  int rowid;
  string username;
  int timestamp;
  int action;
  string content;
};


class Database_NoteActions
{
public:
  void create ();
  void clear ();
  void optimize ();
  void record (const string& username, int note, int action, const string& content);
  vector <int> getNotes ();
  vector <Database_Note_Action> getNoteData (int note);
  void updateNotes (int oldId, int newId);
  void erase (int rowid);
  bool exists (int note);
private:
  sqlite3 * connect ();
};


#endif
