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


#ifndef INCLUDED_DATABASE_USERRESOURCES_H
#define INCLUDED_DATABASE_USERRESOURCES_H


#include <config/libraries.h>


class Database_UserResources
{
public:
  static vector <string> names ();
  static void remove (const string& name);
  static string url (const string& name);
  static void url (const string& name, const string & value);
  static string book (const string& name, int id);
  static void book (const string& name, int id, const string & fragment);
private:
  static string folder ();
  static string fragment ();
  static string file (const string& name);
  static string load (const string & name, size_t offset);
  static void save (const string & name, size_t offset, const string & value);
};


#endif
