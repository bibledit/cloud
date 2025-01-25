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
#include <database/styles.h>
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
#include <filter/text.h>

class Editor_Usfm2Html
{
public:
  void load (std::string usfm);
  void stylesheet (const std::string& stylesheet);
  void run ();
  std::string get ();
  size_t m_text_tength {0};
  std::map <int, int> m_verse_start_offsets {};
  std::string m_current_paragraph_style {};
  std::string m_current_paragraph_content {};
  void set_preview ();
private:
  // Strings alternating between USFM and text.
  std::vector <std::string> m_markers_and_text {};
  unsigned int m_markers_and_text_pointer {0};
  
  // All the style information.
  std::map <std::string, database::styles1::Item> m_styles {};
  
  // XML nodes.
  pugi::xml_document m_document {};
  pugi::xml_node m_body_node {};
  pugi::xml_node m_notes_node {};
  pugi::xml_node m_word_level_attributes_node {};
  
  // Standard content markers for notes.
  std::string m_standard_content_marker_foot_end_note {};
  std::string m_standard_content_marker_cross_reference {};

  pugi::xml_node m_current_p_node {}; // The current p node.
  bool m_current_p_open {false};
  std::vector <std::string> m_current_text_styles {};
  
  int m_note_count {0};
  pugi::xml_node m_note_p_node {}; // The p DOM element of the current footnote, if any.
  bool m_note_p_open {false};
  std::vector <std::string> m_current_note_text_styles {};
  
  // The note citations.
  filter::note::citations m_note_citations {};

  // Whether note is open.
  bool m_note_opened {false};
  
  // Lengths and offsets.
  bool m_first_line_done {false};
  
  // Whether it runs in preview-mode.
  bool m_preview { false };
  
  void preprocess ();
  void process ();
  void output_as_is (const std::string& marker, const bool is_opening_marker);
  void new_paragraph (std::string style = std::string());
  void close_paragraph ();
  void open_text_style (const database::styles1::Item& style, const bool embed);
  void close_text_style (const bool embed);
  void add_text (const std::string& text);
  void add_note (const std::string& citation, const std::string& style);
  void add_note_text (const std::string& text);
  void close_current_note ();
  void add_notel_link (pugi::xml_node& dom_node, const int identifier, const std::string& style, const std::string& text);
  bool road_is_clear ();
  
  // Word-level attributes.
  int m_word_level_attributes_id {0};
  int get_word_level_attributes_id(const bool next);
  std::optional<std::string> m_pending_word_level_attributes{};
  void extract_word_level_attributes();
  void add_word_level_attributes(const std::string id);
};
