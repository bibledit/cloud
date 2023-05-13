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

class Styles_Css
{
public:
  Styles_Css (void * webserver_request, const std::string & stylesheet);
  Styles_Css(const Styles_Css&) = delete;
  Styles_Css operator=(const Styles_Css&) = delete;
  void editor ();
  void exports ();
  void generate ();
  std::string css (std::string path = std::string());
  void customize (const std::string& bible);
private:
  void * m_webserver_request {nullptr};
  std::string m_stylesheet {};
  std::vector <std::string> m_code {};
  bool editor_enabled {false}; // Whether to generate CSS for the Bible text editor.
  bool exports_enabled {false}; // Whether to generate CSS for exported Bibles.
  void evaluate (void * database_styles_item);
  void add (void * database_styles_item, bool paragraph, bool keepwithnext);
  void add_exports_styles ();
  void add_editor_styles ();
};
