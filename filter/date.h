/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#ifndef INCLUDED_FILTER_DATE_H
#define INCLUDED_FILTER_DATE_H


#include <config/libraries.h>


int filter_date_numerical_second (int seconds);
int filter_date_numerical_minute (int seconds);
int filter_date_numerical_hour (int seconds);
int filter_date_numerical_month_day (int seconds);
int filter_date_numerical_week_day (int seconds);
int filter_date_numerical_month (int seconds);
int filter_date_numerical_year (int seconds);
int filter_date_numerical_microseconds ();
int filter_date_seconds_since_epoch ();
int filter_date_seconds_since_epoch (int year, int month, int day);
int filter_date_local_seconds (int seconds);
bool filter_date_is_first_business_day_of_month (int monthday, int weekday);
int filter_date_get_last_business_day_of_month (int year, int month);
bool filter_date_is_business_day (int year, int month, int day);
void filter_date_get_previous_month (int & month, int & year);
void filter_date_get_next_month (int & month, int & year);
string filter_date_day_rfc822 (int day);
string filter_date_month_rfc822 (int month);
string filter_date_rfc822 (int seconds);


#endif
