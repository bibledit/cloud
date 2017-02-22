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


#include <database/noteassignment.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// Database resilience:
// Data is stored in multiple text files.


string Database_NoteAssignment::path (string user)
{
  return filter_url_create_root_path ("databases", "client", "noteassignment_" + user + ".txt");
}


bool Database_NoteAssignment::exists (string user)
{
  return file_or_dir_exists (path (user));
}


void Database_NoteAssignment::assignees (string user, vector <string> assignees)
{
  filter_url_file_put_contents (path (user), filter_string_implode (assignees, "\n"));
}


vector <string> Database_NoteAssignment::assignees (string user)
{
  string contents = filter_url_file_get_contents (path (user));
  return filter_string_explode (contents, '\n');
}


bool Database_NoteAssignment::exists (string user, string assignee)
{
  vector <string> users = assignees (user);
  return in_array (assignee, users);
}


void Database_NoteAssignment::remove (string user)
{
  filter_url_unlink (path (user));
}
