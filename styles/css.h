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

class Webserver_Request;
namespace stylesv2 { struct Style; }

class Styles_Css
{
public:
  Styles_Css (const std::string& stylesheet);
  Styles_Css(const Styles_Css&) = delete;
  Styles_Css operator=(const Styles_Css&) = delete;
  void editor ();
  void exports ();
  void generate ();
  std::string css (std::string path = std::string());
  void customize (const std::string& bible);
private:
  std::string m_stylesheet {};
  std::vector <std::string> m_code {};
  bool editor_enabled {false}; // Whether to generate CSS for the Bible text editor.
  bool exports_enabled {false}; // Whether to generate CSS for exported Bibles.
  void evaluate_v2 (const stylesv2::Style* style);
  void add_v2 (const stylesv2::Style* style, const bool paragraph, const bool keep_with_next);
  void add_exports_styles ();
  void add_editor_styles ();
};
