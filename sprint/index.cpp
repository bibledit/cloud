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


#include <sprint/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <filter/date.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/sprint.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <access/bible.h>
#include <dialog/select.h>
#include <sprint/burndown.h>
#include <menu/logic.h>
#include <email/send.h>


std::string sprint_index_url ()
{
  return "sprint/index";
}


bool sprint_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string sprint_index ([[maybe_unused]] Webserver_Request& webserver_request)
{
#ifdef HAVE_CLIENT
  return std::string();
#endif

#ifdef HAVE_CLOUD
  
  Database_Sprint database_sprint;

  
  std::string page;
  Assets_Header header = Assets_Header (translate("Sprint"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  if (webserver_request.query.count ("previoussprint")) {
    int month = webserver_request.database_config_user()->get_sprint_month ();
    int year = webserver_request.database_config_user()->get_sprint_year ();
    filter::date::get_previous_month (month, year);
    webserver_request.database_config_user()->set_sprint_month (month);
    webserver_request.database_config_user()->set_sprint_year (year);
  }
  
  
  if (webserver_request.query.count ("currentprint")) {
    webserver_request.database_config_user()->set_sprint_month (filter::date::numerical_month (filter::date::seconds_since_epoch ()));
    webserver_request.database_config_user()->set_sprint_year (filter::date::numerical_year (filter::date::seconds_since_epoch ()));
  }
  
  
  if (webserver_request.query.count ("nextsprint")) {
    int month = webserver_request.database_config_user()->get_sprint_month ();
    int year = webserver_request.database_config_user()->get_sprint_year ();
    filter::date::get_next_month (month, year);
    webserver_request.database_config_user()->set_sprint_month (month);
    webserver_request.database_config_user()->set_sprint_year (year);
  }
  
  
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible ());
  int month = webserver_request.database_config_user()->get_sprint_month ();
  int year = webserver_request.database_config_user()->get_sprint_year ();
  

  if (webserver_request.post_count("id")) {
    std::string id = webserver_request.post_get("id");
    std::string checked = webserver_request.post_get("checked");
    if (id.length () >= 9) {
      // Remove "task".
      id.erase (0, 4);
      // Convert the fragment to an integer.
      int identifier = filter::strings::convert_to_int (id);
      // Find the fragment "box".
      size_t pos = id.find ("box");
      if (pos != std::string::npos) {
        // Remove the fragment "box".
        id.erase (0, pos + 3);
        // Convert the box to an integer.
        int box = filter::strings::convert_to_int (id);
        std::string categorytext = database::config::bible::get_sprint_task_completion_categories (bible);
        std::vector <std::string> categories = filter::strings::explode (categorytext, '\n');
        size_t category_count = categories.size ();
        float category_percentage = 100.0f / static_cast<float>(category_count);
        int percentage {0};
        bool on = (checked == "true");
        if (on) percentage = static_cast <int> (round (static_cast<float>(box + 1) * category_percentage));
        else percentage = static_cast <int> (round (static_cast<float>(box) * category_percentage));
        database_sprint.updateComplete (identifier, percentage);
      }
    }
    return std::string();
  }
  
  
  if (webserver_request.post_count("add")) {
    std::string title = webserver_request.post_get("add");
    database_sprint.storeTask (bible, year, month, title);
    view.set_variable ("success", translate("New task added"));
    // Focus the entry for adding tasks only in case a new task was added.
    view.set_variable ("autofocus", "autofocus");
  }
  
  
  if (webserver_request.query.count ("mail")) {
    int mail_year = webserver_request.database_config_user()->get_sprint_year ();
    int mail_month = webserver_request.database_config_user()->get_sprint_month ();
    sprint_burndown (bible, mail_year, mail_month);
    view.set_variable ("success", translate("The information was mailed to the subscribers"));
  }
  
  
  bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->get_bible ());
  {
    constexpr const char* identification {"bible"};
    if (webserver_request.post_count(identification)) {
      bible = webserver_request.post_get(identification);
      webserver_request.database_config_user()->set_bible (bible);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = access_bible::bibles (webserver_request),
      .selected = bible,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }

 
  int id = filter::strings::convert_to_int (webserver_request.query ["id"]);
  
  
  if (webserver_request.query.count ("remove")) {
    database_sprint.deleteTask (id);
    view.set_variable ("success", translate("The task was removed"));
  }
  
  
  if (webserver_request.query.count ("moveback")) {
    filter::date::get_previous_month (month, year);
    database_sprint.updateMonthYear (id, month, year);
    view.set_variable ("success", translate("The task was moved to the previous sprint"));
    webserver_request.database_config_user()->set_sprint_month (month);
    webserver_request.database_config_user()->set_sprint_year (year);
  }
                        
                        
  if (webserver_request.query.count ("moveforward")) {
    filter::date::get_next_month (month, year);
    database_sprint.updateMonthYear (id, month, year);
    view.set_variable ("success", translate("The task was moved to the next sprint"));
    webserver_request.database_config_user()->set_sprint_month (month);
    webserver_request.database_config_user()->set_sprint_year (year);
  }

  
  if (webserver_request.post_count("categories")) {
    std::string categories = webserver_request.post_get("categories");
    std::vector <std::string> categories2;
    categories = filter::strings::trim (categories);
    std::vector <std::string> vcategories = filter::strings::explode (categories, '\n');
    for (auto category : vcategories) {
      category = filter::strings::trim (category);
      if (category != "") categories2.push_back (category);
    }
    categories = filter::strings::implode (categories2, "\n");
    database::config::bible::set_sprint_task_completion_categories (bible, categories);
  }
  
  
  view.set_variable ("sprint", locale_logic_month (month) + " " + std::to_string (year));

  
  std::string categorytext = database::config::bible::get_sprint_task_completion_categories (bible);
  view.set_variable ("categorytext", categorytext);
  std::vector <std::string> vcategories = filter::strings::explode (categorytext, '\n');
  std::string categories;
  for (auto category : vcategories) {
    categories.append ("<td>" + category + "</td>\n");
  }
  view.set_variable ("categories", categories);
  
  
  std::string tasks;
  std::vector <int> v_tasks = database_sprint.getTasks (bible, year, month);
  for (auto & task_id : v_tasks) {
    std::string title = filter::strings::escape_special_xml_characters (database_sprint.getTitle (task_id));
    int percentage = database_sprint.getComplete (task_id);
    tasks.append ("<tr id=\"a" + std::to_string (task_id) + "\">\n");
    tasks.append ("<td><a href=\"?id=" + std::to_string (task_id) + "&remove=\">" + filter::strings::emoji_wastebasket () + "</a></td>\n");
    tasks.append ("<td></td>\n");
    tasks.append ("<td><a href=\"?id=" + std::to_string (task_id) + "&moveback=\"> « </a></td>\n");
    tasks.append ("<td>" + title + "</td>\n");
    size_t category_count = vcategories.size();
    float category_percentage = 100.0f / static_cast<float>(category_count);
    for (size_t i2 = 0; i2 < vcategories.size (); i2++) {
      int high = static_cast <int> (round (static_cast<float>(i2 + 1) * category_percentage));
      tasks.append ("<td>\n");
      tasks.append ("<input type=\"checkbox\" id=\"task");
      tasks.append (std::to_string (task_id));
      tasks.append ("box");
      tasks.append (std::to_string (i2));
      tasks.append ("\"");
      if (percentage >= high)
        tasks.append (" checked");
      else
        tasks.append ("");
      tasks.append (">");
      
      tasks.append ("</td>\n");
    }
    tasks.append ("<td><a href=\"?id=" + std::to_string (task_id) + "&moveforward=\"> » </a></td>\n");
    tasks.append ("</tr>\n");
  }
  view.set_variable ("tasks", tasks);

  
  view.set_variable ("chart", sprint_create_burndown_chart (bible, year, month));
  
  
  view.set_variable ("mailer", email::setup_information (true, false));


  page += view.render ("sprint", "index");
  page += assets_page::footer ();
  return page;
#endif
}
