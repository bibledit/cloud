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


#include <database/strong.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// This is the database for the Strong's definitions.
// Resilience: It is never written to. 
// Chances of corruption are nearly zero.


constexpr const auto greekstrong {"greekstrong"};


// Get Strong's definition for the $strong's number.
std::string Database_Strong::definition (std::string strong)
{
  sqlite3 * db;
  {
    // Search Greek lexicon.
    SqliteDatabase sql (greekstrong);
    sql.add ("SELECT definition FROM greekstrong WHERE id =");
    sql.add (strong);
    sql.add (";");
    const std::vector <std::string> definitions = sql.query () ["definition"];
    if (!definitions.empty ()) return definitions.at(0);
  }
  // Nothing found.
  return std::string();
}


// Get Strong's number(s) for the $lemma.
// Most lemma's refer to one Strong's number, but some lemma's refer to more than one.
std::vector <std::string> Database_Strong::strong (std::string lemma)
{
  // Search Greek lexicon.
  SqliteDatabase sql (greekstrong);
  sql.add ("SELECT id FROM greekstrong WHERE lemma =");
  sql.add (lemma);
  sql.add (";");
  const std::vector <std::string> ids = sql.query () ["id"];
  if (!ids.empty ())
    return ids;
  // Nothing found.
  return {};
}
