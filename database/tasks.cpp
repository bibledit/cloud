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


#include <database/tasks.h>
#include <database/sqlite.h>
#include "logs.h"
#include "filter/string.h"
#include "filter/url.h"
#include "tasks/logic.h"


namespace database::tasks {
[[nodiscard]] static constexpr const char* db_name() noexcept
{
    return "tasks";
}


static void remove_db_file ()
{
    filter_url_unlink(sqlite::get_file (db_name ()));
}


void save(const std::deque<Task>& tasks)
{
    remove_db_file();
    SqliteDatabase sql(db_name());
    sql.add("CREATE TABLE IF NOT EXISTS tasks ("
        "task integer,"
        "parameter1 text,"
        "parameter2 text,"
        "parameter3 text,"
        "parameter4 text"
        ");");
    sql.execute();

    // Do only one disk sync for everything.
    sql.clear();
    sql.add("BEGIN;");
    sql.execute();

    for (const auto& [task, parameters] : tasks)
    {
        const auto [p1, p2, p3, p4] = ::tasks::extract(parameters);
        sql.clear();
        sql.add("INSERT INTO tasks (task, parameter1, parameter2, parameter3, parameter4) VALUES (");
        sql.add(static_cast<int>(task));
        sql.add(",");
        sql.add(p1);
        sql.add(",");
        sql.add(p2);
        sql.add(",");
        sql.add(p3);
        sql.add(",");
        sql.add(p4);
        sql.add(");");
        sql.execute ();
    }

    sql.clear();
    sql.add("COMMIT;");
    sql.execute();
}


std::deque<Task> load()
{
    std::deque<Task> tasks;
    if (file_or_dir_exists(sqlite::get_file (db_name ())))
    {
        try
        {
            SqliteDatabase sql (db_name());
            sql.add ("SELECT task, parameter1, parameter2, parameter3, parameter4 FROM tasks;");
            std::map <std::string, std::vector<std::string>> result = sql.query();
            const std::vector<std::string>& db_task = result.at("task");
            const std::vector <std::string>& db_parameter1 = result.at("parameter1");
            const std::vector <std::string>& db_parameter2 = result.at("parameter2");
            const std::vector <std::string>& db_parameter3 = result.at("parameter3");
            const std::vector <std::string>& db_parameter4 = result.at("parameter4");
            for (std::size_t i {0}; i < db_task.size (); ++i) {
                Task task {
                    .task = static_cast<::tasks::enums::task>(filter::string::convert_to_int(db_task.at(i))),
                    .parameters = {
                        db_parameter1.at(i),
                        db_parameter2.at(i),
                        db_parameter3.at(i),
                        db_parameter4.at(i)
                    },
                };
                tasks.push_back(std::move(task));
            }
        }
        catch (const std::exception& exception)
        {
            logs::log(exception.what());
        }
        remove_db_file();
    }
    return tasks;
}


}
