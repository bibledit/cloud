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


#pragma once

#include <config/libraries.h>
#include <filter/passage.h>

class Webserver_Request;

namespace navigation_passage {

std::string get_mouse_navigator (Webserver_Request& webserver_request, const std::string& bible);
std::string get_books_fragment (Webserver_Request& webserver_request, const std::string& bible);
std::string get_chapters_fragment (const std::string& bible, const int book, const int chapter);
std::string get_verses_fragment (const std::string& bible, const int book, const int chapter, const int verse);
std::string code (const std::string& bible);
void set_book (Webserver_Request& webserver_request, const int book);
void set_chapter (Webserver_Request& webserver_request, const int chapter);
void set_verse (Webserver_Request& webserver_request, const int verse);
void set_passage (Webserver_Request& webserver_request, const std::string& bible, std::string passage);
void goto_next_book (Webserver_Request& webserver_request, const std::string& bible);
void goto_previous_book (Webserver_Request& webserver_request, const std::string& bible);
void goto_next_chapter (Webserver_Request& webserver_request, const std::string& bible);
void goto_previous_chapter (Webserver_Request& webserver_request, const std::string& bible);
void goto_next_verse (Webserver_Request& webserver_request, const std::string& bible);
void goto_previous_verse (Webserver_Request& webserver_request, const std::string& bible);
void record_history (Webserver_Request& webserver_request, const int book, const int chapter, const int verse);
void go_back (Webserver_Request& webserver_request);
void go_forward (Webserver_Request& webserver_request);
std::string get_keyboard_navigator (Webserver_Request& webserver_request, const std::string& bible);
void interpret_keyboard_navigator (Webserver_Request& webserver_request, std::string bible, std::string passage);
std::string get_history_back (Webserver_Request& webserver_request);
std::string get_history_forward (Webserver_Request& webserver_request);
void go_history (Webserver_Request& webserver_request, std::string message);

}
