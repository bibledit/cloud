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
#include <stylesv2/logic.h>

namespace database::styles {

void create_database ();
void create_sheet (const std::string& sheet);
std::vector <std::string> get_sheets ();
void delete_sheet (const std::string& sheet);
void grant_write_access (const std::string& user, const std::string& sheet);
void revoke_write_access (const std::string& user, const std::string& sheet);
bool has_write_access (const std::string& user, const std::string& sheet);

} // End namespace styles


namespace database::styles1 {

struct Item
{
  std::string marker {};
  std::string name {};
  std::string info {};
  std::string category {};
  int type {0};
  int subtype {0};
  float fontsize {12.0f};
  int italic {0};
  int bold {0};
  int underline {0};
  int smallcaps {0};
  int superscript {0};
  int justification {0};
  float spacebefore {0.0f};
  float spaceafter {0.0f};
  float leftmargin {0.0f};
  float rightmargin {0.0f};
  float firstlineindent {0.0f};
  bool spancolumns {false};
  std::string color {"#000000"};
  bool print {false};
  bool userbool1 {false};
  bool userbool2 {false};
  bool userbool3 {false};
  int userint1 {0};
  int userint2 {0};
  int userint3 {0};
  std::string userstring1 {};
  std::string userstring2 {};
  std::string userstring3 {};
  std::string backgroundcolor {"#FFFFFF"};
};


std::map <std::string, std::string> get_markers_and_names (const std::string& sheet);
std::vector <std::string> get_markers (const std::string& sheet);
Item get_marker_data (const std::string& sheet, const std::string& marker);

} // End namespace styles1


namespace database::styles2 {

void add_marker (const std::string& sheet, const std::string& marker);
void delete_marker (const std::string& sheet, const std::string& marker);
void reset_marker (const std::string& sheet, const std::string& marker);
const std::list<stylesv2::Style>& get_styles(const std::string& sheet);
std::vector <std::string> get_markers (const std::string& sheet);
std::map <std::string, std::string> get_markers_and_names (const std::string& sheet);
const stylesv2::Style* get_marker_data (const std::string& sheet, const std::string& marker);
void save_style(const std::string& sheet, const stylesv2::Style& style);
std::optional<stylesv2::Style> load_style(const std::string& sheet, const std::string& marker);


} // End namespace styles2
