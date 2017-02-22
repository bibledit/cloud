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


#include <manage/write.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/books.h>
#include <database/volatile.h>
#include <locale/translate.h>
#include <menu/logic.h>
#include <manage/users.h>
#include <database/privileges.h>


string manage_write_url ()
{
  return "manage/write";
}


bool manage_write_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string manage_write (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  string page;

  Assets_Header header = Assets_Header (translate("Read/write"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (manage_users_url (), menu_logic_manage_users_text ());
  page = header.run ();

  Assets_View view;

  int userid = filter_string_user_identifier (webserver_request);
  
  string user;
  if (request->query.count ("user")) {
    user = request->query["user"];
    Database_Volatile::setValue (userid, "manage_write_user", user);
  }
  user = Database_Volatile::getValue (userid, "manage_write_user");
  view.set_variable ("user", user);
  
  string bible;
  if (request->query.count ("bible")) {
    bible = request->query["bible"];
    Database_Volatile::setValue (userid, "manage_write_bible", bible);
  }
  bible = Database_Volatile::getValue (userid, "manage_write_bible");
  view.set_variable ("bible", bible);

  bool bible_read_access, bible_write_access;
  Database_Privileges::getBible (user, bible, bible_read_access, bible_write_access);

  // Toggle write access to Bible book.
  if (!request->post.empty ()) {
    string checkbox = request->post["checkbox"];
    checkbox.erase (0, 4);
    int book = convert_to_int (checkbox);
    if (book) {
      if (bible_read_access) {
        string checkbox = request->post ["checkbox"];
        bool checked = convert_to_bool (request->post ["checked"]);
        Database_Privileges::setBibleBook (user, bible, book, checked);
        database_privileges_client_create (user, true);
      }
    }
    return "";
  }

  // Toggle write access to Testament.
  string testament = request->query ["testament"];
  if (!testament.empty ()) {
    // Count the majority 'write' access situation for the Bible.
    int majority = 0;
    vector <int> books = request->database_bibles ()->getBooks (bible);
    for (auto & book : books) {
      string type = Database_Books::getType (book);
      if (type == testament) {
        bool read, write;
        Database_Privileges::getBibleBook (user, bible, book, read, write);
        if (write) majority++;
        else majority--;
      }
    }
    // Update the write access privileges for the books of the Testament,
    // by setting the write privileges to the opposite of the majority state.
    for (auto & book : books) {
      string type = Database_Books::getType (book);
      if (type == testament) {
        Database_Privileges::setBibleBook (user, bible, book, (majority < 0));
      }
    }
    // Update privileges for the clients.
    database_privileges_client_create (user, true);
  }
  
  // Read or write access to display.
  vector <int> books = request->database_bibles ()->getBooks (bible);
  for (size_t i = 0; i < books.size (); i++) {
    int book = books[i];
    string bookname = Database_Books::getEnglishFromId (book);
    string checkboxname = "book" + convert_to_string (book);
    bool read, write;
    Database_Privileges::getBibleBook (user, bible, book, read, write);
    string checked = get_checkbox_status (write);
    view.add_iteration ("write", { make_pair ("bookname", bookname), make_pair ("checkboxname", checkboxname), make_pair ("checked", checked) } );
  }
  
  page += view.render ("manage", "write");

  page += Assets_Page::footer ();

  return page;
}
