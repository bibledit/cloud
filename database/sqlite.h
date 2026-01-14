/*
Copyright (©) 2003-2026 Teus Benschop.

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

sqlite3 * connect_file (const std::string& filename);
std::string get_file (const std::string& database);
std::string suffix ();
sqlite3 * connect (const std::string& database);
std::string no_sql_injection (const std::string& sql);
void exec (sqlite3 * db, const std::string& sql);
std::map <std::string, std::vector <std::string> > query (sqlite3 * db, const std::string& sql);
void disconnect (sqlite3 * database);
bool healthy (const std::string& database);
void error (sqlite3 * database, const std::string& prefix, char * error);

}


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
  const std::string& get_sql() const;
  void set_sql (const std::string& sql);
  void push_sql();
  void pop_sql();
  void execute ();
  std::map <std::string, std::vector <std::string> > query ();
  void disconnect ();
private:
  std::string m_filename {};
  sqlite3 * db {nullptr};
  std::string m_sql {};
  std::string m_save_restore {};
};
