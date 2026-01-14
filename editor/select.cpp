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


#include <editor/select.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <menu/logic.h>
#include <edit/index.h>
#include <editone/index.h>
#include <editusfm/index.h>
#include <webserver/request.h>


std::string editor_select_url ()
{
  return "editor/select";
}


bool editor_select_acl (Webserver_Request& webserver_request)
{
  if (roles::access_control (webserver_request, roles::translator))
    return true;
  const auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


std::string editor_select (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Select editor"), webserver_request);
  page = header.run();
  Assets_View view;
  
  std::vector <std::string> urls;
  
  if (edit_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, true, true)) {
      const std::string label = menu_logic_editor_menu_text (true, true);
      const std::string url = edit_index_url ();
      view.add_iteration ("editor", { std::pair ("url", url), std::pair ("label", label) } );
      urls.push_back (url);
    }
  }
  
  if (editone_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, true, false)) {
      const std::string label = menu_logic_editor_menu_text (true, false);
      const std::string url = editone_index_url ();
      view.add_iteration ("editor", { std::pair ("url", url), std::pair ("label", label) } );
      urls.push_back (url);
    }
  }
  
  if (editusfm_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, false, true)) {
      const std::string label = menu_logic_editor_menu_text (false, true);
      const std::string url = editusfm_index_url ();
      view.add_iteration ("editor", { std::pair ("url", url), std::pair ("label", label) } );
      urls.push_back (url);
    }
  }
  
  // Check on whether the Bible was passed.
  // If so, write that to the active Bible in the user configuration.
  // More info: https://github.com/bibledit/cloud/issues/1003
  const std::string bible = webserver_request.query ["bible"];
  if (!bible.empty()) {
    webserver_request.database_config_user()->set_bible(bible);
  }
  
  // Checking on whether to switch to another editor, through the keyboard shortcut.
  std::string from = webserver_request.query ["from"];
  from.append ("/");
  if (!from.empty ()) {
    if (!urls.empty ()) {
      urls.push_back (urls[0]);
      bool match = false;
      for (auto url : urls) {
        if (match) {
          redirect_browser (webserver_request, url);
          return std::string();
        }
        if (url.find (from) == 0) match = true;
      }
    }
  }
  
  page += view.render ("editor", "select");
  page += assets_page::footer ();
  return page;
}
