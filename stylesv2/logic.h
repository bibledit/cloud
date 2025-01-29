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

#pragma once

#include <config/libraries.h>

namespace stylesv2 { // Todo the stylesv2 logic here.


enum class Type : int {
  none,
  book_id,
//  encoding,
//  comment,
//  running_header,
//  long_toc,
//  short_toc,
//  book_abbrev,
//  chapter_label,
//  published_chapter_marker,
//  comment_with_endmarker,
//  published_verse_marker,
};

enum class Capability : int {
  // Whether this marker starts a new page (with no matter an even or odd page number).
  starts_new_page,
  // Whether this marker starts a new page with an odd page number.
  // Not implemented due to limitations in OpenDocument.
  // starts_odd_page,
};


// With monostate (empty variant) below, the capability is enabled if found as parameters.
// With the bool, the capability can be enabled or disabled.
// With the int and the string, the capability has an extra paramter.
using Parameter = std::pair<Capability,std::variant<std::monostate,bool,int,std::string>>;


struct Style final {
  const char* marker {""};
  const Type type {Type::none};
  const char* name {""};
  const char* info {""};
  // The capabilities indicate what this style is capable of.
  const std::vector<Capability> capabilities{};
  // The parameters indicate the enabled capabilities beyond the capabilities already implied in the marker type.
  const std::vector<Parameter> parameters{};
  // Whether this style has been implemented throughout the code.
  const bool implemented {false};
};


inline bool operator==(const Style& style, const std::string& marker) noexcept { return style.marker == marker; }


extern const std::list<Style> styles;


} // Namespace.
