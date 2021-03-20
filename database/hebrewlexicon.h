/*
Copyright (©) 2003-2021 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_HEBREWLEXICON_H
#define INCLUDED_DATABASE_HEBREWLEXICON_H


#include <config/libraries.h>


class Database_HebrewLexicon
{
public:
  void create ();
  void optimize ();
  void setaug (string aug, string target);
  void setbdb (string id, string definition);
  void setmap (string id, string bdb);
  void setpos (string code, string name);
  void setstrong (string strong, string definition);
  string getaug (string aug);
  string getbdb (string id);
  string getmap (string id);
  string getpos (string code);
  string getstrong (string strong);
private:
  const char * filename ();
};


#endif
