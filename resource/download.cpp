/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <resource/download.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/usfmresources.h>
#include <database/logs.h>
#include <database/cache.h>
#include <database/config/general.h>
#include <tasks/logic.h>
#include <journal/index.h>
#include <menu/logic.h>
#include <resource/cache.h>
#include <client/logic.h>


std::string resource_download_url ()
{
  return "resource/download";
}


bool resource_download_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::member);
}


std::string resource_download (Webserver_Request& webserver_request)
{
  Database_UsfmResources database_usfmresources;
  
  
  std::string page;
  Assets_Header header = Assets_Header (translate("Download resource"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (resource_cache_url (), menu_logic_resources_text ());
  page = header.run ();
  Assets_View view;

  
  std::string name = webserver_request.query["name"];
  view.set_variable ("name", name);

  
  if (webserver_request.query.count ("clear")) {
    // The client clears the installed resource.
    database::cache::sql::remove (name);
  }
  
  
  if (webserver_request.query.count ("download")) {
    // Trigger caching the resource.
    // Add the resource to the general configuration to be cached, if it is not already there.
    std::vector <std::string> resources = database::config::general::get_resources_to_cache ();
    if (!filter::string::in_array (name, resources)) {
      resources.push_back (name);
      database::config::general::set_resources_to_cache (resources);
    }
    tasks_logic_queue (task::sync_resources);
    redirect_browser (webserver_request, journal_index_url ());
    return std::string();
  }
  
  
  int count = 0;
  if (count == 0) {
    std::vector <int> books = database_usfmresources.getBooks (name);
    for (auto book : books) {
      std::vector <int> chapters = database_usfmresources.getChapters (name, book);
      // Rough average of 20 verses per chapter.
      count += static_cast<int>(20 * chapters.size());
    }
  }
  if (count == 0) 
    count = database::cache::sql::count (name);
  view.set_variable ("count", std::to_string (count));
  
  
  if (resource_logic_can_cache (name)) {
    bool cache = !filter::string::in_array(name, client_logic_no_cache_resources_get ());
    if (cache) view.enable_zone ("installable");
    else view.enable_zone("nocache");
  } else {
    view.enable_zone ("embedded");
  }
  
  
  page += view.render ("resource", "download");
  page += assets_page::footer ();
  return page;
}
