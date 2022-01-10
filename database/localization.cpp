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


#include <database/localization.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>
#include <locale/logic.h>


// Database resilience.
// It is written to once upon Bibledit setup.
// After that it is only read.
// In case of corruption, upgrade Bibledit and it will recreate the database.


Database_Localization::Database_Localization (const string& language_in)
{
  language = language_in;
}


Database_Localization::~Database_Localization ()
{
}


sqlite3 * Database_Localization::connect ()
{
  return database_sqlite_connect ("localization_" + language);
}


void Database_Localization::create (string po)
{
  sqlite3 * db = connect ();
  database_sqlite_exec (db, "PRAGMA temp_store = MEMORY;");
  database_sqlite_exec (db, "PRAGMA synchronous = OFF;");
  database_sqlite_exec (db, "PRAGMA journal_mode = OFF;");
  database_sqlite_exec (db, "DROP TABLE IF EXISTS localization;");
  database_sqlite_exec (db, "VACUUM;");
  database_sqlite_exec (db, "CREATE TABLE IF NOT EXISTS localization (msgid text, msgstr text);");
  map <string, string> translations = locale_logic_read_po (po);
  for (auto & element : translations) {
    SqliteSQL sql = SqliteSQL ();
    sql.add ("INSERT INTO localization VALUES (");
    sql.add (element.first);
    sql.add (",");
    sql.add (element.second);
    sql.add (");");
    database_sqlite_exec (db, sql.sql);
  }
  database_sqlite_disconnect (db);
}


string Database_Localization::translate (const string& english)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT msgstr FROM localization WHERE msgid =");
  sql.add (english);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> msgstrs = database_sqlite_query (db, sql.sql) ["msgstr"];
  database_sqlite_disconnect (db);
  if (!msgstrs.empty ()) if (!msgstrs[0].empty ()) return msgstrs [0];
  return english;
}


string Database_Localization::backtranslate (const string& localization)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT msgid FROM localization WHERE msgstr =");
  sql.add (localization);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> msgids = database_sqlite_query (db, sql.sql) ["msgid"];
  database_sqlite_disconnect (db);
  if (!msgids.empty ()) if (!msgids[0].empty ()) return msgids [0];
  return localization;
}
