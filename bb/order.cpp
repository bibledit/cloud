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
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  // The name of the Bible.
  string bible = access_bible::clamp (request, request->query ["bible"]);
  view.set_variable ("bible", escape_special_xml_characters (bible));

  // The order the user wants for the Bibles.
  string order = request->query ["order"];
  
  // Deuterocanonicals or Apocrypha interspersed among the books of the Hebrew Bible.
  if (order == "interspersed") {
    vector <int> interspersed = {
      database::books::get_id_from_english ("Front Matter"),
      database::books::get_id_from_english ("Introduction Matter"),
      database::books::get_id_from_english ("Genesis"),
      database::books::get_id_from_english ("Exodus"),
      database::books::get_id_from_english ("Leviticus"),
      database::books::get_id_from_english ("Numbers"),
      database::books::get_id_from_english ("Deuteronomy"),
      database::books::get_id_from_english ("Joshua"),
      database::books::get_id_from_english ("Judges"),
      database::books::get_id_from_english ("Ruth"),
      database::books::get_id_from_english ("1 Samuel"),
      database::books::get_id_from_english ("2 Samuel"),
      database::books::get_id_from_english ("1 Kings"),
      database::books::get_id_from_english ("2 Kings"),
      database::books::get_id_from_english ("1 Chronicles"),
      database::books::get_id_from_english ("2 Chronicles"),
      database::books::get_id_from_english ("Ezra"),
      database::books::get_id_from_english ("Nehemiah"),
      database::books::get_id_from_english ("Tobit"),
      database::books::get_id_from_english ("Judith"),
      database::books::get_id_from_english ("Esther"),
      database::books::get_id_from_english ("1 Maccabees"),
      database::books::get_id_from_english ("2 Maccabees"),
      database::books::get_id_from_english ("Job"),
      database::books::get_id_from_english ("Psalms"),
      database::books::get_id_from_english ("Proverbs"),
      database::books::get_id_from_english ("Ecclesiastes"),
      database::books::get_id_from_english ("Song of Solomon"),
      database::books::get_id_from_english ("Wisdom of Solomon"),
      database::books::get_id_from_english ("Sirach"),
      database::books::get_id_from_english ("Isaiah"),
      database::books::get_id_from_english ("Jeremiah"),
      database::books::get_id_from_english ("Lamentations"),
      database::books::get_id_from_english ("Baruch"),
      database::books::get_id_from_english ("Ezekiel"),
      database::books::get_id_from_english ("Daniel"),
      database::books::get_id_from_english ("Hosea"),
      database::books::get_id_from_english ("Joel"),
      database::books::get_id_from_english ("Amos"),
      database::books::get_id_from_english ("Obadiah"),
      database::books::get_id_from_english ("Jonah"),
      database::books::get_id_from_english ("Micah"),
      database::books::get_id_from_english ("Nahum"),
      database::books::get_id_from_english ("Habakkuk"),
      database::books::get_id_from_english ("Zephaniah"),
      database::books::get_id_from_english ("Haggai"),
      database::books::get_id_from_english ("Zechariah"),
      database::books::get_id_from_english ("Malachi"),
    };
    vector <string> v_book_order;
    for (auto & book : interspersed) v_book_order.push_back (convert_to_string (book));
    string s_book_order = filter_string_implode (v_book_order, " ");
    Database_Config_Bible::setBookOrder (bible, s_book_order);
  }

  
  // Deuterocanonicals or Apocrypha between the Hebrew Bible and the New Testament.
  if (order == "between") {
    vector <int> interspersed = {
      database::books::get_id_from_english ("Front Matter"),
      database::books::get_id_from_english ("Introduction Matter"),
      database::books::get_id_from_english ("Genesis"),
      database::books::get_id_from_english ("Exodus"),
      database::books::get_id_from_english ("Leviticus"),
      database::books::get_id_from_english ("Numbers"),
      database::books::get_id_from_english ("Deuteronomy"),
      database::books::get_id_from_english ("Joshua"),
      database::books::get_id_from_english ("Judges"),
      database::books::get_id_from_english ("Ruth"),
      database::books::get_id_from_english ("1 Samuel"),
      database::books::get_id_from_english ("2 Samuel"),
      database::books::get_id_from_english ("1 Kings"),
      database::books::get_id_from_english ("2 Kings"),
      database::books::get_id_from_english ("1 Chronicles"),
      database::books::get_id_from_english ("2 Chronicles"),
      database::books::get_id_from_english ("Ezra"),
      database::books::get_id_from_english ("Nehemiah"),
      database::books::get_id_from_english ("Esther"),
      database::books::get_id_from_english ("Job"),
      database::books::get_id_from_english ("Psalms"),
      database::books::get_id_from_english ("Proverbs"),
      database::books::get_id_from_english ("Ecclesiastes"),
      database::books::get_id_from_english ("Song of Solomon"),
      database::books::get_id_from_english ("Isaiah"),
      database::books::get_id_from_english ("Jeremiah"),
      database::books::get_id_from_english ("Lamentations"),
      database::books::get_id_from_english ("Ezekiel"),
      database::books::get_id_from_english ("Daniel"),
      database::books::get_id_from_english ("Hosea"),
      database::books::get_id_from_english ("Joel"),
      database::books::get_id_from_english ("Amos"),
      database::books::get_id_from_english ("Obadiah"),
      database::books::get_id_from_english ("Jonah"),
      database::books::get_id_from_english ("Micah"),
      database::books::get_id_from_english ("Nahum"),
      database::books::get_id_from_english ("Habakkuk"),
      database::books::get_id_from_english ("Zephaniah"),
      database::books::get_id_from_english ("Haggai"),
      database::books::get_id_from_english ("Zechariah"),
      database::books::get_id_from_english ("Malachi"),
      database::books::get_id_from_english ("Tobit"),
      database::books::get_id_from_english ("Judith"),
      database::books::get_id_from_english ("1 Maccabees"),
      database::books::get_id_from_english ("2 Maccabees"),
      database::books::get_id_from_english ("Wisdom of Solomon"),
      database::books::get_id_from_english ("Sirach"),
      database::books::get_id_from_english ("Baruch"),
    };
    vector <string> v_book_order;
    for (auto & book : interspersed) v_book_order.push_back (convert_to_string (book));
    string s_book_order = filter_string_implode (v_book_order, " ");
    Database_Config_Bible::setBookOrder (bible, s_book_order);
  }

  
  // Deuterocanonicals or Apocrypha at the end of the entire Bible.
  if (order == "end") {
    Database_Config_Bible::setBookOrder (bible, string());
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
    string bookname = database::books::get_english_from_id (books[i]);
    bookname = translate (bookname);
    view.add_iteration ("order", { pair ("offset", convert_to_string (i)), pair ("bookname", bookname) } );
  }

  view.set_variable ("uparrow", unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", unicode_black_down_pointing_triangle ());

  page += view.render ("bb", "order");
  
  page += Assets_Page::footer ();
  
  return page;
}
