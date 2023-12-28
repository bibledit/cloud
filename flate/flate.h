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

class Flate
{
public:
  void set_variable (std::string key, std::string value);
  void enable_zone (std::string zone);
  std::string render (std::string html);
  void add_iteration (std::string key, std::map <std::string, std::string> value);
  std::map <std::string, std::vector <std::map <std::string, std::string> > > iterations {};
private:
  std::map <std::string, std::string> variables {};
  std::map <std::string, bool> zones {};
  void process_iterations (std::string & rendering);
  void process_zones (std::string& rendering);
  void process_variables (std::string& rendering);
  void process_translate (std::string& rendering);
};
