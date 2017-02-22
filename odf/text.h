/*
Copyright (©) 2003-2016 Teus Benschop.

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


#ifndef INCLUDED_ODF_TEXT_H
#define INCLUDED_ODF_TEXT_H


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
  void newParagraph (string style = styles_logic_standard_sheet ());
  void addText (string text);
  void newHeading1 (string text, bool hide = false);
  void createPageBreakStyle ();
  void newPageBreak ();
  void createParagraphStyle (string name, float fontsize, int italic, int bold, int underline, int smallcaps, int alignment, float spacebefore, float spaceafter, float leftmargin, float rightmargin, float firstlineindent, bool keepWithNext, int dropcaps);
  void updateCurrentParagraphStyle (string name);
  void openTextStyle (Database_Styles_Item style, bool note, bool embed);
  void closeTextStyle (bool note, bool embed);
  void placeTextInFrame (string text, string style, float fontsize, int italic, int bold);
  void createSuperscriptStyle ();
  void addNote (string caller, string style, bool endnote = false);
  void addNoteText (string text);
  void closeCurrentNote ();
  void save (string name);
  string currentParagraphStyle;
  string currentParagraphContent;
  vector <string> currentTextStyle;
private:
  string bible;
  string unpackedOdtFolder;
  xml_document contentDom; // The content.xml DOMDocument.
  xml_node officeTextDomNode; // The office:text DOMNode.
  xml_document stylesDom; // The styles.xml DOMDocument.
  vector <string> createdStyles; // An array with styles already created in the $stylesDom.
  xml_node officeStylesDomNode; // The office:styles DOMNode.
  //xml_node officeAutomaticStylesDomNode; // The office:automatic-styles DOMNode.
  xml_node currentTextPDomElement; // The current text:p DOMElement.
  bool current_text_p_opened = false; // Whether the text:p element has been opened.
  xml_attribute currentTextPDomElementNameNode; // The DOMAttr of the name of the style of the current text:p element.
  int frameCount;
  int noteCount;
  xml_node noteTextPDomElement; // The text:p DOMElement of the current footnote, if any.
  bool note_text_p_opened = false; // Whether the text:p for notes has been opened.
  vector <string> currentNoteTextStyle;
  void initialize_content_xml ();
  void initialize_styles_xml ();
  void newNamedHeading (string style, string text, bool hide = false);
  string convertStyleName (string style);
};


#endif
