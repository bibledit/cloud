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


#pragma once

#include <config/libraries.h>


namespace stylesv2 {


enum class Type : int {
  starting_boundary, // Should be the first always.
  none,
  book_id,
  usfm_version,
  file_encoding,
  remark,
  running_header,
  long_toc_text,
  short_toc_text,
  book_abbrev,
  introduction_end,
  title,
  heading,
  paragraph,
  chapter,
  chapter_label,
  published_chapter_marker,
  alternate_chapter_number,
  verse,
  published_verse_marker,
  alternate_verse_marker,
  table_row,
  table_heading,
  table_cell,
  footnote_wrapper,
  endnote_wrapper,
  note_standard_content,
  note_content,
  note_content_with_endmarker,
  note_paragraph,
  crossreference_wrapper,
  crossreference_standard_content,
  crossreference_content,
  crossreference_content_with_endmarker,
  character,
  page_break,
  figure,
  word_list,
  sidebar_begin,
  sidebar_end,
  peripheral,
  milestone,
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
  
  // Whether the marker has been deprecated in the newest USFM specification.
  deprecated,
  
  // Whether to output the marker on the left and/or the right page.
  on_left_page,
  on_right_page,
  
  // Whether this style, normally without an endmarker, should have the endmarker.
  has_endmarker,
  
  // Whether to ouotput this marker at the first verse.
  at_first_verse,
  
  // Whether to restart a paragraph.
  restart_paragraph,
  
  // The note numbering sequence.
  note_numbering_sequence,
  
  // The note numbering restart trigger.
  note_numbering_restart,
  
  // The place where to dump the notes.
  notes_dump,
  
  // Testing value, can go out once a numerical property is in use.
  numerical_test,
  
  // Should be the last always.
  stopping_boundary
};


std::string property_enum_to_value (const Property property);
Property property_value_to_enum (const std::string& value);


enum class Variant { none, boolean, number, text };

Variant property_to_variant (const Property property);

using Parameter = std::variant<std::monostate,bool,int,std::string>;

std::ostream& operator<<(std::ostream& os, const Parameter& parameter);


enum class Category {
  starting_boundary, // Must always be the first one.
  unknown,
  identification,
  introductions,
  titles_headings_labels,
  chapters_verses,
  paragraphs,
  poetry,
  lists,
  tables,
  footnotes,
  cross_references,
  words_characters,
  milestones,
  extended_study_content,
  peripherals,
  stopping_boundary // Must always be the last one.
};

std::ostream& operator<<(std::ostream& os, const Category category);
Category category_value_to_enum (const std::string& value);


enum class FourState { off, on, inherit, toggle };
std::string fourstate_enum_to_value(const FourState state);
FourState fourstate_value_to_enum(const std::string& value);
std::list<FourState> get_four_states();

enum class TwoState { off, on };
std::string twostate_enum_to_value(const TwoState state);
TwoState twostate_value_to_enum(const std::string& value);
std::list<TwoState> get_two_states();

enum class TextAlignment { left, center, right, justify };
std::string textalignment_enum_to_value(const TextAlignment alignment);
TextAlignment textalignment_value_to_enum(const std::string& value);
std::list<TextAlignment> get_text_alignments();


constexpr const char* white {"#FFFFFF"};
constexpr const char* black {"#000000"};


struct Paragraph {
  int font_size {12};
  TwoState italic {TwoState::off};
  TwoState bold {TwoState::off};
  TwoState underline {TwoState::off};
  TwoState smallcaps {TwoState::off};
  TextAlignment text_alignment {TextAlignment::left};
  float space_before {0};
  float space_after {0};
  float left_margin {0};
  float right_margin {0};
  float first_line_indent{0};
};

std::ostream& operator<<(std::ostream& os, const Paragraph paragraph);


struct Character {
  FourState italic {FourState::off};
  FourState bold {FourState::off};
  FourState underline {FourState::off};
  FourState smallcaps {FourState::off};
  TwoState superscript {TwoState::off};
  std::string foreground_color {black};
  std::string background_color {white};
};

std::ostream& operator<<(std::ostream& os, const Character character);


struct Style final {
  std::string marker {};
  Type type {Type::none};
  std::string name {};
  std::string info {};
  std::optional<Paragraph> paragraph {};
  std::optional<Character> character {};
  // The parameters indicate the enabled capabilities beyond the capabilities implied in the style type.
  std::map<Property,Parameter> properties{};
  std::string doc {};
  Category category {};
};


inline bool operator==(const Style& style, const std::string& marker) noexcept { return style.marker == marker; }
std::ostream& operator<<(std::ostream& os, const Style& style);
bool has_property (const Style* style, const Property property);

template<typename T>
const T get_parameter(const Style* style, const Property property)
{
  if (style) {
    const auto iter = style->properties.find(property);
    if (iter != style->properties.cend()) {
      const Parameter& parameter = iter->second;
      if (std::holds_alternative<T>(parameter)) {
        return std::get<T>(parameter);
      }
    }
  }
  T not_found {};
  return not_found;
}


extern const std::list<Style> styles;


constexpr const char* notes_numbering_restart_never {"never"}; // Never restart the notes numbering.
constexpr const char* notes_numbering_restart_book {"book"}; // Restart notes numbering every book.
constexpr const char* notes_numbering_restart_chapter {"chapter"}; // Restart notes numbering every chapter.
constexpr const char* notes_dump_book {"book"}; // Dump the endnotes at the end of each book.
constexpr const char* notes_dump_end {"end"}; // Dump the endnotes at the very end of everything.

bool starts_new_line_in_usfm (const Style* style);
std::string validate_notes_dump (const std::string& input);
std::string standard_sheet ();


} // Namespace.
