/*
Copyright (©) 2003-2022 Teus Benschop.

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

namespace filter::usfm {

class BookChapterData
{
public:
  BookChapterData (int book, int chapter, string data);
  int m_book { 0 };
  int m_chapter { 0 };
  string m_data {};
};

string one_string (string usfm);
vector <string> get_markers_and_text (string code);
string get_marker (string usfm);
vector <BookChapterData> usfm_import (string input, string stylesheet);
vector <int> get_verse_numbers (string usfm);
vector <int> get_chapter_numbers (string usfm);
vector <int> linenumber_to_versenumber (string usfm, unsigned int line_number);
vector <int> offset_to_versenumber (string usfm, unsigned int offset);
int versenumber_to_offset (string usfm, int verse);
string get_verse_text (string usfm, int verse);
string get_verse_text_quill (string usfm, int verse_number);
string get_chapter_text (string usfm, int chapter_number);
string get_verse_range_text (string usfm, int verse_from, int verse_to, const string& exclude_usfm, bool quill);
bool is_usfm_marker (string code);
bool is_opening_marker (string usfm);
bool is_embedded_marker (string usfm);
string get_book_identifier (const vector <string>& usfm, unsigned int pointer);
string get_text_following_marker (const vector <string>& usfm, unsigned int & pointer);
string peek_text_following_marker (const vector <string>& usfm, unsigned int pointer);
string peek_verse_number (string usfm);
string get_opening_usfm (string text, bool embedded = false);
string get_closing_usfm (string text, bool embedded = false);
string save_is_safe (void * webserver_request, string oldtext, string newtext, bool chapter, string & explanation);
string safely_store_chapter (void * webserver_request,
                             string bible, int book, int chapter, string usfm, string & explanation);
string safely_store_verse (void * webserver_request,
                           string bible, int book, int chapter, int verse, string usfm,
                           string & explanation, bool quill);
bool contains_empty_verses (string usfm);
bool handle_verse_range (string verse, vector <int> & verses);
bool handle_verse_sequence (string verse, vector <int> & verses);
const char * marker_v ();
const char * marker_va ();
const char * marker_vp ();
void remove_word_level_attributes (const string & marker,
                                   vector <string> & container, unsigned int & pointer);
string extract_fig (string usfm, string & caption, string & alt, string& src, string& size, string& loc, string& copy, string& ref);
bool is_standard_q_poetry (const string & marker);

}

