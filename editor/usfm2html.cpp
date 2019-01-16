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


#include <editor/usfm2html.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/logs.h>
#include <quill/logic.h>


void Editor_Usfm2Html::load (string usfm)
{
  // Clean up.
  usfm = filter_string_trim (usfm);
  usfm.append ("\n");
  // Separate it into markers and text.
  // Load it into the object.
  markersAndText = usfm_get_markers_and_text (usfm);
  markersAndTextPointer = 0;
}


void Editor_Usfm2Html::stylesheet (string stylesheet)
{
  Database_Styles database_styles;
  styles.clear();
  vector <string> markers = database_styles.getMarkers (stylesheet);
  // Load the style information into the object.
  for (auto & marker : markers) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    styles [marker] = style;
    if (style.type == StyleTypeFootEndNote) {
      if (style.subtype == FootEndNoteSubtypeStandardContent) {
        standardContentMarkerFootEndNote = style.marker;
      }
    }
    if (style.type == StyleTypeCrossreference) {
      if (style.subtype == CrossreferenceSubtypeStandardContent) {
        standardContentMarkerCrossReference = style.marker;
      }
    }
  }
}


// Enable styles suitable for Quill-based editor.
void Editor_Usfm2Html::quill ()
{
  quill_enabled = true;
}


void Editor_Usfm2Html::run ()
{
  preprocess ();
  process ();
  postprocess ();
}


string Editor_Usfm2Html::get ()
{
  // If there are notes, move the notes <div> or <p> after everything else.
  // (It has the <hr> or <br> as a child).
  size_t count = distance (notes_node.begin (), notes_node.end ());
  if (count > 1) {
    body_node.append_move (notes_node);
  }

  // A Quill-based editor does not work with embedded <p> elements.
  // Move the notes out of their parent and append them to the end main body.
  if (quill_enabled) {
    while (xml_node note = notes_node.first_child ().next_sibling ()) {
      body_node.append_move (note);
    }
  }
  
  // Get the html code, including body, without head.
  stringstream output;
  body_node.print (output, "", format_raw);
  string html = output.str ();
  
  // Remain with the stuff within the <body> elements.
  size_t pos = html.find ("<body>");
  if (pos != string::npos) {
    html.erase (0, pos + 6);
    pos = html.find ("</body>");
    if (pos != string::npos) {
      html.erase  (pos);
    }
  }

  // Deal with a blank line.
  // The \b in USFM, a blank line, gets converted to:
  // <p class="b" />
  // But this does not display a blank in the web browser.
  // Therefore convert it to the following:
  // <p class="b"><br></p>
  // This is how the webkit browser naturally represents a new empty line.
  // Also do the other markers similar to the blank line.
  vector <string> blank_lines = { "b", "sd", "sd1", "sd2", "sd3", "sd4" };
  for (auto clas : blank_lines) {
    if (quill_enabled) clas.insert (0, quill_logic_class_prefix_block ());
    html = filter_string_str_replace ("<p class=\"" + clas + "\" />", "<p class=\"" + clas + "\"><br></p>", html);
  }
  
  // Currently the XML library produces hexadecimal character entities.
  // This is unwanted behaviour: Convert them to normal characters.
  html = convert_xml_character_entities_to_characters (html);
  
  // Result.
  return html;
}


void Editor_Usfm2Html::preprocess ()
{
  currentParagraphStyle.clear ();
  currentParagraphContent.clear ();
  currentTextStyles.clear();
  noteCount = 0;
  currentNoteTextStyles.clear();
  textLength = 0;
  verseStartOffsets = { make_pair (0, 0) };
  current_p_open = false;
  note_p_open = false;

  // XPath crashes on Android with libxml2 2.9.2 compiled through the Android NDK.
  // After the move to pugixml, this no longer applies.

  body_node = document.append_child ("body");
  
  // Create notes xml node.
  // It comes at the start of the document.
  // (Later, it will either be deleted, or moved to the end).
  string notes_value = "notes";
  if (quill_enabled) {
    notes_node = document.append_child ("p");
    notes_value.insert (0, quill_logic_class_prefix_block ());
    notes_node.append_attribute ("class") = notes_value.c_str ();
    notes_node.append_child ("br");
  } else {
    notes_node = document.append_child ("div");
    notes_node.append_attribute ("id") = notes_value.c_str ();
    notes_node.append_child ("hr");
  }
}

 
void Editor_Usfm2Html::process ()
{
  markersAndTextPointer = 0;
  unsigned int markersAndTextCount = markersAndText.size();
  for (markersAndTextPointer = 0; markersAndTextPointer < markersAndTextCount; markersAndTextPointer++) {
    string currentItem = markersAndText[markersAndTextPointer];
    if (usfm_is_usfm_marker (currentItem))
    {
      // Store indicator whether the marker is an opening marker.
      bool isOpeningMarker = usfm_is_opening_marker (currentItem);
      bool isEmbeddedMarker = usfm_is_embedded_marker (currentItem);
      // Clean up the marker, so we remain with the basic version, e.g. 'id'.
      string marker = usfm_get_marker (currentItem);
      if (styles.count (marker))
      {
        Database_Styles_Item style = styles [marker];
        switch (style.type)
        {
          case StyleTypeIdentifier:
          case StyleTypeNotUsedComment:
          case StyleTypeNotUsedRunningHeader:
          {
            closeTextStyle (false);
            outputAsIs (marker, isOpeningMarker);
            break;
          }
          case StyleTypeStartsParagraph:
          {
            closeTextStyle (false);
            newParagraph (marker);
            break;
          }
          case StyleTypeInlineText:
          {
            if (isOpeningMarker) {
              // Be sure the road ahead is clear.
              if (roadIsClear ()) {
                openTextStyle (style, isEmbeddedMarker);
              } else {
                addText (usfm_get_opening_usfm (marker));
              }
            } else {
              closeTextStyle (isEmbeddedMarker);
            }
            break;
          }
          case StyleTypeChapterNumber:
          {
            closeTextStyle (false);
            newParagraph (marker);
            break;
          }
          case StyleTypeVerseNumber:
          {
            // Close any existing text style.
            closeTextStyle (false);
            // Output the space before the verse number in case the paragraph already has some text.
            if (currentParagraphContent != "") {
              addText (" ");
            }
            // Open verse style, record verse/length, add verse number, close style again, and add a space.
            openTextStyle (style, false);
            string textFollowingMarker = usfm_get_text_following_marker (markersAndText, markersAndTextPointer);
            string number = usfm_peek_verse_number (textFollowingMarker);
            verseStartOffsets [convert_to_int (number)] = textLength;
            addText (number);
            closeTextStyle (false);
            addText (" ");
            // If there was any text following the \v marker, remove the verse number,
            // put the remainder back into the object, and update the pointer.
            if (textFollowingMarker != "") {
              size_t pos = textFollowingMarker.find (number);
              if (pos != string::npos) {
                textFollowingMarker = textFollowingMarker.substr (pos + number.length());
              }
              textFollowingMarker = filter_string_ltrim (textFollowingMarker);
              markersAndText [markersAndTextPointer] = textFollowingMarker;
              markersAndTextPointer--;
            }
            break;
          }
          case StyleTypeFootEndNote:
          {
            switch (style.subtype)
            {
              case FootEndNoteSubtypeFootnote:
              case FootEndNoteSubtypeEndnote:
              {
                closeTextStyle (false);
                if (isOpeningMarker) {
                  int caller = noteCount % 9 + 1;
                  addNote (convert_to_string (caller), marker, false);
                } else {
                  closeCurrentNote ();
                }
                break;
              }
              case FootEndNoteSubtypeStandardContent:
              case FootEndNoteSubtypeContent:
              case FootEndNoteSubtypeContentWithEndmarker:
              {
                if (isOpeningMarker) {
                  openTextStyle (style, isEmbeddedMarker);
                } else {
                  closeTextStyle (isEmbeddedMarker);
                }
                break;
              }
              case FootEndNoteSubtypeParagraph:
              default:
              {
                closeTextStyle (false);
                break;
              }
            }
            break;
          }
          case StyleTypeCrossreference:
          {
            switch (style.subtype)
            {
              case CrossreferenceSubtypeCrossreference:
              {
                closeTextStyle (false);
                if (isOpeningMarker) {
                  int caller = (noteCount) % 9 + 1;
                  addNote (convert_to_string (caller), marker, false);
                } else {
                  closeCurrentNote ();
                }
                break;
              }
              case CrossreferenceSubtypeContent:
              case CrossreferenceSubtypeContentWithEndmarker:
              case CrossreferenceSubtypeStandardContent:
              {
                if (isOpeningMarker) {
                  openTextStyle (style, isEmbeddedMarker);
                } else {
                  closeTextStyle (isEmbeddedMarker);
                }
                break;
              }
              default:
              {
                closeTextStyle (false);
                break;
              }
            }
            break;
          }
          case StyleTypePeripheral:
          {
            closeTextStyle (false);
            outputAsIs (marker, isOpeningMarker);
            break;
          }
          case StyleTypePicture:
          {
            closeTextStyle (false);
            outputAsIs (marker, isOpeningMarker);
            break;
          }
          case StyleTypePageBreak:
          {
            closeTextStyle (false);
            outputAsIs (marker, isOpeningMarker);
            break;
          }
          case StyleTypeTableElement:
          {
            closeTextStyle (false);
            switch (style.subtype)
            {
              case TableElementSubtypeRow:
              {
                outputAsIs (marker, isOpeningMarker);
                break;
              }
              case TableElementSubtypeHeading:
              case TableElementSubtypeCell:
              {
                openTextStyle (style, false);
                break;
              }
              default:
              {
                openTextStyle (style, false);
                break;
              }
            }
            break;
          }
          case StyleTypeWordlistElement:
          {
            if (isOpeningMarker) {
              openTextStyle (style, false);
            } else {
              closeTextStyle (false);
            }
            break;
          }
          default:
          {
            // This marker is known in the stylesheet, but not yet implemented here.
            closeTextStyle (false);
            outputAsIs (marker, isOpeningMarker);
            break;
          }
        }
      } else {
        // This is a marker unknown in the stylesheet.
        closeTextStyle (false);
        outputAsIs (marker, isOpeningMarker);
      }
    } else {
      // Here is no marker. Treat it as text.
      if (noteOpened) {
        addNoteText (currentItem);
      } else {
        addText (currentItem);
      }
    }
  }
}


void Editor_Usfm2Html::postprocess ()
{
}


void Editor_Usfm2Html::outputAsIs (string marker, bool isOpeningMarker)
{
  // Output the marker in monospace font.
  if (isOpeningMarker) {
    // Add opening marker as it is.
    newParagraph ("mono");
    addText (usfm_get_opening_usfm (marker));
  } else {
    // Add closing marker to existing paragraph.
    addText (usfm_get_closing_usfm (marker));
  }
}


void Editor_Usfm2Html::newParagraph (string style)
{
  currentPnode = body_node.append_child ("p");
  current_p_open = true;
  if (!style.empty()) {
    string style2 (style);
    if (quill_enabled) style2.insert (0, quill_logic_class_prefix_block ());
    currentPnode.append_attribute ("class") = style2.c_str();
  }
  currentParagraphStyle = style;
  currentParagraphContent.clear();
  // A Quill-based editor assigns a length of one to a new line.
  if (quill_enabled) {
    // Skip the first line.
    if (first_line_done) textLength++;
  }
  first_line_done = true;
}


// This opens a text style.
// $style: the array containing the style variables.
// $embed: boolean: Whether to open embedded / nested style.
void Editor_Usfm2Html::openTextStyle (Database_Styles_Item & style, bool embed)
{
  string marker = style.marker;
  if (noteOpened) {
    if (!embed) currentNoteTextStyles.clear();
    currentNoteTextStyles.push_back (marker);
  } else {
    if (!embed) currentTextStyles.clear();
    currentTextStyles.push_back (marker);
  }
}


// This closes any open text style.
// $embed: boolean: Whether to close embedded character style.
void Editor_Usfm2Html::closeTextStyle (bool embed)
{
  if (noteOpened) {
    if (!currentNoteTextStyles.empty ()) currentNoteTextStyles.pop_back ();
    if (!embed) currentNoteTextStyles.clear ();
  } else {
    if (!currentTextStyles.empty()) currentTextStyles.pop_back();
    if (!embed) currentTextStyles.clear ();
  }
}


// This function adds text to the current paragraph.
// $text: The text to add.
void Editor_Usfm2Html::addText (string text)
{
  if (text != "") {
    if (!current_p_open) {
      newParagraph ();
    }
    xml_node spanDomElement = currentPnode.append_child ("span");
    spanDomElement.text ().set (text.c_str());
    if (!currentTextStyles.empty ()) {
      // Take character style(s) as specified in this object.
      string textstyle;
      for (auto & style : currentTextStyles) {
        if (!textstyle.empty ()) {
          // The Quill library is fussy about class names.
          // It accepts class="i-add" but not class="i-add-nd". It fails on that second hyphen.
          // It also does not accept an underscore as part of the class name.
          // That causes the whole class to be removed.
          // Right now the way to deal with a class with two styles is like this "i-add0nd".
          // It has one hyphen. And a "0" to separate the two styles.
          if (quill_enabled) textstyle.append ("0");
          else textstyle.append (" ");
        }
        textstyle.append (style);
      }
      if (quill_enabled) textstyle.insert (0, quill_logic_class_prefix_inline ());
      spanDomElement.append_attribute ("class") = textstyle.c_str();
    }
    currentParagraphContent.append (text);
  }
  textLength += unicode_string_length (text);
}


// This function adds a note to the current paragraph.
// $citation: The text of the note citation.
// $style: Style name for the paragraph in the note body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void Editor_Usfm2Html::addNote (string citation, string style, bool endnote)
{
  // Be sure the road ahead is clear.
  if (!roadIsClear ()) {
    addText (usfm_get_opening_usfm (style));
    return;
  }
  
  // Ensure that a paragraph is open, so that the note can be added to it.
  if (!current_p_open) {
    newParagraph ();
  }
  
  // Not used:
  (void) endnote;
  
  noteCount++;
  noteOpened = true;
  
  // Add the link with all relevant data for the note citation.
  if (quill_enabled) {
    addNoteQuillLink (currentPnode, noteCount, "call", citation);
  } else {
    string reference = "#note" + convert_to_string (noteCount);
    string identifier = "citation" + convert_to_string (noteCount);
    addNoteDomLink (currentPnode, reference, identifier, "superscript", citation);
  }
  
  // Open a paragraph element for the note body.
  notePnode = notes_node.append_child ("p");
  note_p_open = true;
  string cls (style);
  if (quill_enabled) cls.insert (0, quill_logic_class_prefix_block ());
  notePnode.append_attribute ("class") = cls.c_str();
  
  closeTextStyle (false);
  
  // Add the link with all relevant data for the note body.
  if (quill_enabled) {
    addNoteQuillLink (notePnode, noteCount, "body", citation);
  } else {
    string reference = "#citation" + convert_to_string (noteCount);
    string identifier = "note" + convert_to_string (noteCount);
    addNoteDomLink (notePnode, reference, identifier, "", citation);
  }
  
  // Add a space.
  addNoteText (" ");
  
  // Update the text length of the text body, excluding the note.
  textLength += unicode_string_length (citation);
}


// This function adds text to the current footnote.
// $text: The text to add.
void Editor_Usfm2Html::addNoteText (string text)
{
  if (text.empty ()) return;
  if (!note_p_open) {
    addNote ("?", "");
  }
  xml_node spanDomElement = notePnode.append_child ("span");
  spanDomElement.text ().set (text.c_str());
  if (!currentNoteTextStyles.empty()) {
    // Take character style(s) as specified in this object.
    string classs;
    if (quill_enabled) {
      classs = filter_string_implode (currentNoteTextStyles, "0");
    } else {
      classs = filter_string_implode (currentNoteTextStyles, " ");
    }
    if (quill_enabled) {
      classs.insert (0, quill_logic_class_prefix_inline ());
    }
    spanDomElement.append_attribute ("class") = classs.c_str();
  }
}


// This function closes the current footnote.
void Editor_Usfm2Html::closeCurrentNote ()
{
  // If a note was opened, close that, else close the standard text.
  closeTextStyle (false);
  note_p_open = false;
  noteOpened = false;
}


// This adds a link as a mechanism to connect body text with a note body.
// $domNode: The DOM node where to add the link to.
// $reference: The link's href, e.g. where it links to.
// $identifier: The link's identifier. Others can link to it.
// $style: The link text's style.
// $text: The link's text.
// It also deals with a Quill-based editor, in a slightly different way.
void Editor_Usfm2Html::addNoteDomLink (xml_node domNode, string reference, string identifier, string style, string text)
{
  xml_node aDomElement = domNode.append_child ("a");
  aDomElement.append_attribute ("href") = reference.c_str();
  aDomElement.append_attribute ("id") = identifier.c_str();
  // The link itself, for the notes, is not editable, so it can be clicked.
  // It was disabled again due to Chrome removing content on backspace.
  // $aDomElement->setAttribute ("contenteditable", "false");
  // Remove the blue color from the link, and the underline.
  // The reason for this is that, if the blue and underline are there, people expect one can click on it.
  // Clicking it does nothing. Therefore it's better to remove the typical link style.
  aDomElement.append_attribute ("style") = "text-decoration:none; color: inherit;";
  // Style = class.
  if (style != "") aDomElement.append_attribute ("class") = style.c_str();
  aDomElement.text ().set (text.c_str());
}


// This adds a link as a mechanism to connect body text with a note body.
// $domNode: The DOM node where to add the link to.
// $identifier: The link's identifier.
// $style: A style for the note citation, and one for the note body.
// $text: The link's text.
// It also deals with a Quill-based editor, in a slightly different way.
void Editor_Usfm2Html::addNoteQuillLink (xml_node domNode, int identifier, string style, string text)
{
  xml_node aDomElement = domNode.append_child ("span");
  string cls = "i-note" + style + convert_to_string (identifier);
  aDomElement.append_attribute ("class") = cls.c_str();
  aDomElement.text ().set (text.c_str());
}


// Returns true if the road ahead is clear for the current marker.
bool Editor_Usfm2Html::roadIsClear ()
{
  // Determine the input.
  string input_marker;
  bool input_opener = false;
  bool input_embedded = false;
  int input_type = 0;
  int input_subtype = 0;
  {
    string currentItem = markersAndText[markersAndTextPointer];
    if (!usfm_is_usfm_marker (currentItem)) return true;
    input_opener = usfm_is_opening_marker (currentItem);
    input_embedded = usfm_is_embedded_marker (currentItem);
    string marker = usfm_get_marker (currentItem);
    input_marker = marker;
    if (!styles.count (marker)) return true;
    Database_Styles_Item style = styles [marker];
    input_type = style.type;
    input_subtype = style.subtype;
  }
  
  // Determine the road ahead.
  vector <string> markers;
  vector <int> types;
  vector <int> subtypes;
  vector <bool> openers;
  vector <bool> embeddeds;

  bool end_chapter_reached = false;
  {
    bool done = false;
    unsigned int markersAndTextCount = markersAndText.size();
    for (size_t pointer = markersAndTextPointer + 1; pointer < markersAndTextCount; pointer++) {
      if (done) continue;
      string currentItem = markersAndText[pointer];
      if (usfm_is_usfm_marker (currentItem))
      {
        string marker = usfm_get_marker (currentItem);
        if (styles.count (marker))
        {
          Database_Styles_Item style = styles [marker];
          markers.push_back (marker);
          types.push_back (style.type);
          subtypes.push_back (style.subtype);
          openers.push_back (usfm_is_opening_marker (currentItem));
          embeddeds.push_back (usfm_is_embedded_marker (currentItem));
          // Don't go beyond the next verse marker.
          if (style.type == StyleTypeVerseNumber) done = true;
        }
      }
    }
    if (!done) end_chapter_reached = true;
  }
  
  // Go through the road ahead, and assess it.
  for (size_t i = 0; i < types.size (); i++) {
    
    string marker = markers [i];
    int type = types [i];
    int subtype = subtypes [i];
    int opener = openers [i];
    int embedded = embeddeds [i];
    
    if (embedded) {}
    
    // The input is a note opener.
    if (input_type == StyleTypeFootEndNote) {
      if (input_opener) {
        if ((input_subtype == FootEndNoteSubtypeFootnote) || (input_subtype == FootEndNoteSubtypeEndnote)) {
          // Encounters note closer: road is clear.
          // Encounters another note opener: blocker.
          if (type == StyleTypeFootEndNote) {
            if ((subtype == FootEndNoteSubtypeFootnote) || (subtype == FootEndNoteSubtypeEndnote)) {
              if (opener) return false;
              else return true;
            }
          }
          // Encounters a verse: blocker.
          if (type == StyleTypeVerseNumber) return false;
          // Encounters cross reference opener: blocker.
          // Other \x.. markup is allowed.
          if (type == StyleTypeCrossreference) {
            if (subtype == CrossreferenceSubtypeCrossreference) {
              return false;
            }
          }
        }
      }
    }
    
    // The input is a cross reference opener.
    if (input_type == StyleTypeCrossreference) {
      if (input_opener) {
        if (input_subtype == CrossreferenceSubtypeCrossreference) {
          // Encounters xref closer: road is clear.
          // Encounters another xref opener: blocker.
          if (type == StyleTypeCrossreference) {
            if (subtype == CrossreferenceSubtypeCrossreference) {
              if (opener) return false;
              else return true;
            }
          }
          // Encounters a verse: blocker.
          if (type == StyleTypeVerseNumber) return false;
          // Encounters foot- or endnote opener: blocker.
          // Other \f.. markup is allowed.
          if (type == StyleTypeFootEndNote) {
            if ((subtype == FootEndNoteSubtypeFootnote) || (subtype == FootEndNoteSubtypeEndnote)) {
              return false;
            }
          }
        }
      }
    }
    
    // The input to check the road ahead for is an inline text opener, non-embedded, like "\add ".
    if (input_type == StyleTypeInlineText) {
      // If the input is embedded, declare the road ahead to be clear.
      if (input_embedded) return true;
      if (input_opener && !input_embedded) {
        if (type == StyleTypeInlineText) {
          if (embedded) {
            // An embedded inline marker is OK: Road ahead is clear.
            return true;
          } else {
            // It it encounters another non-embedded inline opening marker, that's a blocker.
            if (opener) return false;
            // If it finds a matching closing marker: OK.
            if (input_marker == marker) {
              if (!opener) return true;
            }
          }
        }
        // The inline text opener encounters a verse: blocker.
        if (type == StyleTypeVerseNumber) return false;
        // The inline text opener encounters a paragraph: blocker.
        if (type == StyleTypeStartsParagraph) return false;
      }
    }
    
  }

  // Nothing clearing the way was found, and if it reached the end of the chapter, that's a blocker.
  if (end_chapter_reached) return false;
  
  // No blockers found: The road is clear.
  return true;
}
