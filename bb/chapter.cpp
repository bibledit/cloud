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
#include <dialog/list.h>
#include <dialog/books.h>
#include <access/bible.h>
#include <book/create.h>
#include <client/logic.h>
using namespace std;


string bible_chapter_url ()
{
  return "bible/chapter";
}


bool bible_chapter_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string bible_chapter (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  string page {};
  
  page = assets_page::header (translate ("Chapter"), webserver_request);
  
  Assets_View view {};
  
  string success_message {};
  string error_message {};
  
  // The name of the Bible.
  string bible = access_bible::clamp (request, request->query["bible"]);
  view.set_variable ("bible", escape_special_xml_characters (bible));
  
  // The book.
  int book = convert_to_int (request->query ["book"]);
  view.set_variable ("book", convert_to_string (book));
  string book_name = database::books::get_english_from_id (static_cast<book_id>(book));
  view.set_variable ("book_name", escape_special_xml_characters (book_name));
  
  // The chapter.
  string chapter = request->query ["chapter"];
  view.set_variable ("chapter", escape_special_xml_characters (chapter));
  
  // Whether the user has write access to this Bible book.
  if (bool write_access = access_bible::book_write (request, string(), bible, book); write_access) view.enable_zone ("write_access");
  
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  
  if (!client_logic_client_enabled ()) view.enable_zone ("server");

  page += view.render ("bb", "chapter");
  
  page += assets_page::footer ();
  
  return page;
}
