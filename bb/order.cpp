/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


#include <bb/order.h>
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
#include <bb/manage.h>


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
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  string page;

  Assets_Header header = Assets_Header (translate("Order"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  // The name of the Bible.
  string bible = AccessBible::Clamp (request, request->query ["bible"]);
  view.set_variable ("bible", escape_special_xml_characters (bible));

  // The order the user wants for the Bibles.
  string order = request->query ["order"];
  
  // Deuterocanonicals or Apocrypha interspersed among the books of the Hebrew Bible.
  if (order == "interspersed") {
    vector <int> interspersed = {
      Database_Books::get_id_from_english ("Front Matter"),
      Database_Books::get_id_from_english ("Introduction Matter"),
      Database_Books::get_id_from_english ("Genesis"),
      Database_Books::get_id_from_english ("Exodus"),
      Database_Books::get_id_from_english ("Leviticus"),
      Database_Books::get_id_from_english ("Numbers"),
      Database_Books::get_id_from_english ("Deuteronomy"),
      Database_Books::get_id_from_english ("Joshua"),
      Database_Books::get_id_from_english ("Judges"),
      Database_Books::get_id_from_english ("Ruth"),
      Database_Books::get_id_from_english ("1 Samuel"),
      Database_Books::get_id_from_english ("2 Samuel"),
      Database_Books::get_id_from_english ("1 Kings"),
      Database_Books::get_id_from_english ("2 Kings"),
      Database_Books::get_id_from_english ("1 Chronicles"),
      Database_Books::get_id_from_english ("2 Chronicles"),
      Database_Books::get_id_from_english ("Ezra"),
      Database_Books::get_id_from_english ("Nehemiah"),
      Database_Books::get_id_from_english ("Tobit"),
      Database_Books::get_id_from_english ("Judith"),
      Database_Books::get_id_from_english ("Esther"),
      Database_Books::get_id_from_english ("1 Maccabees"),
      Database_Books::get_id_from_english ("2 Maccabees"),
      Database_Books::get_id_from_english ("Job"),
      Database_Books::get_id_from_english ("Psalms"),
      Database_Books::get_id_from_english ("Proverbs"),
      Database_Books::get_id_from_english ("Ecclesiastes"),
      Database_Books::get_id_from_english ("Song of Solomon"),
      Database_Books::get_id_from_english ("Wisdom of Solomon"),
      Database_Books::get_id_from_english ("Sirach"),
      Database_Books::get_id_from_english ("Isaiah"),
      Database_Books::get_id_from_english ("Jeremiah"),
      Database_Books::get_id_from_english ("Lamentations"),
      Database_Books::get_id_from_english ("Baruch"),
      Database_Books::get_id_from_english ("Ezekiel"),
      Database_Books::get_id_from_english ("Daniel"),
      Database_Books::get_id_from_english ("Hosea"),
      Database_Books::get_id_from_english ("Joel"),
      Database_Books::get_id_from_english ("Amos"),
      Database_Books::get_id_from_english ("Obadiah"),
      Database_Books::get_id_from_english ("Jonah"),
      Database_Books::get_id_from_english ("Micah"),
      Database_Books::get_id_from_english ("Nahum"),
      Database_Books::get_id_from_english ("Habakkuk"),
      Database_Books::get_id_from_english ("Zephaniah"),
      Database_Books::get_id_from_english ("Haggai"),
      Database_Books::get_id_from_english ("Zechariah"),
      Database_Books::get_id_from_english ("Malachi"),
    };
    vector <string> v_book_order;
    for (auto & book : interspersed) v_book_order.push_back (convert_to_string (book));
    string s_book_order = filter_string_implode (v_book_order, " ");
    Database_Config_Bible::setBookOrder (bible, s_book_order);
  }

  
  // Deuterocanonicals or Apocrypha between the Hebrew Bible and the New Testament.
  if (order == "between") {
    vector <int> interspersed = {
      Database_Books::get_id_from_english ("Front Matter"),
      Database_Books::get_id_from_english ("Introduction Matter"),
      Database_Books::get_id_from_english ("Genesis"),
      Database_Books::get_id_from_english ("Exodus"),
      Database_Books::get_id_from_english ("Leviticus"),
      Database_Books::get_id_from_english ("Numbers"),
      Database_Books::get_id_from_english ("Deuteronomy"),
      Database_Books::get_id_from_english ("Joshua"),
      Database_Books::get_id_from_english ("Judges"),
      Database_Books::get_id_from_english ("Ruth"),
      Database_Books::get_id_from_english ("1 Samuel"),
      Database_Books::get_id_from_english ("2 Samuel"),
      Database_Books::get_id_from_english ("1 Kings"),
      Database_Books::get_id_from_english ("2 Kings"),
      Database_Books::get_id_from_english ("1 Chronicles"),
      Database_Books::get_id_from_english ("2 Chronicles"),
      Database_Books::get_id_from_english ("Ezra"),
      Database_Books::get_id_from_english ("Nehemiah"),
      Database_Books::get_id_from_english ("Esther"),
      Database_Books::get_id_from_english ("Job"),
      Database_Books::get_id_from_english ("Psalms"),
      Database_Books::get_id_from_english ("Proverbs"),
      Database_Books::get_id_from_english ("Ecclesiastes"),
      Database_Books::get_id_from_english ("Song of Solomon"),
      Database_Books::get_id_from_english ("Isaiah"),
      Database_Books::get_id_from_english ("Jeremiah"),
      Database_Books::get_id_from_english ("Lamentations"),
      Database_Books::get_id_from_english ("Ezekiel"),
      Database_Books::get_id_from_english ("Daniel"),
      Database_Books::get_id_from_english ("Hosea"),
      Database_Books::get_id_from_english ("Joel"),
      Database_Books::get_id_from_english ("Amos"),
      Database_Books::get_id_from_english ("Obadiah"),
      Database_Books::get_id_from_english ("Jonah"),
      Database_Books::get_id_from_english ("Micah"),
      Database_Books::get_id_from_english ("Nahum"),
      Database_Books::get_id_from_english ("Habakkuk"),
      Database_Books::get_id_from_english ("Zephaniah"),
      Database_Books::get_id_from_english ("Haggai"),
      Database_Books::get_id_from_english ("Zechariah"),
      Database_Books::get_id_from_english ("Malachi"),
      Database_Books::get_id_from_english ("Tobit"),
      Database_Books::get_id_from_english ("Judith"),
      Database_Books::get_id_from_english ("1 Maccabees"),
      Database_Books::get_id_from_english ("2 Maccabees"),
      Database_Books::get_id_from_english ("Wisdom of Solomon"),
      Database_Books::get_id_from_english ("Sirach"),
      Database_Books::get_id_from_english ("Baruch"),
    };
    vector <string> v_book_order;
    for (auto & book : interspersed) v_book_order.push_back (convert_to_string (book));
    string s_book_order = filter_string_implode (v_book_order, " ");
    Database_Config_Bible::setBookOrder (bible, s_book_order);
  }

  
  // Deuterocanonicals or Apocrypha at the end of the entire Bible.
  if (order == "end") {
    Database_Config_Bible::setBookOrder (bible, "");
  }

  // Handle updates to the custom book order.
  string moveup = request->query ["moveup"];
  string movedown = request->query ["movedown"];
  if (!moveup.empty () || !movedown.empty ()) {
    size_t move = static_cast<size_t>(convert_to_int (moveup + movedown));
    vector <int> books = filter_passage_get_ordered_books (bible);
    vector <string> s_books;
    for (auto & book : books) s_books.push_back (convert_to_string (book));
    array_move_up_down (s_books, move, !moveup.empty ());
    string s_order = filter_string_implode (s_books, " ");
    Database_Config_Bible::setBookOrder (bible, s_order);
  }
  
  vector <int> books = filter_passage_get_ordered_books (bible);
  for (size_t i = 0; i < books.size (); i++) {
    string bookname = Database_Books::get_english_from_id (books[i]);
    bookname = translate (bookname);
    view.add_iteration ("order", { pair ("offset", convert_to_string (i)), pair ("bookname", bookname) } );
  }

  view.set_variable ("uparrow", unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", unicode_black_down_pointing_triangle ());

  page += view.render ("bb", "order");
  
  page += Assets_Page::footer ();
  
  return page;
}
