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


namespace export_logic {

void schedule_all ();
void schedule_text_and_basic_usfm (const std::string & bible, bool log);
void schedule_usfm (const std::string & bible, bool log);
void schedule_open_document (const std::string & bible, bool log);
void schedule_info (const std::string & bible, bool log);
void schedule_html (const std::string & bible, bool log);
void schedule_web (const std::string & bible, bool log);
void schedule_web_index (const std::string & bible, bool log);
void schedule_online_bible (const std::string & bible, bool log);
void schedule_e_sword (const std::string & bible, bool log);
std::string main_directory ();
std::string bible_directory (const std::string & bible);
std::string usfm_directory (const std::string & bible, int type);
std::string web_directory (const std::string & bible);
std::string web_back_link_directory (const std::string & bible);
std::string base_book_filename (const std::string & bible, int book);
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


