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


#include <styles/logic.h>
#include <locale/translate.h>
#include <filter/string.h>
#include <filter/usfm.h>


namespace stylesv2 {


std::string type_enum_to_value (const Type type, const bool describe)
{
  switch (type) {
    case Type::starting_boundary:
      return "starting_boundary";
    case Type::none:
      return "none";
    case Type::book_id:
      if (describe)
        return "book id";
      return "book_id";
    case Type::usfm_version:
      if (describe)
        return "usfm version";
      return "usfm_version";
    case Type::file_encoding:
      if (describe)
        return "file encoding";
      return "file_encoding";
    case Type::remark:
      return "remark";
    case Type::running_header:
      if (describe)
        return "running header";
      return "running_header";
    case Type::long_toc_text:
      if (describe)
        return "long toc text";
      return "long_toc_text";
    case Type::short_toc_text:
      if (describe)
        return "short toc text";
      return "short_toc_text";
    case Type::book_abbrev:
      if (describe)
        return "book abbrev";
      return "book_abbrev";
    case Type::introduction_end:
      if (describe)
        return "introduction end";
      return "introduction_end";
    case Type::title:
      return "title";
    case Type::heading:
      return "heading";
    case Type::paragraph:
      return "paragraph";
    case Type::chapter:
      return "chapter";
    case Type::chapter_label:
      if (describe)
        return "chapter label";
      return "chapter_label";
    case Type::published_chapter_marker:
      if (describe)
        return "published chapter marker";
      return "published_chapter_marker";
    case Type::alternate_chapter_number:
      if (describe)
        return "alternate chapter number";
      return "alternate_chapter_number";
    case Type::verse:
      return "verse";
    case Type::published_verse_marker:
      if (describe)
        return "published verse marker";
      return "published_verse_marker";
    case Type::alternate_verse_marker:
      if (describe)
        return "alternate verse marker";
      return "alternate_verse_marker";
    case Type::table_row:
      if (describe)
        return "table row";
      return "table_row";
    case Type::table_heading:
      if (describe)
        return "table heading";
      return "table_heading";
    case Type::table_cell:
      if (describe)
        return "table cell";
      return "table_cell";
    case Type::footnote_wrapper:
      if (describe)
        return "footnote wrapper";
      return "footnote_wrapper";
    case Type::endnote_wrapper:
      if (describe)
        return "endnote wrapper";
      return "endnote_wrapper";
    case Type::note_standard_content:
      if (describe)
        return "note standard content";
      return "note_standard_content";
    case Type::note_content:
      if (describe)
        return "note content";
      return "note_content";
    case Type::note_content_with_endmarker:
      if (describe)
        return "note content with endmarker";
      return "note_content_with_endmarker";
    case Type::note_paragraph:
      if (describe)
        return "note paragraph";
      return "note_paragraph";
    case Type::crossreference_wrapper:
      if (describe)
        return "crossreference wrapper";
      return "crossreference_wrapper";
    case Type::crossreference_standard_content:
      if (describe)
        return "crossreference standard content";
      return "crossreference_standard_content";
    case Type::crossreference_content:
      if (describe)
        return "crossreference content";
      return "crossreference_content";
    case Type::crossreference_content_with_endmarker:
      if (describe)
        return "crossreference content with endmarker";
      return "crossreference_content_with_endmarker";
    case Type::character:
      return "character";
    case Type::page_break:
      if (describe)
        return "page break";
      return "page_break";
    case Type::figure:
      return "figure";
    case Type::word_list:
      if (describe)
        return "word list";
      return "word_list";
    case Type::sidebar_begin:
      if (describe)
        return "sidebar begin";
      return "sidebar_begin";
    case Type::sidebar_end:
      if (describe)
        return "sidebar end";
      return "sidebar_end";
    case Type::peripheral:
      return "peripheral";
    case Type::milestone:
      return "milestone";
    case Type::stopping_boundary:
      return "stopping_boundary";
    default:
      return "unknown";
  }
}


Type type_value_to_enum (const std::string& value)
{
  // Iterate over the enum values and if a match is found, return the matching enum value.
  for (int i {static_cast<int>(Type::starting_boundary)+1};
       i < static_cast<int>(Type::stopping_boundary); i++)
    if (value == type_enum_to_value(static_cast<Type>(i)))
      return static_cast<Type>(i);
  // No match found.
  return Type::none;
}


std::string property_enum_to_value (const Property property)
{
  switch (property) {
    case Property::starting_boundary:
      return "starting_boundary";
    case Property::none:
      return "none";
    case Property::starts_new_page:
      return "starts_new_page";
    case Property::deprecated:
      return "deprecated";
    case Property::on_left_page:
      return "on_left_page";
    case Property::on_right_page:
      return "on_right_page";
    case Property::has_endmarker:
      return "has_endmarker";
    case Property::at_first_verse:
      return "at_first_verse";
    case Property::restart_paragraph:
      return "restart_paragraph";
    case Property::note_numbering_sequence:
      return "note_numbering_sequence";
    case Property::note_numbering_restart:
      return "note_numbering_restart";
    case Property::notes_dump:
      return "notes_dump";
    case Property::numerical_test:
      return "numerical_test";
    case Property::stopping_boundary:
      return "stopping_boundary";
    default:
      return "unknown";
  }
}


Property property_value_to_enum (const std::string& value)
{
  // Iterate over the enum values and if a match is found, return the matching enum value.
  for (int i {static_cast<int>(Property::starting_boundary)+1};
       i < static_cast<int>(Property::stopping_boundary); i++)
    if (value == property_enum_to_value(static_cast<Property>(i)))
      return static_cast<Property>(i);
  // No match found.
  return Property::none;
}


Variant property_to_variant (const Property property)
{
  switch (property) {
    case Property::starting_boundary:
    case Property::none:
      return Variant::none;
    case Property::starts_new_page:
      return Variant::boolean;
    case Property::deprecated:
      return Variant::none;
    case Property::on_left_page:
    case Property::on_right_page:
    case Property::has_endmarker:
    case Property::at_first_verse:
    case Property::restart_paragraph:
      return Variant::boolean;
    case Property::note_numbering_sequence:
    case Property::note_numbering_restart:
    case Property::notes_dump:
      return Variant::text;
    case Property::numerical_test:
      return Variant::number;
    case Property::stopping_boundary:
    default:
      return Variant::none;
  }
}


std::ostream& operator<<(std::ostream& os, const Parameter& parameter)
{
  if (std::holds_alternative<bool>(parameter))
    os << filter::strings::convert_to_string(std::get<bool>(parameter));
  if (std::holds_alternative<int>(parameter))
    os << std::get<int>(parameter);
  if (std::holds_alternative<std::string>(parameter))
    os << std::get<std::string>(parameter);
  return os;
}


std::ostream& operator<<(std::ostream& os, const Category category)
{
  switch (category) {
    case Category::identification:
      os << "Identification";
      break;
    case Category::introductions:
      os << "Introductions";
      break;
    case Category::titles_headings_labels:
      os << "Titles, Headings, Labels";
      break;
    case Category::chapters_verses:
      os << "Chapters, Verses";
      break;
    case Category::paragraphs:
      os << "Paragraphs";
      break;
    case Category::poetry:
      os << "Poetry";
      break;
    case Category::lists:
      os << "Lists";
      break;
    case Category::tables:
      os << "Tables";
      break;
    case Category::footnotes:
      os << "Footnotes";
      break;
    case Category::cross_references:
      os << "Cross References";
      break;
    case Category::words_characters:
      os << "Words, Characters";
      break;
    case Category::milestones:
      os << "Milestones";
      break;
    case Category::extended_study_content:
      os << "Extended Study Content";
      break;
    case Category::peripherals:
      os << "Peripherals";
      break;
    case Category::unknown:
      os << "Unknown";
      break;
    case Category::starting_boundary:
    case Category::stopping_boundary:
    default:
      os << "";
      break;
  }
  return os;
}


Category category_value_to_enum (const std::string& value)
{
  for (int i {static_cast<int>(stylesv2::Category::starting_boundary) + 1};
       i < static_cast<int>(stylesv2::Category::stopping_boundary); i++) {
    const Category category {static_cast<Category>(i)};
    std::stringstream ss{};
    ss << category;
    if (value == ss.str())
      return category;
  }
  return Category::unknown;
}


std::string fourstate_enum_to_value(const FourState state)
{
  switch (state) {
    case FourState::off: return "off";
    case FourState::on: return "on";
    case FourState::inherit: return "inherit";
    case FourState::toggle: return "toggle";
    default: return "unknown";
  }
}


FourState fourstate_value_to_enum(const std::string& value)
{
  for (const auto fourstate : get_four_states()) {
    if (value == fourstate_enum_to_value(fourstate))
      return fourstate;
  }
  return FourState::off;
}


std::list<FourState> get_four_states()
{
  return {FourState::on, FourState::off, FourState::inherit, FourState::toggle};
}


std::string twostate_enum_to_value(const TwoState state)
{
  switch (state) {
    case TwoState::off: return "off";
    case TwoState::on: return "on";
    default: return "unknown";
  }
}


TwoState twostate_value_to_enum(const std::string& value)
{
  for (const auto twostate : get_two_states()) {
    if (value == twostate_enum_to_value(twostate))
      return twostate;
  }
  return TwoState::off;
}


std::list<TwoState> get_two_states()
{
  return {TwoState::on, TwoState::off};
}


std::string textalignment_enum_to_value(const TextAlignment alignment)
{
  switch (alignment) {
    case TextAlignment::left: return "left";
    case TextAlignment::center: return "center";
    case TextAlignment::right: return "right";
    case TextAlignment::justify: return "justify";
    default: return "unknown";
  }
}


TextAlignment textalignment_value_to_enum(const std::string& value)
{
  for (const auto alignment : get_text_alignments()) {
    if (value == textalignment_enum_to_value(alignment))
      return alignment;
  }
  return TextAlignment::left;
}


std::list<TextAlignment> get_text_alignments()
{
  return {TextAlignment::left, TextAlignment::center, TextAlignment::right, TextAlignment::justify};
}


std::ostream& operator<<(std::ostream& os, const Character character)
{
  os << "character:" << std::endl;
  os << "  italic: " << fourstate_enum_to_value(character.italic) << std::endl;
  os << "  bold: " << fourstate_enum_to_value(character.bold) << std::endl;
  os << "  underline: " << fourstate_enum_to_value(character.underline) << std::endl;
  os << "  smallcaps: " << fourstate_enum_to_value(character.smallcaps) << std::endl;
  os << "  superscript: " << twostate_enum_to_value(character.superscript) << std::endl;
  os << "  foreground_color: " << character.foreground_color << std::endl;
  os << "  background_color: " << character.background_color << std::endl;
  return os;
}


std::ostream& operator<<(std::ostream& os, const Paragraph paragraph)
{
  os << "paragraph:" << std::endl;
  os << "  font_size: " << paragraph.font_size << std::endl;
  os << "  italic: " << twostate_enum_to_value(paragraph.italic) << std::endl;
  os << "  bold: " << twostate_enum_to_value(paragraph.bold) << std::endl;
  os << "  underline: " << twostate_enum_to_value(paragraph.underline) << std::endl;
  os << "  smallcaps: " << twostate_enum_to_value(paragraph.smallcaps) << std::endl;
  os << "  text_alignment: " << textalignment_enum_to_value(paragraph.text_alignment) << std::endl;
  os << "  space_before: " << paragraph.space_before << std::endl;
  os << "  space_after: " << paragraph.space_after << std::endl;
  os << "  left_margin: " << paragraph.left_margin << std::endl;
  os << "  right_margin: " << paragraph.right_margin << std::endl;
  os << "  first_line_indent: " << paragraph.first_line_indent << std::endl;
  return os;
}


std::ostream& operator<<(std::ostream& os, const Style& style)
{
  os << "marker: " << style.marker << std::endl;
  os << "type: " << type_enum_to_value(style.type) << std::endl;
  os << "name: " << style.name << std::endl;
  os << "info: " << style.info << std::endl;
  if (style.paragraph)
    os << style.paragraph.value();
  if (style.character)
    os << style.character.value();
  for (const auto& [property, parameter] : style.properties) {
    os << "capability: " << property_enum_to_value(property) << ": ";
    if (std::holds_alternative<bool>(parameter))
      os << std::get<bool>(parameter);
    if (std::holds_alternative<int>(parameter))
      os << std::get<int>(parameter);
    if (std::holds_alternative<std::string>(parameter))
      os << std::get<std::string>(parameter);
    os << std::endl;
  }
  os << "doc: " << style.doc << std::endl;
  os << "category: " << style.category << std::endl;
  return os;
}


bool has_property (const Style* style, const Property property)
{
  return (style->properties.find(property) != style->properties.cend());
}


// Insert the style definitions here.
#include "definitions.hpp"


// Whether this style starts a new line in USFM.
bool starts_new_line_in_usfm (const Style* style)
{
  switch (style->type) {
    case Type::starting_boundary:
    case Type::none:
    case Type::book_id:
    case Type::usfm_version:
    case Type::file_encoding:
    case Type::remark:
    case Type::running_header:
    case Type::long_toc_text:
    case Type::short_toc_text:
    case Type::book_abbrev:
    case Type::introduction_end:
    case Type::title:
    case Type::heading:
    case Type::paragraph:
    case Type::chapter:
    case Type::chapter_label:
    case Type::published_chapter_marker:
    case Type::alternate_chapter_number:
      return true;
    case Type::verse:
      return true;
    case Type::published_verse_marker:
    case Type::alternate_verse_marker:
      return false;
    case Type::table_row:
      return true;
    case Type::table_heading:
    case Type::table_cell:
      return false;
    case Type::footnote_wrapper:
    case Type::endnote_wrapper:
    case Type::note_standard_content:
    case Type::note_content:
    case Type::note_content_with_endmarker:
    case Type::note_paragraph:
      return false;
    case Type::crossreference_wrapper:
    case Type::crossreference_standard_content:
    case Type::crossreference_content:
    case Type::crossreference_content_with_endmarker:
      return false;
    case Type::character:
      return false;
    case Type::page_break:
      return true;
    case Type::figure:
      return true;
    case Type::word_list:
      return false;
    case Type::sidebar_begin:
    case Type::sidebar_end:
      return true;
    case Type::peripheral:
      return true;
    case Type::milestone:
      return false;
    case Type::stopping_boundary:
    default:
      return true;
  }
}


std::string validate_notes_dump (const std::string& input)
{
  // Validate the input.
  if (input == "book")
    return input;
  if (input == "end")
    return input;
  if (filter::usfm::is_usfm_marker(input))
    return input;
  // On validation failure, return default notes dump location.
  return "book";
}


// The name of the "Standard" stylesheet.
std::string standard_sheet ()
{
  return "Standard";
}


} // Namespace.
