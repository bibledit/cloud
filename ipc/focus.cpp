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


#include <ipc/focus.h>
#include <webserver/request.h>
#include <database/cache.h>
#include <filter/string.h>


namespace ipc_focus {


// Sets the focused passage.
void set_passage (Webserver_Request& webserver_request, const int book, const int chapter, const int verse)
{
  // Only set book if it was changed. Same for chapter and verse.
  if (book != get_book(webserver_request))
    webserver_request.database_config_user()->set_focused_book(book);
  if (chapter != get_chapter(webserver_request))
    webserver_request.database_config_user()->set_focused_chapter(chapter);
  if (verse != get_verse(webserver_request))
    webserver_request.database_config_user()->set_focused_verse(verse);
}


// Gets the focused book.
int get_book (Webserver_Request& webserver_request)
{
  return webserver_request.database_config_user()->get_focused_book();
}


// Gets the focused chapter.
int get_chapter (Webserver_Request& webserver_request)
{
  return webserver_request.database_config_user()->get_focused_chapter();
}


// Gets the focused verse.
int get_verse (Webserver_Request& webserver_request)
{
  return webserver_request.database_config_user()->get_focused_verse();
}


};
