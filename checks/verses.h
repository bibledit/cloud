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

namespace checks_verses {

void missing_punctuation_at_end (const std::string & bible, int book, int chapter,
                                 const std::map <int, std::string> & verses,
                                 const std::string & center_marks, const std::string & end_marks,
                                 const std::string & disregards);
void patterns (const std::string & bible, int book, int chapter,
               const std::map <int, std::string> & verses, const std::vector <std::string> & patterns);

}
