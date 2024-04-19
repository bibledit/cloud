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


#include <database/bibleimages.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/logic.h>


// Database resilience: 
// The data is stored as images and files in the filesystem.
// That should be resilient enough.


std::vector <std::string> Database_BibleImages::get ()
{
  std::vector <std::string> files = filter_url_scandir (folder ());
  std::vector <std::string> images;
  for (auto file : files) {
    std::string extension = filter_url_get_extension (file);
    if (extension == "o") continue;
    if (extension == "h") continue;
    if (extension == "cpp") continue;
    if (extension == "html") continue;
    images.push_back (file);
  }
  return images;
}


void Database_BibleImages::store (std::string file)
{
  std::string image = filter_url_basename (file);
  filter_url_file_cp (file, path (image));
}


std::string Database_BibleImages::get (std::string image)
{
  std::string contents = filter_url_file_get_contents (path(image));
  return contents;
}


void Database_BibleImages::erase (std::string image)
{
  std::string filepath = path(image);
  filter_url_unlink (filepath);
}


std::string Database_BibleImages::folder ()
{
  return filter_url_create_root_path ({"images"});
}


std::string Database_BibleImages::path (std::string image)
{
  return filter_url_create_path ({folder (), image});
}
