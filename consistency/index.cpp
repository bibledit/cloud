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


#include <consistency/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <database/usfmresources.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <dialog/list.h>
#include <resource/logic.h>
#include <menu/logic.h>


std::string consistency_index_url ()
{
  return "consistency/index";
}


bool consistency_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string consistency_index (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Consistency"), webserver_request);
  header.add_bread_crumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;

  
  std::string add = webserver_request.post ["add"];
  if (!add.empty ()) {
    std::vector <std::string> resources = webserver_request.database_config_user()->getConsistencyResources ();
    resources.push_back (add);
    webserver_request.database_config_user()->setConsistencyResources (resources);
  }
  
  
  std::string remove = webserver_request.query ["remove"];
  if (!remove.empty ()) {
    std::vector <std::string> resources = webserver_request.database_config_user()->getConsistencyResources ();
    resources = filter::strings::array_diff (resources, {remove});
    webserver_request.database_config_user()->setConsistencyResources (resources);
  }
  
  
  std::stringstream resourceblock;
  std::vector <std::string> resources = webserver_request.database_config_user()->getConsistencyResources ();
  for (auto resource : resources) {
    resourceblock << resource;
    resourceblock << "\n";
    resourceblock << "<a href=" << std::quoted("?remove=" + resource) << ">[" << translate("remove") << "]</a>";
    resourceblock << " | ";
  }
  view.set_variable ("resourceblock", resourceblock.str());

  
  page += view.render ("consistency", "index");
  page += assets_page::footer ();
  return page;
}
