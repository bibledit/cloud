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
#pragma GCC diagnostic ignored "-Wcharacter-conversion"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <tasks/logic.h>


TEST(tasks, logic)
{
    refresh_sandbox(false);

    constexpr auto task1 = tasks::enums::task::check_bible;
    constexpr auto task2 = tasks::enums::task::cache_resources;
    constexpr auto task3 = tasks::enums::task::convert_bible_to_resource;
    constexpr auto task4 = tasks::enums::task::export_all;
    constexpr auto parameter = [](const int number = 0) -> std::string
    {
        return "parameter" + (number ? std::to_string(number) : std::string());
    };

    tasks_logic_queue("task1");
    tasks_logic_queue( task1 );
    tasks_logic_queue("task3");
    tasks_logic_queue( task3 );
    tasks_logic_queue("task4", {"parameter1", "parameter2"});
    tasks_logic_queue( task4,  { parameter(1), parameter(2)});
    EXPECT_TRUE(tasks_logic_queued ("task1"));
    EXPECT_TRUE(tasks_logic_queued ( task1));
    EXPECT_FALSE(tasks_logic_queued ("task2"));
    EXPECT_FALSE(tasks_logic_queued ( task2));
    EXPECT_FALSE(tasks_logic_queued ("task1", { "parameter"  }));
    EXPECT_FALSE(tasks_logic_queued ( task1,  {  parameter() }));
    EXPECT_TRUE(tasks_logic_queued ("task4"));
    EXPECT_FALSE(tasks_logic_queued ( task4 ));
    EXPECT_TRUE(tasks_logic_queued ("task4", { "parameter1" }));
    EXPECT_FALSE(tasks_logic_queued (task4, { parameter(1) }));
    EXPECT_TRUE(tasks_logic_queued ("task4", { "parameter1", "parameter2" }));
    EXPECT_TRUE(tasks_logic_queued (task4, { parameter(1), parameter(2) }));
    EXPECT_FALSE(tasks_logic_queued ("task4", { "parameter1", "parameter3" }));
    EXPECT_FALSE(tasks_logic_queued (task4, { parameter(1), parameter(3) }));
    EXPECT_FALSE(tasks_logic_queued ("task4", { "parameter2" }));
    EXPECT_FALSE(tasks_logic_queued (task4, { parameter(2) }));
}

#endif
