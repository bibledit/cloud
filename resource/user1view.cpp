/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
using namespace std;


string resource_user1view_url ()
{
  return "resource/user1view";
}


bool resource_user1view_acl (void * webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


string resource_user1view (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  string page {};
  Assets_Header header = Assets_Header (translate("Resources"), request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  Assets_View view {};
  

  string name = request->query ["name"];

  
  vector <string> code {};
  string url = Database_UserResources::url (name);
  code.push_back ("var userResourceUrl = \"" + url + "\";");
  code.push_back ("var userResourceBooks = [];");
  vector <book_id> ids = database::books::get_ids ();
  for (auto id : ids) {
    book_type type = database::books::get_type (id);
    if ((type == book_type::old_testament) || (type == book_type::new_testament)) {
      string book = Database_UserResources::book (name, static_cast<int> (id));
      if (book.empty ()) book = filter::strings::convert_to_string (static_cast<int>(id));
      code.push_back ("userResourceBooks [" + filter::strings::convert_to_string (static_cast<int>(id)) + "] = \"" + book + "\";");
    }
  }
  string script = filter::strings::implode (code, "\n");
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  
  
  page += view.render ("resource", "user1view");
  page += assets_page::footer ();
  return page;
}
