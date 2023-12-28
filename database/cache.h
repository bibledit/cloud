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

class Database_Cache
{
public:
  static void create (std::string resource, int book);
  static void remove (std::string resource);
  static void remove (std::string resource, int book);
  static bool exists (std::string resource);
  static bool exists (std::string resource, int book);
  static bool exists (std::string resource, int book, int chapter, int verse);
  static void cache (std::string resource, int book, int chapter, int verse, std::string value);
  static std::string retrieve (std::string resource, int book, int chapter, int verse);
  static int count (std::string resource);
  static bool ready (std::string resource, int book);
  static void ready (std::string resource, int book, bool ready);
  static int size (std::string resource, int book);
  static std::string fragment ();
  static std::string path (std::string resource, int book);
private:
  static std::string filename (std::string resource, int book);
};


bool database_filebased_cache_exists (std::string schema);
void database_filebased_cache_put (std::string schema, std::string contents);
std::string database_filebased_cache_get (std::string schema);
void database_filebased_cache_remove (std::string schema);
std::string database_filebased_cache_name_by_ip (std::string address, std::string id);
std::string database_filebased_cache_name_by_session_id (std::string sid, std::string id);


std::string focused_book_filebased_cache_filename (std::string sid);
std::string focused_chapter_filebased_cache_filename (std::string sid);
std::string focused_verse_filebased_cache_filename (std::string sid);
std::string general_font_size_filebased_cache_filename (std::string sid);
std::string menu_font_size_filebased_cache_filename (std::string sid);
std::string resource_font_size_filebased_cache_filename (std::string sid);
std::string hebrew_font_size_filebased_cache_filename (std::string sid);
std::string greek_font_size_filebased_cache_filename (std::string sid);
std::string current_theme_filebased_cache_filename (std::string sid);


void database_cache_trim (bool clear);


bool database_cache_can_cache (const std::string & error, const std::string & html);
