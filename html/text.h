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

using namespace pugi;

class Html_Text
{
public:
  Html_Text (std::string title);
  void new_paragraph (std::string style = std::string());
  void add_text (std::string text);
  std::string get_html ();
  std::string get_inner_html ();
  void new_heading1 (std::string text, bool hide = false);
  void new_page_break ();
  void open_text_style (Database_Styles_Item style, bool note, bool embed);
  void close_text_style (bool note, bool embed);
  void add_note (std::string citation, std::string style, bool endnote = false);
  void add_note_text (std::string text);
  void close_current_note ();
  void add_link (xml_node node,
                 std::string reference, std::string identifier,
                 std::string title, std::string style, std::string text,
                 bool add_popup = false);
  xml_node new_table ();
  xml_node new_table_row (xml_node tableElement);
  xml_node new_table_data (xml_node tableRowElement, bool alignRight = false);
  void save (std::string name);
  xml_node current_p_node {}; // The current p element.
  std::string current_paragraph_style {};
  std::string current_paragraph_content {};
  std::vector <std::string> current_text_style {};
  // This class to be added to each paragraph. The class to be defined in the stylesheet.css.
  std::string custom_class {};
  void have_popup_notes ();
  void add_image (std::string style, std::string alt, std::string src, std::string caption);
private:
  xml_document document {};
  xml_node head_node {};
  xml_node body_node {};
  xml_node notes_div_node {};
  bool current_p_node_open {false};
  int note_count {0};
  xml_node note_p_node {}; // The p element of the current footnote, if any.
  bool note_p_node_open {false};
  std::vector <std::string> current_note_text_style {};
  void new_named_heading (std::string style, std::string text, bool hide = false);
  bool add_popup_notes {false};
  xml_node popup_node {};
};
