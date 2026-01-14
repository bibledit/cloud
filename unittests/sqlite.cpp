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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <database/sqlite.h>


TEST (sqlite, functions)
{
  refresh_sandbox (false);

  sqlite3 * db = database::sqlite::connect ("sqlite");
  EXPECT_NE (db, nullptr);
  database::sqlite::exec (db, "CREATE TABLE test (column1 integer, column2 integer, column3 integer);");
  database::sqlite::exec (db, "INSERT INTO test VALUES (123, 456, 789);");
  database::sqlite::exec (db, "INSERT INTO test VALUES (234, 567, 890);");
  database::sqlite::exec (db, "INSERT INTO test VALUES (345, 678, 901);");
  std::map <std::string, std::vector <std::string> > actual = database::sqlite::query (db, "SELECT column1, column2, column3 FROM test;");
  EXPECT_EQ ("567", actual ["column2"] [1]);
  database::sqlite::disconnect (db);
  database::sqlite::disconnect (nullptr);

  EXPECT_TRUE (database::sqlite::healthy ("sqlite"));
  unlink (database::sqlite::get_file ("sqlite").c_str());
  EXPECT_FALSE (database::sqlite::healthy ("sqlite"));

  EXPECT_EQ ("He''s", database::sqlite::no_sql_injection ("He's"));
  
  refresh_sandbox (false);
}


TEST (sqlite, sqlite_database)
{
  refresh_sandbox (false);

  SqliteDatabase sql ("sqlite");
  
  sql.clear();
  sql.add ("CREATE TABLE test (column1 integer, column2 integer, column3 integer);");
  sql.execute();
  
  sql.clear();
  sql.add ("INSERT INTO test VALUES (123, 456, 789);");
  sql.execute();

  sql.clear();
  sql.add ("INSERT INTO test VALUES (234, 567, 890);");
  sql.execute();

  sql.clear();
  sql.add ("INSERT INTO test VALUES (345, 678, 901);");
  sql.execute();

  sql.clear();
  sql.add ("SELECT column1, column2, column3 FROM test;");
  std::map <std::string, std::vector <std::string> > actual = sql.query ();
  EXPECT_EQ (3, actual.size());
  EXPECT_EQ ("567", actual ["column2"] [1]);
  
  refresh_sandbox (false);
}


#endif

