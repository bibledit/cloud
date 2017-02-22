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


#ifndef INCLUDED_DATABASE_MAPPINGS_H
#define INCLUDED_DATABASE_MAPPINGS_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Mappings
{
public:
  Database_Mappings ();
  ~Database_Mappings ();
  void create1 ();
  void create2 ();
  void defaults ();
  void optimize ();
  void import (const string& name, const string& data);
  string output (const string& name);
  void create (const string & name);
  void erase (const string & name);
  vector <string> names ();
  string original ();
  vector <Passage> translate (const string& input, const string& output, int book, int chapter, int verse);
private:
  sqlite3 * connect ();
};


#endif
