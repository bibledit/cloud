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


#include <unittests/sqlite.h>
#include <unittests/utilities.h>
#include <database/sqlite.h>


void test_sqlite ()
{
  trace_unit_tests (__func__);
  
  // Tests for SQLite.
  sqlite3 * db = database_sqlite_connect ("sqlite");
  if (!db) error_message (__LINE__, __func__, "pointer", "NULL");
  database_sqlite_exec (db, "CREATE TABLE test (column1 integer, column2 integer, column3 integer);");
  database_sqlite_exec (db, "INSERT INTO test VALUES (123, 456, 789);");
  database_sqlite_exec (db, "INSERT INTO test VALUES (234, 567, 890);");
  database_sqlite_exec (db, "INSERT INTO test VALUES (345, 678, 901);");
  map <string, vector <string> > actual = database_sqlite_query (db, "SELECT column1, column2, column3 FROM test;");
  evaluate (__LINE__, __func__, "567", actual ["column2"] [1]);
  database_sqlite_disconnect (db);
  database_sqlite_disconnect (NULL);

  evaluate (__LINE__, __func__, true, database_sqlite_healthy ("sqlite"));
  unlink (database_sqlite_file ("sqlite").c_str());
  evaluate (__LINE__, __func__, false, database_sqlite_healthy ("sqlite"));

  evaluate (__LINE__, __func__, "He''s", database_sqlite_no_sql_injection ("He's"));
}
