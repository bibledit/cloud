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
#include <filter/date.h>


TEST(filter, date)
{
    // Test the date and time related functions.
    {
        const int month = filter::date::get_month_within_year(filter::date::get_seconds_since_epoch());
        EXPECT_FALSE (month < 1 or month > 12);
        const int year = filter::date::get_year_ad(filter::date::get_seconds_since_epoch());
        EXPECT_FALSE(year < 2014 or year > 2050);
        timeval tv{};
        gettimeofday(&tv, nullptr);
        const int reference_second = static_cast<int>(tv.tv_sec);
        const int actual_second = filter::date::get_seconds_since_epoch();
        EXPECT_NEAR(actual_second, reference_second, 1);
        int microseconds = filter::date::get_microseconds_within_second();
        EXPECT_FALSE (microseconds < 0 or microseconds > 1'000'000);
    }

    // Seconds since Unix epoch.
    {
        int year, month, day, seconds;

        year = 2011;
        month = 2;
        day = 5;
        seconds = filter::date::get_seconds_since_epoch(year, month, day);
        EXPECT_EQ(1296864000, seconds);
        EXPECT_EQ(year, filter::date::get_year_ad (seconds));
        EXPECT_EQ(month, filter::date::get_month_within_year (seconds));
        EXPECT_EQ(day, filter::date::get_day_within_month (seconds));
        EXPECT_EQ(6, filter::date::get_day_within_week(seconds)); // Saturday.

        year = 2015;
        month = 3;
        day = 15;
        seconds = filter::date::get_seconds_since_epoch(year, month, day);
        EXPECT_EQ(1426377600, seconds);
        EXPECT_EQ(year, filter::date::get_year_ad (seconds));
        EXPECT_EQ(month, filter::date::get_month_within_year (seconds));
        EXPECT_EQ(day, filter::date::get_day_within_month (seconds));
        EXPECT_EQ(0, filter::date::get_day_within_week(seconds)); // Sunday.

        year = 2030;
        month = 12;
        day = 31;
        seconds = filter::date::get_seconds_since_epoch(year, month, day);
        EXPECT_EQ(1924905600, seconds);
        EXPECT_EQ(year, filter::date::get_year_ad(seconds));
        EXPECT_EQ(month, filter::date::get_month_within_year(seconds));
        EXPECT_EQ(day, filter::date::get_day_within_month(seconds));
        EXPECT_EQ(2, filter::date::get_day_within_week(seconds)); // Tuesday.
    }

    {
        EXPECT_EQ(filter::date::get_second_within_minute(123456), 36);
        EXPECT_EQ(filter::date::get_second_within_minute(1785774475), 55);
        EXPECT_EQ(filter::date::get_second_within_minute(1785774540), 0);
    }

    {
        EXPECT_EQ(filter::date::get_minute_within_hour(123456), 17);
        EXPECT_EQ(filter::date::get_minute_within_hour(1785774864), 34);
        EXPECT_EQ(filter::date::get_minute_within_hour(1785774907), 35);
    }

    {
        EXPECT_EQ(filter::date::get_hour_within_day(123456), 10);
        EXPECT_EQ(filter::date::get_hour_within_day(1785774864), 16);
        EXPECT_EQ(filter::date::get_hour_within_day(1234567890), 23);
    }
}

#endif
