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

namespace stylesv2 {


enum class Type : int {
  starting_boundary, // Should be the first always.
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
  stopping_boundary // Should be the last always.
};


std::string type_enum_to_value (const Type type, const bool describe = false);
Type type_value_to_enum (const std::string& value);


enum class Capability : int {
  starting_boundary, // Should be the first always.
  none,
  // Whether this marker starts a new page (with no matter an even or odd page number).
  starts_new_page,
  // Whether this marker starts a new page with an odd page number.
  // Not implemented due to limitations in OpenDocument.
  // starts_odd_page,
  stopping_boundary // Should be the last always.
};


std::string capability_enum_to_value (const Capability capability);
Capability capability_value_to_enum (const std::string& value);


enum class Variant { none, boolean, number, text };

Variant capability_to_variant (const Capability capability);

using Parameter = std::variant<bool,int,std::string>;

std::ostream& operator<<(std::ostream& os, const Parameter& parameter);


struct Style final {
  std::string marker {};
  Type type {Type::none};
  std::string name {};
  std::string info {};
  // The parameters indicate the enabled capabilities beyond the capabilities implied in the style type.
  std::map<Capability,Parameter> parameters{};
  // Whether this style has been implemented throughout the code.
  bool implemented {false};
};


inline bool operator==(const Style& style, const std::string& marker) noexcept { return style.marker == marker; }
std::ostream& operator<<(std::ostream& os, const Style& style);


extern const std::list<Style> styles;


} // Namespace.
