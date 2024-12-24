/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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
  void set_word_level_attributes (std::map<int,std::string> attributes);
private:
  pugi::xml_document document {}; // DOMDocument holding the html.
  std::map <std::string, Database_Styles_Item> styles {}; // Style information.
  std::vector <std::string> output {}; // Output USFM.
  std::string current_line {}; // Growing current USFM line.
  bool mono {false}; // Monospace font.
  std::set <std::string> suppressEndMarkers {}; // Markers which should not have endmarkers, e.g. \v does not have \v*
  std::set <std::string> noteOpeners {};
  std::vector <std::string> characterStyles {}; // Active character styles.
  bool processingNote {false}; // Note processing flag.
  std::string lastNoteStyle {}; // The most recent style opened inside a note.
  void preprocess ();
  void flush_line ();
  void postprocess ();
  void process ();
  void process_node (pugi::xml_node& node);
  void open_element_node (pugi::xml_node& node);
  void close_element_node (const pugi::xml_node& node);
  void open_online (const std::string& class_name);
  void process_note_citation (pugi::xml_node& node);
  std::string clean_usfm (std::string usfm);
  pugi::xml_node get_note_pointer (const pugi::xml_node& body, const std::string& id);
  std::string update_quill_class (std::string classname);
  std::map<int,std::string> m_word_level_attributes{};
};

std::string editor_export_verse_quill (std::string stylesheet, std::string html);
