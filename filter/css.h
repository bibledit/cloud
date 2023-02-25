/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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

class Filter_Css
{
public:
  static std::string directionUnspecified (int value);
  static std::string directionLeftToRight (int value);
  static std::string directionRightToLeft (int value);
  static std::string ltr ();
  static std::string rtl ();
  static int directionValue (std::string direction);
  static std::string writingModeUnspecified (int value);
  static std::string writingModeTopBottomLeftRight (int value);
  static std::string writingModeTopBottomRightLeft (int value);
  static std::string writingModeBottomTopLeftRight (int value);
  static std::string writingModeBottomTopRightLeft (int value);
  static std::string tb_lr ();
  static std::string tb_rl ();
  static std::string bt_lr ();
  static std::string bt_rl ();
  static int writingModeValue (std::string mode);
  static std::string getClass (std::string bible);
  static std::string get_css (std::string class_, std::string font, int directionvalue, int lineheight = 100, int letterspacing = 0);
  static void distinction_set_basic ();
  static std::string distinction_set_light (int itemstyleindex);
  static std::string distinction_set_dark (int itemstyleindex);
  static std::string distinction_set_redblue_light (int itemstyleindex);
  static std::string distinction_set_redblue_dark (int itemstyleindex);
  static std::string distinction_set_notes (int itemstyleindex);
  static std::string theme_picker (int themestyleindex, int itemstyleindex);
};

std::string filter_css_grey_background ();
