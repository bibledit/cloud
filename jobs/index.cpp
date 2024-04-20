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


#include <jobs/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <tasks/logic.h>
#include <database/jobs.h>


std::string jobs_index_url ()
{
  return "jobs/index";
}


bool jobs_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


std::string jobs_index (Webserver_Request& webserver_request)
{
  std::string page;
  
  Assets_Header header = Assets_Header (translate ("Job"), webserver_request);
  
  Assets_View view;

  const int id = filter::strings::convert_to_int (webserver_request.query ["id"]);

  // Get information about this job.
  Database_Jobs database_jobs = Database_Jobs ();
  const bool exists = database_jobs.id_exists (id);
  const int level = database_jobs.get_level (id);
  const std::string start = database_jobs.get_start (id);
  const std::string percentage = database_jobs.get_percentage (id);
  const std::string progress = database_jobs.get_progress (id);
  const std::string result = database_jobs.get_result (id);

  // Access control for the user.
  const int userlevel = webserver_request.session_logic()->currentLevel ();

  std::string contents;
  if (!exists) {
    // Check on existence of the job.
    contents = translate("This job does not exist.");
  } else if (level > userlevel) {
    // Check user access to the job.
    contents = translate("This job is not available to you.");
  } else if (!result.empty ()) {
    contents = result;
  } else if (!start.empty () | !progress.empty () | !percentage.empty ()) {
    contents = start;
    if (!percentage.empty ()) {
      view.enable_zone ("percentage");
      view.set_variable ("percentage", percentage);
    }
    if (!progress.empty ()) {
      view.enable_zone ("progress");
      view.set_variable ("progress", progress);
    }
  } else {
    contents = translate("The job is scheduled to start shortly.");
  }
  view.set_variable ("contents", contents);

  // If the result is still pending, refresh the page shortly.
  if (result.empty ()) {
    header.refresh (1);
  }
  
  header.set_editor_stylesheet ();

  page += header.run ();

  page += view.render ("jobs", "index");
  
  page += assets_page::footer ();
  
  return page;
}
