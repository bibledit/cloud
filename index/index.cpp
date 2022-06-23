/*
Copyright (©) 2003-2022 Teus Benschop.

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


const char * index_index_url ()
{
  return "index/index";
}


bool index_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string index_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  filter_webview_log_user_agent (request->user_agent);
  
  Assets_Header header = Assets_Header (translate ("Bibledit"), webserver_request);
  
  if (config_logic_demo_enabled ()) {
    // The demo, when there's no active menu, forwards to the active workspace.
    // This is disabled see https://github.com/bibledit/cloud/issues/789
//    if (request->query.empty ()) {
//      header.refresh (5, "/" + workspace_index_url ());
//    }
    // Indonesian Cloud Free
    // Forwards to read/index instead.
    if (config_logic_indonesian_cloud_free_simple ()) {
      if (request->query.empty ()) {
        header.refresh (5, "/" + read_index_url ());
      }
    }
  }
  
  // Basic or advanced mode setting.
  string mode = request->query ["mode"];
  if (!mode.empty ()) {
    bool basic = (mode == "basic");
    request->database_config_user ()->setBasicInterfaceMode (basic);
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
  if (request->query.count ("item") == 0) {
    request->query ["item"] = "main";
  }

  string page = header.run ();
  
  Assets_View view;

  view.set_variable ("warning", bible_logic_unsent_unreceived_data_warning ());
  
  page += view.render ("index", "index");
  page += Assets_Page::footer ();
  return page;
}
