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


#include <resource/bible2resource.h>
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
#include <bible/manage.h>


string resource_bible2resource_url ()
{
  return "resource/bible2resource";
}


bool resource_bible2resource_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string resource_bible2resource (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();

  
  string page;
  Assets_Header header = Assets_Header (translate("Convert"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  Assets_View view;

  
  string bible = request->query["bible"];
  view.set_variable ("bible", bible);
                      
  
  vector <string> usfmResources = database_usfmresources.getResources ();
  if (find (usfmResources.begin(), usfmResources.end (), bible) != usfmResources.end ()) {
    view.set_variable ("error", translate("A USFM Resource with this name already exists"));
  }
  vector <string> externalResources = resource_external_names ();
  if (find (externalResources.begin(), externalResources.end (), bible) != externalResources.end ()) {
    view.set_variable ("error", translate("An external resource with this name already exists"));
  }
  
  
  if (request->query.count ("convert")) {
    if (access_bible_write (request, bible)) {
      tasks_logic_queue (CONVERTBIBLE2RESOURCE, {bible});
      redirect_browser (request, journal_index_url ());
      return "";
    } else {
      Assets_Page::error (translate("Insufficient privileges to complete operation."));
    }
  }

  
  page += view.render ("resource", "bible2resource");
  page += Assets_Page::footer ();
  return page;
}
