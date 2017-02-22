/*
Copyright (©) 2003-2016 Teus Benschop.

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


// Database resilience: 
// The data is stored as separate files in the filesystem.
// That is resilient enough.


// Returns the names of the available user-defined resources.
vector <string> Database_UserResources::names ()
{
  vector <string> names;
  vector <string> files = filter_url_scandir (folder ());
  for (auto name : files) {
    if (name.find (fragment ()) != string::npos) {
      name.erase (0, fragment ().size ());
      name = filter_url_filename_unclean (name);
      names.push_back (name);
    }
  }
  return names;
}


// Removes a user-defined resource from disk.
void Database_UserResources::remove (const string& name)
{
  filter_url_unlink (file (name));
}


string Database_UserResources::url (const string& name)
{
  return load (name, 0);
}


void Database_UserResources::url (const string& name, const string & value)
{
  save (name, 0, value);
}


// Returns the text fragent for a Bible book with $id.
string Database_UserResources::book (const string& name, int id)
{
  if (id < 1) return "";
  return load (name, id);
}


// Stores the text fragment for a Bible book in the database.
void Database_UserResources::book (const string& name, int id, const string & fragment)
{
  if (id > 0) save (name, id, fragment);
}


// The folder for storing the user-defined resource definition files.
string Database_UserResources::folder ()
{
  return filter_url_create_root_path ("databases", "client");
}


// The fragment that always occurs in the name of a user-defined resource definition file.
string Database_UserResources::fragment ()
{
  return "user_resource_";
}


// The full path of the definition file.
string Database_UserResources::file (const string& name)
{
  return filter_url_create_path (folder (), fragment () + filter_url_filename_clean (name));
}


// Load one value from the user-defined resource definition $name.
// The value is at line number $offset.
string Database_UserResources::load (const string & name, size_t offset)
{
  string path = file (name);
  string contents = filter_url_file_get_contents (path);
  vector <string> lines = filter_string_explode (contents, '\n');
  if (offset >= lines.size ()) return "";
  return lines [offset];
}


// Save one value to the user-defined resource definition $name.
// It saves the $value to line number $offset.
void Database_UserResources::save (const string & name, size_t offset, const string & value)
{
  string path = file (name);
  string contents = filter_url_file_get_contents (path);
  vector <string> lines = filter_string_explode (contents, '\n');
  while (lines.size () <= offset) lines.push_back ("");
  lines [offset] = value;
  contents = filter_string_implode (lines, "\n");
  filter_url_file_put_contents (path, contents);
}
