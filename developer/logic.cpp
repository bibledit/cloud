/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#include <developer/logic.h>
#include <filter/date.h>
#include <filter/url.h>
#include <filter/string.h>


void developer_logic_timing (int order, bool initialize)
{
  int now = filter_date_seconds_since_epoch ();
  int unow = filter_date_numerical_microseconds ();
  static int seconds = 0;
  static int useconds = 0;
  if (initialize) {
    seconds = now;
    useconds = unow;
  }
  cout << order << ": " << 1000000 * (now - seconds) + (unow - useconds) << endl;
}


void developer_logic_log (string message)
{
  string path = filter_url_create_root_path ("developer", "log.txt");
  if (!file_or_dir_exists (path)) return;
  filter_url_file_put_contents_append (path, filter_string_trim (message) + "\n");
}
