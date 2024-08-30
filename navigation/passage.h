/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


#pragma once

#include <config/libraries.h>
#include <filter/passage.h>

class Webserver_Request;

class Navigation_Passage
{
public:
  static std::string get_mouse_navigator (Webserver_Request& webserver_request, std::string bible);
  static std::string get_books_fragment (Webserver_Request& webserver_request, std::string bible);
  static std::string get_chapters_fragment (std::string bible, int book, int chapter);
  static std::string get_verses_fragment (std::string bible, int book, int chapter, int verse);
  static std::string code (std::string bible);
  static void set_book (Webserver_Request& webserver_request, int book);
  static void set_chapter (Webserver_Request& webserver_request, int chapter);
  static void set_verse (Webserver_Request& webserver_request, int verse);
  static void set_passage (Webserver_Request& webserver_request, std::string bible, std::string passage);
  static void goto_next_chapter (Webserver_Request& webserver_request, std::string bible);
  static void goto_previous_chapter (Webserver_Request& webserver_request, std::string bible);
  static void goto_next_verse (Webserver_Request& webserver_request, std::string bible);
  static void goto_previous_verse (Webserver_Request& webserver_request, std::string bible);
  static void record_history (Webserver_Request& webserver_request, int book, int chapter, int verse);
  static void go_back (Webserver_Request& webserver_request);
  static void go_forward (Webserver_Request& webserver_request);
  static std::string get_keyboard_navigator (Webserver_Request& webserver_request, std::string bible);
  static void interpret_keyboard_navigator (Webserver_Request& webserver_request, std::string bible, std::string passage);
  static std::string get_history_back (Webserver_Request& webserver_request);
  static std::string get_history_forward (Webserver_Request& webserver_request);
  static void go_history (Webserver_Request& webserver_request, std::string message);
private:
  static void add_selector_link (std::string& html, std::string id, std::string href, std::string text, bool selected, std::string extra_class);
  static Passage get_next_chapter (std::string bible, int book, int chapter);
  static Passage get_previous_chapter (std::string bible, int book, int chapter);
  static Passage get_next_verse (std::string bible, int book, int chapter, int verse);
  static Passage get_previous_verse (std::string bible, int book, int chapter, int verse);
};
