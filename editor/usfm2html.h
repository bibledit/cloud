/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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

using namespace pugi;

class Editor_Usfm2Html
{
public:
  void load (std::string usfm);
  void stylesheet (std::string stylesheet);
  void run ();
  std::string get ();
  size_t m_text_tength {0};
  std::map <int, int> m_verse_start_offsets {};
  std::string m_current_paragraph_style {};
  std::string m_current_paragraph_content {};
  void set_preview ();
private:
  std::vector <std::string> m_markers_and_text {}; // Strings alternating between USFM and text.
  unsigned int m_markers_and_text_pointer {0};
  
  // All the style information.
  std::map <std::string, Database_Styles_Item> m_styles {};
  
  // XML nodes.
  xml_document m_document {};
  xml_node m_body_node {};
  xml_node m_notes_node {};
  
  // Standard content markers for notes.
  std::string m_standard_content_marker_foot_end_note {};
  std::string m_standard_content_marker_cross_reference {};

  xml_node m_current_p_node {}; // The current p node.
  bool m_current_p_open {false};
  std::vector <std::string> m_current_text_styles {};
  
  int m_note_count {0};
  xml_node m_note_p_node {}; // The p DOM element of the current footnote, if any.
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
  void postprocess ();
  void output_as_is (std::string marker, bool is_opening_marker);
  void new_paragraph (std::string style = std::string());
  void close_paragraph ();
  void open_text_style (Database_Styles_Item & style, bool embed);
  void close_text_style (bool embed);
  void add_text (std::string text);
  void add_note (std::string citation, std::string style, bool endnote = false);
  void add_note_text (std::string text);
  void close_current_note ();
  void add_notel_link (xml_node domNode, int identifier, std::string style, std::string text);
  
  bool road_is_clear ();
};
