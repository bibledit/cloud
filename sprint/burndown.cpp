/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#include <sprint/burndown.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/date.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/sprint.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <client/logic.h>
#include <email/send.h>


// This function runs the sprint burndown history logger for $bible.
// If no $bible is passed, it will do all Bibles.
// It may be passed a manual year and manual month.
// In those cases it will mail the burndown chart to the subscribed users.
// Else it decides on its own whether to mail the chart to the users.
void sprint_burndown (string bible, int manualyear, int manualmonth)
{
  (void) bible;
  (void) manualyear;
  (void) manualmonth;
#ifdef HAVE_CLOUD
  int localseconds = filter_date_local_seconds (filter_date_seconds_since_epoch ());
  int year = filter_date_numerical_year (localseconds);
  int month = filter_date_numerical_month (localseconds);
  int monthday = filter_date_numerical_month_day (localseconds); // 1 to 31.
  int weekday = filter_date_numerical_week_day (localseconds); // 0 (for Sunday) through 6 (for Saturday).
  int hour = filter_date_numerical_hour (localseconds);
  bool sprintstart = false;
  bool sprintfinish = false;
  bool email = false;
  
  if (manualyear) email = true;

  // Every Friday at 2 PM (14:00h) it sends email about the sprint progress.
  if ((weekday == 5) && (hour == 14)) email = true;
  // On the first business day of the month, at 10 AM, send email about the start of the sprint.
  if (filter_date_is_first_business_day_of_month (monthday, weekday) && (hour == 10)) {
    email = true;
    sprintstart = true;
  }
  // On the last business day of the month, at 2 PM (14:00h), send email about the end of the sprint.
  if ((monthday == filter_date_get_last_business_day_of_month (year, month)) && (hour == 14)) {
    email = true;
    sprintfinish = true;
  }
  
  
  // Determine what to do, or to quit.
  if (!email && !sprintstart && !sprintfinish) {
    if (hour != 1) return;
  }
  

  Database_Logs::log ("Updating Sprint information", Filter_Roles::manager ());

  Webserver_Request request;
  Database_Sprint database_sprint = Database_Sprint ();

  
  // Determine year / month / day of the current sprint.
  // If this script runs from midnight till early morning,
  // it applies to the day before.
  // If the script runs during the day, it applies to today.
  if (hour <= 6) {
    localseconds -= (3600 * 6);
  }
  year = filter_date_numerical_year (localseconds);
  month = filter_date_numerical_month (localseconds);
  monthday = filter_date_numerical_month_day (localseconds); // 1 to 31.
  
  
  // It sending the planning manually, update the date.
  if (manualyear) {
    year = manualyear;
    month = manualmonth;
    monthday = 0;
  }
  
  
  vector <string> bibles = {bible};
  if (bible == "") {
    bibles = request.database_bibles()->getBibles ();
  }
  

  for (auto bible : bibles) {

    
    // Get the total number of tasks for this sprint,
    // and the average percentage of completion of them,
    // and store this information in the sprint history table.
    vector <int> ids = database_sprint.getTasks (bible, year, month);
    vector <int> percentages;
    for (auto id : ids) {
      percentages.push_back (database_sprint.getComplete (id));
    }
    int tasks = static_cast<int>(ids.size ());
    int complete = 0;
    if (tasks != 0) {
      for (auto percentage : percentages) complete += percentage;
      complete = round ((float) complete / (float) tasks);
    }
    database_sprint.logHistory (bible, year, month, monthday, tasks, complete);
    
    
    // Send email if requested.
    if (email) {
      if (tasks) {
        // Only mail if the current sprint contains tasks.
        string scategories = Database_Config_Bible::getSprintTaskCompletionCategories (bible);
        vector <string> categories = filter_string_explode (scategories, '\n');
        size_t category_count = categories.size();
        int category_percentage = round (100 / category_count);
        vector <string> users = request.database_users ()->get_users ();
        for (auto user : users) {
          if (request.database_config_user()->getUserSprintProgressNotification (user)) {
            
            string subject = translate("Team's progress in Sprint");
            if (sprintstart) subject = translate("Sprint has started");
            if (sprintfinish) subject = translate("Sprint has finished");
            subject +=  " | " + bible;
            
            vector <string> body;
            
            body.push_back ("<h4>" + bible + "</h4>");
            body.push_back ("<h4>" + locale_logic_month (month) + "</h4>");
            body.push_back ("<h4>" + translate("Sprint Planning and Team's Progress") + "</h4>");
            body.push_back ("<table>");
            vector <int> tasks = database_sprint.getTasks (bible, year, month);
            for (auto id : tasks) {
              body.push_back ("<tr>");
              string title = database_sprint.getTitle (id);
              body.push_back ("<td>" + title + "</td>");
              int complete = database_sprint.getComplete (id);
              string text;
              for (int i = 0; i < round (complete / category_percentage); i++) text.append ("▓");
              for (int i = 0; i < category_count - round (complete / category_percentage); i++) text.append ("▁");
              body.push_back ("<td>" + text + "</td>");
              body.push_back ("</tr>");
            }
            body.push_back ("</table>");
            
            body.push_back ("<h4>" + translate("Sprint Burndown Chart - Remaining Tasks") + "</h4>");
            string burndownchart = sprint_create_burndown_chart (bible, year, month);
            body.push_back ("<p>" + burndownchart + "</p>");
            
            if (!body.empty ()) {
              string mailbody = filter_string_implode (body, "\n");
              email_schedule (user, subject, mailbody);
            }
            
          }
        }
      } else {
        
        // Since there are no tasks, no mail will be sent: Make a logbook entry.
        Database_Logs::log ("No tasks in this Sprint: No email was sent");
      }
    }
  }
#endif
}


// This function creates a text-based burndown chart for sprint $bible / $year / $month.
string sprint_create_burndown_chart (string bible, int year, int month)
{
#ifdef HAVE_CLIENT
  (void) bible;
  (void) year;
  (void) month;
  return "";
#endif
  
#ifdef HAVE_CLOUD
  
  // Get the seconds for the first of the month.
  int seconds = filter_date_seconds_since_epoch (year, month, 1);
  
  // The business days in the month for on the X-axis.
  vector <int> days_in_month;
  for (unsigned int day = 1; day <= 31; day++) {
    int mymonth = filter_date_numerical_month (seconds);
    if (mymonth == month) {
      if (filter_date_is_business_day (year, month, day)) {
        days_in_month.push_back (day);
      }
    }
    seconds += 86400;
  }
  
  // Assemble history of this sprint.
  Database_Sprint database_sprint = Database_Sprint ();
  vector <Database_Sprint_Item> history = database_sprint.getHistory (bible, year, month);
  map <int, int> data;
  for (auto day : days_in_month) {
    data [day] = 0;
    for (auto item : history) {
      if (day == item.day) {
        int tasks = item.tasks;
        int complete = item.complete;
        tasks = round (tasks * (100 - complete) / 100);
        data [day] = tasks;
      }
    }
  }
  
  vector <string> lines;
  lines.push_back ("<table class='burndown'>");
  lines.push_back ("<tr>");
  for (auto element : data) {
    int tasks = element.second;
    string text;
    for (int i = 0; i < tasks; i++) text.append ("▓<br>");
    lines.push_back ("<td  style=\"vertical-align: bottom;\" class='day'>" + text + "</td>");
  }
  lines.push_back ("</tr>");
  
  // Write number of days along the X-axis.
  lines.push_back ("<tr>");
  for (auto element : data) {
    int day = element.first;
    lines.push_back ("<td class='day'>" + convert_to_string (day) + "</td>");
  }
  lines.push_back ("</tr>");
                                      
  // Write "days" below the X-axis.
  lines.push_back ("<tr>");
  int columncount = static_cast<int>(data.size ());
  string text = translate("days");
  lines.push_back ("<td colspan=\"" + convert_to_string (columncount) + "\">" + text + "</td>");
  lines.push_back ("</tr>");
                                    
  lines.push_back ("</table>");
                                                                      
  string chart = filter_string_implode (lines, "\n");
  return chart;
#endif
}

