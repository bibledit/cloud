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


#include <resource/studylight.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <tasks/logic.h>
#include <journal/index.h>
#include <menu/logic.h>
#include <pugixml/pugixml.hpp>


using namespace pugi;


string resource_studylight_url ()
{
  return "resource/studylight";
}


bool resource_studylight_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string resource_studylight (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate("Resources"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;

  
  if (request->query.count ("refresh")) {
    string error = resource_logic_study_light_module_list_refresh ();
    if (error.empty ()) {
      view.set_variable ("success", translate ("The list was updated"));
    }
    view.set_variable ("error", error);
  }
  
  
  string path = resource_logic_study_light_module_list_path ();
  string moduleblock = filter_url_file_get_contents (path);
  vector <string> lines = filter_string_explode (moduleblock, '\n');
  moduleblock.clear ();
  for (auto line : lines) {
    moduleblock.append ("<p>");
    moduleblock.append (line);
    moduleblock.append ("</p>\n");
  }
  view.set_variable ("moduleblock", moduleblock);

  
  page += view.render ("resource", "studylight");
  page += Assets_Page::footer ();
  return page;
}
