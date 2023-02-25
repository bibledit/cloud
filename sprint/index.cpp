/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
#include <dialog/list.h>
#include <sprint/burndown.h>
#include <menu/logic.h>
#include <email/send.h>
using namespace std;


string sprint_index_url ()
{
  return "sprint/index";
}


bool sprint_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string sprint_index ([[maybe_unused]] void * webserver_request)
{
#ifdef HAVE_CLIENT
  return string();
#endif

#ifdef HAVE_CLOUD
  
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Sprint database_sprint;

  
  string page;
  Assets_Header header = Assets_Header (translate("Sprint"), request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  if (request->query.count ("previoussprint")) {
    int month = request->database_config_user()->getSprintMonth ();
    int year = request->database_config_user()->getSprintYear ();
    filter::date::get_previous_month (month, year);
    request->database_config_user()->setSprintMonth (month);
    request->database_config_user()->setSprintYear (year);
  }
  
  
  if (request->query.count ("currentprint")) {
    request->database_config_user()->setSprintMonth (filter::date::numerical_month (filter::date::seconds_since_epoch ()));
    request->database_config_user()->setSprintYear (filter::date::numerical_year (filter::date::seconds_since_epoch ()));
  }
  
  
  if (request->query.count ("nextsprint")) {
    int month = request->database_config_user()->getSprintMonth ();
    int year = request->database_config_user()->getSprintYear ();
    filter::date::get_next_month (month, year);
    request->database_config_user()->setSprintMonth (month);
    request->database_config_user()->setSprintYear (year);
  }
  
  
  string bible = access_bible::clamp (webserver_request, request->database_config_user()->getBible ());
  int month = request->database_config_user()->getSprintMonth ();
  int year = request->database_config_user()->getSprintYear ();
  

  if (request->post.count ("id")) {
    string id = request->post ["id"];
    string checked = request->post ["checked"];
    if (id.length () >= 9) {
      // Remove "task".
      id.erase (0, 4);
      // Convert the fragment to an integer.
      int identifier = convert_to_int (id);
      // Find the fragment "box".
      size_t pos = id.find ("box");
      if (pos != string::npos) {
        // Remove the fragment "box".
        id.erase (0, pos + 3);
        // Convert the box to an integer.
        int box = convert_to_int (id);
        string categorytext = Database_Config_Bible::getSprintTaskCompletionCategories (bible);
        vector <string> categories = filter_string_explode (categorytext, '\n');
        size_t category_count = categories.size ();
        float category_percentage = 100.0f / static_cast<float>(category_count);
        int percentage {0};
        bool on = (checked == "true");
        if (on) percentage = static_cast <int> (round (static_cast<float>(box + 1) * category_percentage));
        else percentage = static_cast <int> (round (static_cast<float>(box) * category_percentage));
        database_sprint.updateComplete (identifier, percentage);
      }
    }
    return "";
  }
  
  
  if (request->post.count ("add")) {
    string title = request->post ["add"];
    database_sprint.storeTask (bible, year, month, title);
    view.set_variable ("success", translate("New task added"));
    // Focus the entry for adding tasks only in case a new task was added.
    view.set_variable ("autofocus", "autofocus");
  }
  
  
  if (request->query.count ("mail")) {
    int mail_year = request->database_config_user()->getSprintYear ();
    int mail_month = request->database_config_user()->getSprintMonth ();
    sprint_burndown (bible, mail_year, mail_month);
    view.set_variable ("success", translate("The information was mailed to the subscribers"));
  }
  
  
  if (request->query.count ("bible")) {
    bible = request->query ["bible"];
    if (bible.empty()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to display the Sprint for"), "", "");
      vector <string> bibles = access_bible::bibles (request);
      for (auto & selection_bible : bibles) {
        dialog_list.add_row (selection_bible, "bible", selection_bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (bible);
    }
  }
  
  
  bible = access_bible::clamp (webserver_request, request->database_config_user()->getBible ());
  
  
  int id = convert_to_int (request->query ["id"]);
  
  
  if (request->query.count ("remove")) {
    database_sprint.deleteTask (id);
    view.set_variable ("success", translate("The task was removed"));
  }
  
  
  if (request->query.count ("moveback")) {
    filter::date::get_previous_month (month, year);
    database_sprint.updateMonthYear (id, month, year);
    view.set_variable ("success", translate("The task was moved to the previous sprint"));
    request->database_config_user()->setSprintMonth (month);
    request->database_config_user()->setSprintYear (year);
  }
                        
                        
  if (request->query.count ("moveforward")) {
    filter::date::get_next_month (month, year);
    database_sprint.updateMonthYear (id, month, year);
    view.set_variable ("success", translate("The task was moved to the next sprint"));
    request->database_config_user()->setSprintMonth (month);
    request->database_config_user()->setSprintYear (year);
  }

  
  if (request->post.count ("categories")) {
    string categories = request->post ["categories"];
    vector <string> categories2;
    categories = filter_string_trim (categories);
    vector <string> vcategories = filter_string_explode (categories, '\n');
    for (auto category : vcategories) {
      category = filter_string_trim (category);
      if (category != "") categories2.push_back (category);
    }
    categories = filter_string_implode (categories2, "\n");
    Database_Config_Bible::setSprintTaskCompletionCategories (bible, categories);
  }
  
  
  view.set_variable ("bible", bible);
  view.set_variable ("sprint", locale_logic_month (month) + " " + convert_to_string (year));

  
  string categorytext = Database_Config_Bible::getSprintTaskCompletionCategories (bible);
  view.set_variable ("categorytext", categorytext);
  vector <string> vcategories = filter_string_explode (categorytext, '\n');
  string categories;
  for (auto category : vcategories) {
    categories.append ("<td>" + category + "</td>\n");
  }
  view.set_variable ("categories", categories);
  
  
  string tasks;
  vector <int> v_tasks = database_sprint.getTasks (bible, year, month);
  for (auto & task_id : v_tasks) {
    string title = escape_special_xml_characters (database_sprint.getTitle (task_id));
    int percentage = database_sprint.getComplete (task_id);
    tasks.append ("<tr id=\"a" + convert_to_string (task_id) + "\">\n");
    tasks.append ("<td><a href=\"?id=" + convert_to_string (task_id) + "&remove=\">" + emoji_wastebasket () + "</a></td>\n");
    tasks.append ("<td></td>\n");
    tasks.append ("<td><a href=\"?id=" + convert_to_string (task_id) + "&moveback=\"> « </a></td>\n");
    tasks.append ("<td>" + title + "</td>\n");
    size_t category_count = vcategories.size();
    float category_percentage = 100.0f / static_cast<float>(category_count);
    for (size_t i2 = 0; i2 < vcategories.size (); i2++) {
      int high = static_cast <int> (round (static_cast<float>(i2 + 1) * category_percentage));
      tasks.append ("<td>\n");
      tasks.append ("<input type=\"checkbox\" id=\"task");
      tasks.append (convert_to_string (task_id));
      tasks.append ("box");
      tasks.append (convert_to_string (i2));
      tasks.append ("\"");
      if (percentage >= high)
        tasks.append (" checked");
      else
        tasks.append ("");
      tasks.append (">");
      
      tasks.append ("</td>\n");
    }
    tasks.append ("<td><a href=\"?id=" + convert_to_string (task_id) + "&moveforward=\"> » </a></td>\n");
    tasks.append ("</tr>\n");
  }
  view.set_variable ("tasks", tasks);

  
  view.set_variable ("chart", sprint_create_burndown_chart (bible, year, month));
  
  
  view.set_variable ("mailer", email_setup_information (true, false));


  page += view.render ("sprint", "index");
  page += assets_page::footer ();
  return page;
#endif
}
