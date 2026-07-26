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

std::string direction_unspecified (int value);
std::string direction_left_to_right (int value);
std::string direction_right_to_left (int value);
std::string ltr ();
std::string rtl ();
int direction_value (const std::string& direction);
std::string writing_mode_unspecified (int value);
std::string writing_mode_top_bottom_left_right (int value);
std::string writing_mode_top_bottom_right_left (int value);
std::string writing_mode_bottom_top_left_right (int value);
std::string writing_mode_bottom_top_right_left (int value);
std::string tb_lr ();
std::string tb_rl ();
std::string bt_lr ();
std::string bt_rl ();
int writing_mode_value (const std::string& mode);
std::string get_class (const std::string& bible);
std::string get_css(const std::string& class_,
                    std::string font,
                    const std::string& focused_verse_font,
                    int direction_value, int line_height = 100, int letter_spacing = 0);
std::string distinction_set_light (int item_style_index);
std::string distinction_set_dark (int item_style_index);
std::string distinction_set_red_blue_light (int item_style_index);
std::string distinction_set_red_blue_dark (int item_style_index);
std::string distinction_set_notes (int item_style_index);
std::string theme_picker (int theme_style_index, int item_style_index);
std::string grey_background ();

}

