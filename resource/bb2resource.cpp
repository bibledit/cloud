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


#include <resource/bb2resource.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/usfmresources.h>
#include <access/bible.h>
#include <tasks/logic.h>
#include <journal/index.h>
#include <resource/external.h>
#include <menu/logic.h>
#include <bb/manage.h>


std::string resource_bible2resource_url ()
{
  return "resource/bb2resource";
}


bool resource_bible2resource_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string resource_bible2resource (Webserver_Request& webserver_request)
{
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();

  
  std::string page;
  Assets_Header header = Assets_Header (translate("Convert"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  Assets_View view;

  
  const std::string bible = webserver_request.query["bible"];
  view.set_variable ("bible", bible);
                      
  
  const std::vector <std::string> usfm_resources = database_usfmresources.getResources ();
  if (find (usfm_resources.begin(), usfm_resources.end (), bible) != usfm_resources.end ()) {
    view.set_variable ("error", translate("A USFM Resource with this name already exists"));
  }
  const std::vector <std::string> external_resources = resource_external_names ();
  if (find (external_resources.begin(), external_resources.end (), bible) != external_resources.end ()) {
    view.set_variable ("error", translate("An external resource with this name already exists"));
  }
  
  
  if (webserver_request.query.count ("convert")) {
    if (access_bible::write (webserver_request, bible)) {
      tasks_logic_queue (task::convert_bible_to_resource, {bible});
      redirect_browser (webserver_request, journal_index_url ());
      return std::string();
    } else {
      assets_page::error (translate("Insufficient privileges to complete operation."));
    }
  }

  
  page += view.render ("resource", "bb2resource");
  page += assets_page::footer ();
  return page;
}
