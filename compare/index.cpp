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


#include <compare/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <tasks/logic.h>
#include <database/jobs.h>
#include <database/usfmresources.h>
#include <jobs/index.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <bible/manage.h>


string compare_index_url ()
{
  return "compare/index";
}


bool compare_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string compare_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Compare"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  string bible = request->query ["bible"];
  view.set_variable ("bible", bible);
  
  if (request->query.count ("compare")) {
    string compare = request->query ["compare"];
    Database_Jobs database_jobs = Database_Jobs ();
    int jobId = database_jobs.getNewId ();
    database_jobs.setLevel (jobId, Filter_Roles::consultant ());
    tasks_logic_queue (COMPAREUSFM, {bible, compare, convert_to_string (jobId)});
    redirect_browser (request, jobs_index_url () + "?id=" + convert_to_string (jobId));
    return "";
  }

  // Names of the Bibles and the USFM Resources.
  vector <string> names;
  
  names = request->database_bibles ()->getBibles ();

  Database_UsfmResources database_usfmresources;
  vector <string> usfm_resources = database_usfmresources.getResources ();
  names.insert (names.end (), usfm_resources.begin(), usfm_resources.end ());

  sort (names.begin (), names.end ());
  
  names = filter_string_array_diff (names, {bible});
  string bibleblock;
  for (auto & name : names) {
    bibleblock.append ("<li><a href=\"index?bible=" + bible + "&compare=" + name + "\">" + name + "</a></li>\n");
  }
  view.set_variable ("bibleblock", bibleblock);

  page += view.render ("compare", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}
