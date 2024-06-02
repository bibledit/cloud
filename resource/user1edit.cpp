/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


std::string resource_user1edit_url ()
{
  return "resource/user1edit";
}


bool resource_user1edit_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string resource_user1edit (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("User-defined resources"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view {};
  std::string error, success;
  
  
  std::string name = webserver_request.query ["name"];
  view.set_variable ("name", name);
  

  if (webserver_request.post.count ("submit")) {
    std::string data = webserver_request.post["data"];
    std::vector <std::string> lines = filter::strings::explode (data, '\n');
    int count = 0;
    int bookcount = 0;
    for (auto line : lines) {
      line = filter::strings::trim (line);
      if (line.empty ()) continue;
      if (count == 0) {
        Database_UserResources::url (name, line);
      } else {
        std::vector <std::string> bits = filter::strings::explode (line, '=');
        if (bits.size () == 2) {
          std::string english = filter::strings::trim (bits [0]);
          book_id id = database::books::get_id_from_english (english);
          if (id != book_id::_unknown) {
            std::string fragment = filter::strings::trim (bits [1]);
            Database_UserResources::book (name, static_cast<int>(id), fragment);
            bookcount++;
          } else {
            error.append (" ");
            error.append ("Failure interpreting " + english);
          }
        }
      }
      count++;
    }
    success = translate ("Number of defined books:") + " " + std::to_string (bookcount);
  }
  
  
  std::vector <std::string> lines;
  lines.push_back (Database_UserResources::url (name));
  std::vector <book_id> ids = database::books::get_ids ();
  for (auto id : ids) {
    book_type type = database::books::get_type (id);
    if ((type == book_type::old_testament) || (type == book_type::new_testament)) {
      std::string english = database::books::get_english_from_id (id);
      std::string book = Database_UserResources::book (name, static_cast<int>(id));
      lines.push_back (english + " = " + book);
    }
  }
  view.set_variable ("data", filter::strings::implode (lines, "\n"));
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  view.set_variable ("url", resource_logic_default_user_url ());
  page += view.render ("resource", "user1edit");
  page += assets_page::footer ();
  return page;
}
