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


#include <versification/logic.h>
#include <filter/string.h>
#include <filter/url.h>


// Get the names of the available versification systems that come with Bibledit.
std::vector <std::string> versification_logic_names ()
{
  std::vector <std::string> names;

  std::string directory = filter_url_create_root_path ({"versification"});
  std::vector <std::string> files = filter_url_scandir (directory);
  for (auto file : files) {
    if (filter_url_get_extension (file) == "txt") {
      // Remove the dot and extension.
      file = file.substr (0, file.length () - 4);
      // Change underscores to spaces for the names.
      file = filter::string::replace ("_", " ", file);
      names.push_back (file);
    }
  }
  
  return names;
}


// Return the raw data of default versification system $name.
std::string versification_logic_data (std::string name)
{
  name = filter::string::replace (" ", "_", name);
  name.append (".txt");
  std::string file = filter_url_create_root_path ({"versification", name});
  return filter_url_file_get_contents (file);
}
