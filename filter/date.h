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


#pragma once

#include <config/libraries.h>

class Webserver_Request;

namespace filter::date {

int numerical_second (int seconds);
int numerical_minute (int seconds);
int numerical_hour (int seconds);
int numerical_month_day (int seconds);
int numerical_week_day (int seconds);
int numerical_month (int seconds);
int numerical_year (int seconds);
int numerical_microseconds ();
int seconds_since_epoch ();
int seconds_since_epoch (int year, int month, int day);
int local_seconds (int seconds);
bool is_first_business_day_of_month (int monthday, int weekday);
int get_last_business_day_of_month (int year, int month);
bool is_business_day (int year, int month, int day);
void get_previous_month (int & month, int & year);
void get_next_month (int & month, int & year);
std::string day_rfc822 (int day);
std::string month_rfc822 (int month);
std::string rfc822 (int seconds);
long elapsed_microseconds (long start);
std::string localized_date_format ();

enum date_format {
  dd_mm_yyyy = 0,
  mm_dd_yyyy = 1,
  yyyy_mn_dd = 2
};
std::string date_format_to_text (date_format format);
std::string localized_date_format (Webserver_Request& webserver_request);

}
