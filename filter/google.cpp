/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <filter/google.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <filter/string.h>
#include <database/logs.h>
#include <config/logic.h>


namespace filter::google {


tuple <string, string> get_json_key_value_error ()
{
  string path = config_logic_google_translate_json_key_path ();

  if (!file_or_dir_exists (path)) {
    return { string(), "Cannot find the JSON key to access Google Translate. Looking for this file: " + path };
  }
  
  string error;
  string value = filter_url_file_get_contents (path);
  if (value.empty()) error = "The key at " + path + " is empty";

  return { value, error };
}


}
