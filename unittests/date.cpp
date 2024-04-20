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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <filter/date.h>
#include <filter/string.h>


TEST (filter, date)
{
  // Test the date and time related functions.
  {
    int month = filter::date::numerical_month (filter::date::seconds_since_epoch ());
    if ((month < 1) || (month > 12)) EXPECT_EQ ("current month", filter::strings::convert_to_string (month));
    int year = filter::date::numerical_year (filter::date::seconds_since_epoch ());
    if ((year < 2014) || (year > 2050)) EXPECT_EQ ("current year", filter::strings::convert_to_string (year));
    timeval tv;
    gettimeofday (&tv, nullptr);
    int reference_second = static_cast<int>(tv.tv_sec);
    int actual_second = filter::date::seconds_since_epoch ();
    if (abs (actual_second - reference_second) > 1) EXPECT_EQ (reference_second, actual_second);
    int usecs = filter::date::numerical_microseconds ();
    if ((usecs < 0) || (usecs > 1'000'000)) {
      EXPECT_EQ ("0-1000000", filter::strings::convert_to_string (usecs));
    }
  }
  
  // First business day of month.
  {
    // Sunday the 1st.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (1, 0));
    // Monday the 1st.
    EXPECT_EQ (true, filter::date::is_first_business_day_of_month (1, 1));
    // Tuesday the 1st.
    EXPECT_EQ (true, filter::date::is_first_business_day_of_month (1, 2));
    // Wednesday the 1st.
    EXPECT_EQ (true, filter::date::is_first_business_day_of_month (1, 3));
    // Thirsday the 1st.
    EXPECT_EQ (true, filter::date::is_first_business_day_of_month (1, 4));
    // Friday the 1st.
    EXPECT_EQ (true, filter::date::is_first_business_day_of_month (1, 5));
    // Saturday the 1st.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (1, 6));
    // Sunday the 2nd.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (2, 0));
    // Monday the 2nd.
    EXPECT_EQ (true, filter::date::is_first_business_day_of_month (2, 1));
    // Tuesday the 2nd.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (2, 2));
    // Sunday the 3nd.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (3, 0));
    // Monday the 3nd.
    EXPECT_EQ (true, filter::date::is_first_business_day_of_month (3, 1));
    // Tuesday the 3nd.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (3, 2));
    // Sunday the 4nd.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (4, 0));
    // Monday the 4nd.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (4, 1));
    // Tuesday the 4nd.
    EXPECT_EQ (false, filter::date::is_first_business_day_of_month (4, 2));
  }

  // Last business day of month.
  {
    EXPECT_EQ (30, filter::date::get_last_business_day_of_month (2013, 9));
    EXPECT_EQ (31, filter::date::get_last_business_day_of_month (2013, 10));
    EXPECT_EQ (29, filter::date::get_last_business_day_of_month (2013, 11));
    EXPECT_EQ (31, filter::date::get_last_business_day_of_month (2013, 12));
    EXPECT_EQ (31, filter::date::get_last_business_day_of_month (2014, 1));
    EXPECT_EQ (28, filter::date::get_last_business_day_of_month (2014, 2));
    EXPECT_EQ (31, filter::date::get_last_business_day_of_month (2014, 3));
    EXPECT_EQ (30, filter::date::get_last_business_day_of_month (2014, 4));
    EXPECT_EQ (30, filter::date::get_last_business_day_of_month (2014, 5));
    EXPECT_EQ (30, filter::date::get_last_business_day_of_month (2014, 6));
  }

  // Is business day.
  {
    EXPECT_EQ (false, filter::date::is_business_day (2013, 9, 1));
    EXPECT_EQ (true, filter::date::is_business_day (2013, 9, 2));
    EXPECT_EQ (true, filter::date::is_business_day (2013, 9, 3));
    EXPECT_EQ (true, filter::date::is_business_day (2013, 9, 4));
    EXPECT_EQ (true, filter::date::is_business_day (2013, 9, 5));
    EXPECT_EQ (true, filter::date::is_business_day (2013, 9, 6));
    EXPECT_EQ (false, filter::date::is_business_day (2013, 9, 7));
    EXPECT_EQ (false, filter::date::is_business_day (2013, 9, 8));
    EXPECT_EQ (true, filter::date::is_business_day (2013, 9, 30));
    EXPECT_EQ (false, filter::date::is_business_day (2015, 3, 1));
    EXPECT_EQ (false, filter::date::is_business_day (2015, 2, 32));
    
  }

  // Seconds since Unix epoch.
  {
    int year, month, day, seconds;
    
    year = 2011;
    month = 2;
    day = 5;
    seconds = filter::date::seconds_since_epoch (year, month, day);
    EXPECT_EQ (year, filter::date::numerical_year (seconds));
    EXPECT_EQ (month, filter::date::numerical_month (seconds));
    EXPECT_EQ (day, filter::date::numerical_month_day (seconds));
    
    year = 2015;
    month = 3;
    day = 15;
    seconds = filter::date::seconds_since_epoch (year, month, day);
    EXPECT_EQ (year, filter::date::numerical_year (seconds));
    EXPECT_EQ (month, filter::date::numerical_month (seconds));
    EXPECT_EQ (day, filter::date::numerical_month_day (seconds));
    
    year = 2030;
    month = 12;
    day = 31;
    seconds = filter::date::seconds_since_epoch (year, month, day);
    EXPECT_EQ (year, filter::date::numerical_year (seconds));
    EXPECT_EQ (month, filter::date::numerical_month (seconds));
    EXPECT_EQ (day, filter::date::numerical_month_day (seconds));
  }

}

#endif

