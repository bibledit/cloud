/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


#ifndef INCLUDED_NAVIGATION_PASSAGE_H
#define INCLUDED_NAVIGATION_PASSAGE_H


#include <config/libraries.h>
#include <filter/passage.h>


class Navigation_Passage
{
public:
  static string get_mouse_navigator (void * webserver_request, string bible);
  static string get_books_fragment (void * webserver_request, string bible);
  static string get_chapters_fragment (void * webserver_request, string bible, int book, int chapter);
  static string get_verses_fragment (void * webserver_request, string bible, int book, int chapter, int verse);
  static string code (string bible);
  static void set_book (void * webserver_request, int book);
  static void set_chapter (void * webserver_request, int chapter);
  static void set_verse (void * webserver_request, int verse);
  static void set_passage (void * webserver_request, string bible, string passage);
  static void goto_next_chapter (void * webserver_request, string bible);
  static void goto_previous_chapter (void * webserver_request, string bible);
  static void goto_next_verse (void * webserver_request, string bible);
  static void goto_previous_verse (void * webserver_request, string bible);
  static void record_history (void * webserver_request, int book, int chapter, int verse);
  static void go_back (void * webserver_request);
  static void go_forward (void * webserver_request);
  static string get_keyboard_navigator (void * webserver_request, string bible);
  static void interpret_keyboard_navigator (void * webserver_request, string bible, string passage);
  static string get_history_back (void * webserver_request);
  static string get_history_forward (void * webserver_request);
  static void go_history (void * webserver_request, string message);
private:
  static void add_selector_link (string& html, string id, string href, string text, bool selected, string extra_class);
  static Passage get_next_chapter (void * webserver_request, string bible, int book, int chapter);
  static Passage get_previous_chapter (void * webserver_request, string bible, int book, int chapter);
  static Passage get_next_verse (void * webserver_request, string bible, int book, int chapter, int verse);
  static Passage get_previous_verse (void * webserver_request, string bible, int book, int chapter, int verse);
};


#endif
