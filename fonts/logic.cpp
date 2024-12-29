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


#include <fonts/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <database/config/bible.h>


namespace fonts::logic {


static std::string folder ()
{
  return filter_url_create_root_path ({"fonts"});
}


std::vector <std::string> get_fonts ()
{
  const std::vector <std::string> files = filter_url_scandir (folder());
  std::vector <std::string> fonts;
  for (const auto& file : files) {
    const std::string suffix = filter_url_get_extension (file);
    if (suffix == "txt") continue;
    if (suffix == "html") continue;
    if (suffix == "h") continue;
    if (suffix == "cpp") continue;
    if (suffix == "o") continue;
    fonts.push_back (file);
  }
  return fonts;
}


bool font_exists (const std::string& font)
{
  const std::string path = filter_url_create_path ({folder (), font});
  return file_or_dir_exists (path);
}


std::string get_font_path (const std::string& font)
{
  // Case of no font.
  if (font.empty()) return font;
  
  // Case when the font exists within Bibledit.
  if (font_exists (font)) {
    return filter_url_create_path ({"../fonts", font});
  }
  
  // Case when the font is available from the browser independent of Bibledit.
  if (filter_url_basename (font) == font) {
    return font;
  }
  
  // Font is on external location.
  return font;
}


void erase (const std::string& font)
{
  const std::string path = filter_url_create_path ({folder (), font});
  filter_url_unlink (path);
}


// When a font is set for a Bible in Bibledit Cloud, this becomes the default font for the clients.
// Ahd when the client sets its own font, this font will be taken instead.
std::string get_text_font (const std::string& bible)
{
  std::string font = database::config::bible::get_text_font (bible);
#ifdef HAVE_CLIENT
  const std::string client_font = database::config::bible::get_text_font_client (bible);
  if (!client_font.empty ()) {
    font = client_font;
  }
#endif
  return font;
}


// Returns true if the $font path has a font suffix.
bool is_font (const std::string& suffix)
{
  return (suffix == "ttf")
  || (suffix == "otf")
  || (suffix == "otf")
  || (suffix == "woff");
}


} // namespace
