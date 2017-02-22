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


string consistency_index_url ()
{
  return "consistency/index";
}


bool consistency_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string consistency_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate("Consistency"), webserver_request);
  header.addBreadCrumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;

  
  string add = request->post ["add"];
  if (!add.empty ()) {
    vector <string> resources = request->database_config_user()->getConsistencyResources ();
    resources.push_back (add);
    request->database_config_user()->setConsistencyResources (resources);
  }
  
  
  string remove = request->query ["remove"];
  if (!remove.empty ()) {
    vector <string> resources = request->database_config_user()->getConsistencyResources ();
    resources = filter_string_array_diff (resources, {remove});
    request->database_config_user()->setConsistencyResources (resources);
  }
  
  
  string resourceblock;
  vector <string> resources = request->database_config_user()->getConsistencyResources ();
  for (auto resource : resources) {
    resourceblock.append (resource);
    resourceblock.append ("\n");
    resourceblock.append ("<a href=\"?remove=" + resource + "\">[" + translate("remove") + "]</a>");
    resourceblock.append (" | ");
  }
  view.set_variable ("resourceblock", resourceblock);

  
  page += view.render ("consistency", "index");
  page += Assets_Page::footer ();
  return page;
}
