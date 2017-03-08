/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#include <filter/date.h>
#include <filter/string.h>
#include <database/config/general.h>


// Gets the second within the minute from the seconds since the Unix epoch.
int filter_date_numerical_second (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int second = utc_tm.tm_sec;
  return second;
}


// Gets the minute within the hour from the seconds since the Unix epoch.
int filter_date_numerical_minute (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int minute = utc_tm.tm_min;
  return minute;
}


// Gets the hour within the day from the seconds since the Unix epoch.
int filter_date_numerical_hour (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int hour = utc_tm.tm_hour;
  return hour;
}


// The numerical day of the month from 1 to 31.
int filter_date_numerical_month_day (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int day = utc_tm.tm_mday;
  return day;
}


// The numerical day of the week: 0 (for Sunday) through 6 (for Saturday)
int filter_date_numerical_week_day (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int day = utc_tm.tm_wday;
  return day;
}


// A C++ equivalent for PHP's date ("n") function.
// Numeric representation of a month: 1 through 12.
int filter_date_numerical_month (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int month = utc_tm.tm_mon + 1;
  return month;
}


// A C++ equivalent for PHP's date ("Y") function.
// A full numeric representation of a year, 4 digits: 2014.
int filter_date_numerical_year (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  // Get years since 1900, and correct to get years since birth of Christ.
  int year = utc_tm.tm_year + 1900;
  return year;
}


// This function gives the number of microseconds within the current second.
int filter_date_numerical_microseconds ()
{
  auto now = chrono::system_clock::now ();
  auto duration = now.time_since_epoch ();
  auto microseconds = chrono::duration_cast<std::chrono::microseconds>(duration).count();
  int usecs = microseconds % 1000000;
  return usecs;
}


// This function returns the seconds since the Unix epoch, which is 1 January 1970 UTC.
int filter_date_seconds_since_epoch ()
{
  auto now = chrono::system_clock::now ();
  auto duration = now.time_since_epoch ();
  int seconds = (int) chrono::duration_cast<std::chrono::seconds>(duration).count();
  return seconds;
}


// Returns the seconds since the Unix epoch for $year and $month and $day.
int filter_date_seconds_since_epoch (int year, int month, int day)
{
  int seconds = 0;
  bool done = false;
  bool hit = false;
  do {
    seconds += 86400;
    int myyear = filter_date_numerical_year (seconds);
    int mymonth = filter_date_numerical_month (seconds);
    int myday = filter_date_numerical_month_day (seconds);
    if ((year == myyear) && (month == mymonth)) hit = true;
    done = ((year == myyear) && (month == mymonth) && (day == myday));
    if (hit) if (month != mymonth) done = true;
  } while (!done);
  return seconds;
}


// This function takes the "seconds" parameter,
// corrects it according to the local timezone,
// and returns it.
int filter_date_local_seconds (int seconds)
{
  int offset = Database_Config_General::getTimezone ();
  seconds += (offset * 3600);
  return seconds;
}


bool filter_date_is_first_business_day_of_month (int monthday, int weekday)
{
  if (monthday == 1) {
    if (weekday == 1) return true;
    if (weekday == 2) return true;
    if (weekday == 3) return true;
    if (weekday == 4) return true;
    if (weekday == 5) return true;
  }
  if (weekday == 1) {
    if (monthday == 2) return true;
    if (monthday == 3) return true;
  }
  return false;
}


int filter_date_get_last_business_day_of_month (int year, int month)
{
  int myyear = year;
  int mymonth = month;
  filter_date_get_next_month (mymonth, myyear);
  int seconds = filter_date_seconds_since_epoch (myyear, mymonth, 1);
  int iterations = 0;
  do {
    seconds -= 86400;
    int weekday = filter_date_numerical_week_day (seconds);
    if ((weekday == 1) || (weekday == 2) || (weekday == 3) || (weekday == 4) || (weekday == 5)) {
      return filter_date_numerical_month_day (seconds);
    }
    iterations++;
  } while (iterations < 10);
  return 28;
}


bool filter_date_is_business_day (int year, int month, int day)
{
  int seconds = filter_date_seconds_since_epoch (year, month, day);
  int weekday = filter_date_numerical_week_day (seconds);
  if ((weekday == 1) || (weekday == 2) || (weekday == 3) || (weekday == 4) || (weekday == 5)) {
    return true;
  }
  return false;
}


void filter_date_get_previous_month (int & month, int & year)
{
  month--;
  if (month <= 0) {
    month = 12;
    year--;
  }
}


void filter_date_get_next_month (int & month, int & year)
{
  month++;
  if (month > 12) {
    month = 1;
    year++;
  }
}


string filter_date_day_rfc822 (int day)
{
  if (day == 0) return "Sun";
  if (day == 1) return "Mon";
  if (day == 2) return "Tue";
  if (day == 3) return "Wed";
  if (day == 4) return "Thu";
  if (day == 5) return "Fri";
  if (day == 6) return "Sat";
  return "";
}


string filter_date_month_rfc822 (int month)
{
  if (month ==  0) return "Jan";
  if (month ==  1) return "Feb";
  if (month ==  2) return "Mar";
  if (month ==  3) return "Apr";
  if (month ==  4) return "May";
  if (month ==  5) return "Jun";
  if (month ==  6) return "Jul";
  if (month ==  7) return "Aug";
  if (month ==  8) return "Sep";
  if (month ==  9) return "Oct";
  if (month == 10) return "Nov";
  if (month == 11) return "Dec";
  return "";
}


// Converts the number of $seconds since the Unix epoch
// to date and time values according to RFC 822,
// e.g.: Wed, 02 Oct 2002 15:00:00 +0200.
string filter_date_rfc822 (int seconds)
{
  string rfc822;
  // The feed validator at https://validator.w3.org/feed/ says:
  // <pubDate>Wed, 18 Feb 2017 12:26:39 +0100</pubDate>
  // This feed does not validate: Incorrect day of week: Wed (2 occurrences).
  // Yet, 18 February 2017 is on a Wednesday.
  // It continues to say that:
  // If it turns out that computing the correct day of week is impractical using the software you have available, then RFC 822 permits omitting both the day of the week and the subsequent comma from the value.
  // So to work around the error in the validator, the day of the week is left out.
  // int weekday = filter_date_numerical_week_day (seconds);
  // rfc822.append (filter_date_day_rfc822 (weekday));
  // rfc822.append (", ");
  string monthday = convert_to_string (filter_date_numerical_month_day (seconds));
  rfc822.append (filter_string_fill (monthday, 2, '0'));
  rfc822.append (" ");
  int month = filter_date_numerical_month (seconds);
  rfc822.append (filter_date_month_rfc822 (month));
  rfc822.append (" ");
  int year = filter_date_numerical_year (seconds);
  rfc822.append (convert_to_string (year));
  rfc822.append (" ");
  string hour = convert_to_string (filter_date_numerical_hour (seconds));
  rfc822.append (filter_string_fill (hour, 2, '0'));
  rfc822.append (":");
  string minute = convert_to_string (filter_date_numerical_minute (seconds));
  rfc822.append (filter_string_fill (minute, 2, '0'));
  rfc822.append (":");
  string second = convert_to_string (filter_date_numerical_second (seconds));
  rfc822.append (filter_string_fill (second, 2, '0'));
  rfc822.append (" ");
  int timezone = Database_Config_General::getTimezone ();
  if (timezone >= 0) rfc822.append ("+");
  else rfc822.append ("-");
  if (timezone < 0) timezone = 0 - timezone;
  rfc822.append (filter_string_fill (convert_to_string (timezone), 2, '0'));
  rfc822.append ("00");
  return rfc822;
}
