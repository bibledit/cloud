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


#include <database/bibleimages.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <database/logic.h>


// Database resilience: 
// The data is stored as images and files in the filesystem.
// That should be resilient enough.


static std::string folder ()
{
  return filter_url_create_root_path ({"images"});
}


static std::string path (const std::string& image)
{
  return filter_url_create_path ({folder (), image});
}


namespace database::bible_images {


std::vector <std::string> get ()
{
  const std::vector <std::string> files = filter_url_scandir (folder ());
  std::vector <std::string> images;
  for (const auto& file : files) {
    const std::string extension = filter_url_get_extension (file);
    if (extension == "o") continue;
    if (extension == "h") continue;
    if (extension == "cpp") continue;
    if (extension == "html") continue;
    images.push_back (file);
  }
  return images;
}


void store (const std::string& file)
{
  const std::string image = filter_url_basename (file);
  filter_url_file_cp (file, path (image));
}


std::string get (const std::string& image)
{
  return filter_url_file_get_contents (path(image));
}


void erase (const std::string& image)
{
  const std::string filepath = path(image);
  filter_url_unlink (filepath);
}


} // Namespace.

