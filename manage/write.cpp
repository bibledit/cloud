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


#include <manage/write.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/books.h>
#include <database/temporal.h>
#include <locale/translate.h>
#include <menu/logic.h>
#include <manage/users.h>
#include <database/privileges.h>


std::string manage_write_url ()
{
  return "manage/write";
}


bool manage_write_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string manage_write (Webserver_Request& webserver_request)
{
  std::string page {};

  Assets_Header header = Assets_Header (translate("Read/write"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (manage_users_url (), menu_logic_manage_users_text ());
  page = header.run ();

  Assets_View view {};

  int userid = filter::strings::user_identifier (webserver_request);
  
  std::string user {};
  if (webserver_request.query.count ("user")) {
    user = webserver_request.query["user"];
    database::temporal::set_value (userid, "manage_write_user", user);
  }
  user = database::temporal::get_value (userid, "manage_write_user");
  view.set_variable ("user", user);
  
  std::string bible {};
  if (webserver_request.query.count ("bible")) {
    bible = webserver_request.query["bible"];
    database::temporal::set_value (userid, "manage_write_bible", bible);
  }
  bible = database::temporal::get_value (userid, "manage_write_bible");
  view.set_variable ("bible", bible);

  auto [ bible_read_access, bible_write_access ] = DatabasePrivileges::get_bible (user, bible);

  // Toggle write access to Bible book.
  if (!webserver_request.post.empty ()) {
    std::string checkbox = webserver_request.post_get("checkbox");
    std::string s_book (checkbox);
    s_book.erase (0, 4);
    int book = filter::strings::convert_to_int (s_book);
    if (book) {
      if (bible_read_access) {
        bool checked = filter::strings::convert_to_bool (webserver_request.post_get("checked"));
        DatabasePrivileges::set_bible_book (user, bible, book, checked);
        database_privileges_client_create (user, true);
      }
    }
    return std::string();
  }

  // Toggle write access to Testament.
  std::string testament = webserver_request.query ["testament"];
  if (!testament.empty ()) {
    // Count the majority 'write' access situation for the Bible.
    int majority = 0;
    std::vector <int> books = database::bibles::get_books (bible);
    for (auto & book : books) {
      std::string type = database::books::book_type_to_string (database::books::get_type (static_cast<book_id>(book)));
      if (type == testament) {
        bool read, write;
        DatabasePrivileges::get_bible_book (user, bible, book, read, write);
        if (write) majority++;
        else majority--;
      }
    }
    // Update the write access privileges for the books of the Testament,
    // by setting the write privileges to the opposite of the majority state.
    for (auto & book : books) {
      std::string type = database::books::book_type_to_string (database::books::get_type (static_cast<book_id>(book)));
      if (type == testament) {
        DatabasePrivileges::set_bible_book (user, bible, book, (majority < 0));
      }
    }
    // Update privileges for the clients.
    database_privileges_client_create (user, true);
  }
  
  // Read or write access to display.
  std::vector <int> books = database::bibles::get_books (bible);
  for (size_t i = 0; i < books.size (); i++) {
    int book = books[i];
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    std::string checkboxname = "book" + std::to_string (book);
    bool read, write;
    DatabasePrivileges::get_bible_book (user, bible, book, read, write);
    std::string checked = filter::strings::get_checkbox_status (write);
    view.add_iteration ("write", { std::pair ("bookname", bookname), std::pair ("checkboxname", checkboxname), std::pair ("checked", checked) } );
  }
  
  page += view.render ("manage", "write");

  page += assets_page::footer ();

  return page;
}
