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
using namespace std;


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
  
  string page {};

  Assets_Header header = Assets_Header (translate("Order"), request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view {};
  
  // The name of the Bible.
  string bible = access_bible::clamp (request, request->query ["bible"]);
  view.set_variable ("bible", escape_special_xml_characters (bible));

  // The order the user wants for the Bibles.
  string order = request->query ["order"];
  
  // Deuterocanonicals or Apocrypha interspersed among the books of the Hebrew Bible.
  if (order == "interspersed") {
    vector <book_id> interspersed = {
      book_id::_front_matter,
      book_id::_introduction_matter,
      book_id::_genesis,
      book_id::_exodus,
      book_id::_leviticus,
      book_id::_numbers,
      book_id::_deuteronomy,
      book_id::_joshua,
      book_id::_judges,
      book_id::_ruth,
      book_id::_1_samuel,
      book_id::_2_samuel,
      book_id::_1_kings,
      book_id::_2_kings,
      book_id::_1_chronicles,
      book_id::_2_chronicles,
      book_id::_ezra,
      book_id::_nehemiah,
      book_id::_tobit,
      book_id::_judith,
      book_id::_esther,
      book_id::_1_maccabees,
      book_id::_2_maccabees,
      book_id::_job,
      book_id::_psalms,
      book_id::_proverbs,
      book_id::_ecclesiastes,
      book_id::_song_of_solomon,
      book_id::_wisdom_of_solomon,
      book_id::_sirach,
      book_id::_isaiah,
      book_id::_jeremiah,
      book_id::_lamentations,
      book_id::_baruch,
      book_id::_ezekiel,
      book_id::_daniel,
      book_id::_hosea,
      book_id::_joel,
      book_id::_amos,
      book_id::_obadiah,
      book_id::_jonah,
      book_id::_micah,
      book_id::_nahum,
      book_id::_habakkuk,
      book_id::_zephaniah,
      book_id::_haggai,
      book_id::_zechariah,
      book_id::_malachi,
    };
    vector <string> v_book_order {};
    for (const auto book : interspersed) v_book_order.push_back (convert_to_string (static_cast<int>(book)));
    string s_book_order = filter_string_implode (v_book_order, " ");
    Database_Config_Bible::setBookOrder (bible, s_book_order);
  }

  
  // Deuterocanonicals or Apocrypha between the Hebrew Bible and the New Testament.
  if (order == "between") {
    vector <book_id> interspersed = {
      book_id::_front_matter,
      book_id::_introduction_matter,
      book_id::_genesis,
      book_id::_exodus,
      book_id::_leviticus,
      book_id::_numbers,
      book_id::_deuteronomy,
      book_id::_joshua,
      book_id::_judges,
      book_id::_ruth,
      book_id::_1_samuel,
      book_id::_2_samuel,
      book_id::_1_kings,
      book_id::_2_kings,
      book_id::_1_chronicles,
      book_id::_2_chronicles,
      book_id::_ezra,
      book_id::_nehemiah,
      book_id::_esther,
      book_id::_job,
      book_id::_psalms,
      book_id::_proverbs,
      book_id::_ecclesiastes,
      book_id::_song_of_solomon,
      book_id::_isaiah,
      book_id::_jeremiah,
      book_id::_lamentations,
      book_id::_ezekiel,
      book_id::_daniel,
      book_id::_hosea,
      book_id::_joel,
      book_id::_amos,
      book_id::_obadiah,
      book_id::_jonah,
      book_id::_micah,
      book_id::_nahum,
      book_id::_habakkuk,
      book_id::_zephaniah,
      book_id::_haggai,
      book_id::_zechariah,
      book_id::_malachi,
      book_id::_tobit,
      book_id::_judith,
      book_id::_1_maccabees,
      book_id::_2_maccabees,
      book_id::_wisdom_of_solomon,
      book_id::_sirach,
      book_id::_baruch,
    };
    vector <string> v_book_order {};
    for (const auto book : interspersed) v_book_order.push_back (convert_to_string (static_cast<int>(book)));
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
    string bookname = database::books::get_english_from_id (static_cast<book_id>(books[i]));
    bookname = translate (bookname);
    view.add_iteration ("order", { pair ("offset", convert_to_string (i)), pair ("bookname", bookname) } );
  }

  view.set_variable ("uparrow", unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", unicode_black_down_pointing_triangle ());

  page += view.render ("bb", "order");
  
  page += assets_page::footer ();
  
  return page;
}
