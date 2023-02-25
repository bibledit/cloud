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
#include <filter/string.hpp>

std::vector <std::string> filter_string_explode (std::string value, char delimiter);
std::vector <std::string> filter_string_explode (std::string value, std::string delimiters);
std::string filter_string_implode (std::vector <std::string>& values, std::string delimiter);
std::string filter_string_str_replace (std::string search, std::string replace, std::string subject, int * count = nullptr);
bool filter_string_replace_between (std::string& line, const std::string& start, const std::string& end, const std::string& replacement);
std::string substr_replace (std::string original, std::string replacement, size_t start, size_t length);
std::string convert_to_string (size_t i);
std::string convert_to_string (int i);
std::string convert_to_string (char * c);
std::string convert_to_string (const char * c);
std::string convert_to_string (bool b);
std::string convert_to_string (std::string s);
std::string convert_to_string (float f);
int convert_to_int (std::string s);
int convert_to_int (float f);
long long convert_to_long_long (std::string s);
float convert_to_float (std::string s);
bool convert_to_bool (std::string s);
std::string convert_to_true_false (bool b);
std::u16string convert_to_u16string (std::string s);
std::vector <std::string> array_unique (std::vector <std::string> values);
std::vector <int> array_unique (std::vector <int> values);
std::vector <std::string> filter_string_array_diff (std::vector <std::string> from, std::vector <std::string> against);
std::vector <int> filter_string_array_diff (std::vector <int> from, std::vector <int> against);
std::string filter_string_trim (std::string s);
std::string filter_string_ltrim (std::string s);
std::string filter_string_rtrim (std::string s);
std::string filter_string_fill (std::string s, int width, char fill);
bool filter_string_is_numeric (std::string s);
std::string escape_special_xml_characters (std::string s);
std::string unescape_special_xml_characters (std::string s);
std::string any_space_to_standard_space (std::string s);
std::string non_breaking_space_u00A0 ();
std::string soft_hyphen_u00AD ();
std::string en_space_u2002 ();
std::string figure_space_u2007 ();
std::string narrow_non_breaking_space_u202F ();
size_t unicode_string_length (std::string s);
std::string unicode_string_substr (std::string s, size_t pos = 0, size_t len = 0);
size_t unicode_string_strpos (std::string haystack, std::string needle, size_t offset = 0);
size_t unicode_string_strpos_case_insensitive (std::string haystack, std::string needle, size_t offset = 0);
std::string unicode_string_casefold (std::string s);
std::string unicode_string_uppercase (std::string s);
std::string unicode_string_transliterate (std::string s);
bool unicode_string_is_valid (std::string s);
bool unicode_string_is_punctuation (std::string s);
int unicode_string_convert_to_codepoint (std::string s);
std::string unicode_string_str_replace (std::string search, std::string replace, std::string subject);
#ifdef HAVE_ICU
std::string icu_string_normalize (std::string s, bool remove_diacritics, bool casefold);
#endif
int filter_string_rand (int floor, int ceiling);
std::string filter_string_html2text (std::string html);
std::string filter_string_extract_email (std::string input);
std::string filter_string_extract_body (std::string input, std::string year = "", std::string sender = "");
std::string get_checkbox_status (bool enabled);
std::string get_disabled (bool disabled = true);
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
std::string filter_string_markup_words (const std::vector <std::string>& words, std::string text);
std::vector <std::string> filter_string_search_needles (std::string search, std::string text);
int filter_string_user_identifier (void * webserver_request);
std::string bin2hex (std::string bin);
std::string hex2bin (std::string hex);
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
void array_move_up_down (std::vector <std::string> & container, size_t item, bool up);
void array_move_from_to (std::vector <std::string> & container, size_t from, size_t to);
const char * english ();
#ifdef HAVE_WINDOWS
wstd::string std::string2wstring (const std::string& str);
std::string wstd::string2string (const wstd::string& wstr);
#endif
std::string lf2crlf (std::string str);
std::string crlf2lf (std::string str);
std::string filter_text_html_get_element (std::string html, std::string element);
std::string filter_string_fix_invalid_html_gumbo (std::string html);
std::string filter_string_fix_invalid_html_tidy (std::string html);
std::string filter_string_collapse_whitespace (std::string s);
std::string convert_windows1252_to_utf8 (const std::string& input);

