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
#include <vector>
using namespace std;
// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <sqlite/sqlite3.h>
#include <filter/passage.h>

#ifdef HAVE_CLOUD

class Database_Sprint_Item
{
public:
  int day;
  int tasks;
  int complete;
};

class Database_Sprint
{
public:
  void create ();
  void optimize ();
  void storeTask (const string& bible, int year, int month, const string& title);
  void deleteTask (int id);
  vector <int> getTasks (const string& bible, int year, int month);
  string getTitle (int id);
  void updateComplete (int id, int percentage);
  int getComplete (int id);
  void updateMonthYear (int id, int month, int year);
  void logHistory (const string& bible, int year, int month, int day, int tasks, int complete);
  vector <Database_Sprint_Item> getHistory (const string& bible, int year, int month);
  void clearHistory (const string& bible, int year, int month);
private:
  sqlite3 * connect ();
};

#endif
