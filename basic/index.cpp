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


#include <basic/index.h>
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
#include <dialog/list.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <client/logic.h>
#include <styles/logic.h>


const char * basic_index_url ()
{
  return "basic/index";
}


bool basic_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


string basic_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  Assets_Header header = Assets_Header ("Settings", webserver_request);
  string page = header.run ();
  Assets_View view;
  string on_off;
  
  
  if (request->query.count ("changebible")) {
    string changebible = request->query ["changebible"];
    if (changebible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to make the active one for editing"), "", "");
      vector <string> bibles = access_bible_bibles (request);
      for (auto & bible : bibles) {
        dialog_list.add_row (bible, "changebible", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (changebible);
      // Going to another Bible, ensure that the focused book exists there.
      int book = Ipc_Focus::getBook (request);
      vector <int> books = request->database_bibles()->getBooks (changebible);
      if (find (books.begin(), books.end(), book) == books.end()) {
        if (!books.empty ()) book = books [0];
        else book = 0;
        Ipc_Focus::set (request, book, 1, 1);
      }
    }
  }
  string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  view.set_variable ("bible", bible);
  
  
  if (request->query.count ("showchanges")) {
    bool state = request->database_config_user ()->getMenuChangesInBasicMode ();
    request->database_config_user ()->setMenuChangesInBasicMode (!state);
  }
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getMenuChangesInBasicMode ());
  view.set_variable ("showchanges", on_off);
  
  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
  if (client_logic_client_enabled ()) {
    view.enable_zone ("connected");
  }
#else
  view.enable_zone ("cloud");
#endif
  
  
  page += view.render ("basic", "index");
  page += Assets_Page::footer ();
  return page;
}
