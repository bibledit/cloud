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


#include <filter/quill.h>
#include <filter/string.h>


namespace quill {


// Styles for milestones look like "qt-s".
// The issue is that there's a hyphen in the style.
// A Quill editor won't accept such a style, it would throw the style out.
// The converters change the hyphen to an underscore and back again.
std::string extract_class_prefix (std::string& style)
{
  if (style.find(class_prefix_block) == 0) {
    style.erase(0, strlen(class_prefix_block));
    return class_prefix_block;
  }
  if (style.find(class_prefix_inline) == 0) {
    style.erase(0, strlen(class_prefix_inline));
    return class_prefix_inline;
  }
  return std::string();
}


std::string hyphen_to_underscore (std::string style)
{
  const std::string prefix = extract_class_prefix(style);
  style = filter::string::replace ("-", "_", std::move(style));
  style.insert(0, prefix);
  return style;
}


std::string underscore_to_hyphen (std::string style)
{
  const std::string prefix = extract_class_prefix(style);
  style = filter::string::replace ("_", "-", std::move(style));
  style.insert(0, prefix);
  return style;
}


} // Namespace.
