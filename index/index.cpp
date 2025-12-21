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


#include <index/index.h>
#include <assets/view.h>
#include <assets/header.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <locale/translate.h>
#include <edit/index.h>
#include <notes/index.h>
#include <resource/index.h>
#include <changes/changes.h>
#include <workspace/index.h>
#include <session/login.h>
#include <bb/logic.h>
#include <filter/webview.h>
#include <menu/logic.h>
#include <read/index.h>
#include <webserver/request.h>


const char * index_index_url ()
{
  return "index/index";
}


bool index_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::guest);
}


std::string index_index (Webserver_Request& webserver_request)
{
  filter_webview_log_user_agent (webserver_request.user_agent);
  
  Assets_Header header = Assets_Header (translate ("Bibledit"), webserver_request);

  // Basic or advanced mode setting.
  const std::string mode = webserver_request.query ["mode"];
  if (!mode.empty ()) {
    const bool basic = (mode == "basic");
    webserver_request.database_config_user ()->set_basic_interface_mode (basic);
    menu_logic_tabbed_mode_save_json (webserver_request);
  }
  
  // Upon app start, initialize the JSON for tabbed mode.
  // It should be done during the setup phase.
  // But in this case the setup phase does not provide user information.
  // Here on this page, the user information is available.
  static bool tabbed_json_initialized = false;
  if (!tabbed_json_initialized) {
    if (menu_logic_can_do_tabbed_mode ()) {
      menu_logic_tabbed_mode_save_json (webserver_request);
    }
    tabbed_json_initialized = true;
  }
  
  // Normally a page does not show the expanded main menu.
  // This is to save space on the screen.
  // But the home page of Bibledit shows the extended main menu.
  if (webserver_request.query.count ("item") == 0) {
    webserver_request.query ["item"] = "main";
  }

  std::string page = header.run ();
  
  Assets_View view {};

  view.set_variable ("warning", bible_logic::unsent_unreceived_data_warning ());
  
  page += view.render ("index", "index");
  page += assets_page::footer ();
  return page;
}
