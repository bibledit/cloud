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

namespace filter::css {

std::string directionUnspecified (int value);
std::string directionLeftToRight (int value);
std::string directionRightToLeft (int value);
std::string ltr ();
std::string rtl ();
int directionValue (std::string direction);
std::string writingModeUnspecified (int value);
std::string writingModeTopBottomLeftRight (int value);
std::string writingModeTopBottomRightLeft (int value);
std::string writingModeBottomTopLeftRight (int value);
std::string writingModeBottomTopRightLeft (int value);
std::string tb_lr ();
std::string tb_rl ();
std::string bt_lr ();
std::string bt_rl ();
int writingModeValue (std::string mode);
std::string getClass (std::string bible);
std::string get_css (std::string class_, std::string font, int directionvalue, int lineheight = 100, int letterspacing = 0);
void distinction_set_basic ();
std::string distinction_set_light (int itemstyleindex);
std::string distinction_set_dark (int itemstyleindex);
std::string distinction_set_redblue_light (int itemstyleindex);
std::string distinction_set_redblue_dark (int itemstyleindex);
std::string distinction_set_notes (int itemstyleindex);
std::string theme_picker (int themestyleindex, int itemstyleindex);
std::string grey_background ();

}

