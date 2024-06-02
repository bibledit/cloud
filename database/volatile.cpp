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


#include <database/volatile.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/sqlite.h>


// Database resilience: It is stored in the plain filesystem in the temporal location.


namespace database::volatile_ {


static std::string filename (int id, std::string key)
{
  const std::string identifier = filter_url_clean_filename (std::to_string (id));
  key = filter_url_clean_filename (key);
  const std::string path = filter_url_create_root_path ({filter_url_temp_dir (), "volatile__" + identifier + "__" + key});
  return path;
}


std::string get_value (int id, const std::string& key)
{
  return filter_url_file_get_contents (filename (id, key));
}


void set_value (int id, const std::string& key, const std::string& value)
{
  filter_url_file_put_contents (filename (id, key), value);
}


} // Namespace.
