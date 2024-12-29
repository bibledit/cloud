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


#include <collaboration/settings.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/git.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <database/config/bible.h>
#include <database/jobs.h>
#include <dialog/list.h>
#include <collaboration/link.h>
#include <tasks/logic.h>
#include <jobs/index.h>


std::string collaboration_settings_url ()
{
  return "collaboration/settings";
}


bool collaboration_settings_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::admin ());
}


std::string collaboration_settings (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Collaboration"), webserver_request);
  page = header.run ();
  Assets_View view;
  
  
  std::string object = webserver_request.query ["object"];
  view.set_variable ("object", object);

  
  if (webserver_request.post.count ("url")) {
    if (!object.empty ()) {
      std::string url = webserver_request.post["url"];
      database::config::bible::set_remote_repository_url (object, url);
      std::string source = webserver_request.post["source"];
      std::string readwrite = webserver_request.post["readwrite"];
      database::config::bible::set_read_from_git (object, readwrite == "sendreceive");
      Database_Jobs database_jobs = Database_Jobs ();
      int jobId = database_jobs.get_new_id ();
      database_jobs.set_level (jobId, Filter_Roles::admin ());
      database_jobs.set_start (jobId, collaboration_link_header ());
      tasks_logic_queue (task::link_git_repository, {object, std::to_string (jobId), source});
      redirect_browser (webserver_request, jobs_index_url () + "?id=" + std::to_string (jobId));
      return std::string();
    }
  }
  std::string url = database::config::bible::get_remote_repository_url (object);
  view.set_variable ("url", url);
  
  
  page += view.render ("collaboration", "settings");
  page += assets_page::footer ();
  return page;
}
