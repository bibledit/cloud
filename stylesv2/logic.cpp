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


#include <stylesv2/logic.h>
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
    case Type::character_style:
      if (describe)
        return "character style";
    case Type::page_break:
      if (describe)
        return "page break";
      return "page_break";
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
    case Category::extended_study_content:
      os << "Extended Study Content";
      break;
    case Category::peripherals:
      os << "Peripherals";
      break;
    case Category::unknown:
    default:
      os << "Unknown";
      break;
  }
  return os;
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
  return os;
}


bool has_property (const Style* style, const Property property)
{
  return (style->properties.find(property) != style->properties.cend());
}


const std::list<Style> styles {
  {
    .marker = "id",
    .type = Type::book_id,
    .name = "Identification",
    .info = "File identification information (name of file, book name, language, last edited, date, etc.)",
    .properties = {{Property::starts_new_page,true}},
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#index-1",
    .category = Category::identification,
  },
  {
    .marker = "usfm",
    .type = Type::usfm_version,
    .name = "USFM version",
    .info = "USFM version specification for the file. Used to identify the USFM version which a USFM editor / processor will be required to support in order to manage all markup found within the file.",
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#usfm",
    .category = Category::identification,
  },
  {
    .marker = "ide",
    .type = Type::file_encoding,
    .name = "Encoding",
    .info = "File encoding information. Bibledit disregards this marker, as all text in Bibledit is in UTF-8 encoding.",
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#index-3",
    .category = Category::identification,
  },
  {
    .marker = "sts",
    .type = Type::remark,
    .name = "Status",
    .info = "Project text status tracking.",
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#sts",
    .category = Category::identification,
  },
  {
    .marker = "rem",
    .type = Type::remark,
    .name = "Remark",
    .info = "Comments and remarks.",
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#rem",
    .category = Category::identification,
  },
  {
    .marker = "h",
    .type = Type::running_header,
    .name = "Running header",
    .info = "Running header text for a book.",
    .properties = {{Property::on_left_page,true},{Property::on_right_page,true}},
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#h",
    .category = Category::identification,
  },
  {
    .marker = "h1",
    .type = Type::running_header,
    .name = "Running header",
    .info = "Running header text for a book.",
    .properties = {
      {Property::on_left_page,true},
      {Property::on_right_page,true},
      {Property::deprecated,std::monostate()}
    },
      .doc = "https://ubsicap.github.io/usfm/identification/index.html#h",
      .category = Category::identification,
  },
  {
    .marker = "h2",
    .type = Type::running_header,
    .name = "Left running header",
    .info = "Running header text for a book, left page.",
    .properties = {
      {Property::on_left_page,true},
      {Property::on_right_page,false},
      {Property::deprecated,std::monostate()}
    },
      .doc = "https://ubsicap.github.io/usfm/identification/index.html#h",
      .category = Category::identification,
  },
  {
    .marker = "h3",
    .type = Type::running_header,
    .name = "Right running header",
    .info = "Running header text for a book, right page.",
    .properties = {
      {Property::on_left_page,false},
      {Property::on_right_page,true},
      {Property::deprecated,std::monostate()}
    },
      .doc = "https://ubsicap.github.io/usfm/identification/index.html#h",
      .category = Category::identification,
  },
  {
    .marker = "toc1",
    .type = Type::long_toc_text,
    .name = "Long TOC text",
    .info = "Long table of contents text.",
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#toc",
    .category = Category::identification,
  },
  {
    .marker = "toc2",
    .type = Type::short_toc_text,
    .name = "Short TOC text",
    .info = "Short table of contents text.",
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#toc",
    .category = Category::identification,
  },
  {
    .marker = "toc3",
    .type = Type::book_abbrev,
    .name = "Book abbreviation",
    .info = "Book abbreviation for the table of contents.",
    .doc = "https://ubsicap.github.io/usfm/identification/index.html#toc",
    .category = Category::identification,
  },
  {
    .marker = "imt",
    .type = Type::title,
    .name = "Introduction major title level 1",
    .info = "Introduction major title, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 14,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imt",
      .category = Category::introductions,
  },
  {
    .marker = "imt1",
    .type = Type::title,
    .name = "Introduction major title level 1",
    .info = "Introduction major title, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 14,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imt",
      .category = Category::introductions,
  },
  {
    .marker = "imt2",
    .type = Type::title,
    .name = "Introduction major title level 2",
    .info = "Introduction major title, level 2.",
    .paragraph = Paragraph {
      .font_size = 13,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 6,
      .space_after = 3,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imt",
      .category = Category::introductions,
  },
  {
    .marker = "imt3",
    .type = Type::title,
    .name = "Introduction major title level 3",
    .info = "Introduction major title, level 3.",
    .paragraph = Paragraph {
      .font_size = 12,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 2,
      .space_after = 2,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imt",
      .category = Category::introductions,
  },
  {
    .marker = "imt4",
    .type = Type::title,
    .name = "Introduction major title level 4",
    .info = "Introduction major title, level 4.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 2,
      .space_after = 2,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imt",
      .category = Category::introductions,
  },
  {
    .marker = "is",
    .type = Type::heading,
    .name = "Introduction section heading level 1",
    .info = "Introduction section heading, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 14,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#is",
      .category = Category::introductions,
  },
  {
    .marker = "is1",
    .type = Type::heading,
    .name = "Introduction section heading level 1",
    .info = "Introduction section heading, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 14,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#is",
      .category = Category::introductions,
  },
  {
    .marker = "is2",
    .type = Type::heading,
    .name = "Introduction section heading level 2",
    .info = "Introduction section heading, level 2.",
    .paragraph = Paragraph {
      .font_size = 12,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#is",
      .category = Category::introductions,
  },
  {
    .marker = "ip",
    .type = Type::paragraph,
    .name = "Introduction paragraph",
    .info = "Introduction prose paragraph.",
    .paragraph = Paragraph {
      .font_size = 12,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ip",
      .category = Category::introductions,
  },
  {
    .marker = "ipi",
    .type = Type::paragraph,
    .name = "Indented introduction paragraph",
    .info = "Introduction prose paragraph, with first line indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ipi",
      .category = Category::introductions,
  },
  {
    .marker = "im",
    .type = Type::paragraph,
    .name = "Introduction margin paragraph",
    .info = "Introduction prose paragraph, with no first line indent. May occur after poetry.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#im",
      .category = Category::introductions,
  },
  {
    .marker = "imi",
    .type = Type::paragraph,
    .name = "Indented introduction margin paragraph",
    .info = "Introduction prose paragraph text, indented, with no first line indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imi",
      .category = Category::introductions,
  },
  {
    .marker = "ipq",
    .type = Type::paragraph,
    .name = "Introduction quote from text paragraph",
    .info = "Introduction prose paragraph, quote from the body text.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ipq",
      .category = Category::introductions,
  },
  {
    .marker = "imq",
    .type = Type::paragraph,
    .name = "Introduction margin quote from text paragraph",
    .info = "Introduction prose paragraph, quote from the body text, with no first line indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imq",
      .category = Category::introductions,
  },
  {
    .marker = "ipr",
    .type = Type::paragraph,
    .name = "Introduction paragraph right-aligned",
    .info = "Introduction prose paragraph, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::right,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ipr",
      .category = Category::introductions,
  },
  {
    .marker = "iq",
    .type = Type::paragraph,
    .name = "Introduction poetic line",
    .info = "Introduction poetry text, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .left_margin = 25.4f,
      .first_line_indent = -19.1f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#iq",
      .category = Category::introductions,
  },
  {
    .marker = "iq1",
    .type = Type::paragraph,
    .name = "Introduction poetic line level 1",
    .info = "Introduction poetry text, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .left_margin = 25.4f,
      .first_line_indent = -19.1f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#iq",
      .category = Category::introductions,
  },
  {
    .marker = "iq2",
    .type = Type::paragraph,
    .name = "Introduction poetic line level 2",
    .info = "Introduction poetry text, level 2, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .left_margin = 25.4f,
      .first_line_indent = -12.7f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#iq",
      .category = Category::introductions,
  },
  {
    .marker = "iq3",
    .type = Type::paragraph,
    .name = "Introduction poetic line level 3",
    .info = "Introduction poetry text, level 3, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .left_margin = 25.4f,
      .first_line_indent = -6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#iq",
      .category = Category::introductions,
  },
  {
    .marker = "ib",
    .type = Type::paragraph,
    .name = "Introduction blank line",
    .info = "May be used to explicitly indicate additional white space between paragraphs.",
    .paragraph = Paragraph {
      .font_size = 10,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ib",
      .category = Category::introductions,
  },
  {
    .marker = "ili",
    .type = Type::paragraph,
    .name = "Introduction list item",
    .info = "A list entry, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ili",
      .category = Category::introductions,
  },
  {
    .marker = "ili1",
    .type = Type::paragraph,
    .name = "Introduction list item level 1",
    .info = "A list entry, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ili",
      .category = Category::introductions,
  },
  {
    .marker = "ili2",
    .type = Type::paragraph,
    .name = "Introduction list item level 2",
    .info = "A list entry, level 2.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 19.1f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ili",
      .category = Category::introductions,
  },
  {
    .marker = "iot",
    .type = Type::title,
    .name = "Introduction outline title",
    .info = "Introduction outline title.",
    .paragraph = Paragraph {
      .font_size = 12,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#iot",
      .category = Category::introductions,
  },
  {
    .marker = "io",
    .type = Type::heading,
    .name = "Introduction outline entry",
    .info = "Introduction outline text, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#io",
      .category = Category::introductions,
  },
  {
    .marker = "io1",
    .type = Type::heading,
    .name = "Introduction outline entry level 1",
    .info = "Introduction outline text, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#io",
      .category = Category::introductions,
  },
  {
    .marker = "io2",
    .type = Type::heading,
    .name = "Introduction outline entry level 2",
    .info = "Introduction outline text, level 2.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 19.1f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#io",
      .category = Category::introductions,
  },
  {
    .marker = "io3",
    .type = Type::heading,
    .name = "Introduction outline entry level 3",
    .info = "Introduction outline text, level 3.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#io",
      .category = Category::introductions,
  },
  {
    .marker = "io4",
    .type = Type::heading,
    .name = "Introduction outline entry level 4",
    .info = "Introduction outline text, level 4.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 31.8f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#io",
      .category = Category::introductions,
  },
  {
    .marker = "ior",
    .type = Type::character_style,
    .name = "Introduction outline reference range",
    .info = "Introduction references range for outline entry; for marking references separately.",
    .character = Character {
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ior-ior",
      .category = Category::introductions,
  },
  {
    .marker = "iqt",
    .type = Type::character_style,
    .name = "Introduction quoted text",
    .info = "Scripture quotations, or other quoted text, appearing in the introduction.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#iqt-iqt",
      .category = Category::introductions,
  },
  {
    .marker = "iex",
    .type = Type::paragraph,
    .name = "Explanatory or bridge text",
    .info = "Introduction explanatory or bridge text, e.g. explanation of missing book in short Old Testament.",
    .paragraph = Paragraph {
      .font_size = 12,
      .space_before = 4,
      .space_after = 4,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#iex",
      .category = Category::introductions,
  },
  {
    .marker = "imte",
    .type = Type::title,
    .name = "Introduction major title ending",
    .info = "Used to mark a major title indicating the end of the introduction.",
    .paragraph = Paragraph {
      .font_size = 20,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imte",
      .category = Category::introductions,
  },
  {
    .marker = "imte1",
    .type = Type::title,
    .name = "Introduction major title ending level 1",
    .info = "Used to mark a major title indicating the end of the introduction.",
    .paragraph = Paragraph {
      .font_size = 20,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imte",
      .category = Category::introductions,
  },
  {
    .marker = "imte2",
    .type = Type::title,
    .name = "Introduction major title ending level 2",
    .info = "Used to mark a major title indicating the end of the introduction.",
    .paragraph = Paragraph {
      .font_size = 16,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_after = 2,
    },
      .doc = "https://ubsicap.github.io/usfm/introductions/index.html#imte",
      .category = Category::introductions,
  },
  {
    .marker = "ie",
    .type = Type::introduction_end,
    .name = "Introduction end",
    .info = "Optionally included to explicitly indicate the end of the introduction material.",
    .doc = "https://ubsicap.github.io/usfm/introductions/index.html#ie",
    .category = Category::introductions,
  },
  {
    .marker = "mt",
    .type = Type::title,
    .name = "Major title",
    .info = "The main title of the book, if single level.",
    .paragraph = Paragraph {
      .font_size = 20,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mt",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mt1",
    .type = Type::title,
    .name = "Major title level 1",
    .info = "The main title of the book, level 1 if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 20,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mt",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mt2",
    .type = Type::title,
    .name = "Major title level 2",
    .info = "The main title of the book, level 2.",
    .paragraph = Paragraph {
      .font_size = 16,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_after = 2,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mt",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mt3",
    .type = Type::title,
    .name = "Major title level 3",
    .info = "A secondary title, less important than the main title.",
    .paragraph = Paragraph {
      .font_size = 16,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 2,
      .space_after = 2,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mt",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mt4",
    .type = Type::title,
    .name = "Major title level 4",
    .info = "A small secondary title sometimes occurring within parentheses.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::center,
      .space_before = 2,
      .space_after = 2,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mt",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mte",
    .type = Type::title,
    .name = "Major title at ending",
    .info = "The main title of the book repeated at the end of the book, if single level.",
    .paragraph = Paragraph {
      .font_size = 20,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mte",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mte1",
    .type = Type::title,
    .name = "Major title at ending level 1",
    .info = "The main title of the book repeated at the end of the book, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 20,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mte",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mte2",
    .type = Type::title,
    .name = "Major title at ending level 2",
    .info = "A secondary title occurring before or after the ending main title.",
    .paragraph = Paragraph {
      .font_size = 16,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_after = 2,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mte",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "ms",
    .type = Type::title,
    .name = "Major section heading",
    .info = "A major section division heading, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 14,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 16,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#ms",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "ms1",
    .type = Type::title,
    .name = "Major section heading level 1",
    .info = "A major section division heading, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 14,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 16,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#ms",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "ms2",
    .type = Type::title,
    .name = "Major section heading level 2",
    .info = "A major section division heading, level 2.",
    .paragraph = Paragraph {
      .font_size = 14,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 16,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#ms",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "mr",
    .type = Type::title,
    .name = "Major section reference range",
    .info = "The text reference range listed under a major section heading.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#mr",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "s",
    .type = Type::heading,
    .name = "Section heading",
    .info = "A section heading, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 12,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#s",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "s1",
    .type = Type::heading,
    .name = "Section heading level 1",
    .info = "A section heading, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#s",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "s2",
    .type = Type::heading,
    .name = "Section heading level 2",
    .info = "A section heading, level 2.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#s",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "s3",
    .type = Type::heading,
    .name = "Section heading level 3",
    .info = "A section heading, level 3.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 6,
      .space_after = 3,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#s",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "s4",
    .type = Type::heading,
    .name = "Section heading level 4",
    .info = "A section heading, level 4.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 6,
      .space_after = 3,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#s",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "sr",
    .type = Type::heading,
    .name = "Section reference range",
    .info = "The text reference range listed under a section heading.",
    .paragraph = Paragraph {
      .font_size = 12,
      .bold = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#sr",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "r",
    .type = Type::heading,
    .name = "Parallel passage references",
    .info = "Parallel references.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#r",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "rq",
    .type = Type::paragraph,
    .name = "Inline quotation references",
    .info = "A cross-reference indicating the source text for the preceding quotation.",
    .paragraph = Paragraph {
      .font_size = 10,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::right,
      .space_after = 4,
    },
      .properties = {{Property::has_endmarker,true}},
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#rq-rq",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "d",
    .type = Type::title,
    .name = "Descriptive title or Hebrew subtitle",
    .info = "A Hebrew text heading, to provide description, e.g. Psalms.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .text_alignment = TextAlignment::center,
      .space_before = 4,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#d",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "sp",
    .type = Type::heading,
    .name = "Speaker",
    .info = "A heading to identify the speaker.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#sp",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "sd",
    .type = Type::paragraph,
    .name = "Semantic division",
    .info = "Vertical space used to divide the text into sections, in a manner similar to the structure added through the use of a sequence of heading texts.",
    .paragraph = Paragraph {
      .font_size = 20,
      .space_before = 24,
      .space_after = 24,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#sd",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "sd1",
    .type = Type::paragraph,
    .name = "Semantic division level 1",
    .info = "Vertical space used to divide the text into sections, in a manner similar to the structure added through the use of a sequence of heading texts.",
    .paragraph = Paragraph {
      .font_size = 20,
      .space_before = 24,
      .space_after = 24,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#sd",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "sd2",
    .type = Type::paragraph,
    .name = "Semantic division level 2",
    .info = "Vertical space used to divide the text into sections, in a manner similar to the structure added through the use of a sequence of heading texts.",
    .paragraph = Paragraph {
      .font_size = 16,
      .space_before = 18,
      .space_after = 18,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#sd",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "sd3",
    .type = Type::paragraph,
    .name = "Semantic division level 3",
    .info = "Vertical space used to divide the text into sections, in a manner similar to the structure added through the use of a sequence of heading texts.",
    .paragraph = Paragraph {
      .font_size = 16,
      .space_before = 12,
      .space_after = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#sd",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "sd4",
    .type = Type::paragraph,
    .name = "Semantic division level 4",
    .info = "Vertical space used to divide the text into sections, in a manner similar to the structure added through the use of a sequence of heading texts.",
    .paragraph = Paragraph {
      .font_size = 16,
      .space_before = 8,
      .space_after = 8,
    },
      .doc = "https://ubsicap.github.io/usfm/titles_headings/index.html#sd",
      .category = Category::titles_headings_labels,
  },
  {
    .marker = "c",
    .type = Type::chapter,
    .name = "Chapter number",
    .info = "For the chapter number.",
    .paragraph = Paragraph {
      .font_size = 18,
      .bold = TwoState::on,
      .space_before = 8,
      .space_after = 4,
    },
      .properties = {
        {Property::at_first_verse,true},
        {Property::on_left_page,true},
        {Property::on_right_page,true}
      },
      .doc = "https://ubsicap.github.io/usfm/chapters_verses/index.html#c",
      .category = Category::chapters_verses,
  },
  {
    .marker = "ca",
    .type = Type::alternate_chapter_number,
    .name = "Alternate chapter number",
    .info = "Second or alternate chapter number. For coding dual versification. Useful for places where different traditions of chapter breaks need to be supported in the same translation.",
    .doc = "https://ubsicap.github.io/usfm/chapters_verses/index.html#ca-ca",
    .category = Category::chapters_verses,
  },
  {
    .marker = "cl",
    .type = Type::chapter_label,
    .name = "Chapter label",
    .info = "Chapter label used for translations that add a word such as 'Chapter' before chapter numbers, e.g. Psalms.",
    .doc = "https://ubsicap.github.io/usfm/chapters_verses/index.html#cl",
    .category = Category::chapters_verses,
  },
  {
    .marker = "cp",
    .type = Type::published_chapter_marker,
    .name = "Published chapter character",
    .info = "Published chapter number. This is a chapter marking that would be used in the published text.",
    .doc = "https://ubsicap.github.io/usfm/chapters_verses/index.html#cp",
    .category = Category::chapters_verses,
  },
  {
    .marker = "cd",
    .type = Type::paragraph,
    .name = "Chapter description",
    .info = "For a brief description of the chapter content.",
    .paragraph = Paragraph {
      .font_size = 11,
      .space_before = 8,
      .space_after = 4,
    },
      .doc = "https://ubsicap.github.io/usfm/chapters_verses/index.html#cd",
      .category = Category::chapters_verses,
  },
  {
    .marker = "v",
    .type = Type::verse,
    .name = "Verse number",
    .info = "A verse number.",
    .character = Character {
      .superscript = TwoState::on,
    },
      .properties = {{Property::restart_paragraph,false}},
      .doc = "https://ubsicap.github.io/usfm/chapters_verses/index.html#v",
      .category = Category::chapters_verses,
  },
  {
    .marker = "vp",
    .type = Type::published_verse_marker,
    .name = "Published verse marker",
    .info = "Published verse marker. This is a verse marking that would be used in the published text.",
    .doc = "https://ubsicap.github.io/usfm/chapters_verses/index.html#vp-vp",
    .category = Category::chapters_verses,
  },
  {
    .marker = "p",
    .type = Type::paragraph,
    .name = "Normal paragraph",
    .info = "Paragraph text, with first line indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#p",
      .category = Category::paragraphs,
  },
  {
    .marker = "m",
    .type = Type::paragraph,
    .name = "Continuation paragraph",
    .info = "Margin paragraph text, with no first line indent.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#m",
      .category = Category::paragraphs,
  },
  {
    .marker = "po",
    .type = Type::paragraph,
    .name = "Paragraph opening an epistle",
    .info = "Paragraph opening an epistle or letter, without first line indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .space_before = 4,
      .space_after = 4,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#po",
      .category = Category::paragraphs,
  },
  {
    .marker = "pr",
    .type = Type::paragraph,
    .name = "Right-aligned paragraph",
    .info = "Right-aligned paragraph text, without first line indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pr",
      .category = Category::paragraphs,
  },
  {
    .marker = "cls",
    .type = Type::paragraph,
    .name = "Closure of a letter",
    .info = "For a closure of an letter.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#cls",
      .category = Category::paragraphs,
  },
  {
    .marker = "pmo",
    .type = Type::paragraph,
    .name = "Embedded text opening",
    .info = "For an embedded text opening.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pmo",
      .category = Category::paragraphs,
  },
  {
    .marker = "pm",
    .type = Type::paragraph,
    .name = "Embedded text paragraph",
    .info = "For an embedded text paragraph.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pm",
      .category = Category::paragraphs,
  },
  {
    .marker = "pmc",
    .type = Type::paragraph,
    .name = "Embedded text closing",
    .info = "For an embedded text closing.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pmc",
      .category = Category::paragraphs,
  },
  {
    .marker = "pmr",
    .type = Type::paragraph,
    .name = "Embedded text refrain",
    .info = "Embedded text refrain, e.g. Then all the people shall say, Amen!",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pmc",
      .category = Category::paragraphs,
  },
  {
    .marker = "pi",
    .type = Type::paragraph,
    .name = "Indented paragraph",
    .info = "Paragraph text, level 1 indent if single level, with first line indent; often used for discourse.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pi",
      .category = Category::paragraphs,
  },
  {
    .marker = "pi1",
    .type = Type::paragraph,
    .name = "Indented paragraph level 1",
    .info = "Paragraph text, level 1 indent if multiple levels, with first line indent; often used for discourse.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pi",
      .category = Category::paragraphs,
  },
  {
    .marker = "pi2",
    .type = Type::paragraph,
    .name = "Indented paragraph level 2",
    .info = "Paragraph text, level 2 indent if multiple levels, with first line indent; often used for discourse.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
      .right_margin = 6.4f,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pi",
      .category = Category::paragraphs,
  },
  {
    .marker = "pi3",
    .type = Type::paragraph,
    .name = "Indented paragraph level 3",
    .info = "Paragraph text, level 3 indent if multiple levels, with first line indent; often used for discourse.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 19.1f,
      .right_margin = 6.4f,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pi",
      .category = Category::paragraphs,
  },
  {
    .marker = "mi",
    .type = Type::paragraph,
    .name = "Indented flush left paragraph",
    .info = "Margin paragraph text, indented, with no first line indent, often used for discourse.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
      .right_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#mi",
      .category = Category::paragraphs,
  },
  {
    .marker = "nb",
    .type = Type::paragraph,
    .name = "No break with previous paragraph",
    .info = "Paragraph text, with no break from previous paragraph text at chapter boundary.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#nb",
      .category = Category::paragraphs,
  },
  {
    .marker = "pc",
    .type = Type::paragraph,
    .name = "Centered paragraph",
    .info = "Paragraph text, centered, for inscription.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::center,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#pc",
      .category = Category::paragraphs,
  },
  {
    .marker = "ph",
    .type = Type::paragraph,
    .name = "Paragraph with hanging indent",
    .info = "Indented paragraph with hanging indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
      .first_line_indent = -9.5f,
    },
      .properties = {{Property::deprecated,std::monostate()}},
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#ph",
      .category = Category::paragraphs,
  },
  {
    .marker = "ph1",
    .type = Type::paragraph,
    .name = "Paragraph with hanging indent level 1",
    .info = "Indented paragraph with hanging indent, level 1 if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
      .first_line_indent = -9.5f,
    },
      .properties = {{Property::deprecated,std::monostate()}},
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#ph",
      .category = Category::paragraphs,
  },
  {
    .marker = "ph2",
    .type = Type::paragraph,
    .name = "Paragraph with hanging indent level 2",
    .info = "Indented paragraph with hanging indent, level 2.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 19.1f,
      .first_line_indent = -9.5f,
    },
      .properties = {{Property::deprecated,std::monostate()}},
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#ph",
      .category = Category::paragraphs,
  },
  {
    .marker = "ph3",
    .type = Type::paragraph,
    .name = "Paragraph with hanging indent level 3",
    .info = "Indented paragraph with hanging indent, level 3.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
      .first_line_indent = -9.5f,
    },
      .properties = {{Property::deprecated,std::monostate()}},
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#ph",
      .category = Category::paragraphs,
  },
  {
    .marker = "ph4",
    .type = Type::paragraph,
    .name = "Paragraph with hanging indent level 4",
    .info = "Indented paragraph with hanging indent, level 4.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 31.8f,
      .first_line_indent = -9.5f,
    },
      .properties = {{Property::deprecated,std::monostate()}},
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#ph",
      .category = Category::paragraphs,
  },
  {
    .marker = "b",
    .type = Type::paragraph,
    .name = "Blank line",
    .info = "May be used to explicitly indicate additional white space between paragraphs. Poetry text stanza break.",
    .paragraph = Paragraph {
      .font_size = 10,
    },
      .doc = "https://ubsicap.github.io/usfm/paragraphs/index.html#b",
      .category = Category::paragraphs,
  },
  {
    .marker = "q",
    .type = Type::paragraph,
    .name = "Poetic line",
    .info = "Poetry text, level 1 indent if single level.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 31.8f,
      .first_line_indent = -25.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#q",
      .category = Category::poetry,
  },
  {
    .marker = "q1",
    .type = Type::paragraph,
    .name = "Poetic line",
    .info = "Poetry text, level 1 indent if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 31.8f,
      .first_line_indent = -25.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#q",
      .category = Category::poetry,
  },
  {
    .marker = "q2",
    .type = Type::paragraph,
    .name = "Poetic line",
    .info = "Poetry text, level 2 indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 31.8f,
      .first_line_indent = -19.1f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#q",
      .category = Category::poetry,
  },
  {
    .marker = "q3",
    .type = Type::paragraph,
    .name = "Poetic line",
    .info = "Poetry text, level 3 indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 31.8f,
      .first_line_indent = -12.7f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#q",
      .category = Category::poetry,
  },
  {
    .marker = "qr",
    .type = Type::paragraph,
    .name = "Right-aligned poetic line",
    .info = "Commonly used for a poetic refrain.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qr",
      .category = Category::poetry,
  },
  {
    .marker = "qc",
    .type = Type::paragraph,
    .name = "Centered poetic line",
    .info = "Poetry text, centered.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::center,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qc",
      .category = Category::poetry,
  },
  {
    .marker = "qs",
    .type = Type::character_style,
    .name = "Selah",
    .info = "Poetry text, Selah.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qs-qs",
      .category = Category::poetry,
  },
  {
    .marker = "qa",
    .type = Type::paragraph,
    .name = "Acrostic heading",
    .info = "Poetry text, acrostic marker or heading.",
    .paragraph = Paragraph {
      .font_size = 12,
      .italic = TwoState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qa",
      .category = Category::poetry,
  },
  {
    .marker = "qac",
    .type = Type::character_style,
    .name = "Acrostic letter",
    .info = "Poetry text, acrostic markup of the first character of a line of acrostic poetry.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qac-qac",
      .category = Category::poetry,
  },
  {
    .marker = "qm",
    .type = Type::paragraph,
    .name = "Embedded text poetic line",
    .info = "Poetry text, embedded, level 1 indent, if single level.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
      .first_line_indent = -19.1f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qm",
      .category = Category::poetry,
  },
  {
    .marker = "qm1",
    .type = Type::paragraph,
    .name = "Embedded text poetic line level 1",
    .info = "Poetry text, embedded, level 1 indent, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
      .first_line_indent = -19.1f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qm",
      .category = Category::poetry,
  },
  {
    .marker = "qm2",
    .type = Type::paragraph,
    .name = "Embedded text poetic line level 2",
    .info = "Poetry text, embedded, level 2 indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
      .first_line_indent = -12.7f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qm",
      .category = Category::poetry,
  },
  {
    .marker = "qm3",
    .type = Type::paragraph,
    .name = "Embedded text poetic line level 3",
    .info = "Poetry text, embedded, level 3 indent.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
      .first_line_indent = -6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qm",
      .category = Category::poetry,
  },
  {
    .marker = "qd",
    .type = Type::paragraph,
    .name = "Hebrew note",
    .info = "A Hebrew musical performance comment similar in content to many of the Hebrew Psalm titles, but placed at the end of the poetic section.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 6.4f,
    },
      .doc = "https://ubsicap.github.io/usfm/poetry/index.html#qd",
      .category = Category::poetry,
  },
  {
    .marker = "lh",
    .type = Type::paragraph,
    .name = "List header",
    .info = "Some lists include an introductory and concluding remark. They are an integral part of the list content, but are not list items. A list does not require either or both of these elements.",
    .paragraph = Paragraph {
      .font_size = 12,
      .first_line_indent = 3.2f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#lh",
      .category = Category::lists,
  },
  {
    .marker = "li",
    .type = Type::paragraph,
    .name = "List entry",
    .info = "A list entry, level 1, if single level.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#li",
      .category = Category::lists,
  },
  {
    .marker = "li1",
    .type = Type::paragraph,
    .name = "List entry level 1",
    .info = "A list entry, level 1, if multiple levels.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 12.7f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#li",
      .category = Category::lists,
  },
  {
    .marker = "li2",
    .type = Type::paragraph,
    .name = "List entry level 2",
    .info = "A list entry, level 2.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 19.1f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#li",
      .category = Category::lists,
  },
  {
    .marker = "li3",
    .type = Type::paragraph,
    .name = "List entry level 3",
    .info = "A list entry, level 3.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#li",
      .category = Category::lists,
  },
  {
    .marker = "li4",
    .type = Type::paragraph,
    .name = "List entry level 4",
    .info = "A list entry, level 4.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 31.8f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#li",
      .category = Category::lists,
  },
  {
    .marker = "lf",
    .type = Type::paragraph,
    .name = "List footer",
    .info = "Some lists include an introductory and concluding remark. They are an integral part of the list content, but are not list items. A list does not require either or both of these elements.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 10.0f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#lf",
      .category = Category::lists,
  },
  {
    .marker = "lim",
    .type = Type::paragraph,
    .name = "Embedded list entry",
    .info = "An out-dented paragraph meant to highlight the items of an embedded list.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 19.1f,
      .right_margin = 6.4f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#lim",
      .category = Category::lists,
  },
  {
    .marker = "lim1",
    .type = Type::paragraph,
    .name = "Embedded list entry level 1",
    .info = "An out-dented paragraph meant to highlight the items of an embedded list, level 1.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 19.1f,
      .right_margin = 6.4f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#lim",
      .category = Category::lists,
  },
  {
    .marker = "lim2",
    .type = Type::paragraph,
    .name = "Embedded list entry level 2",
    .info = "An out-dented paragraph meant to highlight the items of an embedded list, level 2.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 25.4f,
      .first_line_indent = -9.5f,
    },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#lim",
      .category = Category::lists,
  },
  {
    .marker = "litl",
    .type = Type::character_style,
    .name = "List entry total",
    .info = "Use in accounting lists for denoting the total component of the text within a list item.",
    .character = Character { },
      .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#litl-litl",
      .category = Category::lists,
  },
  {
    .marker = "lik",
    .type = Type::character_style,
    .name = "List entry key",
    .info = "For list entry key content.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#lik-lik",
    .category = Category::lists,
  },
  {
    .marker = "liv",
    .type = Type::character_style,
    .name = "List entry value",
    .info = "For list entry value content.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#liv-liv",
    .category = Category::lists,
  },
  {
    .marker = "liv1",
    .type = Type::character_style,
    .name = "List entry value level 1",
    .info = "For list entry value content level 1.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#liv-liv",
    .category = Category::lists,
  },
  {
    .marker = "liv2",
    .type = Type::character_style,
    .name = "List entry value level 2",
    .info = "For list entry value content level 2.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/master/lists/index.html#liv-liv",
    .category = Category::lists,
  },
  {
    .marker = "tr",
    .type = Type::table_row,
    .name = "Table row start",
    .info = "A new table row.",
    .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tr",
    .category = Category::tables,
  },
  {
    .marker = "th1",
    .type = Type::table_heading,
    .name = "Table column 1 heading",
    .info = "A table heading, column 1.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#th",
      .category = Category::tables,
  },
  {
    .marker = "th2",
    .type = Type::table_heading,
    .name = "Table column 2 heading",
    .info = "A table heading, column 2.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#th",
      .category = Category::tables,
  },
  {
    .marker = "th3",
    .type = Type::table_heading,
    .name = "Table column 3 heading",
    .info = "A table heading, column 3.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#th",
      .category = Category::tables,
  },
  {
    .marker = "th4",
    .type = Type::table_heading,
    .name = "Table column 4 heading",
    .info = "A table heading, column 4.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#th",
      .category = Category::tables,
  },
  {
    .marker = "thr1",
    .type = Type::table_heading,
    .name = "Right aligned table column 1 heading",
    .info = "A table heading, column 1, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#thr",
      .category = Category::tables,
  },
  {
    .marker = "thr2",
    .type = Type::table_heading,
    .name = "Right aligned table column 2 heading",
    .info = "A table heading, column 2, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#thr",
      .category = Category::tables,
  },
  {
    .marker = "thr3",
    .type = Type::table_heading,
    .name = "Right aligned table column 3 heading",
    .info = "A table heading, column 3, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#thr",
      .category = Category::tables,
  },
  {
    .marker = "thr4",
    .type = Type::table_heading,
    .name = "Right aligned table column 4 heading",
    .info = "A table heading, column 4, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#thr",
      .category = Category::tables,
  },
  {
    .marker = "tc1",
    .type = Type::table_cell,
    .name = "Table column 1 cell",
    .info = "A table cell item, column 1.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tc",
      .category = Category::tables,
  },
  {
    .marker = "tc2",
    .type = Type::table_cell,
    .name = "Table column 2 cell",
    .info = "A table cell item, column 2.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tc",
      .category = Category::tables,
  },
  {
    .marker = "tc3",
    .type = Type::table_cell,
    .name = "Table column 3 cell",
    .info = "A table cell item, column 3.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tc",
      .category = Category::tables,
  },
  {
    .marker = "tc4",
    .type = Type::table_cell,
    .name = "Table column 4 cell",
    .info = "A table cell item, column 4.",
    .paragraph = Paragraph {
      .font_size = 12,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tc",
      .category = Category::tables,
  },
  {
    .marker = "tcr1",
    .type = Type::table_cell,
    .name = "Right aligned table cell column 1",
    .info = "A table cell item, column 1, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tcr",
      .category = Category::tables,
  },
  {
    .marker = "tcr2",
    .type = Type::table_cell,
    .name = "Right aligned table cell column 2",
    .info = "A table cell item, column 2, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tcr",
      .category = Category::tables,
  },
  {
    .marker = "tcr3",
    .type = Type::table_cell,
    .name = "Right aligned table cell column 3",
    .info = "A table cell item, column 3, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tcr",
      .category = Category::tables,
  },
  {
    .marker = "tcr4",
    .type = Type::table_cell,
    .name = "Right aligned table cell column 4",
    .info = "A table cell item, column 4, right aligned.",
    .paragraph = Paragraph {
      .font_size = 12,
      .text_alignment = TextAlignment::right,
    },
      .doc = "https://ubsicap.github.io/usfm/master/tables/index.html#tcr",
      .category = Category::tables,
  },
  {
    .marker = "f",
    .type = Type::footnote_wrapper,
    .name = "Footnote",
    .info = "A footnote text item.",
    .character = Character { },
    .properties = {
      {Property::note_numbering_sequence,"1 2 3 4 5 6 7 8 9"},
      {Property::note_numbering_restart,"chapter"}
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#f-f",
      .category = Category::footnotes,
  },
  {
    .marker = "fe",
    .type = Type::endnote_wrapper,
    .name = "Endnote",
    .info = "An endnote text item.",
    .character = Character { },
    .properties = {
      {Property::note_numbering_sequence,"1 2 3 4 5 6 7 8 9"},
      {Property::notes_dump,"book"}
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fe-fe",
      .category = Category::footnotes,
  },
  {
    .marker = "fr",
    .type = Type::note_content,
    .name = "Footnote origin reference",
    .info = "This is the chapter and verse or verses that note refers to.",
    .character = Character {
      .bold = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fr",
      .category = Category::footnotes,
  },
  {
    .marker = "fq",
    .type = Type::note_content,
    .name = "Footnote translation quotation",
    .info = "A quotation from the current scripture text translation for which the note is being provided.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fq",
      .category = Category::footnotes,
  },
  {
    .marker = "fqa",
    .type = Type::note_content,
    .name = "Footnote alternate translation",
    .info = "Used to distinguish between a quotation of the current scripture text translation, and an alternate translation.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fqa",
      .category = Category::footnotes,
  },
  {
    .marker = "fk",
    .type = Type::note_content,
    .name = "Footnote keyword",
    .info = "The specific keyword or term from the text for which the footnote is being provided.",
    .character = Character {
      .italic = FourState::on,
      .bold = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fk",
      .category = Category::footnotes,
  },
  {
    .marker = "fl",
    .type = Type::note_content,
    .name = "Footnote label text",
    .info = "Can be used for labeling word(s) which are used consistently across certain types of translation notes, such as the words 'Or' in an alternative translation note, 'Others', 'Heb.', 'LXX' etc.",
    .character = Character {
      .italic = FourState::on,
      .bold = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fl",
      .category = Category::footnotes,
  },
  {
    .marker = "fw",
    .type = Type::note_content,
    .name = "Footnote witness list",
    .info = "For distinguishing a list of sigla representing witnesses in critical editions.",
    .character = Character {  },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fw",
    .category = Category::footnotes,
  },
  {
    .marker = "fp",
    .type = Type::note_paragraph,
    .name = "Footnote additional paragraph",
    .info = "Use this marker to if you need to indicate the start of a new paragraph within a footnote.",
    .paragraph = Paragraph {
      .font_size = 12,
      .left_margin = 3,
      .first_line_indent = 3,
    },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fp",
    .category = Category::footnotes,
  },
  {
    .marker = "fv",
    .type = Type::note_content_with_endmarker,
    .name = "Footnote verse number",
    .info = "A verse number within the footnote text.",
    .character = Character { .superscript = TwoState::on, },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fv-fv",
    .category = Category::footnotes,
  },
  {
    .marker = "ft",
    .type = Type::note_standard_content,
    .name = "Footnote text",
    .info = "The essential or explanatory text of the footnote.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#ft",
    .category = Category::footnotes,
  },
  {
    .marker = "fdc",
    .type = Type::note_content_with_endmarker,
    .name = "Footnote Deuterocanonical content",
    .info = "Text between these markers is material to be included only in published editions that contain the Deuterocanonical books.",
    .character = Character { },
    .properties = {{Property::deprecated,std::monostate()}},
    .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fdc-fdc",
    .category = Category::footnotes,
  },
  {
    .marker = "fm",
    .type = Type::note_content_with_endmarker,
    .name = "Footnote reference mark",
    .info = "An additional footnote marker location for a previous footnote.",
    .character = Character { .superscript = TwoState::on },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/fnotes.html#fm-fm",
    .category = Category::footnotes,
  },
  {
    .marker = "x",
    .type = Type::crossreference_wrapper,
    .name = "Cross reference",
    .info = "A list of cross references.",
    .character = Character { },
    .properties = {
      {Property::note_numbering_sequence,"a b c d e f g h i j k l m n o p q r s t u v w x y z"},
      {Property::note_numbering_restart,"chapter"}
    },
      .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#x-x",
      .category = Category::cross_references,
  },
  {
    .marker = "xo",
    .type = Type::crossreference_content,
    .name = "Cross reference origin reference",
    .info = "This is the chapter and verse or verses that target references are being provided for.",
    .character = Character { .bold = FourState::on  },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xo",
    .category = Category::cross_references,
  },
  {
    .marker = "xk",
    .type = Type::crossreference_content,
    .name = "Cross reference keyword",
    .info = "A keyword from the scripture translation text which the target references also refer to.",
    .character = Character { .italic = FourState::on  },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xk",
    .category = Category::cross_references,
  },
  {
    .marker = "xq",
    .type = Type::crossreference_content,
    .name = "Cross reference quotation",
    .info = "A cross reference quotation from the scripture text.",
    .character = Character { .italic = FourState::on  },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xq",
    .category = Category::cross_references,
  },
  {
    .marker = "xt",
    .type = Type::crossreference_standard_content,
    .name = "Cross reference target references",
    .info = "The cross reference target references.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xt",
    .category = Category::cross_references,
  },
  {
    .marker = "xta",
    .type = Type::crossreference_content,
    .name = "Target references added text",
    .info = "Used for marking text added to the target references.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xta",
    .category = Category::cross_references,
  },
  {
    .marker = "xop",
    .type = Type::crossreference_content_with_endmarker,
    .name = "Published cross reference origin text",
    .info = "Supplies the origin text intended for publishing.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xop-xop",
    .category = Category::cross_references,
  },
  {
    .marker = "xot",
    .type = Type::crossreference_content_with_endmarker,
    .name = "Cross reference Old Testament",
    .info = "Content between these markers is for inclusion in editions that contain the Old Testament books.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xot-xot",
    .category = Category::cross_references,
  },
  {
    .marker = "xnt",
    .type = Type::crossreference_content_with_endmarker,
    .name = "Cross reference New Testament",
    .info = "Content between these markers is for inclusion in editions that contain the New Testament books.",
    .character = Character { },
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xnt-xnt",
    .category = Category::cross_references,
  },
  {
    .marker = "xdc",
    .type = Type::crossreference_content_with_endmarker,
    .name = "Cross reference deuterocanonical",
    .info = "Content between these markers is for inclusion in editions that contain the Deuterocanonical books.",
    .character = Character { },
    .properties = {{Property::deprecated,std::monostate()}},
    .doc = "https://ubsicap.github.io/usfm/notes_basic/xrefs.html#xdc-xdc",
    .category = Category::cross_references,
  },
  {
    .marker = "add",
    .type = Type::character_style,
    .name = "Translator’s addition",
    .info = "A translator’s explanation - words added by the translator for clarity – text which is not literally a part of the original language, but which was supplied to make the meaning of the original clear.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#add-add",
      .category = Category::words_characters,
  },
  {
    .marker = "bk",
    .type = Type::character_style,
    .name = "Quoted book title",
    .info = "For the quoted name of a book.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#bk-bk",
      .category = Category::words_characters,
  },
  {
    .marker = "dc",
    .type = Type::character_style,
    .name = "Deuterocanonical",
    .info = "Deuterocanonical / LXX additions or insertions in the Protocanonical text.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#dc-dc",
      .category = Category::words_characters,
  },
  {
    .marker = "k",
    .type = Type::character_style,
    .name = "Keyword",
    .info = "Keyword / keyterm.",
    .character = Character {
      .italic = FourState::on,
      .bold = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#k-k",
      .category = Category::words_characters,
  },
  {
    .marker = "lit",
    .type = Type::paragraph,
    .name = "Liturgical note",
    .info = "A guide which tells the reader or worshipper that he should recite a prayer or recitation etc.",
    .paragraph = Paragraph {
      .font_size = 12,
      .bold = TwoState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#lit",
      .category = Category::words_characters,
  },
  {
    .marker = "nd",
    .type = Type::character_style,
    .name = "Name of God",
    .info = "For name of God (name of Deity).",
    .character = Character {
      .smallcaps = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#nd-nd",
      .category = Category::words_characters,
  },
  {
    .marker = "ord",
    .type = Type::character_style,
    .name = "Ordinal number ending",
    .info = "For the text portion of an ordinal number.",
    .character = Character {
      .superscript = TwoState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#ord-ord",
      .category = Category::words_characters,
  },
  {
    .marker = "pn",
    .type = Type::character_style,
    .name = "Proper name",
    .info = "For a proper name.",
    .character = Character {
      .bold = FourState::on,
      .underline = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#pn-pn",
      .category = Category::words_characters,
  },
  {
    .marker = "png",
    .type = Type::character_style,
    .name = "Geographic proper name",
    .info = "For a geographic proper name.",
    .character = Character {
      .bold = FourState::on,
      .underline = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#png-png",
      .category = Category::words_characters,
  },
  {
    .marker = "addpn",
    .type = Type::character_style,
    .name = "Combined add and pn style",
    .info = "Support for overlapping pn ...pn* and add ...add* occurrences.",
    .character = Character {
      .italic = FourState::on,
    },
      .properties = {{Property::deprecated,std::monostate()}},
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#addpn-addpn",
      .category = Category::words_characters,
  },
  {
    .marker = "qt",
    .type = Type::character_style,
    .name = "Quoted text",
    .info = "Old Testament quotations in the New Testament, or other quotations.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#qt-qt",
      .category = Category::words_characters,
  },
  {
    .marker = "sig",
    .type = Type::character_style,
    .name = "Signature of the author",
    .info = "Signature of the author of a letter or epistle.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#sig-sig",
      .category = Category::words_characters,
  },
  {
    .marker = "sls",
    .type = Type::character_style,
    .name = "Secondary language source",
    .info = "Passage of text based on a secondary language or alternate text source.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#sls-sls",
      .category = Category::words_characters,
  },
  {
    .marker = "tl",
    .type = Type::character_style,
    .name = "Transliterated text",
    .info = "Transliterated or foreign word or text.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#tl-tl",
      .category = Category::words_characters,
  },
  {
    .marker = "wj",
    .type = Type::character_style,
    .name = "Words of Jesus",
    .info = "For marking the words of Jesus.",
    .character = Character {
      .foreground_color = "#FF0000",
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#wj-wj",
      .category = Category::words_characters,
  },
  {
    .marker = "em",
    .type = Type::character_style,
    .name = "Emphasized text",
    .info = "For emphasizing a portion of text.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#em-em",
      .category = Category::words_characters,
  },
  {
    .marker = "bd",
    .type = Type::character_style,
    .name = "Bold text",
    .info = "For making a portion of text bold.",
    .character = Character {
      .bold = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#bd-bd",
      .category = Category::words_characters,
  },
  {
    .marker = "it",
    .type = Type::character_style,
    .name = "Italic text",
    .info = "For putting some text in italics.",
    .character = Character {
      .italic = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#it-it",
      .category = Category::words_characters,
  },
  {
    .marker = "bdit",
    .type = Type::character_style,
    .name = "Bold-italic text",
    .info = "For putting some text in bold and italics.",
    .character = Character {
      .italic = FourState::on,
      .bold = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#bdit-bdit",
      .category = Category::words_characters,
  },
  {
    .marker = "no",
    .type = Type::character_style,
    .name = "Normal text",
    .info = "May be used when a larger paragraph element is set in an alternate font style, e.g. italic, and a selected section of text should be displayed in normal text.",
    .character = Character {  },
    .doc = "https://ubsicap.github.io/usfm/characters/index.html#no-no",
    .category = Category::words_characters,
  },
  {
    .marker = "sc",
    .type = Type::character_style,
    .name = "Small-cap text",
    .info = "For putting some text in small capitals.",
    .character = Character {
      .smallcaps = FourState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#sc-sc",
      .category = Category::words_characters,
  },
  {
    .marker = "sup",
    .type = Type::character_style,
    .name = "Superscript text",
    .info = "Superscript text typically for use in critical edition footnotes.",
    .character = Character {
      .superscript = TwoState::on,
    },
      .doc = "https://ubsicap.github.io/usfm/characters/index.html#sup-sup",
      .category = Category::words_characters,
  },
  {
    .marker = "pb",
    .type = Type::page_break,
    .name = "Page break",
    .info = "Explicit page break.",
    .doc = "https://ubsicap.github.io/usfm/characters/index.html#pb",
    .category = Category::words_characters,
  },

  
  
  
  // Todo adding here.
  
  //  {
  //    /* marker */ "cat",
  //    /* name */ "Category tag",
  //    /* info */ "Extended note or sidebar category tag.",
  //    /* category */ "id",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 1,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "esb",
  //    /* name */ "Sidebar start",
  //    /* info */ "Beginning or opening of the sidebar content section",
  //    /* category */ "p",
  //    /* type */ 3,
  //    /* subtype */ 3,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 3.2f,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "esbe",
  //    /* name */ "Sidebar end",
  //    /* info */ "End or closing of the sidebar content section",
  //    /* category */ "p",
  //    /* type */ 3,
  //    /* subtype */ 3,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 3.2f,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "fig",
  //    /* name */ "Figure / illustration / map",
  //    /* info */ "Illustration [columns to span, height, filename, caption text]",
  //    /* category */ "sf",
  //    /* type */ 10,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 1,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 1,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "jmp",
  //    /* name */ "Link text",
  //    /* info */ "Optionally used for associating linking attributes to a span of text when no other character level markup is applied to the same span.",
  //    /* category */ "sf",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 2,
  //    /* bold */ 2,
  //    /* underline */ 1,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#0000FF",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "lik",
  //    /* name */ "List entry 'key' content",
  //    /* info */ "The 'key' content will be followed by 'value' content",
  //    /* category */ "l",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "liv",
  //    /* name */ "List entry 'value' content",
  //    /* info */ "The 'value' content follows the 'key' content",
  //    /* category */ "l",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "liv1",
  //    /* name */ "List entry 'value' content level 1",
  //    /* info */ "The 'value' content follows the 'key' content, level 1",
  //    /* category */ "l",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "ndx",
  //    /* name */ "Subject index entry",
  //    /* info */ "Surround a word or words with this markup to indicate that it appears or should appear in the subject index.",
  //    /* category */ "sf",
  //    /* type */ 13,
  //    /* subtype */ 3,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 1,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "rb",
  //    /* name */ "Annotated base text",
  //    /* info */ "Used to mark the base text being annotated with ruby characters.",
  //    /* category */ "st",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 1,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "rt",
  //    /* name */ "Ruby annotation text",
  //    /* info */ "In the case where the annotation text is associated with only a single preceding ideogram, only the 'rt ...rt*' marker is required. The base text markup 'rb ...rb*' is optional in these cases.",
  //    /* category */ "st",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 10,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 1,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "toc",
  //    /* name */ "Table of contents",
  //    /* info */ "Front matter table of contents.",
  //    /* category */ "pm",
  //    /* type */ 9,
  //    /* subtype */ 1,
  //    /* fontsize */ 10,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "va",
  //    /* name */ "Alternate verse number",
  //    /* info */ "Second or alternate verse number. For coding dual numeration in Psalms.",
  //    /* category */ "cv",
  //    /* type */ 4,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 1,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "w",
  //    /* name */ "Wordlist entry",
  //    /* info */ "Surround words with this markup to indicate that it appears or should appear in the word list.",
  //    /* category */ "sf",
  //    /* type */ 13,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 2,
  //    /* bold */ 2,
  //    /* underline */ 2,
  //    /* smallcaps */ 2,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "wa",
  //    /* name */ "Aramaic word list entry",
  //    /* info */ "Surround words with this markup to indicate that it appears or should appear in the Aramaic word list.",
  //    /* category */ "sf",
  //    /* type */ 13,
  //    /* subtype */ 0,
  //    /* fontsize */ 12,
  //    /* italic */ 2,
  //    /* bold */ 2,
  //    /* underline */ 2,
  //    /* smallcaps */ 2,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "wg",
  //    /* name */ "Greek word list entry",
  //    /* info */ "A Greek word list text item.",
  //    /* category */ "sf",
  //    /* type */ 13,
  //    /* subtype */ 2,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 1,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //    /* marker */ "wh",
  //    /* name */ "Hebrew word list entry",
  //    /* info */ "A Hebrew word list text item.",
  //    /* category */ "sf",
  //    /* type */ 13,
  //    /* subtype */ 1,
  //    /* fontsize */ 12,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 1,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },
  //  {
  //  // https://ubsicap.github.io/usfm/peripherals/index.html
  //  {
  //    /* marker */ "periph",
  //    /* name */ "Peripheral",
  //    /* info */ "Divisions for peripheral content.",
  //    /* category */ "pm",
  //    /* type */ 9,
  //    /* subtype */ 10,
  //    /* fontsize */ 10,
  //    /* italic */ 0,
  //    /* bold */ 0,
  //    /* underline */ 0,
  //    /* smallcaps */ 0,
  //    /* superscript */ 0,
  //    /* justification */ 0,
  //    /* spacebefore */ 0,
  //    /* spaceafter */ 0,
  //    /* leftmargin */ 0,
  //    /* rightmargin */ 0,
  //    /* firstlineindent */ 0,
  //    /* spancolumns */ 0,
  //    /* color */ "#000000",
  //    /* print */ 1,
  //    /* userbool1 */ 0,
  //    /* userbool2 */ 0,
  //    /* userbool3 */ 0,
  //    /* userint1 */ 0,
  //    /* userint2 */ 0,
  //    /* userint3 */ 0,
  //    /* userstring1 */ "",
  //    /* userstring2 */ "",
  //    /* userstring3 */ "",
  //    /* backgroundcolor */ "#FFFFFF",
  //  },

  
  
  
  {
    .marker = "pro",
    .type = Type::character_style,
    .name = "Pronunciation annotation",
    .info = "For indicating pronunciation in CJK texts.",
    .character = Character {
      .italic = FourState::on,
    },
      .properties = {{Property::deprecated,std::monostate()}},
      .doc = "https://ubsicap.github.io/usfm/characters/index.html?highlight=pronunciation#pro-pro",
      .category = Category::words_characters,
  },
  {
    .marker = "ef",
    .type = Type::footnote_wrapper,
    .name = "Extended footnote",
    .info = "Extended footnote element.",
    .character = Character { },
    .properties = {
      {Property::note_numbering_sequence,"1 2 3 4 5 6 7 8 9"},
      {Property::note_numbering_restart,"chapter"}
    },
      .doc = "https://ubsicap.github.io/usfm/notes_study/efnotes.html#ef-ef",
      .category = Category::extended_study_content,
  },
  {
    .marker = "ex",
    .type = Type::crossreference_wrapper,
    .name = "Extended cross reference",
    .info = "Extended list of cross references.",
    .character = Character { },
    .properties = {
      {Property::note_numbering_sequence,"a b c d e f g h i j k l m n o p q r s t u v w x y z"},
      {Property::note_numbering_restart,"chapter"}
    },
      .doc = "https://ubsicap.github.io/usfm/notes_study/exrefs.html#ex-ex",
      .category = Category::extended_study_content,
  },
};


// Temporal function that indicates whether a marker has moved to version 2.
// The $marker is the one being considered.
bool marker_moved_to_v2 (const std::string& marker)
{
  static std::map<std::string,bool> cache{};
  const std::string key {marker};
  if (cache.count(key)) {
    return cache.at(key);
  }
  const auto iter = std::find(styles.cbegin(), styles.cend(), marker);
  if (iter != styles.cend())
    cache[key] = true;
  else
    cache[key] = false;
  return cache.at(key);
}


// Whether this style starta a new line in USFM.
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
    case Type::character_style:
      return false;
    case Type::page_break:
      return true;
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


} // Namespace.
