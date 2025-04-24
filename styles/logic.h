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


std::string styles_logic_standard_sheet ();
std::string styles_logic_category_text (std::string category);
std::string styles_logic_type_text (int type);
std::string styles_logic_subtype_text (int type, int subtype);
bool styles_logic_fontsize_is_relevant (int type);
bool styles_logic_italic_bold_underline_smallcaps_are_relevant (int type);
bool styles_logic_italic_bold_underline_smallcaps_are_full (int type);
std::string styles_logic_off_on_inherit_toggle_text (int value);
bool styles_logic_superscript_is_relevant (int type);
bool styles_logic_paragraph_treats_are_relevant (int type);
std::string styles_logic_alignment_text (int value);
bool styles_logic_columns_are_relevant (int type, int subtype);
bool styles_logic_color_is_relevant (int type);
bool styles_logic_print_is_relevant ();
int styles_logic_get_userbool1_function ();
std::string styles_logic_get_userbool1_text (int function);
int styles_logic_get_userbool2_function (int type, int subtype);
std::string styles_logic_get_userbool2_text (int function);
int styles_logic_get_userbool3_function (int type, int subtype);
std::string styles_logic_get_userbool3_text (int function);
int styles_logic_get_userint1_function ();
std::string styles_logic_note_numbering_text (int value);
int styles_logic_get_userint2_function ();
std::string styles_logic_note_restart_numbering_text (int value);
std::string styles_logic_end_note_position_text (int value);
int styles_logic_get_userint3_function (int type, int subtype);
int styles_logic_get_userstring1_function (int type);
int styles_logic_get_userstring2_function (int type, int subtype);
int styles_logic_get_userstring3_function (int type, int subtype);
bool styles_logic_starts_new_line_in_usfm (int type, int subtype);
