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


#include <changes/statistics.h>
#include <assets/header.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/statistics.h>


// Internal function declarations.
void changes_statistics_add (Assets_View& view, const std::string& date, int count);


std::string changes_statistics_url ()
{
  return "changes/statistics";
}


bool changes_statistics_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::consultant);
}


void changes_statistics_add (Assets_View& view, const std::string& date, int count)
{
  if (count) {
    std::map <std::string, std::string> values;
    values ["date"] = date;
    values ["count"] = std::to_string (count);
    view.add_iteration ("statistics", values);
  }
}


std::string changes_statistics ([[maybe_unused]] Webserver_Request& webserver_request)
{
#ifdef HAVE_CLIENT
  return std::string();
#endif

#ifdef HAVE_CLOUD

  std::string page {};
  Assets_Header header = Assets_Header (translate("Change statistics"), webserver_request);
  page += header.run ();
  Assets_View view {};
  
  
  const std::string everyone = translate ("Everyone");
  std::string user = webserver_request.query["user"];
  if (user == everyone) user.clear ();

  
  std::vector <std::pair <int, int>> changes = Database_Statistics::get_changes (user);
  std::string last_date {};
  int last_count {0};
  for (const auto& element : changes) {
    const std::string date = locale_logic_date (element.first);
    const int count = element.second;
    if (date == last_date) {
      last_count += count;
    } else {
      changes_statistics_add (view, last_date, last_count);
      last_date = date;
      last_count = count;
    }
  }
  if (last_count) {
    changes_statistics_add (view, last_date, last_count);
  }

  
  std::vector <std::string> users = Database_Statistics::get_users ();
  users.push_back (everyone);
  for (size_t i = 0; i < users.size (); i++) {
    std::map <std::string, std::string> values;
    if (i) values ["divider"] = "|";
    values ["user"] = users[i];
    view.add_iteration ("users", values);
  }
  
  
  if (user.empty ()) user = everyone;
  view.set_variable ("user", user);
  
  
  page += view.render ("changes", "statistics");
  
  
  page += assets_page::footer ();
  return page;
#endif
}
