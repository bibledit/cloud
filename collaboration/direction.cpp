/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <collaboration/direction.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/git.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/bible.h>
#include <database/jobs.h>
#include <dialog/list.h>
#include <collaboration/link.h>
#include <tasks/logic.h>
#include <jobs/index.h>


string collaboration_direction_url ()
{
  return "collaboration/direction";
}


bool collaboration_direction_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::admin ());
}


string collaboration_direction (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  Assets_Header header = Assets_Header (translate("Collaboration"), request);
  page = header.run ();
  Assets_View view;
  
  string object = request->query ["object"];
  view.set_variable ("object", object);
  
  string url = Database_Config_Bible::getRemoteRepositoryUrl (object);
  view.set_variable ("url", url);

  if (request->query.count ("take")) {
    string take = request->query ["take"];
    if (!object.empty ()) {
      Database_Jobs database_jobs = Database_Jobs ();
      int jobId = database_jobs.getNewId ();
      database_jobs.setLevel (jobId, Filter_Roles::admin ());
      database_jobs.setStart (jobId, collaboration_link_header ());
      tasks_logic_queue (LINKGITREPOSITORY, {object, convert_to_string (jobId), take});
      redirect_browser (request, jobs_index_url () + "?id=" + convert_to_string (jobId));
      return "";
    }
  }
  
  page += view.render ("collaboration", "direction");
  page += Assets_Page::footer ();
  return page;
}
