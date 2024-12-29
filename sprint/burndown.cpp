/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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
void sprint_burndown ([[maybe_unused]] std::string bible,
                      [[maybe_unused]] int manualyear,
                      [[maybe_unused]] int manualmonth)
{
#ifdef HAVE_CLOUD
  int localseconds = filter::date::local_seconds (filter::date::seconds_since_epoch ());
  int year = filter::date::numerical_year (localseconds);
  int month = filter::date::numerical_month (localseconds);
  int monthday = filter::date::numerical_month_day (localseconds); // 1 to 31.
  int weekday = filter::date::numerical_week_day (localseconds); // 0 (for Sunday) through 6 (for Saturday).
  int hour = filter::date::numerical_hour (localseconds);
  bool sprintstart = false;
  bool sprintfinish = false;
  bool email = false;
  
  if (manualyear) email = true;

  // Every Friday at 2 PM (14:00h) it sends email about the sprint progress.
  if ((weekday == 5) && (hour == 14)) email = true;
  // On the first business day of the month, at 10 AM, send email about the start of the sprint.
  if (filter::date::is_first_business_day_of_month (monthday, weekday) && (hour == 10)) {
    email = true;
    sprintstart = true;
  }
  // On the last business day of the month, at 2 PM (14:00h), send email about the end of the sprint.
  if ((monthday == filter::date::get_last_business_day_of_month (year, month)) && (hour == 14)) {
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
  year = filter::date::numerical_year (localseconds);
  month = filter::date::numerical_month (localseconds);
  monthday = filter::date::numerical_month_day (localseconds); // 1 to 31.
  
  
  // It sending the planning manually, update the date.
  if (manualyear) {
    year = manualyear;
    month = manualmonth;
    monthday = 0;
  }
  
  
  std::vector <std::string> bibles = {bible};
  if (bible.empty()) {
    bibles = database::bibles::get_bibles ();
  }
  

  for (auto bible2 : bibles) {

    
    // Get the total number of tasks for this sprint,
    // and the average percentage of completion of them,
    // and store this information in the sprint history table.
    std::vector <int> ids = database_sprint.getTasks (bible2, year, month);
    std::vector <int> percentages;
    for (auto id : ids) {
      percentages.push_back (database_sprint.getComplete (id));
    }
    int task_count = static_cast<int>(ids.size ());
    int complete = 0;
    if (task_count != 0) {
      for (auto percentage : percentages) complete += percentage;
      complete = static_cast<int> (round (static_cast<float>(complete) / static_cast<float>(task_count)));
    }
    database_sprint.logHistory (bible2, year, month, monthday, task_count, complete);
    
    
    // Send email if requested.
    if (email) {
      if (task_count) {
        // Only mail if the current sprint contains tasks.
        std::string scategories = database::config::bible::get_sprint_task_completion_categories (bible2);
        std::vector <std::string> categories = filter::strings::explode (scategories, '\n');
        int category_count = static_cast<int>(categories.size());
        int category_percentage = static_cast<int>(round (100 / category_count));
        std::vector <std::string> users = request.database_users ()->get_users ();
        for (auto user : users) {
          if (request.database_config_user()->getUserSprintProgressNotification (user)) {
            
            std::string subject = translate("Team's progress in Sprint");
            if (sprintstart) subject = translate("Sprint has started");
            if (sprintfinish) subject = translate("Sprint has finished");
            subject +=  " | " + bible2;
            
            std::vector <std::string> body;
            
            body.push_back ("<h4>" + bible2 + "</h4>");
            body.push_back ("<h4>" + locale_logic_month (month) + "</h4>");
            body.push_back ("<h4>" + translate("Sprint Planning and Team's Progress") + "</h4>");
            body.push_back ("<table>");
            std::vector <int> tasks = database_sprint.getTasks (bible2, year, month);
            for (auto id : tasks) {
              body.push_back ("<tr>");
              std::string title = database_sprint.getTitle (id);
              body.push_back ("<td>" + title + "</td>");
              int complete_cnt = database_sprint.getComplete (id);
              std::string text;
              for (int i = 0; i < round (complete_cnt / category_percentage); i++) text.append ("▓");
              for (int i = 0; i < category_count - round (complete_cnt / category_percentage); i++) text.append ("▁");
              body.push_back ("<td>" + text + "</td>");
              body.push_back ("</tr>");
            }
            body.push_back ("</table>");
            
            body.push_back ("<h4>" + translate("Sprint Burndown Chart - Remaining Tasks") + "</h4>");
            std::string burndownchart = sprint_create_burndown_chart (bible2, year, month);
            body.push_back ("<p>" + burndownchart + "</p>");
            
            if (!body.empty ()) {
              std::string mailbody = filter::strings::implode (body, "\n");
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
std::string sprint_create_burndown_chart ([[maybe_unused]] std::string bible,
                                          [[maybe_unused]] int year,
                                          [[maybe_unused]] int month)
{
#ifdef HAVE_CLIENT
  return std::string();
#endif
  
#ifdef HAVE_CLOUD
  
  // Get the seconds for the first of the month.
  int seconds = filter::date::seconds_since_epoch (year, month, 1);
  
  // The business days in the month for on the X-axis.
  std::vector <int> days_in_month;
  for (int day = 1; day <= 31; day++) {
    int mymonth = filter::date::numerical_month (seconds);
    if (mymonth == month) {
      if (filter::date::is_business_day (year, month, day)) {
        days_in_month.push_back (day);
      }
    }
    seconds += 86400;
  }
  
  // Assemble history of this sprint.
  Database_Sprint database_sprint = Database_Sprint ();
  std::vector <Database_Sprint_Item> history = database_sprint.getHistory (bible, year, month);
  std::map <int, int> data;
  for (auto day : days_in_month) {
    data [day] = 0;
    for (auto item : history) {
      if (day == item.day) {
        int tasks = item.tasks;
        int complete = item.complete;
        tasks = static_cast <int> (round (tasks * (100 - complete) / 100));
        data [day] = tasks;
      }
    }
  }
  
  std::vector <std::string> lines;
  lines.push_back ("<table class='burndown'>");
  lines.push_back ("<tr>");
  for (auto element : data) {
    int tasks = element.second;
    std::string text;
    for (int i = 0; i < tasks; i++) text.append ("▓<br>");
    lines.push_back ("<td  style=\"vertical-align: bottom;\" class='day'>" + text + "</td>");
  }
  lines.push_back ("</tr>");
  
  // Write number of days along the X-axis.
  lines.push_back ("<tr>");
  for (auto element : data) {
    int day = element.first;
    lines.push_back ("<td class='day'>" + std::to_string (day) + "</td>");
  }
  lines.push_back ("</tr>");
                                      
  // Write "days" below the X-axis.
  lines.push_back ("<tr>");
  int columncount = static_cast<int>(data.size ());
  std::string text = translate("days");
  lines.push_back ("<td colspan=\"" + std::to_string (columncount) + "\">" + text + "</td>");
  lines.push_back ("</tr>");
                                    
  lines.push_back ("</table>");
                                                                      
  std::string chart = filter::strings::implode (lines, "\n");
  return chart;
#endif
}

