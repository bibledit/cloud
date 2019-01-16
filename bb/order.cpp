/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;

  Assets_Header header = Assets_Header (translate("Order"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view;
  
  // The name of the Bible.
  string bible = access_bible_clamp (request, request->query ["bible"]);
  view.set_variable ("bible", escape_special_xml_characters (bible));

  // The order the user wants for the Bibles.
  string order = request->query ["order"];
  
  // Deuterocanonicals or Apocrypha interspersed among the books of the Hebrew Bible.
  if (order == "interspersed") {
    vector <int> interspersed = {
      Database_Books::getIdFromEnglish ("Front Matter"),
      Database_Books::getIdFromEnglish ("Introduction Matter"),
      Database_Books::getIdFromEnglish ("Genesis"),
      Database_Books::getIdFromEnglish ("Exodus"),
      Database_Books::getIdFromEnglish ("Leviticus"),
      Database_Books::getIdFromEnglish ("Numbers"),
      Database_Books::getIdFromEnglish ("Deuteronomy"),
      Database_Books::getIdFromEnglish ("Joshua"),
      Database_Books::getIdFromEnglish ("Judges"),
      Database_Books::getIdFromEnglish ("Ruth"),
      Database_Books::getIdFromEnglish ("1 Samuel"),
      Database_Books::getIdFromEnglish ("2 Samuel"),
      Database_Books::getIdFromEnglish ("1 Kings"),
      Database_Books::getIdFromEnglish ("2 Kings"),
      Database_Books::getIdFromEnglish ("1 Chronicles"),
      Database_Books::getIdFromEnglish ("2 Chronicles"),
      Database_Books::getIdFromEnglish ("Ezra"),
      Database_Books::getIdFromEnglish ("Nehemiah"),
      Database_Books::getIdFromEnglish ("Tobit"),
      Database_Books::getIdFromEnglish ("Judith"),
      Database_Books::getIdFromEnglish ("Esther"),
      Database_Books::getIdFromEnglish ("1 Maccabees"),
      Database_Books::getIdFromEnglish ("2 Maccabees"),
      Database_Books::getIdFromEnglish ("Job"),
      Database_Books::getIdFromEnglish ("Psalms"),
      Database_Books::getIdFromEnglish ("Proverbs"),
      Database_Books::getIdFromEnglish ("Ecclesiastes"),
      Database_Books::getIdFromEnglish ("Song of Solomon"),
      Database_Books::getIdFromEnglish ("Wisdom of Solomon"),
      Database_Books::getIdFromEnglish ("Sirach"),
      Database_Books::getIdFromEnglish ("Isaiah"),
      Database_Books::getIdFromEnglish ("Jeremiah"),
      Database_Books::getIdFromEnglish ("Lamentations"),
      Database_Books::getIdFromEnglish ("Baruch"),
      Database_Books::getIdFromEnglish ("Ezekiel"),
      Database_Books::getIdFromEnglish ("Daniel"),
      Database_Books::getIdFromEnglish ("Hosea"),
      Database_Books::getIdFromEnglish ("Joel"),
      Database_Books::getIdFromEnglish ("Amos"),
      Database_Books::getIdFromEnglish ("Obadiah"),
      Database_Books::getIdFromEnglish ("Jonah"),
      Database_Books::getIdFromEnglish ("Micah"),
      Database_Books::getIdFromEnglish ("Nahum"),
      Database_Books::getIdFromEnglish ("Habakkuk"),
      Database_Books::getIdFromEnglish ("Zephaniah"),
      Database_Books::getIdFromEnglish ("Haggai"),
      Database_Books::getIdFromEnglish ("Zechariah"),
      Database_Books::getIdFromEnglish ("Malachi"),
    };
    vector <string> bookorder;
    for (auto & book : interspersed) bookorder.push_back (convert_to_string (book));
    string order = filter_string_implode (bookorder, " ");
    Database_Config_Bible::setBookOrder (bible, order);
  }

  
  // Deuterocanonicals or Apocrypha between the Hebrew Bible and the New Testament.
  if (order == "between") {
    vector <int> interspersed = {
      Database_Books::getIdFromEnglish ("Front Matter"),
      Database_Books::getIdFromEnglish ("Introduction Matter"),
      Database_Books::getIdFromEnglish ("Genesis"),
      Database_Books::getIdFromEnglish ("Exodus"),
      Database_Books::getIdFromEnglish ("Leviticus"),
      Database_Books::getIdFromEnglish ("Numbers"),
      Database_Books::getIdFromEnglish ("Deuteronomy"),
      Database_Books::getIdFromEnglish ("Joshua"),
      Database_Books::getIdFromEnglish ("Judges"),
      Database_Books::getIdFromEnglish ("Ruth"),
      Database_Books::getIdFromEnglish ("1 Samuel"),
      Database_Books::getIdFromEnglish ("2 Samuel"),
      Database_Books::getIdFromEnglish ("1 Kings"),
      Database_Books::getIdFromEnglish ("2 Kings"),
      Database_Books::getIdFromEnglish ("1 Chronicles"),
      Database_Books::getIdFromEnglish ("2 Chronicles"),
      Database_Books::getIdFromEnglish ("Ezra"),
      Database_Books::getIdFromEnglish ("Nehemiah"),
      Database_Books::getIdFromEnglish ("Esther"),
      Database_Books::getIdFromEnglish ("Job"),
      Database_Books::getIdFromEnglish ("Psalms"),
      Database_Books::getIdFromEnglish ("Proverbs"),
      Database_Books::getIdFromEnglish ("Ecclesiastes"),
      Database_Books::getIdFromEnglish ("Song of Solomon"),
      Database_Books::getIdFromEnglish ("Isaiah"),
      Database_Books::getIdFromEnglish ("Jeremiah"),
      Database_Books::getIdFromEnglish ("Lamentations"),
      Database_Books::getIdFromEnglish ("Ezekiel"),
      Database_Books::getIdFromEnglish ("Daniel"),
      Database_Books::getIdFromEnglish ("Hosea"),
      Database_Books::getIdFromEnglish ("Joel"),
      Database_Books::getIdFromEnglish ("Amos"),
      Database_Books::getIdFromEnglish ("Obadiah"),
      Database_Books::getIdFromEnglish ("Jonah"),
      Database_Books::getIdFromEnglish ("Micah"),
      Database_Books::getIdFromEnglish ("Nahum"),
      Database_Books::getIdFromEnglish ("Habakkuk"),
      Database_Books::getIdFromEnglish ("Zephaniah"),
      Database_Books::getIdFromEnglish ("Haggai"),
      Database_Books::getIdFromEnglish ("Zechariah"),
      Database_Books::getIdFromEnglish ("Malachi"),
      Database_Books::getIdFromEnglish ("Tobit"),
      Database_Books::getIdFromEnglish ("Judith"),
      Database_Books::getIdFromEnglish ("1 Maccabees"),
      Database_Books::getIdFromEnglish ("2 Maccabees"),
      Database_Books::getIdFromEnglish ("Wisdom of Solomon"),
      Database_Books::getIdFromEnglish ("Sirach"),
      Database_Books::getIdFromEnglish ("Baruch"),
    };
    vector <string> bookorder;
    for (auto & book : interspersed) bookorder.push_back (convert_to_string (book));
    string order = filter_string_implode (bookorder, " ");
    Database_Config_Bible::setBookOrder (bible, order);
  }

  
  // Deuterocanonicals or Apocrypha at the end of the entire Bible.
  if (order == "end") {
    Database_Config_Bible::setBookOrder (bible, "");
  }

  // Handle updates to the custom book order.
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
    bookname = translate (bookname);
    view.add_iteration ("order", { make_pair ("offset", convert_to_string (i)), make_pair ("bookname", bookname) } );
  }

  view.set_variable ("uparrow", unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", unicode_black_down_pointing_triangle ());

  page += view.render ("bb", "order");
  
  page += Assets_Page::footer ();
  
  return page;
}
