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


#include <resource/bbgateway.h>
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


std::string resource_biblegateway_url ()
{
  return "resource/bbgateway";
}


bool resource_biblegateway_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


std::string resource_biblegateway (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Resources"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;

  
  if (webserver_request.query.count ("refresh")) {
    std::string error = resource_logic_bible_gateway_module_list_refresh ();
    if (error.empty ()) {
      view.set_variable ("success", translate ("The list was updated"));
    }
    view.set_variable ("error", error);
  }
  
  
  std::string path = resource_logic_bible_gateway_module_list_path ();
  std::string moduleblock = filter_url_file_get_contents (path);
  std::vector <std::string> lines = filter::strings::explode (moduleblock, '\n');
  moduleblock.clear ();
  for (auto line : lines) {
    moduleblock.append ("<p>");
    moduleblock.append (line);
    moduleblock.append ("</p>\n");
  }
  view.set_variable ("moduleblock", moduleblock);

  
  page += view.render ("resource", "bbgateway");
  page += assets_page::footer ();
  return page;
}
