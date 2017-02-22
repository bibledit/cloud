/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#ifndef INCLUDED_SEARCH_LOGIC_H
#define INCLUDED_SEARCH_LOGIC_H


#include <config/libraries.h>
#include <filter/passage.h>


string search_logic_index_folder ();
string search_logic_bible_fragment (string bible);
string search_logic_book_fragment (string bible, int book);
string search_logic_chapter_file (string bible, int book, int chapter);
void search_logic_index_chapter (string bible, int book, int chapter);
vector <Passage> search_logic_search_text (string search, vector <string> bibles);
vector <Passage> search_logic_search_bible_text (string bible, string search);
vector <Passage> search_logic_search_bible_text_case_sensitive (string bible, string search);
vector <Passage> search_logic_search_bible_usfm (string bible, string search);
vector <Passage> search_logic_search_bible_usfm_case_sensitive (string bible, string search);
string search_logic_get_bible_verse_text (string bible, int book, int chapter, int verse);
string search_logic_get_bible_verse_usfm (string bible, int book, int chapter, int verse);
void search_logic_delete_bible (string bible);
void search_logic_delete_book (string bible, int book);
void search_logic_delete_chapter (string bible, int book, int chapter);
int search_logic_get_verse_count (string bible);
void search_logic_copy_bible (string original, string destination);


#endif
