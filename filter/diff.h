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

#include <string>
#include <vector>
using namespace std;

string filter_diff_diff (string oldstring, string newstring,
                         vector <string> * removals = nullptr,
                         vector <string> * additions = nullptr);
void filter_diff_diff_utf16 (const vector<string> & oldinput, const vector<string> & newinput,
                             vector <int> & positions,
                             vector <int> & sizes,
                             vector <bool> & additions,
                             vector <string> & content,
                             int & new_line_diff_count);
int filter_diff_character_similarity (string oldstring, string newstring);
int filter_diff_word_similarity (string oldstring, string newstring);
void filter_diff_produce_verse_level (string bible, string directory);
void filter_diff_run_file (string oldfile, string newfile, string outputfile);
