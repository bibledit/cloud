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
#include <assets/view.h>

class Dialog_Books
{
public:
  Dialog_Books (std::string url, std::string header, std::string info_top, std::string info_bottom, std::string action, std::vector <int> inclusions, std::vector <int> exclusions);
  ~Dialog_Books ();
  Dialog_Books (const Dialog_Books&) = delete;
  Dialog_Books operator=(const Dialog_Books&) = delete;
  void add_query (std::string parameter, std::string value);
  std::string run ();
private:
  Assets_View assets_view {};
  std::string base_url {};
  std::string selection_action {};
  std::vector <int> include {};
  std::vector <int> exclude {};
};
