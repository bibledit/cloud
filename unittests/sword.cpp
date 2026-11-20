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
#include <database/sword.h>


TEST (database, sword)
{
    refresh_sandbox(false);

    constexpr auto module1 {"module1"};
    constexpr auto module2 {"module2"};
    constexpr auto data1 {"data1"};
    constexpr auto data2 {"data2"};

    EXPECT_FALSE(database::sword::exists(module1));
    EXPECT_FALSE(database::sword::exists(module2));

    database::sword::store(module1, data1);
    EXPECT_TRUE(database::sword::exists(module1));
    EXPECT_FALSE(database::sword::exists(module2));

    database::sword::store(module2, data2);
    EXPECT_TRUE(database::sword::exists(module1));
    EXPECT_TRUE(database::sword::exists(module2));

    EXPECT_EQ(database::sword::retrieve(module1), data1);
    EXPECT_EQ(database::sword::retrieve(module2), data2);

    database::sword::remove(module1);
    EXPECT_FALSE(database::sword::exists(module1));
    EXPECT_TRUE(database::sword::exists(module2));

    database::sword::remove(module2);
    EXPECT_FALSE(database::sword::exists(module1));
    EXPECT_FALSE(database::sword::exists(module2));

    refresh_sandbox(false);
}

#endif

