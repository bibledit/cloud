/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <images/logic.h>
#include <tasks/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/archive.h>
#include <locale/translate.h>
#include <database/bibleimages.h>
#include <database/logs.h>
using namespace std;


void images_logic_import_images (string path)
{
  Database_BibleImages database_bibleimages;

  Database_Logs::log ("Importing: " + filter_url_basename (path));

  // To begin with, add the path of the zip file to the main file to the list of paths to be processed.
  vector <string> paths = {path};

  while (!paths.empty ()) {

    // Take the first path and remove it from the container.
    path = paths[0];
    paths.erase (paths.begin());
    string basename = filter_url_basename (path);
    string extension = filter_url_get_extension (path);
    extension = unicode_string_casefold (extension);

    // Store images.
    if (filter_url_is_image (extension)) {
      if (basename.size() > 1) {
        if (basename[0] != '.') {
          database_bibleimages.store (path);
          Database_Logs::log ("Storing image " + basename);
        }
      }
    }

    // Uncompress archives.
    else if (filter_archive_is_archive (path)) {
      Database_Logs::log ("Unpacking archive " + basename);
      string folder = filter_archive_uncompress (path);
      filter_url_recursive_scandir (folder, paths);
    }
   
  }

  Database_Logs::log ("Ready importing Bible images");
}
