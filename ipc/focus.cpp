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


#include <ipc/focus.h>
#include <webserver/request.h>
#include <database/cache.h>
#include <filter/string.h>


namespace ipc_focus {

// Check whether the focusgroup was added to the query.
// If so return the passed group number.
// If the group number is out of bounds, return the default group 0.
// If the group was not given, return the default group number 0.
int get_focus_group(const Webserver_Request& webserver_request)
{
  if (!webserver_request.query.count(focusgroup))
    return 0;
  const int group = filter::string::convert_to_int(webserver_request.query.at(focusgroup));
  if ((group < 0) or (group > 9))
    return 0;
  return group;
}

// Set the book/chapter/verse for the focused group in the passed container.
// If the container is too small, it gets filled up with book/chapter/verse number 1.
static void store_value_for_focus_group(std::vector<int>& values, const int group, const int value)
{
  while (values.size() <= group)
    values.push_back(1);
  values.at(group) = value;
}

// Sets the focused passage.
void set_passage (Webserver_Request& webserver_request, const int book, const int chapter, const int verse)
{
  // The focus group number.
  const int group {get_focus_group(webserver_request)};
  // Only set book if it was changed. Same for chapter and verse.
//  const auto books = get_book(webserver_request);
  if (book != get_book(webserver_request)) {
    auto books = webserver_request.database_config_user()->get_focused_books();
    store_value_for_focus_group(books, group, book);
    webserver_request.database_config_user()->set_focused_books(books);
  }
  if (chapter != get_chapter(webserver_request)) {
    auto chapters = webserver_request.database_config_user()->get_focused_chapters();
    store_value_for_focus_group(chapters, group, chapter);
    webserver_request.database_config_user()->set_focused_chapters(chapters);
  }
  if (verse != get_verse(webserver_request)) {
    auto verses = webserver_request.database_config_user()->get_focused_verses();
    store_value_for_focus_group(verses, group, verse);
    webserver_request.database_config_user()->set_focused_verses(verses);
  }
}


// Gets the focused book.
int get_book (Webserver_Request& webserver_request)
{
  // The focus group number.
  const int group {get_focus_group(webserver_request)};
  const auto books = webserver_request.database_config_user()->get_focused_books();
  if (group < books.size())
    return books.at(group);
  // Default book.
  return 1;
}


// Gets the focused chapter.
int get_chapter (Webserver_Request& webserver_request)
{
  // The focus group number.
  const int group {get_focus_group(webserver_request)};
  const auto chapters = webserver_request.database_config_user()->get_focused_chapters();
  if (group < chapters.size())
    return chapters.at(group);
  // Default chapter.
  return 1;
}


// Gets the focused verse.
int get_verse (Webserver_Request& webserver_request)
{
  // The focus group number.
  const int group {get_focus_group(webserver_request)};
  const auto verses = webserver_request.database_config_user()->get_focused_verses();
  if (group < verses.size())
    return verses.at(group);
  // Default verse.
  return 1;
}


};
