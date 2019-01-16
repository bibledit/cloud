/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


#include <styles/css.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/css.h>
#include <config/globals.h>
#include <webserver/request.h>
#include <styles/logic.h>
#include <database/config/bible.h>
#include <fonts/logic.h>
#include <quill/logic.h>


Styles_Css::Styles_Css (void * webserver_request_in, string stylesheet_in)
{
  webserver_request = webserver_request_in;
  stylesheet = stylesheet_in;
}


// Call this method to add styles suitable for the Bible editor.
void Styles_Css::editor ()
{
  editor_enabled = true;
}


// Call this method to add styles suitable for exported Bible.
void Styles_Css::exports ()
{
  exports_enabled = true;
}


// Generates the CSS.
void Styles_Css::generate ()
{
  code.push_back (".superscript, [class^='i-note'] { font-size: x-small; vertical-align: super; }");
  if (exports_enabled) {
    add_exports_styles ();
  }
  if (editor_enabled) {
    add_editor_styles ();
  }
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  vector <string> markers = request->database_styles ()->getMarkers (stylesheet);
  for (auto & marker : markers) {
    Database_Styles_Item style = request->database_styles ()->getMarkerData (stylesheet, marker);
    evaluate (&style);
  }
}


// Evaluates the style so as to decide how it should look.
void Styles_Css::evaluate (void * database_styles_item)
{
  Database_Styles_Item * style = (Database_Styles_Item *) database_styles_item;
  
  switch (style->type)
  {
    case StyleTypeIdentifier: break;
    case StyleTypeNotUsedComment: break;
    case StyleTypeNotUsedRunningHeader: break;
    case StyleTypeStartsParagraph:
    {
      switch (style->subtype)
      {
        case ParagraphSubtypeMainTitle:
        case ParagraphSubtypeSubTitle:
        case ParagraphSubtypeSectionHeading:
        {
          add (style, true, true);
          break;
        }
        case ParagraphSubtypeNormalParagraph:
        {
          add (style, true, false);
          break;
        }
      }
      break;
    }
    case StyleTypeInlineText:
    {
      add (style, false, false);
      break;
    }
    case StyleTypeChapterNumber:
    {
      add (style, true, false);
      break;
    }
    case StyleTypeVerseNumber:
    {
      add (style, false, false);
      break;
    }
    case StyleTypeFootEndNote:
    {
      switch (style->subtype)
      {
        case FootEndNoteSubtypeFootnote:
        case FootEndNoteSubtypeEndnote:
        {
          add (style, true, false);
          break;
        }
        case FootEndNoteSubtypeStandardContent:
        case FootEndNoteSubtypeContent:
        case FootEndNoteSubtypeContentWithEndmarker:
        case FootEndNoteSubtypeParagraph:
        {
          add (style, false, false);
          break;
        }
      }
      break;
    }
    case StyleTypeCrossreference:
    {
      switch (style->subtype)
      {
        case CrossreferenceSubtypeCrossreference:
        {
          add (style, true, false);
          break;
        }
        case CrossreferenceSubtypeStandardContent:
        case CrossreferenceSubtypeContent:
        case CrossreferenceSubtypeContentWithEndmarker:
        {
          add (style, false, false);
          break;
        }
      }
      break;
    }
  }
}


// This function adds a style to the internal CSS.
// $style: Array with the Bibledit style information.
// $paragraph: True: Is paragraph. False: Is inline text.
// $keepwithnext: Keep text in this style together with the next paragraph.
void Styles_Css::add (void * database_styles_item, bool paragraph, bool keepwithnext)
{
  Database_Styles_Item * style = (Database_Styles_Item *) database_styles_item;

  string class_ = style->marker;

  // The name of the class as used in a Quill-based editor.
  string quill_class = ", .";
  if (paragraph) {
    quill_class.append (quill_logic_class_prefix_block ());
  } else {
    quill_class.append (quill_logic_class_prefix_inline ());
  }
  quill_class.append (class_);
  
  // Start with the class. Notice the dot.
  code.push_back ("." + class_ + quill_class + " {");
  
  // Font size.
  // Since it is html and not pdf for paper, a font size of 12pt is considered to be equal to 100%.
  if (paragraph) {
    float points = style->fontsize;
    float percents = points * 100 / 12;
    int fontsize = convert_to_int (percents);
    if (fontsize != 100) {
      code.push_back ("font-size: " + convert_to_string (fontsize) + "%;");
    }
  }
  
  // Italics, bold, underline, small caps.
  int italic = style->italic;
  int bold = style->bold;
  int underline = style->underline;
  int smallcaps = style->smallcaps;
  
  // Italics, bold, underline, small caps can be either ooitOff or ooitOn for a paragraph.
  if (paragraph) {
    if (italic != ooitOff) {
      code.push_back ("font-style: italic;");
    }
    if (bold != ooitOff) {
      code.push_back ("font-weight: bold;");
    }
    if (underline != ooitOff) {
      code.push_back ("text-decoration: underline;");
    }
    if (smallcaps != ooitOff) {
      code.push_back ("font-variant: small-caps;");
    }
  }
  
  // For inline text.
  // Italics, bold, underline, small caps can be ooitOff or ooitOn or ooitInherit or ooitToggle.
  // Not all features have been implemented.
  if (!paragraph) {
    if ((italic == ooitOn) || (italic == ooitToggle)) {
      code.push_back ("font-style: italic;");
    }
    if ((bold == ooitOn) || (bold == ooitToggle)) {
      code.push_back ("font-weight: bold;");
    }
    if ((underline == ooitOn) || (underline == ooitToggle)) {
      code.push_back ("text-decoration: underline;");
    }
    if ((smallcaps == ooitOn) || (smallcaps == ooitToggle)) {
      code.push_back ("font-variant: small-caps;");
    }
  }
  
  // Paragraph layout properties
  if (paragraph) {
    string spacebefore = convert_to_string (style->spacebefore);
    string spaceafter = convert_to_string (style->spaceafter);
    string leftmargin = convert_to_string (style->leftmargin);
    string rightmargin = convert_to_string (style->rightmargin);
    string firstlineindent = convert_to_string (style->firstlineindent);
    
    // Text alignment options.
    string alignment;
    switch (style->justification) {
      case AlignmentLeft:    alignment = ""; break;
      case AlignmentCenter:  alignment = "center"; break;
      case AlignmentRight:   alignment = "right"; break;
      case AlignmentJustify: alignment = "justify"; break;
    }
    if (alignment != "") {
      code.push_back ("text-align: " + alignment + ";");
    }
    
    // Paragraph measurements; given in mm.
    if (spacebefore != "0") {
      code.push_back ("margin-top: " + spacebefore + "mm;");
    }
    if (spaceafter != "0") {
      code.push_back ("margin-bottom: " + spaceafter + "mm;");
    }
    if (leftmargin != "0") {
      code.push_back ("margin-left: " + leftmargin + "mm;");
    }
    if (rightmargin != "0") {
      code.push_back ("margin-right: " + rightmargin + "mm;");
    }
    if (firstlineindent != "0") {
      code.push_back ("text-indent: " + firstlineindent + "mm;");
    }
    
    // Columns have not yet been implemented.
    //bool spancolumns = style->spancolumns;
    
    // Drop caps have not yet been implemented.
    //bool dropcaps = false;
    
    // Keeping text with the next paragraph.
    if (keepwithnext) {
      code.push_back ("page-break-inside: avoid;");
    }
    
  }
  
  // Superscript and colors for inline text.
  if (!paragraph) {
    
    bool superscript = style->superscript;
    if (superscript) {
      code.push_back ("font-size: x-small;");
      code.push_back ("vertical-align: super;");
    }
    
    string color = style->color;
    if (color != "#000000") {
      code.push_back ("color: " + color + ";");
    }
    
    string backgroundcolor = style->backgroundcolor;
    if (backgroundcolor != "#FFFFFF") {
      code.push_back ("background-color: " + backgroundcolor + ";");
    }
    
  }
  
  // Close style.
  code.push_back ("}");
}


// Deliver the created CSS.
// $path: If given, it saves the CSS to $path.
// The function returns the CSS as a string.
string Styles_Css::css (string path)
{
  string css = filter_string_implode (code, "\n");
  if (path != "") {
    filter_url_file_put_contents (path, css);
  }
  return css;
}


// This adds the styles for the exports.
void Styles_Css::add_exports_styles ()
{
  code.push_back ("body { }");
  code.push_back ("p { margin-top: 0; margin-bottom: 0; }");
  code.push_back ("p.page { page-break-after: always; }");
  code.push_back ("span.dropcaps { float: left; font-size: 300%; line-height: 0.85em; margin-right: 0.03em; margin-bottom:-0.25em; }");
  code.push_back ("a { text-decoration: none; background: none; }");
  code.push_back ("a:visited { color: #5a3696; }");
  code.push_back ("a:active { color: #faa700; }");
  code.push_back ("a:hover { text-decoration: underline; }");
  code.push_back (".breadcrumbs { font-size: normal; }");
  code.push_back (".navigationbar { font-size: normal; }");
}


// This adds the styles for the Bible text editor.
void Styles_Css::add_editor_styles ()
{
}


void Styles_Css::customize (const string& bible)
{
  string cls = Filter_Css::getClass (bible);
  string font = Fonts_Logic::getTextFont (bible);
  bool uploaded_font = Fonts_Logic::fontExists (font);
  font = Fonts_Logic::getFontPath (font);
  int direction = Database_Config_Bible::getTextDirection (bible);
  string css = Filter_Css::getCss (cls, font, direction);
  if (uploaded_font) css = filter_string_str_replace ("../fonts/", "", css);
  code.push_back (css);
}


