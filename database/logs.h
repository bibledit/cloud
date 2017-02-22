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


#ifndef INCLUDED_DATABASE_LOGS_H
#define INCLUDED_DATABASE_LOGS_H


#include <config/libraries.h>


class Database_Logs
{
public:
  static void log (string description, int level = 5);
  static void log (string subject, string body, int level = 5);
  static void rotate ();
  static vector <string> get (string & lastfilename);
  static string next (string &filename);
  static void clear ();
  static string folder ();
};


#endif
