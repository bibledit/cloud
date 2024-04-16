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


#include <changes/manage.h>
#include <assets/header.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <database/modifications.h>
#include <database/notes.h>
#include <database/jobs.h>
#include <trash/handler.h>
#include <ipc/focus.h>
#include <access/user.h>
#include <changes/logic.h>
#include <menu/logic.h>
#include <tasks/logic.h>
#include <jobs/index.h>


std::string changes_manage_url ()
{
  return "changes/manage";
}


bool changes_manage_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string changes_manage (Webserver_Request& webserver_request)
{
  Database_Modifications database_modifications {};
  
  
  std::string page {};
  Assets_Header header = Assets_Header (translate("Changes"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view {};
  
  
  if (webserver_request.query.count("clear")) {
    const std::string username = webserver_request.query["clear"];
    // This may take time in case there are many change notifications to clear.
    // If there's 2000+ notifications, it takes a considerable time.
    // For that reason, it starts a background job to clear the change notifications.
    // The app will remain responsive to the user.
    Database_Jobs database_jobs {};
    const int jobId = database_jobs.get_new_id ();
    database_jobs.set_level (jobId, Filter_Roles::manager ());
    database_jobs.set_start (jobId, translate ("Clearing change notifications."));
    tasks_logic_queue (DELETECHANGES, {filter::strings::convert_to_string (jobId), username});
    redirect_browser (webserver_request, jobs_index_url () + "?id=" + filter::strings::convert_to_string (jobId));
    return std::string();
  }
  
  
  if (webserver_request.query.count("generate")) {
    changes_logic_start ();
    view.set_variable ("success", translate ("Will generate lists of changes"));
  }
  
  
  bool notifications {false};
  std::vector <std::string> users = access_user::assignees (webserver_request);
  for (const auto& user : users) {
    std::string any_bible {};
    const std::vector <int> ids = database_modifications.getNotificationIdentifiers (user, any_bible);
    if (!ids.empty ()) {
      notifications = true;
      std::map <std::string, std::string> values {};
      values ["user"] = user;
      values ["count"] = filter::strings::convert_to_string (ids.size ());
      view.add_iteration ("notifications", values);
    }
  }
  if (notifications) view.enable_zone ("notifications");

  
  view.set_variable ("interlinks", changes_interlinks (webserver_request, changes_manage_url ()));

  
  page += view.render ("changes", "manage");
  page += assets_page::footer ();
  return page;
}
