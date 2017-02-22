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


#ifndef INCLUDED_DATABASE_CHECK_H
#define INCLUDED_DATABASE_CHECK_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Check_Hit
{
public:
  int rowid;
  string bible;
  int book;
  int chapter;
  int verse;
  string data;
};


class Database_Check
{
public:
  void create ();
  void optimize ();
  void truncateOutput (string bible);
  void recordOutput (string bible, int book, int chapter, int verse, string data);
  vector <Database_Check_Hit> getHits ();
  void approve (int id);
  void erase (int id);
  Passage getPassage (int id);
  vector <Database_Check_Hit> getSuppressions ();
  void release (int id);
private:
  const char * filename ();
};


#endif
