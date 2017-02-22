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


#include <tmp/tmp.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/date.h>
#include <database/logs.h>


void tmp_tmp ()
{
  Database_Logs::log ("Removing expired temporal files", Filter_Roles::manager ());
  int expired = filter_date_seconds_since_epoch () - (3600 * 24 * 3);
  string directory = filter_url_create_root_path (filter_url_temp_dir ());
  vector <string> names = filter_url_scandir (directory);
  for (auto & name : names) {
    if (name.find ("tmp.") == 0) continue;
    string filename = filter_url_create_path (directory, name);
    int mtime = filter_url_file_modification_time (filename);
    if (mtime < expired) {
      filter_url_rmdir (filename);
      filter_url_unlink (filename);
    }
  }
}
