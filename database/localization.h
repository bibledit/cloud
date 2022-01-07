/*
Copyright (©) 2003-2022 Teus Benschop.

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

#include <string>
using namespace std;
#include <sqlite/sqlite3.h>

class Database_Localization
{
public:
  Database_Localization (const string& language_in);
  ~Database_Localization ();
  void create (string po);
  string translate (const string& english);
  string backtranslate (const string& localization);
private:
  string language;
  sqlite3 * connect ();
};

