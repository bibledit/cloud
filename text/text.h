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

class Text_Text
{
public:
  void paragraph (std::string text = "");
  std::string line ();
  void addtext (std::string text);
  std::string get ();
  void save (std::string name);
  void note (std::string text = "");
  void addnotetext (std::string text);
  std::string getnote ();
private:
  std::vector <std::string> output {};
  std::string thisline {};
  std::vector <std::string> notes {};
  std::string thisnoteline {};
};
