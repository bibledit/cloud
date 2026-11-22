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

#include <database/sword.h>
#include <database/sqlite.h>
#include <filter/url.h>

#include "filter/string.h"

// Databases resilience:
// They contain cached data.
// Rarely written to.
// Often read from.

namespace database::sword {

static std::string database_path (const std::string& module)
{
    return filter_url_create_root_path({"databases", "sword_" + module + ".sqlite"});
}

bool exists(const std::string& module)
{
    return file_or_dir_exists(database_path(module));
}

void store_all(const std::string& module, const std::vector<VerseText>& data)
{
    if (exists(module))
        remove(module);

    SqliteDatabase sql(database_path(module));

    // Greatly speed up data insertion.
    sql.add("PRAGMA synchronous=OFF;");
    sql.execute();

    sql.clear();

    // The primary key speeds up verse text retrieval.
    sql.add("CREATE TABLE verses (book INTEGER, chapter INTEGER, verse INTEGER, text TEXT, PRIMARY KEY (book, chapter, verse));");
    sql.execute();

    sql.clear();

    // Use transaction to speed insertion up a lot.
    sql.add("BEGIN;");
    sql.execute();

    for (const auto& [book, chapter, verse, text] : data)
    {
        sql.clear();
        sql.add("INSERT INTO verses (book, chapter, verse, text) VALUES (");
        sql.add(book);
        sql.add(",");
        sql.add(chapter);
        sql.add(",");
        sql.add(verse);
        sql.add(",");
        sql.add(text);
        sql.add(");");
        sql.execute();
    }

    sql.clear();

    sql.add("COMMIT;");
    sql.execute();
}

std::string get_verse(const std::string& module, const int book, const int chapter, const int verse)
{
    SqliteDatabase sql(database_path(module));
    sql.add("SELECT text FROM verses WHERE book =");
    sql.add(book);
    sql.add("AND chapter =");
    sql.add(chapter);
    sql.add("AND verse =");
    sql.add(verse);
    sql.add(";");
    if (const std::vector<std::string> result = sql.query()["text"]; not result.empty())
        return result.front();
    return {};
}

void remove(const std::string& module)
{
    filter_url_unlink(database_path(module));
}

void remove_all()
{
    constexpr auto tag {"removeall"};
    const std::string path = filter::string::replace(tag, "*", database_path(tag));
    const std::string command = "rm " + path;
    system (command.c_str());
}

}
