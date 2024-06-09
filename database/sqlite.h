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

namespace database::sqlite {

sqlite3 * connect_file (std::string filename);
std::string file (std::string database);
std::string suffix ();
sqlite3 * connect (std::string database);
std::string no_sql_injection (std::string sql);
void exec (sqlite3 * db, std::string sql);
std::map <std::string, std::vector <std::string> > query (sqlite3 * db, std::string sql);
void disconnect (sqlite3 * database);
bool healthy (std::string database);
void error (sqlite3 * database, const std::string & prefix, char * error);

}


// Creates a database SQL query.
class SqliteSQL
{
public:
  void clear ();
  void add (const char * fragment);
  void add (int value);
  void add (std::string value);
  std::string sql {};
};

class SqliteDatabase
{
public:
  SqliteDatabase (const std::string& filename);
  ~SqliteDatabase ();
  SqliteDatabase(const SqliteDatabase&) = delete;
  SqliteDatabase operator=(const SqliteDatabase&) = delete;
  void clear ();
  void add (const char * fragment);
  void add (int value);
  void add (std::string value);
  const std::string& get_sql();
  void set_sql (const std::string& sql);
  void execute ();
  std::map <std::string, std::vector <std::string> > query ();
private:
  sqlite3 * db {nullptr};
  std::string m_sql {};
};
