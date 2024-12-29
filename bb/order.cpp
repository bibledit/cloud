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


std::string bible_order_url ()
{
  return "bible/order";
}


bool bible_order_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


std::string bible_order (Webserver_Request& webserver_request)
{
  std::string page {};

  Assets_Header header = Assets_Header (translate("Order"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view {};
  
  // The name of the Bible.
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.query ["bible"]);
  view.set_variable ("bible", filter::strings::escape_special_xml_characters (bible));

  // The order the user wants for the Bibles.
  const std::string order = webserver_request.query ["order"];
  
  // Deuterocanonicals or Apocrypha interspersed among the books of the Hebrew Bible.
  if (order == "interspersed") {
    const  std::vector <book_id> interspersed = {
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
    std::vector <std::string> v_book_order {};
    for (const auto book : interspersed) v_book_order.push_back (std::to_string (static_cast<int>(book)));
    const std::string s_book_order = filter::strings::implode (v_book_order, " ");
    database::config::bible::set_book_order (bible, s_book_order);
  }

  
  // Deuterocanonicals or Apocrypha between the Hebrew Bible and the New Testament.
  if (order == "between") {
    const std::vector <book_id> interspersed = {
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
    std::vector <std::string> v_book_order {};
    for (const auto book : interspersed) v_book_order.push_back (std::to_string (static_cast<int>(book)));
    const std::string s_book_order = filter::strings::implode (v_book_order, " ");
    database::config::bible::set_book_order (bible, s_book_order);
  }

  
  // Deuterocanonicals or Apocrypha at the end of the entire Bible.
  if (order == "end") {
    database::config::bible::set_book_order (bible, std::string());
  }

  // Handle updates to the custom book order.
  const std::string moveup = webserver_request.query ["moveup"];
  const std::string movedown = webserver_request.query ["movedown"];
  if (!moveup.empty () || !movedown.empty ()) {
    size_t move = static_cast<size_t>(filter::strings::convert_to_int (moveup + movedown));
    const std::vector <int> books = filter_passage_get_ordered_books (bible);
    std::vector <std::string> s_books;
    for (const auto& book : books)
      s_books.push_back (std::to_string (book));
    filter::strings::array_move_up_down (s_books, move, !moveup.empty ());
    const std::string s_order = filter::strings::implode (s_books, " ");
    database::config::bible::set_book_order (bible, s_order);
  }
  
  const std::vector <int> books = filter_passage_get_ordered_books (bible);
  for (size_t i = 0; i < books.size (); i++) {
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(books[i]));
    bookname = translate (bookname);
    view.add_iteration ("order", { std::pair ("offset", std::to_string (i)), std::pair ("bookname", bookname) } );
  }

  view.set_variable ("uparrow", filter::strings::unicode_black_up_pointing_triangle ());
  view.set_variable ("downarrow", filter::strings::unicode_black_down_pointing_triangle ());

  page += view.render ("bb", "order");
  
  page += assets_page::footer ();
  
  return page;
}
