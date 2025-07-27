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


#include <bb/book.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/versifications.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/mappings.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <dialog/books.h>
#include <access/bible.h>
#include <book/create.h>
#include <bb/logic.h>
#include <client/logic.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <bb/manage.h>


std::string bible_book_url ()
{
  return "bible/book";
}


bool bible_book_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string bible_book (Webserver_Request& webserver_request)
{
  std::string page {};
  
  Assets_Header header = Assets_Header (translate("Book"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view {};
  
  std::string success_message {};
  std::string error_message {};
  
  // The name of the Bible.
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.query["bible"]);
  view.set_variable ("bible", filter::strings::escape_special_xml_characters (bible));
  
  // The book.
  const int book = filter::strings::convert_to_int (webserver_request.query ["book"]);
  view.set_variable ("book", std::to_string (book));
  const std::string book_name = database::books::get_english_from_id (static_cast<book_id>(book));
  view.set_variable ("book_name", filter::strings::escape_special_xml_characters (book_name));
  
  // Whether the user has write access to this Bible book.
  const bool write_access = access_bible::book_write (webserver_request, std::string(), bible, book);
  if (write_access) view.enable_zone ("write_access");
  
  // Delete chapter.
  const std::string deletechapter = webserver_request.query ["deletechapter"];
  if (!deletechapter.empty()) {
    const std::string confirm = webserver_request.query ["confirm"];
    if (confirm.empty()) {
      Dialog_Yes dialog_yes = Dialog_Yes ("book", translate("Would you like to delete this chapter?"));
      dialog_yes.add_query ("bible", bible);
      dialog_yes.add_query ("book", std::to_string (book));
      dialog_yes.add_query ("deletechapter", deletechapter);
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      if (write_access) bible_logic::delete_chapter (bible, book, filter::strings::convert_to_int (deletechapter));
    }
  }
  
  // Add chapter.
  if (webserver_request.query.count ("createchapter")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("book", translate("Please enter the number for the new chapter"), "", "createchapter", "");
    dialog_entry.add_query ("bible", bible);
    dialog_entry.add_query ("book", std::to_string (book));
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("createchapter")) {
    const int createchapter = filter::strings::convert_to_int (webserver_request.post ["entry"]);
    const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    // Only create the chapters if it does not yet exist.
    if (find (chapters.begin(), chapters.end(), createchapter) == chapters.end()) {
      std::vector <std::string> feedback{};
      bool result {true};
      if (write_access) result = book_create (bible, static_cast<book_id>(book), createchapter, feedback);
      const std::string message = filter::strings::implode (feedback, " ");
      if (result) success_message = message;
      else error_message = message;
    } else {
      error_message = translate ("This chapter already exists");
    }
  }
  
  // Available chapters.
  const std::vector <int> chapters = database::bibles::get_chapters (bible, book);
  std::string chapterblock {};
  for (const auto& chapter : chapters) {
    chapterblock.append (R"(<a href="chapter?bible=)");
    chapterblock.append (bible);
    chapterblock.append ("&book=");
    chapterblock.append (std::to_string (book));
    chapterblock.append ("&chapter=");
    chapterblock.append (std::to_string (chapter));
    chapterblock.append (R"(">)");
    chapterblock.append (std::to_string (chapter));
    chapterblock.append ("</a>\n");
  }
  view.set_variable ("chapterblock", chapterblock);
  
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  
  if (!client_logic_client_enabled ()) view.enable_zone ("server");

  page += view.render ("bb", "book");
  
  page += assets_page::footer ();
  
  return page;
}
