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
#include <filter/string.hpp>

class Webserver_Request;

namespace filter::string {

std::vector <std::string> explode (const std::string& value, char delimiter);
std::vector <std::string> explode (std::string value, const std::string& delimiters);
std::string implode (const std::vector <std::string>& values, std::string delimiter);
void implode_from_beginning_remain_with_max_n_bits (std::vector<std::string>& input, const int n, const std::string& joiner);
std::string replace (const std::string& search, const std::string& replace, std::string subject, int * count = nullptr);
bool replace_between (std::string& line, const std::string& start, const std::string& end, const std::string& replacement);
std::string convert_to_string (const char * c);
std::string convert_to_string (const bool b);
std::string convert_to_string (const std::string& s);
std::string convert_to_string (const float f, const int precision = 0);
int convert_to_int (const std::string& s);
int convert_to_int (const float f);
long long convert_to_long_long (const std::string& s);
float convert_to_float (const std::string& s);
bool convert_to_bool (const std::string& s);
std::string convert_to_true_false (const bool b);
std::u16string convert_to_u16string (const std::string& s);
std::vector <std::string> array_unique (const std::vector <std::string>& values);
std::vector <int> array_unique (const std::vector <int>& values);
std::string trim (const std::string& s);
std::string ltrim (const std::string& s);
std::string rtrim (std::string s);
std::string fill (const std::string& s, const int width, const char fill);
bool is_numeric (const std::string& s);
std::string escape_special_xml_characters (std::string s);
std::string unescape_special_xml_characters (std::string s);
std::string any_space_to_standard_space (std::string s);
std::string non_breaking_space_u00A0 ();
std::string soft_hyphen_u00AD ();
std::string en_space_u2002 ();
std::string figure_space_u2007 ();
std::string narrow_non_breaking_space_u202F ();
size_t unicode_string_length (const std::string& s);
std::string unicode_string_substr (std::string s, size_t pos = 0, size_t len = 0);
size_t unicode_string_strpos (const std::string& haystack, const std::string& needle, const size_t offset = 0);
size_t unicode_string_strpos_case_insensitive (std::string haystack, std::string needle, size_t offset = 0);
std::string unicode_string_casefold (const std::string& s);
std::string unicode_string_uppercase (const std::string& s);
std::string unicode_string_transliterate (const std::string& s);
bool unicode_string_is_valid (const std::string& s);
bool unicode_string_is_punctuation (std::string s);
int unicode_string_convert_to_codepoint (std::string s);
std::string unicode_string_str_replace (const std::string& search, const std::string& replace, std::string subject);
#ifdef HAVE_ICU
std::string icu_string_normalize (const std::string& s, const bool remove_diacritics, const bool casefold);
#endif
int rand (const int floor, const int ceiling);
std::string html2text (std::string html);
std::string extract_email (std::string input);
std::string extract_body (const std::string& input, const std::string year = "", const std::string sender = "");
std::string get_checkbox_status (const bool enabled);
std::string get_disabled (const bool disabled = true);
std::string get_reload ();
void quick_sort (std::vector <unsigned int>& one, std::vector <std::string>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <std::string>& one, std::vector <unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <unsigned int>& one, std::vector <unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <unsigned int>& one, std::vector <bool>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <int>& one, std::vector <unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <std::string>& one, std::vector <std::string>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <std::string>& one, std::vector <bool>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <std::string>& one, unsigned int beg, unsigned int end);
void quick_sort (std::vector <long unsigned int>& one, std::vector <long unsigned int>& two, unsigned int beg, unsigned int end);
void quick_sort (std::vector <int> & one, std::vector <int> & two, unsigned int beg, unsigned int end);
std::string number_in_string (const std::string & str);
std::string markup_words (const std::vector <std::string>& words, std::string text);
std::vector <std::string> search_needles (const std::string& search, const std::string& text);
int user_identifier (Webserver_Request& webserver_request);
std::string bin2hex (const std::string& bin);
std::string hex2bin (const std::string& hex);
std::string html_tidy (std::string html);
std::string html2xml (std::string html);
std::string convert_xml_character_entities_to_characters (std::string data);
std::string encrypt_decrypt (std::string key, std::string data);
std::string get_new_random_string ();
std::string unicode_non_breaking_space_entity ();
std::string unicode_black_up_pointing_triangle ();
std::string unicode_black_right_pointing_triangle ();
std::string unicode_black_down_pointing_triangle ();
std::string unicode_black_left_pointing_triangle ();
std::string emoji_black_right_pointing_triangle ();
std::string emoji_file_folder ();
std::string emoji_open_book ();
std::string emoji_wastebasket ();
std::string emoji_smiling_face_with_smiling_eyes ();
std::string emoji_heavy_plus_sign ();
void array_move_up_down (std::vector <std::string>& container, const size_t item, const bool up);
void array_move_from_to (std::vector <std::string>& container, size_t from, size_t to);
const char * english ();
#ifdef HAVE_WINDOWS
std::wstring string2wstring (const std::string& str);
std::string wstring2string (const std::wstring& wstr);
#endif
std::string lf2crlf (std::string str);
std::string crlf2lf (std::string str);
std::string html_get_element (std::string html, std::string element);
std::string fix_invalid_html_gumbo (std::string html);
std::string fix_invalid_html_tidy (std::string html);
std::string collapse_whitespace (std::string s);
std::string convert_windows1252_to_utf8 (const std::string& input);
void hex_dump (const void* addr, const int len);

}
