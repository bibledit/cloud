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

std::string search_logic_index_folder ();
std::string search_logic_bible_fragment (std::string bible);
std::string search_logic_book_fragment (std::string bible, int book);
std::string search_logic_chapter_file (std::string bible, int book, int chapter);
void search_logic_index_chapter (std::string bible, int book, int chapter);
std::vector <Passage> search_logic_search_text (std::string search, std::vector <std::string> bibles);
std::vector <Passage> search_logic_search_bible_text (std::string bible, std::string search);
std::vector <Passage> search_logic_search_bible_text_case_sensitive (std::string bible, std::string search);
std::vector <Passage> search_logic_search_bible_usfm (std::string bible, std::string search);
std::vector <Passage> search_logic_search_bible_usfm_case_sensitive (std::string bible, std::string search);
std::string search_logic_get_bible_verse_text (std::string bible, int book, int chapter, int verse);
std::string search_logic_get_bible_verse_usfm (std::string bible, int book, int chapter, int verse);
void search_logic_delete_bible (std::string bible);
void search_logic_delete_book (std::string bible, int book);
void search_logic_delete_chapter (std::string bible, int book, int chapter);
int search_logic_get_verse_count (std::string bible);
void search_logic_copy_bible (std::string original, std::string destination);
std::string search_logic_plain_replace_verse_text (std::string usfm);
