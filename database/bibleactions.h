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

namespace database::bible_actions {

void create ();
void clear ();
void optimize ();
void record (std::string bible, int book, int chapter, std::string usfm);
std::vector <std::string> get_bibles ();
std::vector <int> get_books (std::string bible);
std::vector <int> get_chapters (std::string bible, int book);
std::string get_usfm (std::string bible, int book, int chapter);
void erase (std::string bible, int book, int chapter);

}
