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


#ifndef INCLUDED_DATABASE_MORPHOLOGY_H
#define INCLUDED_DATABASE_MORPHOLOGY_H


#include <config/libraries.h>


class Database_MorphGnt
{
public:
  void create ();
  void optimize ();
  void store (int book, int chapter, int verse,
              string pos, string parsing, string word, string lemma);
  vector <int> rowids (int book, int chapter, int verse);
  string pos (int rowid);
  string parsing (int rowid);
  string word (int rowid);
  string lemma (int rowid);
private:
  const char * filename ();
  int get_id (const char * table_row, string item);
  string get_item (const char * item, int rowid);
};


#endif
