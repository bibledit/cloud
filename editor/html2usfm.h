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

class Editor_Html2Usfm
{
public:
  void load (std::string html);
  void stylesheet (const std::string& stylesheet);
  void run ();
  std::string get ();
private:
  pugi::xml_document document {}; // DOMDocument holding the html.
  std::map <std::string, Database_Styles_Item> styles {}; // Style information.
  std::vector <std::string> output {}; // Output USFM.
  std::string current_line {}; // Growing current USFM line.
  bool mono {false}; // Monospace font.
  std::set <std::string> suppress_end_markers {}; // Markers which should not have endmarkers, e.g. \v does not have \v*
  std::set <std::string> note_openers {};
  std::vector <std::string> character_styles {}; // Active character styles.
  bool processing_note {false}; // Note processing flag.
  std::string last_note_style {}; // The most recent style opened inside a note.
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
  // Storge fot all the word-level attributes.
  std::map<std::string,std::string> m_word_level_attributes{};
  // The last added text fragment, for analysis.
  std::string m_last_added_text_fragment{};
};

std::string editor_export_verse_quill (const std::string& stylesheet, std::string html);
