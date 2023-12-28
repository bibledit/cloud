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

std::string locale_logic_month (int month);
std::string locale_logic_date (int seconds);
std::string locale_logic_date_time (int seconds);
std::map <std::string, std::string> locale_logic_localizations ();
std::unordered_map <std::string, std::string> locale_logic_read_msgid_msgstr (std::string file);

std::string locale_logic_text_loaded ();
std::string locale_logic_text_will_save ();
std::string locale_logic_text_updating ();
std::string locale_logic_text_updated ();
std::string locale_logic_text_saving ();
std::string locale_logic_text_saved ();
std::string locale_logic_text_retrying ();
std::string locale_logic_text_reformat ();
std::string locale_logic_text_no_privileges_modify_book ();
std::string locale_logic_text_reload ();

std::string locale_logic_space_get_name (std::string space, bool english);

std::string locale_logic_deobfuscate (std::string value);
void locale_logic_obfuscate_initialize ();
