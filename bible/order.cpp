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


#include <bible/order.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/config/bible.h>
#include <database/books.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <menu/logic.h>
#include <bible/manage.h>


string bible_order_url ()
{
  return "bible/order";
}


bool bible_order_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string bible_order (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;

  Assets_Header header = Assets_Header (translate("Order"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  // The name of the Bible.
  string bible = access_bible_clamp (request, request->query ["bible"]);
  view.set_variable ("bible", filter_string_sanitize_html (bible));

  if (request->query.count ("reset")) {
    Database_Config_Bible::setBookOrder (bible, "");
  }

  string moveup = request->query ["moveup"];
  string movedown = request->query ["movedown"];
  if (!moveup.empty () || !movedown.empty ()) {
    size_t move = convert_to_int (moveup + movedown);
    vector <int> books = filter_passage_get_ordered_books (bible);
    vector <string> s_books;
    for (auto & book : books) s_books.push_back (convert_to_string (book));
    array_move_up_down (s_books, move, !moveup.empty ());
    string order = filter_string_implode (s_books, " ");
    Database_Config_Bible::setBookOrder (bible, order);
  }
  
  vector <int> books = filter_passage_get_ordered_books (bible);
  for (size_t i = 0; i < books.size (); i++) {
    string bookname = Database_Books::getEnglishFromId (books[i]);
    view.add_iteration ("order", { make_pair ("offset", convert_to_string (i)), make_pair ("bookname", bookname) } );
  }

  view.set_variable ("uparrow", unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", unicode_black_down_pointing_triangle ());

  page += view.render ("bible", "order");
  
  page += Assets_Page::footer ();
  
  return page;
}
