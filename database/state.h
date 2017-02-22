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


#ifndef INCLUDED_DATABASE_STATE_H
#define INCLUDED_DATABASE_STATE_H


#include <config/libraries.h>


class Database_State
{
public:
  static void create ();
  static void putNotesChecksum (int first, int last, const string& checksum);
  static string getNotesChecksum (int first, int last);
  static void eraseNoteChecksum (int identifier);
  static void setExport (const string & bible, int book, int format);
  static bool getExport (const string & bible, int book, int format);
  static void clearExport (const string & bible, int book, int format);
private:
  static sqlite3 * connect ();
  static const char * name ();
};


#endif
