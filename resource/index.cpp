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


#include <resource/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <sword/logic.h>
#include <demo/logic.h>
#include <resource/external.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <config/globals.h>
#include <database/config/general.h>


std::string resource_index_url ()
{
  return "resource/index";
}


bool resource_index_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


std::string resource_index (Webserver_Request& webserver_request)
{
  const bool touch = webserver_request.session_logic ()->get_touch_enabled ();

  
  std::string page;
  Assets_Header header = Assets_Header (translate("Resources"), webserver_request);
  header.set_navigator ();
  header.set_stylesheet ();
  if (touch) header.jquery_touch_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  Assets_View view;
  
  
  std::vector <std::string> resources = webserver_request.database_config_user()->get_active_resources ();


  // If no resources are displayed, set a default selection of them.
  // If a default selection hasn't been set by an administrator, use the
  // default set from demo.
  if (resources.empty ()) {
    std::vector <std::string> default_resources = database::config::general::get_default_active_resources ();
    if (default_resources.empty ()) resources = demo_logic_default_resources ();
    else resources = default_resources;
    webserver_request.database_config_user()->set_active_resources (resources);
  }

  
  std::string resourceblock;
  for (size_t i = 1; i <= resources.size (); i++) {
    resourceblock.append ("<div id=\"line" + std::to_string (i) + "\" style=\"clear:both\">\n");
    std::string resource = resources[i - 1];
    if (!sword_logic_get_remote_module (resource).empty ()) {
      if (!sword_logic_get_installed_module (resource).empty ()) {
        resource = sword_logic_get_name (resource);
      }
    }
    resourceblock.append ("<span id=\"name" + std::to_string (i) + "\" class=\"title\">" + resource + "</span>\n");
    resourceblock.append ("<span id=\"content" + std::to_string (i) + "\" class=\"resource\"></span>\n");
    resourceblock.append ("<hr style=\"clear:both\">");
    resourceblock.append ("</div>\n");
  }
  view.set_variable ("resourceblock", resourceblock);
  
  
  size_t resource_count = resources.size ();
  const std::string& username = webserver_request.session_logic ()->get_username ();
  int window_position = config_globals_resource_window_positions [username];
  std::string script = "var resourceCount = " + std::to_string (resource_count) + ";\n"
                  "var resourceWindowPosition = " + std::to_string (window_position) + ";";
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  
  
  bool can_organize_active_resources = true;
  if (can_organize_active_resources) view.enable_zone("organize");
  
  
  page += view.render ("resource", "index");
  page += assets_page::footer ();
  return page;
}


/*

 The Digital Bible Library
 https://thedigitalbiblelibrary.org/
 DBL@BibleSocieties.org
 
 Question:
 The Digital Bible Library (https://thedigitalbiblelibrary.org/) has quite an amount of Bibles. 
 Would it be any use for translators using Bibledit to be able to access that data? 
 And would there be any way, legally, to do so?
 Answer:
 Getting direct access to the Digital Bible Library is hard, 
 and only open to organizations, not individuals.
 However, I got access as eBible.org. 
 Whatever is there that I can legally get and post for you to use 
 can be found at http://eBible.org/Scriptures/.
 Conslusion:
 It is useful what eBible.org already can get and have posted on eBible.org. 
 Isn't these the Bibles that also have SWORD modules made from them on the same website? 
 If that's the case, then Bibledit is already providing access to them via those modules.

*/
