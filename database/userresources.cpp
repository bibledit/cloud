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


#include <database/userresources.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/logic.h>
#include <database/logic.h>


// Database resilience: 
// The data is stored as separate files in the filesystem.
// That is resilient enough.


// Returns the names of the available user-defined resources.
std::vector <std::string> Database_UserResources::names ()
{
  std::vector <std::string> names;
  std::vector <std::string> files = filter_url_scandir (folder ());
  for (auto name : files) {
    if (name.find (fragment ()) != std::string::npos) {
      name.erase (0, fragment ().size ());
      name = filter_url_filename_unclean (name);
      names.push_back (name);
    }
  }
  return names;
}


// Removes a user-defined resource from disk.
void Database_UserResources::remove (const std::string& name)
{
  filter_url_unlink (file (name));
}


std::string Database_UserResources::url (const std::string& name)
{
  return load (name, 0);
}


void Database_UserResources::url (const std::string& name, const std::string& value)
{
  save (name, 0, value);
}


// Returns the text fragent for a Bible book with $id.
std::string Database_UserResources::book (const std::string& name, int id)
{
  if (id < 1) return std::string();
  return load (name, static_cast<size_t>(id));
}


// Stores the text fragment for a Bible book in the database.
void Database_UserResources::book (const std::string& name, int id, const std::string& fragment)
{
  if (id > 0) save (name, static_cast<size_t>(id), fragment);
}


// The folder for storing the user-defined resource definition files.
std::string Database_UserResources::folder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client"});
}


// The fragment that always occurs in the name of a user-defined resource definition file.
std::string Database_UserResources::fragment ()
{
  return "user_resource_";
}


// The full path of the definition file.
std::string Database_UserResources::file (const std::string& name)
{
  return filter_url_create_path ({folder (), fragment () + filter_url_filename_clean (name)});
}


// Load one value from the user-defined resource definition $name.
// The value is at line number $offset.
std::string Database_UserResources::load (const std::string& name, size_t offset)
{
  std::string path = file (name);
  std::string contents = filter_url_file_get_contents (path);
  std::vector <std::string> lines = filter::strings::explode (contents, '\n');
  if (offset >= lines.size ()) return std::string();
  return lines [offset];
}


// Save one value to the user-defined resource definition $name.
// It saves the $value to line number $offset.
void Database_UserResources::save (const std::string& name, size_t offset, const std::string& value)
{
  std::string path = file (name);
  std::string contents = filter_url_file_get_contents (path);
  std::vector <std::string> lines = filter::strings::explode (contents, '\n');
  while (lines.size () <= offset) lines.push_back ("");
  lines [offset] = value;
  contents = filter::strings::implode (lines, "\n");
  filter_url_file_put_contents (path, contents);
}
