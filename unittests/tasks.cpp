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
#include "database/tasks.h"


TEST(tasks, queuing)
{
    constexpr auto task1 = tasks::enums::task::check_bible;
    constexpr auto task2 = tasks::enums::task::cache_resources;
    constexpr auto task3 = tasks::enums::task::convert_bible_to_resource;
    constexpr auto task4 = tasks::enums::task::export_all;
    constexpr auto parameter = [](const int number = 0) -> std::string
    {
        return "parameter" + (number ? std::to_string(number) : std::string());
    };

    tasks::tasks_logic_queue( task1 );
    tasks::tasks_logic_queue( task3 );
    tasks::tasks_logic_queue( task4,  { parameter(1), parameter(2)});
    EXPECT_TRUE (tasks::tasks_logic_queued ( task1 ));
    EXPECT_FALSE(tasks::tasks_logic_queued ( task2 ));
    EXPECT_FALSE(tasks::tasks_logic_queued ( task1, { parameter() }));
    EXPECT_FALSE(tasks::tasks_logic_queued ( task4 ));
    EXPECT_FALSE(tasks::tasks_logic_queued ( task4, { parameter(1) }));
    EXPECT_TRUE (tasks::tasks_logic_queued ( task4, { parameter(1), parameter(2) }));
    EXPECT_FALSE(tasks::tasks_logic_queued ( task4, { parameter(1), parameter(3) }));
    EXPECT_FALSE(tasks::tasks_logic_queued ( task4, { parameter(2) }));
}


TEST(tasks, extract_parameters)
{
    constexpr auto one   {"one"};
    constexpr auto two   {"two"};
    constexpr auto three {"three"};
    constexpr auto four  {"four"};
    constexpr auto five  {"five"};

    struct TestCase {
        std::vector<std::string> input;
        std::string expected_1;
        std::string expected_2;
        std::string expected_3;
        std::string expected_4;
    };

    std::vector<TestCase> test_cases = {
        {.input = {},                            .expected_1 = "",  .expected_2 = "",  .expected_3 = "",    .expected_4 = ""  },
        {.input = {one},                         .expected_1 = one, .expected_2 = "",  .expected_3 = "",    .expected_4 = ""  },
        {.input = {one, two},                    .expected_1 = one, .expected_2 = two, .expected_3 = "",    .expected_4 = ""  },
        {.input = {one, two, three},             .expected_1 = one, .expected_2 = two, .expected_3 = three, .expected_4 = ""  },
        {.input = {one, two, three, four},       .expected_1 = one, .expected_2 = two, .expected_3 = three, .expected_4 = four},
        {.input = {one, two, three, four, five}, .expected_1 = one, .expected_2 = two, .expected_3 = three, .expected_4 = four},
    };

    for (auto& [input, e1, e2, e3, e4] : test_cases) {
        const auto [p1, p2, p3, p4] = tasks::extract(input);
        EXPECT_EQ(p1, e1);
        EXPECT_EQ(p2, e2);
        EXPECT_EQ(p3, e3);
        EXPECT_EQ(p4, e4);
    }
}


TEST(tasks, database)
{
    refresh_sandbox(false);
    using namespace database::tasks;
    using enum tasks::enums::task;

    // Test saving/loading empty queue.
    {
        std::deque<Task> queue = load();
        EXPECT_TRUE(queue.empty());
        save(queue);
        EXPECT_TRUE(load().empty());
    }

    // Save a task without parameters. Test loading it.
    {
        std::deque queue1 {Task {.task = create_css, .parameters = {}}};
        save(queue1);

        std::deque<Task> queue2 = load();
        EXPECT_EQ(queue2.size(), 1u);

        const auto& [task, parameters] = queue2.front();
        EXPECT_EQ(task, create_css);
        EXPECT_EQ(parameters.size(), 4);
        EXPECT_TRUE(std::ranges::all_of(parameters, [](const auto& p) { return p.empty(); }));
    }

    // Save two tasks with varying parameters. Test properly loading them.
    {
        std::deque queue {
            Task {.task = create_css, .parameters = {"p1"}},
            Task {.task = clean_tmp_files, .parameters = {"p1", "p2", "p3", "p4"}},
        };
        save(queue);

        queue = load();
        EXPECT_EQ(queue.size(), 2u);

        auto task = queue.front();
        EXPECT_EQ(queue[0].task, create_css);

        std::vector<std::string> parameters = {"p1", "", "", ""};
        EXPECT_EQ(queue[0].parameters, parameters);

        EXPECT_EQ(queue[1].task, clean_tmp_files);
        parameters = {"p1", "p2", "p3", "p4"};
        EXPECT_EQ(queue[1].parameters, parameters);
    }
}


#endif
