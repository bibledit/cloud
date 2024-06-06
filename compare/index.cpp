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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


std::string compare_index_url ()
{
  return "compare/index";
}


bool compare_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


std::string compare_index (Webserver_Request& webserver_request)
{
  std::string page{};
  
  Assets_Header header = Assets_Header (translate("Compare"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view {};
  
  const std::string bible = webserver_request.query ["bible"];
  view.set_variable ("bible", bible);
  
  if (webserver_request.query.count ("compare")) {
    const std::string compare = webserver_request.query ["compare"];
    Database_Jobs database_jobs = Database_Jobs ();
    const int job_id = database_jobs.get_new_id ();
    database_jobs.set_level (job_id, Filter_Roles::consultant ());
    tasks_logic_queue (COMPAREUSFM, {bible, compare, std::to_string (job_id)});
    redirect_browser (webserver_request, jobs_index_url () + "?id=" + std::to_string (job_id));
    return std::string();
  }

  // Names of the Bibles and the USFM Resources.
  std::vector <std::string> names = database::bibles::get_bibles ();

  Database_UsfmResources database_usfmresources;
  const std::vector <std::string> usfm_resources = database_usfmresources.getResources ();
  names.insert (names.end (), usfm_resources.begin(), usfm_resources.end ());

  sort (names.begin (), names.end ());
  
  names = filter::strings::array_diff (names, {bible});
  pugi::xml_document document;
  for (const auto& name : names) {
    pugi::xml_node li_node = document.append_child("li");
    pugi::xml_node a_node = li_node.append_child("a");
    a_node.append_attribute("href") = ("index?bible=" + bible + "&compare=" + name).c_str();
    a_node.text().set(name.c_str());
  }
  std::stringstream ss{};
  document.print(ss, "", pugi::format_raw);
  view.set_variable ("bibleblock", ss.str());

  page.append (view.render ("compare", "index"));
  
  page.append (assets_page::footer ());
  
  return page;
}
