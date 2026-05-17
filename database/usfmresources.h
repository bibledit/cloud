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


#pragma once

#include <config/libraries.h>

class Database_UsfmResources
{
public:
    static std::vector<std::string> get_resources();
    static void delete_resource(const std::string& name);
    static void delete_book(const std::string& name, int book);
    static void delete_chapter(const std::string& name, int book, int chapter);
    static void store_chapter(const std::string& name, int book, int chapter, const std::string& usfm);
    static std::vector<int> get_books(const std::string& name);
    static std::vector<int> get_chapters(const std::string& name, int book);
    static std::string get_usfm(const std::string& name, int book, int chapter);
    static int get_size(const std::string& name, int book, int chapter);
};
