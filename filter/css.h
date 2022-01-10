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

class Filter_Css
{
public:
  static string directionUnspecified (int value);
  static string directionLeftToRight (int value);
  static string directionRightToLeft (int value);
  static string ltr ();
  static string rtl ();
  static int directionValue (string direction);
  static string writingModeUnspecified (int value);
  static string writingModeTopBottomLeftRight (int value);
  static string writingModeTopBottomRightLeft (int value);
  static string writingModeBottomTopLeftRight (int value);
  static string writingModeBottomTopRightLeft (int value);
  static string tb_lr ();
  static string tb_rl ();
  static string bt_lr ();
  static string bt_rl ();
  static int writingModeValue (string mode);
  static string getClass (string bible);
  static string getCss (string class_, string font, int directionvalue, int lineheight = 100, int letterspacing = 0);
  static void distinction_set_basic ();
  static string distinction_set_light (int itemstyleindex);
  static string distinction_set_dark (int itemstyleindex);
  static string distinction_set_redblue_light (int itemstyleindex);
  static string distinction_set_redblue_dark (int itemstyleindex);
  static string distinction_set_notes (int itemstyleindex);
  static string theme_picker (int themestyleindex, int itemstyleindex);
};

string filter_css_grey_background ();
