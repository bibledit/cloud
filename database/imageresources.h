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

class Database_ImageResources
{
public:
  std::vector <std::string> names ();
  void create (std::string name);
  void erase (std::string name);
  void erase (std::string name, std::string image);
  std::string store (std::string name, std::string file);
  void assign (std::string name, std::string image,
               int book1, int chapter1, int verse1,
               int book2, int chapter2, int verse2);
  std::vector <std::string> get (std::string name, int book, int chapter, int verse);
  std::vector <std::string> get (std::string name);
  void get (std::string name, std::string image,
            int & book1, int & chapter1, int & verse1,
            int & book2, int & chapter2, int & verse2);
  std::string get (std::string name, std::string image);
private:
  std::string mainFolder ();
  std::string resourceFolder (const std::string& name);
  std::string imagePath (std::string name, std::string image);
  std::string databaseFile ();
  sqlite3 * connect (std::string name);
};
