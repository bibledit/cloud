/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
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
#include <bb/manage.h>
#include <pugixml/pugixml.hpp>
#include <sstream>


using namespace pugi;


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
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
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
    int jobId = database_jobs.get_new_id ();
    database_jobs.set_level (jobId, Filter_Roles::consultant ());
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
  xml_document document;
  for (auto & name : names) {
    xml_node li_node = document.append_child("li");
    xml_node a_node = li_node.append_child("a");
    a_node.append_attribute("href") = string("index?bible=" + bible + "&compare=" + name).c_str();
    a_node.text().set(name.c_str());
  }
  stringstream ss;
  document.print(ss, "", format_raw);
  view.set_variable ("bibleblock", ss.str());

  page += view.render ("compare", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}
