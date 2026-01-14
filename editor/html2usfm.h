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

class Editor_Html2Usfm
{
public:
  void load (std::string html);
  void stylesheet (const std::string& stylesheet);
  void run ();
  std::string get ();
private:
  // DOMDocument holding the html.
  pugi::xml_document m_document {};
  // Output USFM.
  std::vector <std::string> m_output {};
  // Growing current USFM line.
  std::string m_current_line {};
  // Monospace font.
  bool m_mono {false};
  // Markers which should not have endmarkers, e.g. \v does not have \v*.
  std::set <std::string> m_suppress_end_markers {};
  // Markers which should have endmarkers, e.g. \rq is a paragraph but should still have \rq*.
  std::set <std::string> m_force_end_markers {};
  // Markers that open notes.
  std::set <std::string> m_note_openers {};
  // Active character styles.
  std::vector <std::string> m_character_styles {};
  // Milestone markers.
  std::set <std::string> m_milestone_markers {};
  // Note processing flag.
  bool m_processing_note {false};
  // The most recent style opened inside a note.
  std::string m_last_note_style {};
  void pre_process ();
  void flush_line ();
  void post_process ();
  void main_process ();
  void process_node (pugi::xml_node& node);
  void open_element_node (pugi::xml_node& node);
  void close_element_node (const pugi::xml_node& node);
  void open_inline (const std::string& class_name);
  void process_note_citation (pugi::xml_node& node);
  std::string clean_usfm (std::string usfm);
  pugi::xml_node get_note_pointer (const pugi::xml_node& body, const std::string& id);
  std::string update_quill_class (std::string classname);
  // Storage fot all the word-level attributes.
  std::map<std::string,std::string> m_word_level_attributes{};
  // Storage fot all the milestone attributes.
  std::map<std::string,std::string> m_milestone_attributes{};
  // The last added text fragment, for analysis.
  std::string m_last_added_text_fragment{};
};

std::string editor_export_verse_quill (const std::string& stylesheet, std::string html);
