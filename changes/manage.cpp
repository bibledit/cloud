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


string changes_manage_url ()
{
  return "changes/manage";
}


bool changes_manage_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string changes_manage (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_Modifications database_modifications;
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Changes"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;
  
  
  if (request->query.count("clear")) {
    string username = request->query["clear"];
    // This may take time in case there are many change notifications to clear.
    // If there's 2000+ notifications, it takes a considerable time.
    // For that reason, it starts a background job to clear the change notifications.
    // The app will remain responsive to the user.
    Database_Jobs database_jobs = Database_Jobs ();
    int jobId = database_jobs.getNewId ();
    database_jobs.setLevel (jobId, Filter_Roles::manager ());
    database_jobs.setStart (jobId, translate ("Clearing change notifications."));
    tasks_logic_queue (DELETECHANGES, {convert_to_string (jobId), username});
    redirect_browser (request, jobs_index_url () + "?id=" + convert_to_string (jobId));
    return "";
  }
  
  
  if (request->query.count("generate")) {
    changes_logic_start ();
    view.set_variable ("success", translate ("Will generate lists of changes"));
  }
  
  
  bool notifications = false;
  vector <string> users = access_user_assignees (webserver_request);
  for (auto user : users) {
    vector <int> ids = database_modifications.getNotificationIdentifiers (user);
    if (!ids.empty ()) {
      notifications = true;
      map <string, string> values;
      values ["user"] = user;
      values ["count"] = convert_to_string (ids.size ());
      view.add_iteration ("notifications", values);
    }
  }
  if (notifications) view.enable_zone ("notifications");

  
  view.set_variable ("interlinks", changes_interlinks (webserver_request, changes_manage_url ()));

  
  page += view.render ("changes", "manage");
  page += Assets_Page::footer ();
  return page;
}
