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


#ifndef INCLUDED_DATABASE_IMAGERESOURCES_H
#define INCLUDED_DATABASE_IMAGERESOURCES_H


#include <config/libraries.h>


class Database_ImageResources
{
public:
  vector <string> names ();
  void create (string name);
  void erase (string name);
  void erase (string name, string image);
  string store (string name, string file);
  void assign (string name, string image,
               int book1, int chapter1, int verse1,
               int book2, int chapter2, int verse2);
  vector <string> get (string name, int book, int chapter, int verse);
  vector <string> get (string name);
  void get (string name, string image,
            int & book1, int & chapter1, int & verse1,
            int & book2, int & chapter2, int & verse2);
  string get (string name, string image);
private:
  string mainFolder ();
  string resourceFolder (const string& name);
  string imagePath (string name, string image);
  string databaseFile ();
  sqlite3 * connect (string name);
};


#endif
