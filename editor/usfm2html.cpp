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


#include <editor/usfm2html.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/logs.h>
#include <quill/logic.h>


void Editor_Usfm2Html::load (std::string usfm)
{
  // Clean up.
  usfm = filter::strings::trim (usfm);
  usfm.append ("\n");
  // Separate it into markers and text.
  // Load it into the object.
  m_markers_and_text = filter::usfm::get_markers_and_text (usfm);
  m_markers_and_text_pointer = 0;
}


void Editor_Usfm2Html::stylesheet (std::string stylesheet)
{
  Database_Styles database_styles;
  m_styles.clear();
  std::vector <std::string> markers = database_styles.getMarkers (stylesheet);
  // Load the style information into the object.
  for (const auto& marker : markers) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    m_styles [marker] = style;
    if (style.type == StyleTypeFootEndNote) {
      if (style.subtype == FootEndNoteSubtypeStandardContent) {
        m_standard_content_marker_foot_end_note = style.marker;
      }
    }
    if (style.type == StyleTypeCrossreference) {
      if (style.subtype == CrossreferenceSubtypeStandardContent) {
        m_standard_content_marker_cross_reference = style.marker;
      }
    }
    m_note_citations.evaluate_style(style);
  }
}


void Editor_Usfm2Html::run ()
{
  preprocess ();
  process ();
  postprocess ();
}


std::string Editor_Usfm2Html::get ()
{
  close_paragraph ();

  // If there are notes, move the notes <div> or <p> after everything else.
  // (It has the <hr> or <br> as a child).
  const long int count = std::distance (m_notes_node.begin (), m_notes_node.end ());
  if (count > 1) {
    m_body_node.append_move (m_notes_node);
  }

  // A Quill-based editor does not work with embedded <p> elements.
  // Move the notes out of their parent and append them to the end of the main body.
  while (pugi::xml_node note = m_notes_node.first_child ().next_sibling ()) {
    m_body_node.append_move (note);
  }
  
  // Get the html code, including body, without head.
  std::stringstream output {};
  m_body_node.print (output, "", pugi::format_raw);
  std::string html = output.str ();
  
  // Remain with the stuff within the <body> elements.
  size_t pos = html.find ("<body>");
  if (pos != std::string::npos) {
    html.erase (0, pos + 6);
    pos = html.find ("</body>");
    if (pos != std::string::npos) {
      html.erase  (pos);
    }
  }
  
  // Currently the XML library produces hexadecimal character entities.
  // This is unwanted behaviour: Convert them to normal characters.
  html = filter::strings::convert_xml_character_entities_to_characters (html);
  
  // Result.
  return html;
}


void Editor_Usfm2Html::preprocess ()
{
  m_current_paragraph_style.clear ();
  m_current_paragraph_content.clear ();
  m_current_text_styles.clear();
  m_note_count = 0;
  m_current_note_text_styles.clear();
  m_text_tength = 0;
  m_verse_start_offsets = { std::pair (0, 0) };
  m_current_p_open = false;
  m_note_p_open = false;

  // XPath crashes on Android with libxml2 2.9.2 compiled through the Android NDK.
  // After the move to pugixml, this no longer applies.

  m_body_node = m_document.append_child ("body");
  
  // Create notes xml node.
  // It comes at the start of the document.
  // (Later, it will either be deleted, or moved to the end).
  std::string notes_value = "notes";
  m_notes_node = m_document.append_child ("p");
  notes_value.insert (0, quill_logic_class_prefix_block ());
  m_notes_node.append_attribute ("class") = notes_value.c_str ();
  m_notes_node.text().set(filter::strings::non_breaking_space_u00A0().c_str());
}


void Editor_Usfm2Html::process ()
{
  m_markers_and_text_pointer = 0;
  size_t markersAndTextCount = m_markers_and_text.size();
  for (m_markers_and_text_pointer = 0; m_markers_and_text_pointer < markersAndTextCount; m_markers_and_text_pointer++) {
    std::string currentItem = m_markers_and_text[m_markers_and_text_pointer];
    if (filter::usfm::is_usfm_marker (currentItem))
    {
      // Store indicator whether the marker is an opening marker.
      bool is_opening_marker = filter::usfm::is_opening_marker (currentItem);
      bool isEmbeddedMarker = filter::usfm::is_embedded_marker (currentItem);
      // Clean up the marker, so we remain with the basic version, e.g. 'id'.
      std::string marker = filter::usfm::get_marker (currentItem);
      // Handle preview mode: Strip word-level attributes.
      if (m_preview) if (is_opening_marker) filter::usfm::remove_word_level_attributes (marker, m_markers_and_text, m_markers_and_text_pointer);

      if (m_styles.count (marker))
      {
        Database_Styles_Item style = m_styles [marker];
        switch (style.type)
        {
          case StyleTypeIdentifier:
          {
            if (style.subtype == IdentifierSubtypePublishedVerseMarker) {
              // Treat the \vp ...\vp* marker as inline text.
              if (is_opening_marker) {
                open_text_style (style, isEmbeddedMarker);
              } else {
                close_text_style (isEmbeddedMarker);
              }
            } else {
              // Any other identifier: Plain text.
              close_text_style (false);
              output_as_is (marker, is_opening_marker);
            }
            break;
          }
          case StyleTypeNotUsedComment:
          case StyleTypeNotUsedRunningHeader:
          {
            close_text_style (false);
            output_as_is (marker, is_opening_marker);
            break;
          }
          case StyleTypeStartsParagraph:
          {
            close_text_style (false);
            close_paragraph ();
            new_paragraph (marker);
            break;
          }
          case StyleTypeInlineText:
          {
            if (is_opening_marker) {
              // Be sure the road ahead is clear.
              if (road_is_clear ()) {
                open_text_style (style, isEmbeddedMarker);
              } else {
                add_text (filter::usfm::get_opening_usfm (marker));
              }
            } else {
              close_text_style (isEmbeddedMarker);
            }
            break;
          }
          case StyleTypeChapterNumber:
          {
            close_text_style (false);
            close_paragraph ();
            new_paragraph (marker);
            break;
          }
          case StyleTypeVerseNumber:
          {
            // Close any existing text style.
            close_text_style (false);
            // Output the space before the verse number in case the paragraph already has some text.
            if (!m_current_paragraph_content.empty()) {
              add_text (" ");
            }
            // Open verse style, record verse/length, add verse number, close style again, and add a space.
            open_text_style (style, false);
            std::string textFollowingMarker = filter::usfm::get_text_following_marker (m_markers_and_text, m_markers_and_text_pointer);
            std::string number = filter::usfm::peek_verse_number (textFollowingMarker);
            m_verse_start_offsets [filter::strings::convert_to_int (number)] = static_cast<int>(m_text_tength);
            add_text (number);
            close_text_style (false);
            add_text (" ");
            // If there was any text following the \v marker, remove the verse number,
            // put the remainder back into the object, and update the pointer.
            if (textFollowingMarker != "") {
              size_t pos = textFollowingMarker.find (number);
              if (pos != std::string::npos) {
                textFollowingMarker = textFollowingMarker.substr (pos + number.length());
              }
              textFollowingMarker = filter::strings::ltrim (textFollowingMarker);
              m_markers_and_text [m_markers_and_text_pointer] = textFollowingMarker;
              m_markers_and_text_pointer--;
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
                close_text_style (false);
                if (is_opening_marker) {
                  const std::string caller = m_note_citations.get (style.marker, "+");
                  add_note (caller, marker, false);
                } else {
                  close_current_note ();
                }
                break;
              }
              case FootEndNoteSubtypeStandardContent:
              case FootEndNoteSubtypeContent:
              case FootEndNoteSubtypeContentWithEndmarker:
              {
                if (is_opening_marker) {
                  open_text_style (style, isEmbeddedMarker);
                } else {
                  close_text_style (isEmbeddedMarker);
                }
                break;
              }
              case FootEndNoteSubtypeParagraph:
              default:
              {
                close_text_style (false);
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
                close_text_style (false);
                if (is_opening_marker) {
                  const std::string caller = m_note_citations.get (style.marker, "+");
                  add_note (caller, marker, false);
                } else {
                  close_current_note ();
                }
                break;
              }
              case CrossreferenceSubtypeContent:
              case CrossreferenceSubtypeContentWithEndmarker:
              case CrossreferenceSubtypeStandardContent:
              {
                if (is_opening_marker) {
                  open_text_style (style, isEmbeddedMarker);
                } else {
                  close_text_style (isEmbeddedMarker);
                }
                break;
              }
              default:
              {
                close_text_style (false);
                break;
              }
            }
            break;
          }
          case StyleTypePeripheral:
          {
            close_text_style (false);
            output_as_is (marker, is_opening_marker);
            break;
          }
          case StyleTypePicture:
          {
            close_text_style (false);
            output_as_is (marker, is_opening_marker);
            break;
          }
          case StyleTypePageBreak:
          {
            close_text_style (false);
            output_as_is (marker, is_opening_marker);
            break;
          }
          case StyleTypeTableElement:
          {
            close_text_style (false);
            switch (style.subtype)
            {
              case TableElementSubtypeRow:
              {
                output_as_is (marker, is_opening_marker);
                break;
              }
              case TableElementSubtypeHeading:
              case TableElementSubtypeCell:
              {
                open_text_style (style, false);
                break;
              }
              default:
              {
                open_text_style (style, false);
                break;
              }
            }
            break;
          }
          case StyleTypeWordlistElement:
          {
            if (is_opening_marker) {
              open_text_style (style, false);
            } else {
              close_text_style (false);
            }
            break;
          }
          default:
          {
            // This marker is known in the stylesheet, but not yet implemented here.
            close_text_style (false);
            output_as_is (marker, is_opening_marker);
            break;
          }
        }
      } else {
        // This is a marker unknown in the stylesheet.
        close_text_style (false);
        output_as_is (marker, is_opening_marker);
      }
    } else {
      // Here is no marker. Treat it as text.
      if (m_note_opened) {
        add_note_text (currentItem);
      } else {
        add_text (currentItem);
      }
    }
  }
}


void Editor_Usfm2Html::postprocess ()
{
}


void Editor_Usfm2Html::output_as_is (std::string marker, bool is_opening_marker)
{
  // Output the marker in monospace font.
  if (is_opening_marker) {
    // Add opening marker as it is.
    close_paragraph ();
    new_paragraph ("mono");
    add_text (filter::usfm::get_opening_usfm (marker));
  } else {
    // Add closing marker to existing paragraph.
    add_text (filter::usfm::get_closing_usfm (marker));
  }
}


void Editor_Usfm2Html::new_paragraph (std::string style)
{
  // Handle new paragraph.
  m_current_p_node = m_body_node.append_child ("p");
  m_current_p_open = true;
  if (!style.empty()) {
    std::string style2 (style);
    style2.insert (0, quill_logic_class_prefix_block ());
    m_current_p_node.append_attribute ("class") = style2.c_str();
  }
  m_current_paragraph_style = style;
  m_current_paragraph_content.clear();
  // A Quill-based editor assigns a length of one to a new line.
  // Skip the first line.
  if (m_first_line_done) m_text_tength++;
  m_first_line_done = true;
}


void Editor_Usfm2Html::close_paragraph ()
{
  // Deal with a blank line.
  // If the paragraph is empty, add a <br> to it.
  // This is how the Quill editor naturally represents a new empty line.
  // This causes that empty paragraph to be displayed properly in the Quill editor.
  // This <br> is also needed for live editor updates.
  if (m_current_p_open) {
    if (m_current_paragraph_content.empty()) {
      m_current_p_node.append_child("br");
    }
  }
}


// This opens a text style.
// $style: the array containing the style variables.
// $embed: boolean: Whether to open embedded / nested style.
void Editor_Usfm2Html::open_text_style (Database_Styles_Item & style, bool embed)
{
  const std::string marker = style.marker;
  if (m_note_opened) {
    if (!embed) m_current_note_text_styles.clear();
    m_current_note_text_styles.push_back (marker);
  } else {
    if (!embed) m_current_text_styles.clear();
    m_current_text_styles.push_back (marker);
  }
}


// This closes any open text style.
// $embed: boolean: Whether to close embedded character style.
void Editor_Usfm2Html::close_text_style (bool embed)
{
  if (m_note_opened) {
    if (!m_current_note_text_styles.empty ()) m_current_note_text_styles.pop_back ();
    if (!embed) m_current_note_text_styles.clear ();
  } else {
    if (!m_current_text_styles.empty()) m_current_text_styles.pop_back();
    if (!embed) m_current_text_styles.clear ();
  }
}


// This function adds text to the current paragraph.
// $text: The text to add.
void Editor_Usfm2Html::add_text (std::string text)
{
  if (text != "") {
    if (!m_current_p_open) {
      new_paragraph ();
    }
    pugi::xml_node spanDomElement = m_current_p_node.append_child ("span");
    spanDomElement.text ().set (text.c_str());
    if (!m_current_text_styles.empty ()) {
      // Take character style(s) as specified in this object.
      std::string textstyle {};
      for (const auto& style : m_current_text_styles) {
        if (!textstyle.empty ()) {
          // The Quill library is fussy about class names.
          // It accepts class="i-add" but not class="i-add-nd". It fails on that second hyphen.
          // It also does not accept an underscore as part of the class name.
          // That causes the whole class to be removed.
          // Right now the way to deal with a class with two styles is like this "i-add0nd".
          // It has one hyphen. And a "0" to separate the two styles.
          textstyle.append ("0");
        }
        textstyle.append (style);
      }
      textstyle.insert (0, quill_logic_class_prefix_inline ());
      spanDomElement.append_attribute ("class") = textstyle.c_str();
    }
    m_current_paragraph_content.append (text);
  }
  m_text_tength += filter::strings::unicode_string_length (text);
}


// This function adds a note to the current paragraph.
// $citation: The text of the note citation.
// $style: Style name for the paragraph in the note body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void Editor_Usfm2Html::add_note (std::string citation, std::string style, [[maybe_unused]] bool endnote)
{
  // Be sure the road ahead is clear.
  if (!road_is_clear ()) {
    add_text (filter::usfm::get_opening_usfm (style));
    return;
  }
  
  // Ensure that a paragraph is open, so that the note can be added to it.
  if (!m_current_p_open) {
    new_paragraph ();
  }
  
  m_note_count++;
  m_note_opened = true;
  
  // Add the link with all relevant data for the note citation.
  add_notel_link (m_current_p_node, m_note_count, "call", citation);
  
  // Open a paragraph element for the note body.
  m_note_p_node = m_notes_node.append_child ("p");
  m_note_p_open = true;
  std::string cls (style);
  cls.insert (0, quill_logic_class_prefix_block ());
  m_note_p_node.append_attribute ("class") = cls.c_str();
  
  close_text_style (false);
  
  // Add the link with all relevant data for the note body.
  add_notel_link (m_note_p_node, m_note_count, "body", citation);
  
  // Add a space.
  add_note_text (" ");
  
  // Update the text length of the text body, excluding the note.
  m_text_tength += filter::strings::unicode_string_length (citation);
}


// This function adds text to the current footnote.
// $text: The text to add.
void Editor_Usfm2Html::add_note_text (std::string text)
{
  if (text.empty ()) return;
  if (!m_note_p_open) {
    add_note ("?", "");
  }
  pugi::xml_node spanDomElement = m_note_p_node.append_child ("span");
  spanDomElement.text ().set (text.c_str());
  if (!m_current_note_text_styles.empty()) {
    // Take character style(s) as specified in this object.
    std::string classs;
    classs = filter::strings::implode (m_current_note_text_styles, "0");
    classs.insert (0, quill_logic_class_prefix_inline ());
    spanDomElement.append_attribute ("class") = classs.c_str();
  }
}


// This function closes the current footnote.
void Editor_Usfm2Html::close_current_note ()
{
  // If a note was opened, close that, else close the standard text.
  close_text_style (false);
  m_note_p_open = false;
  m_note_opened = false;
}


// This adds a link as a mechanism to connect body text with a note body.
// $domNode: The DOM node where to add the link to.
// $identifier: The link's identifier.
// $style: A style for the note citation, and one for the note body.
// $text: The link's text.
// It also deals with a Quill-based editor, in a slightly different way.
void Editor_Usfm2Html::add_notel_link (pugi::xml_node domNode, int identifier, std::string style, std::string text)
{
  pugi::xml_node aDomElement = domNode.append_child ("span");
  std::string cls = "i-note" + style + filter::strings::convert_to_string (identifier);
  aDomElement.append_attribute ("class") = cls.c_str();
  aDomElement.text ().set (text.c_str());
}


// Returns true if the road ahead is clear for the current marker.
bool Editor_Usfm2Html::road_is_clear ()
{
  // Determine the input.
  std::string input_marker;
  bool input_opener = false;
  bool input_embedded = false;
  int input_type = 0;
  int input_subtype = 0;
  {
    std::string currentItem = m_markers_and_text[m_markers_and_text_pointer];
    if (!filter::usfm::is_usfm_marker (currentItem)) return true;
    input_opener = filter::usfm::is_opening_marker (currentItem);
    input_embedded = filter::usfm::is_embedded_marker (currentItem);
    std::string marker = filter::usfm::get_marker (currentItem);
    input_marker = marker;
    if (!m_styles.count (marker)) return true;
    Database_Styles_Item style = m_styles [marker];
    input_type = style.type;
    input_subtype = style.subtype;
  }
  
  // Determine the road ahead.
  std::vector <std::string> markers;
  std::vector <int> types;
  std::vector <int> subtypes;
  std::vector <bool> openers;
  std::vector <bool> embeddeds;

  bool end_chapter_reached = false;
  {
    bool done = false;
    size_t markersAndTextCount = m_markers_and_text.size();
    for (size_t pointer = m_markers_and_text_pointer + 1; pointer < markersAndTextCount; pointer++) {
      if (done) continue;
      std::string currentItem = m_markers_and_text[pointer];
      if (filter::usfm::is_usfm_marker (currentItem))
      {
        std::string marker = filter::usfm::get_marker (currentItem);
        if (m_styles.count (marker))
        {
          Database_Styles_Item style = m_styles [marker];
          markers.push_back (marker);
          types.push_back (style.type);
          subtypes.push_back (style.subtype);
          openers.push_back (filter::usfm::is_opening_marker (currentItem));
          embeddeds.push_back (filter::usfm::is_embedded_marker (currentItem));
          // Don't go beyond the next verse marker.
          if (style.type == StyleTypeVerseNumber) done = true;
        }
      }
    }
    if (!done) end_chapter_reached = true;
  }
  
  // Go through the road ahead, and assess it.
  for (size_t i = 0; i < types.size (); i++) {
    
    std::string marker = markers [i];
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


void Editor_Usfm2Html::set_preview ()
{
  m_preview = true;
}
