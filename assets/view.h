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

class Assets_View
{
public:
  Assets_View ();
  void set_variable (std::string key, std::string value);
  void enable_zone (std::string zone);
  void disable_zone (std::string zone);
  void add_iteration (std::string key, std::map <std::string, std::string> value);
  std::string render (std::string tpl1, std::string tpl2);
private:
  std::map <std::string, std::string> m_variables {};
  std::map <std::string, bool> m_zones {};
  std::map <std::string, std::vector <std::map <std::string, std::string> > > m_iterations {};
};
