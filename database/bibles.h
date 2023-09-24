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


#pragma once

#include <config/libraries.h>

class Database_Bibles
{
public:
  std::vector <std::string> get_bibles ();
  void create_bible (const std::string& name);
  void delete_bible (const std::string& name);
  void store_chapter (const std::string& name, int book, int chapter_number, std::string chapter_text);
  void update_search_fields (const std::string& name, int book, int chapter);
  std::vector <int> get_books (const std::string& bible);
  void delete_book (const std::string& bible, int book);
  std::vector <int> get_chapters (const std::string& bible, int book);
  void delete_chapter (const std::string& bible, int book, int chapter);
  std::string get_chapter (const std::string& bible, int book, int chapter);
  int get_chapter_id (const std::string& bible, int book, int chapter);
  int get_chapter_age (const std::string& bible, int book, int chapter);
  void optimize ();
private:
  std::string main_folder ();
public:
  std::string bible_folder (const std::string& bible);
private:
  std::string book_folder (const std::string& bible, int book);
  std::string chapter_folder (const std::string& bible, int book, int chapter);
};
