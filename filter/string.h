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


#ifndef INCLUDED_FILTER_STRING_H
#define INCLUDED_FILTER_STRING_H


#include <config/libraries.h>
#include <filter/string.hpp>


vector <string> filter_string_explode (string value, char delimiter);
vector <string> filter_string_explode (string value, string delimiters);
string filter_string_implode (vector <string>& values, string delimiter);
string filter_string_str_replace (string search, string replace, string subject, int * count = NULL);
bool filter_string_replace_between (string& line, const string& start, const string& end, const string& replacement);
string substr_replace (string original, string replacement, size_t start, size_t length);
string convert_to_string (size_t i);
string convert_to_string (int i);
string convert_to_string (char * c);
string convert_to_string (const char * c);
string convert_to_string (bool b);
string convert_to_string (string s);
string convert_to_string (float f);
int convert_to_int (string s);
int convert_to_int (float f);
long long convert_to_long_long (string s);
float convert_to_float (string s);
bool convert_to_bool (string s);
string convert_to_true_false (bool b);
u16string convert_to_u16string (string s);
vector <string> array_unique (vector <string> values);
vector <int> array_unique (vector <int> values);
vector <string> filter_string_array_diff (vector <string> from, vector <string> against);
vector <int> filter_string_array_diff (vector <int> from, vector <int> against);
string filter_string_trim (string s);
string filter_string_ltrim (string s);
string filter_string_fill (string s, int width, char fill);
bool filter_string_is_numeric (string s);
string escape_special_xml_characters (string s);
string unescape_special_xml_characters (string s);
string any_space_to_standard_space (string s);
string non_breaking_space_u00A0 ();
string soft_hyphen_u00AD ();
string en_space_u2002 ();
string figure_space_u2007 ();
string narrow_non_breaking_space_u202F ();
size_t unicode_string_length (string s);
string unicode_string_substr (string s, size_t pos = 0, size_t len = 0);
size_t unicode_string_strpos (string haystack, string needle, size_t offset = 0);
size_t unicode_string_strpos_case_insensitive (string haystack, string needle, size_t offset = 0);
string unicode_string_casefold (string s);
string unicode_string_uppercase (string s);
string unicode_string_transliterate (string s);
bool unicode_string_is_valid (string s);
bool unicode_string_is_punctuation (string s);
int unicode_string_convert_to_codepoint (string s);
string unicode_string_str_replace (string search, string replace, string subject);
#ifdef HAVE_ICU
string icu_string_normalize (string s, bool remove_diacritics, bool casefold);
#endif
int filter_string_rand (int floor, int ceiling);
string filter_string_html2text (string html);
string filter_string_extract_email (string input);
string filter_string_extract_body (string input, string year = "", string sender = "");
string get_checkbox_status (bool enabled);
string get_disabled (bool disabled);
void quick_sort (vector <unsigned int>& one, vector <string>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <string>& one, vector <unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <unsigned int>& one, vector <unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <unsigned int>& one, vector <bool>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <int>& one, vector <unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <string>& one, vector <string>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <string>& one, vector <bool>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <string>& one, unsigned int beg, unsigned int end);
void quick_sort (vector <long unsigned int>& one, vector <long unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (vector <int> & one, vector <int> & two, unsigned int beg, unsigned int end);
string number_in_string (const string & str);
string filter_string_markup_words (const vector <string>& words, string text);
vector <string> filter_string_search_needles (string search, string text);
int filter_string_user_identifier (void * webserver_request);
string bin2hex (string bin);
string hex2bin (string hex);
string html_tidy (string html);
string html2xml (string html);
string convert_xml_character_entities_to_characters (string data);
string encrypt_decrypt (string key, string data);
string get_new_random_string ();
string unicode_non_breaking_space_entity ();
string unicode_black_up_pointing_triangle ();
string unicode_black_right_pointing_triangle ();
string unicode_black_down_pointing_triangle ();
string unicode_black_left_pointing_triangle ();
string emoji_black_right_pointing_triangle ();
string emoji_file_folder ();
string emoji_open_book ();
string emoji_wastebasket ();
string emoji_smiling_face_with_smiling_eyes ();
string emoji_heavy_plus_sign ();
void array_move_up_down (vector <string> & container, size_t item, bool up);
void array_move_from_to (vector <string> & container, size_t from, size_t to);
const char * english ();
#ifdef HAVE_WINDOWS
wstring string2wstring (const string& str);
string wstring2string (const wstring& wstr);
#endif
string lf2crlf (string str);
string crlf2lf (string str);
string filter_text_html_get_element (string html, string element);
string filter_string_tidy_invalid_html (string html);


#endif
