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


#include <resource/user1view.h>
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
#include <database/userresources.h>
#include <database/books.h>


string resource_user1view_url ()
{
  return "resource/user1view";
}


bool resource_user1view_acl (void * webserver_request)
{
  return access_logic_privilege_view_resources (webserver_request);
}


string resource_user1view (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Resources"), request);
  header.setNavigator ();
  header.addBreadCrumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  Assets_View view;
  

  string name = request->query ["name"];

  
  vector <string> code;
  string url = Database_UserResources::url (name);
  code.push_back ("var userResourceUrl = \"" + url + "\";");
  code.push_back ("var userResourceBooks = [];");
  vector <int> ids = Database_Books::getIDs ();
  for (auto id : ids) {
    string type = Database_Books::getType (id);
    if ((type == "ot") || (type == "nt")) {
      string book = Database_UserResources::book (name, id);
      if (book.empty ()) book = convert_to_string (id);
      code.push_back ("userResourceBooks [" + convert_to_string (id) + "] = \"" + book + "\";");
    }
  }
  string script = filter_string_implode (code, "\n");
  config_logic_swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  
  
  page += view.render ("resource", "user1view");
  page += Assets_Page::footer ();
  return page;
}
