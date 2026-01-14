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


Database_Localization::Database_Localization (const std::string& language)
{
  m_language = language;
}


std::string Database_Localization::database () const
{
  return "localization_" + m_language;
}


void Database_Localization::create (std::string po)
{
  SqliteDatabase sql (database());
  sql.set_sql ("PRAGMA temp_store = MEMORY;");
  sql.execute();
  sql.set_sql ("PRAGMA synchronous = OFF;");
  sql.execute();
  sql.set_sql ("PRAGMA journal_mode = OFF;");
  sql.execute();
  sql.set_sql ("DROP TABLE IF EXISTS localization;");
  sql.execute();
  sql.set_sql ("VACUUM;");
  sql.execute();
  sql.set_sql ("CREATE TABLE IF NOT EXISTS localization (msgid text, msgstr text);");
  sql.execute();
  const std::unordered_map <std::string, std::string> translations = locale_logic_read_msgid_msgstr (po);
  for (const auto& element : translations) {
    sql.clear();
    sql.add ("INSERT INTO localization VALUES (");
    sql.add (element.first);
    sql.add (",");
    sql.add (element.second);
    sql.add (");");
    sql.execute ();
  }
}


std::string Database_Localization::translate (const std::string& english)
{
  SqliteDatabase sql (database());
  sql.add ("SELECT msgstr FROM localization WHERE msgid =");
  sql.add (english);
  sql.add (";");
  const std::vector <std::string> msgstrs = sql.query () ["msgstr"];
  if (!msgstrs.empty ())
    if (!msgstrs.at(0).empty ())
      return msgstrs.at(0);
  return english;
}


std::string Database_Localization::backtranslate (const std::string& localization)
{
  SqliteDatabase sql (database());
  sql.add ("SELECT msgid FROM localization WHERE msgstr =");
  sql.add (localization);
  sql.add (";");
  const std::vector <std::string> msgids = sql.query () ["msgid"];
  if (!msgids.empty ())
    if (!msgids.at(0).empty ())
      return msgids.at(0);
  return localization;
}
