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


#include <database/bibleactions.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


constexpr auto database_name {"bibleactions"};


namespace database::bible_actions {


void create()
{
    SqliteDatabase sql(database_name);
    sql.add("CREATE TABLE IF NOT EXISTS bibleactions ("
        " bible text,"
        " book integer,"
        " chapter integer,"
        " usfm text"
        ");");
    sql.execute();
}


void clear()
{
    SqliteDatabase sql(database_name);
    sql.add("DROP TABLE IF EXISTS bibleactions;");
    sql.execute();
}


void optimize()
{
    SqliteDatabase sql(database_name);
    sql.add("VACUUM;");
    sql.execute();
}


void record(const std::string& bible, const int book, const int chapter, const std::string& usfm)
{
    if (get_usfm(bible, book, chapter).empty())
    {
        SqliteDatabase sql(database_name);
        sql.add("INSERT INTO bibleactions VALUES (");
        sql.add(bible);
        sql.add(",");
        sql.add(book);
        sql.add(",");
        sql.add(chapter);
        sql.add(",");
        sql.add(usfm);
        sql.add(");");
        sql.execute();
    }
}


std::vector<std::string> get_bibles()
{
    SqliteDatabase sql(database_name);
    sql.add("SELECT DISTINCT bible FROM bibleactions ORDER BY bible;");
    const std::vector<std::string> notes = sql.query()["bible"];
    return notes;
}


std::vector<int> get_books(const std::string& bible)
{
    SqliteDatabase sql(database_name);
    sql.add("SELECT DISTINCT book FROM bibleactions WHERE bible =");
    sql.add(bible);
    sql.add("ORDER BY book;");
    const std::vector<std::string> result = sql.query()["book"];
    std::vector<int> books;
    std::ranges::for_each(result, [&books](const auto& book)
    {
        books.push_back(filter::string::convert_to_int(book));
    });
    return books;
}


std::vector<int> get_chapters(const std::string& bible, const int book)
{
    SqliteDatabase sql(database_name);
    sql.add("SELECT DISTINCT chapter FROM bibleactions WHERE bible =");
    sql.add(bible);
    sql.add("AND book =");
    sql.add(book);
    sql.add("ORDER BY chapter;");
    const std::vector<std::string> result = sql.query()["chapter"];
    std::vector<int> chapters;
    std::ranges::for_each(result, [&chapters](const auto& chapter)
    {
        chapters.push_back(filter::string::convert_to_int(chapter));
    });
    return chapters;
}


std::string get_usfm(const std::string& bible, const int book, const int chapter)
{
    SqliteDatabase sql(database_name);
    sql.add("SELECT usfm FROM bibleactions WHERE bible =");
    sql.add(bible);
    sql.add("AND book =");
    sql.add(book);
    sql.add("AND chapter =");
    sql.add(chapter);
    sql.add(";");
    std::vector<std::string> result = sql.query()["usfm"];
    if (!result.empty())
    {
        const std::string usfm = result.at(0);
        return usfm;
    };
    return {};
}


void erase(const std::string& bible, const int book, const int chapter)
{
    SqliteDatabase sql(database_name);
    sql.add("DELETE FROM bibleactions WHERE bible =");
    sql.add(bible);
    sql.add("AND book =");
    sql.add(book);
    sql.add("AND chapter =");
    sql.add(chapter);
    sql.add(";");
    sql.execute();
}


} // Namespace.

