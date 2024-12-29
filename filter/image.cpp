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


#include <filter/image.h>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#ifdef COMPILERGCC
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#pragma GCC diagnostic pop


void filter_image_get_sizes (std::string image_path, int & width, int & height)
{
  int bpp;
  uint8_t* rgb_image = stbi_load (image_path.c_str(), &width, &height, &bpp, 0);
  if (rgb_image) {
    stbi_image_free(rgb_image);
  }
}
