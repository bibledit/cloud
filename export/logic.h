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


namespace export_logic {

void schedule_all ();
void schedule_text_and_basic_usfm (string bible, bool log);
void schedule_usfm (string bible, bool log);
void schedule_open_document (string bible, bool log);
void schedule_info (string bible, bool log);
void schedule_html (string bible, bool log);
void schedule_web (string bible, bool log);
void schedule_web_index (string bible, bool log);
void schedule_online_bible (string bible, bool log);
void schedule_e_sword (string bible, bool log);
string main_directory ();
string bible_directory (string bible);
string usfm_directory (string bible, int type);
string web_directory (string bible);
string web_back_link_directory (string bible);
string base_book_filename (int book);
constexpr int export_needed { 0 };
constexpr int export_text_and_basic_usfm { 1 };
constexpr int export_full_usfm { 2 };
constexpr int export_opendocument { 3 };
constexpr int export_info { 4 };
constexpr int export_html { 5 };
constexpr int export_web { 6 };
constexpr int export_web_index { 7 };
constexpr int export_online_bible { 8 };
constexpr int export_esword { 9 };
constexpr int export_end { 10 };

}; // End of namespace.


