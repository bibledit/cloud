/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#ifndef INCLUDED_FONTS_LOGIC_H
#define INCLUDED_FONTS_LOGIC_H


#include <config/libraries.h>


class Fonts_Logic
{
public:
  static vector <string> getFonts ();
  static bool fontExists (string font);
  static string getFontPath (string font);
  static void erase (string font);
  static string getTextFont (string bible);
  static bool isFont (string suffix);
private:
  static string folder ();
};


#endif
