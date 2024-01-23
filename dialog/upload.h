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
#include <assets/view.h>

class Dialog_Upload
{
public:
  Dialog_Upload (std::string url, std::string question);
  ~Dialog_Upload ();
  Dialog_Upload(const Dialog_Upload&) = delete;
  Dialog_Upload operator=(const Dialog_Upload&) = delete;
  void add_upload_query (std::string parameter, std::string value);
  std::string run ();
private:
  Assets_View assets_view {};
  std::string base_url {};
  std::map <std::string, std::string> upload_query {};
};
