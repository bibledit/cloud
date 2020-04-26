/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


#include <resource/user1edit.h>
#include <resource/img.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/userresources.h>
#include <database/books.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>
#include <tasks/logic.h>
#include <menu/logic.h>


string resource_user1edit_url ()
{
  return "resource/user1edit";
}


bool resource_user1edit_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string resource_user1edit (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate("User-defined resources"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;
  string error, success;
  
  
  string name = request->query ["name"];
  view.set_variable ("name", name);
  

  if (request->post.count ("submit")) {
    string data = request->post["data"];
    vector <string> lines = filter_string_explode (data, '\n');
    int count = 0;
    int bookcount = 0;
    for (auto line : lines) {
      line = filter_string_trim (line);
      if (line.empty ()) continue;
      if (count == 0) {
        Database_UserResources::url (name, line);
      } else {
        vector <string> bits = filter_string_explode (line, '=');
        if (bits.size () == 2) {
          string english = filter_string_trim (bits [0]);
          int id = Database_Books::getIdFromEnglish (english);
          if (id) {
            string fragment = filter_string_trim (bits [1]);
            Database_UserResources::book (name, id, fragment);
            bookcount++;
          } else {
            error.append (" ");
            error.append ("Failure interpreting " + english);
          }
        }
      }
      count++;
    }
    success = translate ("Number of defined books:") + " " + convert_to_string (bookcount);
  }
  
  
  vector <string> lines;
  lines.push_back (Database_UserResources::url (name));
  vector <int> ids = Database_Books::getIDs ();
  for (auto id : ids) {
    string type = Database_Books::getType (id);
    if ((type == "ot") || (type == "nt")) {
      string english = Database_Books::getEnglishFromId (id);
      string book = Database_UserResources::book (name, id);
      lines.push_back (english + " = " + book);
    }
  }
  view.set_variable ("data", filter_string_implode (lines, "\n"));
                   
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  view.set_variable ("url", resource_logic_default_user_url ());
  page += view.render ("resource", "user1edit");
  page += Assets_Page::footer ();
  return page;
}
