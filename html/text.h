/*
Copyright (©) 2003-2022 Teus Benschop.

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
#include <pugixml/pugixml.hpp>
#pragma GCC diagnostic pop

using namespace pugi;

class Html_Text
{
public:
  Html_Text (string title);
  void new_paragraph (string style = "");
  void add_text (string text);
  string get_html ();
  string get_inner_html ();
  void new_heading1 (string text, bool hide = false);
  void new_page_break ();
  void open_text_style (Database_Styles_Item style, bool note, bool embed);
  void close_text_style (bool note, bool embed);
  void add_note (string citation, string style, bool endnote = false);
  void add_note_text (string text);
  void close_current_note ();
  void add_link (xml_node node,
                string reference, string identifier,
                string title, string style, string text,
                bool add_popup = false);
  xml_node new_table ();
  xml_node new_table_row (xml_node tableElement);
  xml_node new_table_data (xml_node tableRowElement, bool alignRight = false);
  void save (string name);
  xml_node current_p_node; // The current p element.
  string current_paragraph_style;
  string current_paragraph_content;
  vector <string> current_text_style;
  // This class to be added to each paragraph. The class to be defined in the stylesheet.css.
  string custom_class = "";
  void have_popup_notes ();
  void add_image (string alt, string src, string caption);
private:
  xml_document document;
  xml_node head_node;
  xml_node body_node;
  xml_node notes_div_node;
  bool current_p_node_open = false;
  int note_count;
  xml_node note_p_node; // The p element of the current footnote, if any.
  bool note_p_node_open = false;
  vector <string> current_note_text_style;
  void new_named_heading (string style, string text, bool hide = false);
  bool add_popup_notes = false;
  xml_node popup_node;
};
