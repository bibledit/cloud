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


#include <database/noteassignment.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>
#include <database/logic.h>


// Database resilience:
// Data is stored in multiple text files.


std::string Database_NoteAssignment::path (std::string user)
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "noteassignment_" + user + ".txt"});
}


bool Database_NoteAssignment::exists (std::string user)
{
  return file_or_dir_exists (path (user));
}


void Database_NoteAssignment::assignees (std::string user, std::vector <std::string> assignees)
{
  filter_url_file_put_contents (path (user), filter::string::implode (assignees, "\n"));
}


std::vector <std::string> Database_NoteAssignment::assignees (std::string user)
{
  std::string contents = filter_url_file_get_contents (path (user));
  return filter::string::explode (contents, '\n');
}


bool Database_NoteAssignment::exists (std::string user, std::string assignee)
{
  std::vector <std::string> users = assignees (user);
  return filter::string::in_array (assignee, users);
}


void Database_NoteAssignment::remove (std::string user)
{
  filter_url_unlink (path (user));
}
