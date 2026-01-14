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
#include <database/styles.h>
#include <styles/logic.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop

class odf_text
{
public:
  odf_text (std::string bible);
  ~odf_text ();
  void new_paragraph (std::string style = stylesv2::standard_sheet ());
  void add_text (std::string text);
  void new_heading1 (std::string text, bool hide = false);
  void create_page_break_style ();
  void new_page_break ();
  void create_paragraph_style (const std::string& name,
                               std::string font_name,
                               const float font_size,
                               const stylesv2::TwoState italic,
                               const stylesv2::TwoState bold,
                               const stylesv2::TwoState underline,
                               const stylesv2::TwoState smallcaps,
                               const stylesv2::TextAlignment text_alignment,
                               const float space_before, const float space_after,
                               const float left_margin, const float right_margin,
                               const float first_line_indent,
                               const bool keep_with_next,
                               const int dropcaps);
  void update_current_paragraph_style (std::string name);
  void open_text_style (const stylesv2::Style* stylev2, bool note, bool embed);
  void close_text_style (bool note, bool embed);
  void place_text_in_frame (const std::string& text, const std::string& style,
                            const float fontsize, const stylesv2::TwoState italic, const stylesv2::TwoState bold);
  void create_superscript_style ();
  void add_note (std::string caller, std::string style, bool endnote = false);
  void add_note_text (std::string text);
  void close_current_note ();
  void save (std::string name);
  std::string m_current_paragraph_style {};
  std::string m_current_paragraph_content {};
  std::vector <std::string> m_current_text_style {};
  void add_image (std::string style, std::string alt, std::string src, std::string caption);
  void add_tab ();
private:
  std::string m_bible {};
  std::string unpacked_odt_folder {};
  pugi::xml_document content_dom {}; // The content.xml DOMDocument.
  pugi::xml_node office_text_node {}; // The office:text DOMNode.
  pugi::xml_document styles_dom {}; // The styles.xml DOMDocument.
  std::vector <std::string> created_styles {}; // An array with styles already created in the $stylesDom.
  pugi::xml_node office_styles_node {}; // The office:styles DOMNode.
  pugi::xml_node current_text_p_node {}; // The current text:p DOMElement.
  bool m_current_text_p_node_opened {false}; // Whether the text:p element has been opened.
  pugi::xml_attribute current_text_p_node_style_name {}; // The DOMAttr of the name of the style of the current text:p element.
  int m_frame_count {0};
  int m_note_count {0};
  pugi::xml_node note_text_p_dom_element {}; // The text:p DOMElement of the current footnote, if any.
  bool m_note_text_p_opened {false}; // Whether the text:p for notes has been opened.
  std::vector <std::string> m_current_note_text_style {};
  void initialize_content_xml ();
  void initialize_styles_xml ();
  void new_named_heading (std::string style, std::string text, bool hide = false);
  std::string convert_style_name (std::string style);
  int m_image_counter {0};
  bool automatic_note_caller {false};
};

