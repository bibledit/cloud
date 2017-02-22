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
#include <editverse/index.h>


string editor_select_url ()
{
  return "editor/select";
}


bool editor_select_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  bool read, write;
  access_a_bible (webserver_request, read, write);
  return write;
}


string editor_select (void * webserver_request)
{
  Webserver_Request * request = static_cast <Webserver_Request *> (webserver_request);
  
  string page;
  Assets_Header header = Assets_Header (translate("Select editor"), webserver_request);
  page = header.run();
  Assets_View view;
  
  vector <string> urls;
  
  if (edit_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, true, true)) {
      string label = menu_logic_editor_menu_text (webserver_request, true, true);
      string url = edit_index_url ();
      view.add_iteration ("editor", { make_pair ("url", url), make_pair ("label", label) } );
      urls.push_back (url);
    }
  }
  
  if (editone_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, true, false)) {
      string label = menu_logic_editor_menu_text (webserver_request, true, false);
      string url = editone_index_url ();
      view.add_iteration ("editor", { make_pair ("url", url), make_pair ("label", label) } );
      urls.push_back (url);
    }
  }
  
  if (editusfm_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, false, true)) {
      string label = menu_logic_editor_menu_text (webserver_request, false, true);
      string url = editusfm_index_url ();
      view.add_iteration ("editor", { make_pair ("url", url), make_pair ("label", label) } );
      urls.push_back (url);
    }
  }
  
  if (editverse_index_acl (webserver_request)) {
    if (menu_logic_editor_enabled (webserver_request, false, false)) {
      string label = menu_logic_editor_menu_text (webserver_request, false, false);
      string url = editverse_index_url ();
      view.add_iteration ("editor", { make_pair ("url", url), make_pair ("label", label) } );
      urls.push_back (url);
    }
  }

  // Checking on whether to switch to another editor, through the keyboard shortcut.
  string from = request->query ["from"];
  from.append ("/");
  if (!from.empty ()) {
    if (!urls.empty ()) {
      urls.push_back (urls[0]);
      bool match = false;
      for (auto url : urls) {
        if (match) {
          redirect_browser (webserver_request, url);
          return "";
        }
        if (url.find (from) == 0) match = true;
      }
    }
  }
  
  page += view.render ("editor", "select");
  page += Assets_Page::footer ();
  return page;
}
