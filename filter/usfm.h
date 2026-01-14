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

class Webserver_Request;

namespace filter::usfm {

struct BookChapterData
{
  BookChapterData (const int book, const int chapter, std::string data);
  int m_book { 0 };
  int m_chapter { 0 };
  std::string m_data {};
};

std::string one_string (const std::string& usfm);
std::vector <std::string> get_markers_and_text (std::string code);
std::string get_marker (std::string usfm);
std::vector <BookChapterData> usfm_import (std::string input, std::string stylesheet);
std::vector <int> get_verse_numbers (std::string usfm);
std::vector <int> get_chapter_numbers (std::string usfm);
std::vector <int> linenumber_to_versenumber (std::string usfm, unsigned int line_number);
std::vector <int> offset_to_versenumber (std::string usfm, unsigned int offset);
int versenumber_to_offset (std::string usfm, int verse);
std::string get_verse_text (std::string usfm, int verse);
std::string get_verse_text_quill (const std::optional<int> chapter, const int verse, std::string usfm);
std::string get_chapter_text (std::string usfm, int chapter_number);
std::string get_verse_range_text (std::string usfm, int verse_from, int verse_to, const std::string& exclude_usfm, bool quill);
bool is_usfm_marker (std::string code);
bool is_opening_marker (std::string usfm);
bool is_embedded_marker (std::string usfm);
std::string get_book_identifier (const std::vector <std::string>& usfm, unsigned int pointer);
std::string get_text_following_marker (const std::vector <std::string>& usfm, unsigned int & pointer);
std::string peek_text_following_marker (const std::vector <std::string>& usfm, unsigned int pointer);
std::string peek_verse_number (std::string usfm);
std::string get_opening_usfm (std::string text, bool embedded = false);
std::string get_closing_usfm (std::string text, bool embedded = false);
std::string save_is_safe (Webserver_Request& webserver_request, 
                          std::string oldtext, std::string newtext, bool chapter, std::string & explanation);
std::string safely_store_chapter (Webserver_Request& webserver_request,
                                  const std::string& bible, int book, int chapter, const std::string& usfm, std::string & explanation);
std::string safely_store_verse (Webserver_Request& webserver_request,
                                const std::string& bible, int book, int chapter, int verse, std::string usfm,
                                std::string & explanation, bool quill);
bool contains_empty_verses (std::string usfm);
bool handle_verse_range (std::string verse, std::vector <int> & verses);
bool handle_verse_sequence (std::string verse, std::vector <int> & verses);
const char * marker_v ();
const char * marker_va ();
const char * marker_vp ();
void remove_word_level_attributes (const std::string& marker,
                                   std::vector <std::string> & container, const unsigned int pointer);
std::string extract_fig (std::string usfm, std::string & caption, std::string & alt, std::string& src, std::string& size, std::string& loc, std::string& copy, std::string& ref);
bool is_standard_q_poetry (const std::string & marker);
std::string remove_milestone (std::vector <std::string>& container, unsigned int& pointer);
std::string transpose_opening_marker_and_space_sequence(std::string usfm);

}

