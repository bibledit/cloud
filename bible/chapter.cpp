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


#include <bible/chapter.h>
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
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  string page;
  
  page = Assets_Page::header (translate ("Chapter"), webserver_request);
  
  Assets_View view;
  
  string success_message;
  string error_message;
  
  // The name of the Bible.
  string bible = access_bible_clamp (request, request->query["bible"]);
  view.set_variable ("bible", filter_string_sanitize_html (bible));
  
  // The book.
  int book = convert_to_int (request->query ["book"]);
  view.set_variable ("book", convert_to_string (book));
  string book_name = Database_Books::getEnglishFromId (book);
  view.set_variable ("book_name", filter_string_sanitize_html (book_name));
  
  // The chapter.
  string chapter = request->query ["chapter"];
  view.set_variable ("chapter", filter_string_sanitize_html (chapter));
  
  // Whether the user has write access to this Bible book.
  bool write_access = access_bible_book_write (request, "", bible, book);
  if (write_access) view.enable_zone ("write_access");
  
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  
  if (!client_logic_client_enabled ()) view.enable_zone ("server");

  page += view.render ("bible", "chapter");
  
  page += Assets_Page::footer ();
  
  return page;
}
