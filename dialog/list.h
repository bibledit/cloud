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
#include <assets/view.h>

class Dialog_List
{
public:
  Dialog_List (std::string url, std::string question, std::string info_top, std::string info_bottom, bool post = false);
  Dialog_List(const Dialog_List&) = delete;
  Dialog_List operator=(const Dialog_List&) = delete;
  void add_query (std::string parameter, std::string value);
  void add_row (std::string text, std::string parameter, std::string value);
  std::string run ();
private:
  Assets_View assets_view {};
  std::string base_url {};
  std::string list_block {};
  bool post_result {false};
};
