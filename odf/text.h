/*
Copyright (©) 2003-2021 Teus Benschop.

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
#include <pugixml/pugixml.hpp>

using namespace pugi;

class Odf_Text
{
public:
  Odf_Text (string bible_in);
  ~Odf_Text ();
  void new_paragraph (string style = styles_logic_standard_sheet ());
  void add_text (string text);
  void new_heading1 (string text, bool hide = false);
  void createPageBreakStyle ();
  void newPageBreak ();
  void create_paragraph_style (string name,
                               string fontname,
                               float fontsize,
                               int italic, int bold, int underline,
                               int smallcaps,
                               int alignment,
                               float spacebefore, float spaceafter,
                               float leftmargin, float rightmargin,
                               float firstlineindent,
                               bool keepWithNext,
                               int dropcaps);
  void update_current_paragraph_style (string name);
  void open_text_style (Database_Styles_Item style, bool note, bool embed);
  void closeTextStyle (bool note, bool embed);
  void placeTextInFrame (string text, string style, float fontsize, int italic, int bold);
  void createSuperscriptStyle ();
  void addNote (string caller, string style, bool endnote = false);
  void addNoteText (string text);
  void closeCurrentNote ();
  void save (string name);
  string current_paragraph_style;
  string current_paragraph_content;
  vector <string> currentTextStyle;
  void add_image (string alt, string src, string caption);
  void add_tab ();
private:
  string bible;
  string unpackedOdtFolder;
  xml_document contentDom; // The content.xml DOMDocument.
  xml_node office_text_node; // The office:text DOMNode.
  xml_document stylesDom; // The styles.xml DOMDocument.
  vector <string> createdStyles; // An array with styles already created in the $stylesDom.
  xml_node office_styles_node; // The office:styles DOMNode.
  //xml_node officeAutomaticStylesDomNode; // The office:automatic-styles DOMNode.
  xml_node current_text_p_node; // The current text:p DOMElement.
  bool current_text_p_node_opened = false; // Whether the text:p element has been opened.
  xml_attribute current_text_p_node_style_name; // The DOMAttr of the name of the style of the current text:p element.
  int frameCount;
  int noteCount;
  xml_node noteTextPDomElement; // The text:p DOMElement of the current footnote, if any.
  bool note_text_p_opened = false; // Whether the text:p for notes has been opened.
  vector <string> currentNoteTextStyle;
  void initialize_content_xml ();
  void initialize_styles_xml ();
  void newNamedHeading (string style, string text, bool hide = false);
  string convert_style_name (string style);
  int image_counter;
  // string pictures_folder;
};
