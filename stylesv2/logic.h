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
  file_encoding,
  remark,
  running_header,
  long_toc_text,
  short_toc_text,
  book_abbrev,
  chapter_label,
  published_chapter_marker,
  alternate_chapter_number,
  published_verse_marker,
  stopping_boundary // Should be the last always.
};


std::string type_enum_to_value (const Type type, const bool describe = false);
Type type_value_to_enum (const std::string& value);


enum class Property : int {
  // Should be the first always.
  starting_boundary,
  
  none,
  
  // Whether this marker starts a new page (with no matter an even or odd page number).
  starts_new_page,
  
  // Whether this marker starts a new page with an odd page number.
  // Not implemented due to limitations in OpenDocument.
  // starts_odd_page,
  
  // Whether the marker has been deprecated in the newest USFM specs.
  deprecated,
  
  // Whether to output the marker on the left and/or the right page.
  on_left_page,
  on_right_page,
  
  // Should be the last always.
  stopping_boundary
};


std::string property_enum_to_value (const Property property);
Property property_value_to_enum (const std::string& value);


enum class Variant { none, boolean, number, text };

Variant property_to_variant (const Property property);

using Parameter = std::variant<std::monostate,bool,int,std::string>;

std::ostream& operator<<(std::ostream& os, const Parameter& parameter);


struct Style final {
  std::string marker {};
  Type type {Type::none};
  std::string name {};
  std::string info {};
  // The parameters indicate the enabled capabilities beyond the capabilities implied in the style type.
  std::map<Property,Parameter> properties{};
  // Whether this style has been implemented throughout the code.
  bool implemented {false};
  std::string doc {};
};


inline bool operator==(const Style& style, const std::string& marker) noexcept { return style.marker == marker; }
std::ostream& operator<<(std::ostream& os, const Style& style);
bool get_bool_parameter (const Style* style, const Property property);


extern const std::list<Style> styles;


bool marker_moved_to_v2 (const std::string& marker, const std::vector<const char*> extra);


bool starts_new_line_in_usfm (const Style* style);


} // Namespace.
