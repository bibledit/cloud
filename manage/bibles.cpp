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


#include <manage/bibles.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/privileges.h>
#include <access/logic.h>
#include <menu/logic.h>
#include <manage/users.h>
#include <dialog/select.h>


std::string manage_bibles_url ()
{
  return "manage/bibles";
}


bool manage_bibles_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string manage_bibles (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("Read/write"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (manage_users_url (), menu_logic_manage_users_text ());
  page = header.run ();
  Assets_View view {};

  
  // Get the user and the associated level.
  std::string user {webserver_request.query["user"]};
  view.set_variable ("user", user);
  int level {0};
  access_logic::user_level (webserver_request, user, level);


  // Remove Bible from user.
  if (webserver_request.query.count ("remove")) {
    const std::string remove = webserver_request.query.at("remove");
    DatabasePrivileges::remove_bible_book (user, remove, 0);
  }

  
  // Add a Bible to the user.
  constexpr const char* add {"add"};
  if (webserver_request.post.count (add)) {
    const auto added = webserver_request.post.at(add);
    // Write access depends on whether it's a translator role or higher.
    const bool write = (level >= roles::translator);
    DatabasePrivileges::set_bible (user, added, write);
  }

  
  const std::vector <std::string> all_bibles = database::bibles::get_bibles ();

  std::vector<std::string> addable_bibles{};
    
  for (const auto& bible : all_bibles) {
    if (const bool exists = DatabasePrivileges::get_bible_book_exists (user, bible, 0); exists) {
      auto [ read, write ] = DatabasePrivileges::get_bible (user, bible);
      if  (level >= roles::translator)
        write = true;
      int readwrite {0};
      const std::vector <int> books = database::bibles::get_books (bible);
      for (const auto book : books) {
        DatabasePrivileges::get_bible_book (user, bible, book, read, write);
        if (write) readwrite++;
      }
      view.add_iteration ("bible", {
        std::pair ("bible", bible),
        std::pair ("write", std::to_string(readwrite)),
        std::pair ("total", std::to_string(books.size())),
      });
    }
    else {
      addable_bibles.push_back(bible);
    }
  }

  {
    if (!addable_bibles.empty()) {
      addable_bibles.insert(addable_bibles.begin(), std::string());
      view.enable_zone("add");
    }
    dialog::select::Settings settings {
      .identification = add,
      .values = addable_bibles,
      .selected = "",
      .parameters = { {"user", user} },
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(add, dialog::select::form(settings, form));
  }

  page.append(view.render("manage", "bibles"));
  
  page.append(assets_page::footer());
  
  return page;
}
