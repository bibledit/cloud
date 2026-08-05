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


#include <database/config/general.h>
#include <filter/date.h>
#include <filter/string.h>
#include <webserver/request.h>


namespace filter::date {
// Gets the second within the minute from the seconds since the Unix epoch.
int get_second_within_minute(const int seconds)
{
    return seconds % 60;
}


// Gets the minute within the hour from the seconds since the Unix epoch.
int get_minute_within_hour(const int seconds)
{
    return seconds / 60 % 60;
}


// Gets the hour within the day from the seconds since the Unix epoch.
int get_hour_within_day(const int seconds)
{
    return seconds / 3600 % 24;
}


// The numerical day of the month from 1 to 31.
int get_day_within_month(const int seconds)
{
    const std::chrono::seconds duration(seconds);
    const auto epoch_plus_duration = std::chrono::system_clock::time_point{} + duration;
    const std::chrono::year_month_day year_month_day{std::chrono::floor<std::chrono::days>(epoch_plus_duration)};
    const auto day_of_month = static_cast<int>(static_cast<unsigned>(year_month_day.day()));
    return day_of_month;
    // const time_t tt = seconds; // Todo goes out if the above compiles on all OSes.
    // const tm utc_tm = *gmtime(&tt);
    // const int day = utc_tm.tm_mday;
    // return day;
}


// The numerical day of the week: 0 (for Sunday) through 6 (for Saturday)
int get_day_within_week(const int seconds)
{
    // Input is the seconds since 1970-01-01 UTC, get time point.
    const std::chrono::system_clock::time_point time_point{std::chrono::seconds(seconds)};
    // Cast time point down to day precision.
    const auto day_point = std::chrono::floor<std::chrono::days>(time_point);
    // Get the weekday object (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    const std::chrono::weekday weekday{day_point};
    // Get numeric index.
    using func_return_type = decltype(get_day_within_week(0));
    return static_cast<func_return_type>(weekday.c_encoding());
    // const time_t tt = seconds; // Todo goes out if the above compiles on all OSes.
    // const tm utc_tm = *gmtime(&tt);
    // const int day = utc_tm.tm_wday;
    // return day;
}


// Numeric representation of a month: 1 through 12.
int get_month_within_year(const int seconds)
{
    // Input is seconds since 1 Jan 1970 UTC, convert to wall clock timepoint.
    const std::chrono::system_clock::time_point time_point{std::chrono::seconds(seconds)};
    // Cast down to day precision and create a calendar date object.
    const std::chrono::year_month_day year_month_day{std::chrono::floor<std::chrono::days>(time_point)};
    // Get the month object.
    const std::chrono::month month {year_month_day.month()};
    // Get numeric index (1 = January, 2 = February, ..., 12 = December).
    const auto month_index = static_cast<unsigned int>(month);
    return static_cast<decltype(get_month_within_year(0))>(month_index);
    // Todo remove the below if the above compiles on all OSes.
    // const time_t tt = seconds;
    // const tm utc_tm = *gmtime(&tt);
    // const int month = utc_tm.tm_mon + 1;
    // return month;
}


// A full numeric representation of a year, 4 digits: 2014.
int get_year_ad(const int seconds)
{
    // Input is seconds since Unix epoch, convert that o a system seconds timepoint.
    const std::chrono::sys_seconds time_point{std::chrono::seconds(seconds)};
    // Convert time point to a calendar year-month-day structure.
    std::chrono::year_month_day year_month_day{std::chrono::floor<std::chrono::days>(time_point)};
    // Extract the year.
    int year = static_cast<int>(year_month_day.year());
    return year;
    // Todo code below can go out once the code above compiles on all OSes.
    // const time_t tt = seconds;
    // const tm utc_tm = *gmtime(&tt);
    // // Get years since 1900, and correct to get years since birth of Christ.
    // const int year = utc_tm.tm_year + 1900;
    // return year;
}


// This function gives the number of microseconds within the current second.
int get_microseconds_within_second()
{
    const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    const int microseconds_in_second = static_cast<decltype(microseconds_in_second)>(microseconds % 1000000);
    return microseconds_in_second;
}


// This function returns the seconds since the Unix epoch, which is 1 January 1970 UTC.
int get_seconds_since_epoch()
{
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const int seconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
    return seconds;
}


// Returns the seconds since the Unix epoch for $year (yyyy) and $month (1...12) and $day (1...31).
int get_seconds_since_epoch(const int year, const int month, const int day)
{
    // Convert input data to C++20 calendar struct.
    const std::chrono::year_month_day year_month_day{
        std::chrono::year{year},
        std::chrono::month{static_cast<unsigned>(month)},
        std::chrono::day{static_cast<unsigned>(day)}
    };
    // Convert to sys_days (a timepoint that tracks days since the Epoch).
    const std::chrono::sys_days time_point = year_month_day;
    // Extract the total seconds since the Unix epoch.
    const auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(time_point.time_since_epoch()).count();
    return static_cast<decltype(get_seconds_since_epoch(0,0,0))> (seconds_since_epoch);
    // Once the code above builds on all OSes, remove the code below.
    // int seconds = 0;
    // bool done = false;
    // bool hit = false;
    // do
    // {
    //     seconds += 86400;
    //     int myyear = get_year_ad(seconds);
    //     int mymonth = get_month_within_year(seconds);
    //     int myday = get_day_within_month(seconds);
    //     if ((year == myyear) && (month == mymonth)) hit = true;
    //     done = ((year == myyear) && (month == mymonth) && (day == myday));
    //     if (hit) if (month != mymonth) done = true;
    // }
    // while (!done);
    // return seconds;
}


// This function takes the "seconds" parameter,
// corrects it according to the local timezone,
// and returns it.
int get_local_seconds(int seconds)
{
    const int offset = database::config::general::get_timezone();
    constexpr int seconds_in_hour = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(1)).count();
    static_assert(seconds_in_hour == 3600);
    seconds += offset * seconds_in_hour;
    return seconds;
}


void get_previous_month(int& month, int& year)
{
    month--;
    if (month <= 0)
    {
        month = 12;
        year--;
    }
}


void get_next_month(int& month, int& year)
{
    month++;
    if (month > 12)
    {
        month = 1;
        year++;
    }
}


std::string day_rfc822(int day)
{
    if (day == 0) return "Sun";
    if (day == 1) return "Mon";
    if (day == 2) return "Tue";
    if (day == 3) return "Wed";
    if (day == 4) return "Thu";
    if (day == 5) return "Fri";
    if (day == 6) return "Sat";
    return std::string();
}


std::string month_rfc822(int month)
{
    if (month == 1) return "Jan";
    if (month == 2) return "Feb";
    if (month == 3) return "Mar";
    if (month == 4) return "Apr";
    if (month == 5) return "May";
    if (month == 6) return "Jun";
    if (month == 7) return "Jul";
    if (month == 8) return "Aug";
    if (month == 9) return "Sep";
    if (month == 10) return "Oct";
    if (month == 11) return "Nov";
    if (month == 12) return "Dec";
    return std::string();
}


// Converts the number of $seconds since the Unix epoch
// to date and time values according to RFC 822,
// e.g.: Wed, 02 Oct 2002 15:00:00 +0200.
std::string rfc822(int seconds)
{
    std::string rfc822;
    // The feed validator at https://validator.w3.org/feed/ says:
    // <pubDate>Wed, 18 Feb 2017 12:26:39 +0100</pubDate>
    // This feed does not validate: Incorrect day of week: Wed (2 occurrences).
    // Yet, 18 February 2017 is on a Wednesday.
    // It continues to say that:
    // If it turns out that computing the correct day of week is impractical using the software you have available, then RFC 822 permits omitting both the day of the week and the subsequent comma from the value.
    // So to work around the error in the validator, the day of the week is left out.
    // int weekday = numerical_week_day (seconds);
    // rfc822.append (day_rfc822 (weekday));
    // rfc822.append (", ");
    std::string monthday = std::to_string(get_day_within_month(seconds));
    rfc822.append(filter::string::fill(monthday, 2, '0'));
    rfc822.append(" ");
    int month = get_month_within_year(seconds);
    rfc822.append(month_rfc822(month));
    rfc822.append(" ");
    int year = get_year_ad(seconds);
    rfc822.append(std::to_string(year));
    rfc822.append(" ");
    std::string hour = std::to_string(get_hour_within_day(seconds));
    rfc822.append(filter::string::fill(hour, 2, '0'));
    rfc822.append(":");
    std::string minute = std::to_string(get_minute_within_hour(seconds));
    rfc822.append(filter::string::fill(minute, 2, '0'));
    rfc822.append(":");
    std::string second = std::to_string(get_second_within_minute(seconds));
    rfc822.append(filter::string::fill(second, 2, '0'));
    rfc822.append(" ");
    int timezone = database::config::general::get_timezone();
    if (timezone >= 0) rfc822.append("+");
    else rfc822.append("-");
    if (timezone < 0) timezone = 0 - timezone;
    rfc822.append(filter::string::fill(std::to_string(timezone), 2, '0'));
    rfc822.append("00");
    return rfc822;
}


// Calculates the number of microseconds elapsed since $start.
// It returns the elapsed number of microseconds.
long elapsed_microseconds(long start)
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    long elapsed = microseconds - start;
    return elapsed;
}


std::string localized_date_format()
{
    time_t tt;
    time(&tt);
    tm* localtm = localtime(&tt);
    char buffer[20];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-y2k"
    strftime(buffer, sizeof(buffer), "%x", localtm);
#pragma GCC diagnostic pop
    return std::string(buffer);
}


std::string date_format_to_text(date_format format)
{
    switch (format)
    {
    case dd_mm_yyyy: return "dd/mm/yyyy";
    case mm_dd_yyyy: return "mm/dd/yyyy";
    case yyyy_mn_dd: return "yyyy-mm-dd";
    default: return std::string();
    }
    return std::string();
}


std::string localized_date_format(Webserver_Request& webserver_request)
{
    int time = get_seconds_since_epoch();

    std::string day = std::to_string(get_day_within_month(time));
    std::string month = std::to_string(get_month_within_year(time));
    std::string year = std::to_string(get_year_ad(time));

    date_format df = static_cast<date_format>(webserver_request.database_config_user()->get_notes_date_format());

    switch (df)
    {
    case dd_mm_yyyy:
        {
            return day + "/" + month + "/" + year;
        }
    case mm_dd_yyyy:
        {
            return month + "/" + day + "/" + year;
        }
    case yyyy_mn_dd:
    default:
        {
            return year + "-" + month + "-" + day;
        }
    }

    return std::string();
}
}
