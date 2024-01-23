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

namespace checks::space {

void double_space_usfm (const std::string & bible, int book, int chapter, int verse, const std::string & data);
void space_before_punctuation (const std::string & bible, int book, int chapter, const std::map <int, std::string> & texts);
void space_end_verse (const std::string & bible, int book, int chapter, const std::string & usfm);
bool transpose_note_space (std::string & usfm);
void space_before_final_note_markup (const std::string & bible, int book, int chapter, int verse, const std::string & data);

}

