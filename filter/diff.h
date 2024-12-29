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

std::string filter_diff_diff (std::string oldstring, std::string newstring,
                              std::vector <std::string> * removals = nullptr,
                              std::vector <std::string> * additions = nullptr);
void filter_diff_diff_utf16 (const std::vector<std::string> & oldinput,
                             const std::vector<std::string> & newinput,
                             std::vector <int> & positions,
                             std::vector <int> & sizes,
                             std::vector <bool> & additions,
                             std::vector <std::string> & content,
                             int & new_line_diff_count);
int filter_diff_character_similarity (std::string oldstring, std::string newstring);
int filter_diff_word_similarity (std::string oldstring, std::string newstring);
void filter_diff_produce_verse_level (std::string bible, std::string directory);
void filter_diff_run_file (std::string oldfile, std::string newfile, std::string outputfile);
