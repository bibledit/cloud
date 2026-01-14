/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <bb/chapter.h>
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
#include <client/logic.h>


std::string bible_chapter_url ()
{
  return "bible/chapter";
}


bool bible_chapter_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::manager);
}


std::string bible_chapter (Webserver_Request& webserver_request)
{
  std::string page = assets_page::header (translate ("Chapter"), webserver_request);
  
  Assets_View view {};
  
  std::string success_message {};
  std::string error_message {};
  
  // The name of the Bible.
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.query["bible"]);
  view.set_variable ("bible", filter::string::escape_special_xml_characters (bible));
  
  // The book.
  const int book = filter::string::convert_to_int (webserver_request.query ["book"]);
  view.set_variable ("book", std::to_string (book));
  const std::string book_name = database::books::get_english_from_id (static_cast<book_id>(book));
  view.set_variable ("book_name", filter::string::escape_special_xml_characters (book_name));
  
  // The chapter.
  const std::string chapter = webserver_request.query ["chapter"];
  view.set_variable ("chapter", filter::string::escape_special_xml_characters (chapter));
  
  // Whether the user has write access to this Bible book.
  if (bool write_access = access_bible::book_write (webserver_request, std::string(), bible, book); write_access) view.enable_zone ("write_access");
  
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  
  if (!client_logic_client_enabled ()) view.enable_zone ("server");

  page += view.render ("bb", "chapter");
  
  page += assets_page::footer ();
  
  return page;
}
