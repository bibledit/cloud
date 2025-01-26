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
  // Clean the USFM up.
  usfm = filter::strings::trim (usfm);
  usfm.append ("\n");
  // Separate the USFM into markers and text fragments.
  // Load it into the object.
  m_markers_and_text = filter::usfm::get_markers_and_text (usfm);
  m_markers_and_text_pointer = 0;
}


void Editor_Usfm2Html::stylesheet (const std::string& stylesheet)
{
  m_styles.clear();
  const std::vector <std::string> markers = database::styles1::get_markers (stylesheet);
  // Load the style information into the object.
  for (const auto& marker : markers) {
    database::styles1::Item style = database::styles1::get_marker_data (stylesheet, marker);
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
}


std::string Editor_Usfm2Html::get ()
{
  close_paragraph ();
  
  // If there are notes, move the notes <p> after everything else.
  // (It has the <hr> or <br> as a child).
  {
    const long count = std::distance (m_notes_node.begin (), m_notes_node.end ());
    if (count > 1) {
      m_body_node.append_move (m_notes_node);
    }
  }

  // A Quill-based editor does not work with embedded <p> elements.
  // Move the notes out of their parent and append them to the end of the main body.
  while (pugi::xml_node note = m_notes_node.first_child ().next_sibling ()) {
    m_body_node.append_move (note);
  }

  // If there are word-level attributes, move the word-level attributes <p> after everything else.
  {
    const long count = std::distance (m_word_level_attributes_node.begin (), m_word_level_attributes_node.end ());
    if (count > 1) {
      m_body_node.append_move (m_word_level_attributes_node);
    }
  }

  // A Quill-based editor does not work with embedded <p> elements.
  // Move the notes out of their parent and append them to the end of the main body.
  while (pugi::xml_node word_level_attribute = m_word_level_attributes_node.first_child ().next_sibling ()) {
    m_body_node.append_move (word_level_attribute);
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
      html.erase (pos);
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
  
  // Create the xml node for the notes container.
  // It comes at the start of the document.
  // (Later, it will either be deleted, or moved to the end).
  std::string notes_class = quill_notes_class;
  m_notes_node = m_document.append_child ("p");
  notes_class.insert (0, quill_class_prefix_block);
  m_notes_node.append_attribute ("class") = notes_class.c_str ();
  m_notes_node.text().set(filter::strings::non_breaking_space_u00A0().c_str());

  // Create the xml node for the word-level attributes container.
  // It comes near the start of the document.
  // (Later, it will either be deleted, or moved to the end).
  std::string word_level_attributes_class = quill_word_level_attributes_class;
  m_word_level_attributes_node = m_document.append_child ("p");
  word_level_attributes_class.insert (0, quill_class_prefix_block);
  m_word_level_attributes_node.append_attribute ("class") = word_level_attributes_class.c_str ();
  m_word_level_attributes_node.text().set(filter::strings::non_breaking_space_u00A0().c_str());
}


void Editor_Usfm2Html::process ()
{
  m_markers_and_text_pointer = 0;
  const size_t markers_and_text_count = m_markers_and_text.size();
  for (m_markers_and_text_pointer = 0; m_markers_and_text_pointer < markers_and_text_count; m_markers_and_text_pointer++) {
    const std::string current_item = m_markers_and_text[m_markers_and_text_pointer];
    if (filter::usfm::is_usfm_marker (current_item))
    {
      // Store indicator whether the marker is an opening marker.
      const bool is_opening_marker = filter::usfm::is_opening_marker (current_item);
      const bool is_embedded_marker = filter::usfm::is_embedded_marker (current_item);
      // Clean up the marker, so we remain with the basic version, e.g. 'id'.
      const std::string marker = filter::usfm::get_marker (current_item);
      // Handle preview mode: Strip word-level attributes.
      // Likely this can be removed from preview since word-level ttributes get extracted.
      if (m_preview)
        if (is_opening_marker)
          filter::usfm::remove_word_level_attributes (marker, m_markers_and_text, m_markers_and_text_pointer);

      if (m_styles.count (marker))
      {
        const database::styles1::Item& style = m_styles.at(marker);
        switch (style.type)
        {
          case StyleTypeIdentifier:
          {
            if (style.subtype == IdentifierSubtypePublishedVerseMarker) {
              // Treat the \vp ...\vp* marker as inline text.
              if (is_opening_marker) {
                open_text_style (style, is_embedded_marker);
              } else {
                close_text_style (is_embedded_marker);
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
                open_text_style (style, is_embedded_marker);
                extract_word_level_attributes();
              } else {
                add_text (filter::usfm::get_opening_usfm (marker));
              }
            } else {
              close_text_style (is_embedded_marker);
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
            std::string text_following_marker = filter::usfm::get_text_following_marker (m_markers_and_text, m_markers_and_text_pointer);
            const std::string number = filter::usfm::peek_verse_number (text_following_marker);
            m_verse_start_offsets [filter::strings::convert_to_int (number)] = static_cast<int>(m_text_tength);
            add_text (number);
            close_text_style (false);
            add_text (" ");
            // If there was any text following the \v marker, remove the verse number,
            // put the remainder back into the object, and update the pointer.
            if (!text_following_marker.empty()) {
              if (const size_t pos = text_following_marker.find (number);
                  pos != std::string::npos) {
                text_following_marker = text_following_marker.substr (pos + number.length());
              }
              text_following_marker = filter::strings::ltrim (text_following_marker);
              m_markers_and_text [m_markers_and_text_pointer] = text_following_marker;
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
                  add_note (caller, marker);
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
                  open_text_style (style, is_embedded_marker);
                } else {
                  close_text_style (is_embedded_marker);
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
                  add_note (caller, marker);
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
                  open_text_style (style, is_embedded_marker);
                  // Deal in a special way with possible word-level attributes.
                  // Note open | Extract word-level attributes
                  //  yes      |  no
                  //  no       |  yes
                  //-----------------
                  // In other words, if a note is open, which is the normal case,
                  // then don't extract the word-level attributes, but output them as they are.
                  // But if a note is not open, it is assumed to be a link reference in the main text body.
                  // In such a case, extract the word-level attributes as usual.
                  if (!m_note_opened)
                      extract_word_level_attributes();
                } else {
                  close_text_style (is_embedded_marker);
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
              extract_word_level_attributes();
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
        add_note_text (current_item);
      } else {
        add_text (current_item);
      }
    }
  }
}


void Editor_Usfm2Html::output_as_is (const std::string& marker, const bool is_opening_marker)
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
    std::string quill_style (style);
    quill_style.insert (0, quill_class_prefix_block);
    m_current_p_node.append_attribute ("class") = quill_style.c_str();
  }
  m_current_paragraph_style = style;
  m_current_paragraph_content.clear();
  // A Quill-based editor assigns a length of one to a new line.
  // Skip the first line.
  if (m_first_line_done)
    m_text_tength++;
  m_first_line_done = true;
}


void Editor_Usfm2Html::close_paragraph ()
{
  // Deal with a blank line.
  // If the paragraph is empty, add a <br> to it.
  // This is how the Quill editor naturally represents a new empty line.
  // This makes that empty paragraph to be displayed properly in the Quill editor.
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
void Editor_Usfm2Html::open_text_style (const database::styles1::Item& style, const bool embed)
{
  const std::string marker = style.marker;
  if (m_note_opened) {
    if (!embed)
      m_current_note_text_styles.clear();
    m_current_note_text_styles.push_back (marker);
  } else {
    if (!embed)
      m_current_text_styles.clear();
    m_current_text_styles.push_back (marker);
  }
}


// This closes any open text style.
// $embed: boolean: Whether to close embedded character style.
void Editor_Usfm2Html::close_text_style (const bool embed)
{
  if (m_note_opened) {
    if (!m_current_note_text_styles.empty ())
      m_current_note_text_styles.pop_back ();
    if (!embed)
      m_current_note_text_styles.clear ();
  } else {
    if (!m_current_text_styles.empty())
      m_current_text_styles.pop_back();
    if (!embed)
      m_current_text_styles.clear ();
  }
}


// This function adds text to the current paragraph.
// $text: The text to add.
void Editor_Usfm2Html::add_text (const std::string& text)
{
  if (!text.empty()) {
    if (!m_current_p_open) {
      new_paragraph ();
    }
    pugi::xml_node span_dom_element = m_current_p_node.append_child ("span");
    span_dom_element.text ().set (text.c_str());

    const auto assemble_text_style = [this]() -> std::string {
      std::string textstyle {};
      const auto add_style = [&textstyle](const auto& fragment) {
        if (!textstyle.empty ()) {
          // The Quill library is fussy about class names.
          // It accepts class="i-add" but not class="i-add-nd". It fails on that second hyphen.
          // It also does not accept an underscore as part of the class name.
          // That causes the whole class to be removed.
          // If a class is given as "i-add i-nd", then Quill removes the second class and remains with the first.
          // Right now the way to deal with a class with two styles is like this "i-add0nd".
          // It has one hyphen. And a "0" to separate the two styles.
          textstyle.append ("0");
        }
        textstyle.append (fragment);
      };
      if (!m_current_text_styles.empty ()) {
        // Take character style(s) as specified in this object.
        for (const auto& style : m_current_text_styles) {
          add_style(style);
        }
      }
      if (!textstyle.empty())
        textstyle.insert (0, quill_class_prefix_inline);
      // If any word-level attributes were extracted, store them here, and empty the container again.
      // Initially the idea was to have a "data-*" attribute and add this to the current <span> element.
      // This works in html, but fails in the Quill editor.
      // When the Quill editor loads html with "data-*" attributes, it strips and drops them.
      // The word-level attributes are now stored as-is, i.e. in raw format,
      // at the bottom of the html document,
      if (m_pending_word_level_attributes) {
        const std::string id = std::string(quill_word_level_attribute_class_prefix) + std::to_string(get_word_level_attributes_id(true));
        add_style(id);
        add_word_level_attributes(id);
        m_pending_word_level_attributes.reset();
      }
      return textstyle;
    };
    const std::string textstyle = assemble_text_style();
    if (!textstyle.empty())
      span_dom_element.append_attribute ("class") = textstyle.c_str();
    m_current_paragraph_content.append (text);
  }
  m_text_tength += filter::strings::unicode_string_length (text);
}


// This function adds a note to the current paragraph.
// $citation: The text of the note citation.
// $style: Style name for the paragraph in the note body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void Editor_Usfm2Html::add_note (const std::string& citation, const std::string& style)
{
  // Be sure that the road ahead is clear.
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
  std::string class_value (style);
  class_value.insert (0, quill_class_prefix_block);
  m_note_p_node.append_attribute ("class") = class_value.c_str();
  
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
void Editor_Usfm2Html::add_note_text (const std::string& text)
{
  if (text.empty ())
    return;
  if (!m_note_p_open) {
    add_note ("?", std::string());
  }
  pugi::xml_node span_dom_element = m_note_p_node.append_child ("span");
  span_dom_element.text ().set (text.c_str());
  if (!m_current_note_text_styles.empty()) {
    // Take character style(s) as specified in this object.
    std::string classs;
    classs = filter::strings::implode (m_current_note_text_styles, "0");
    classs.insert (0, quill_class_prefix_inline);
    span_dom_element.append_attribute ("class") = classs.c_str();
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
void Editor_Usfm2Html::add_notel_link (pugi::xml_node& dom_node, const int identifier,
                                       const std::string& style, const std::string& text)
{
  pugi::xml_node a_dom_element = dom_node.append_child ("span");
  const std::string class_value = "i-note" + style + std::to_string (identifier);
  a_dom_element.append_attribute ("class") = class_value.c_str();
  a_dom_element.text ().set (text.c_str());
}


// Returns true if the road ahead is clear for the current marker.
bool Editor_Usfm2Html::road_is_clear ()
{
  // Determine the input.
  std::string input_marker {};
  bool input_opener {false};
  bool input_embedded {false};
  int input_type {0};
  int input_subtype {0};
  {
    const std::string current_item = m_markers_and_text[m_markers_and_text_pointer];
    if (!filter::usfm::is_usfm_marker (current_item))
      return true;
    input_opener = filter::usfm::is_opening_marker (current_item);
    input_embedded = filter::usfm::is_embedded_marker (current_item);
    const std::string marker = filter::usfm::get_marker (current_item);
    input_marker = marker;
    if (!m_styles.count (marker))
      return true;
    database::styles1::Item style = m_styles [marker];
    input_type = style.type;
    input_subtype = style.subtype;
  }
  
  // Determine the road ahead.
  std::vector <std::string> markers {};
  std::vector <int> types {};
  std::vector <int> subtypes {};
  std::vector <bool> openers {};
  std::vector <bool> embeddeds {};

  bool end_chapter_reached {false};
  {
    bool done {false};
    const size_t markers_and_text_count = m_markers_and_text.size();
    for (size_t pointer = m_markers_and_text_pointer + 1; pointer < markers_and_text_count; pointer++) {
      if (done)
        continue;
      const std::string& current_item = m_markers_and_text.at(pointer);
      if (filter::usfm::is_usfm_marker (current_item))
      {
        const std::string marker = filter::usfm::get_marker (current_item);
        if (m_styles.count (marker))
        {
          database::styles1::Item& style = m_styles.at(marker);
          markers.push_back (marker);
          types.push_back (style.type);
          subtypes.push_back (style.subtype);
          openers.push_back (filter::usfm::is_opening_marker (current_item));
          embeddeds.push_back (filter::usfm::is_embedded_marker (current_item));
          // Don't go beyond the next verse marker.
          if (style.type == StyleTypeVerseNumber)
            done = true;
        }
      }
    }
    if (!done)
      end_chapter_reached = true;
  }
  
  // Go through the road ahead, and assess it.
  for (size_t i {0}; i < types.size (); i++) {
    
    const std::string& marker = markers.at(i);
    const int type = types [i];
    const int subtype = subtypes [i];
    const bool opener = openers [i];
    [[maybe_unused]] const bool embedded = embeddeds [i];
    
    // The input is a note opener.
    if (input_type == StyleTypeFootEndNote) {
      if (input_opener) {
        if ((input_subtype == FootEndNoteSubtypeFootnote) || (input_subtype == FootEndNoteSubtypeEndnote)) {
          // Encounters note closer: road is clear.
          // Encounters another note opener: blocker.
          if (type == StyleTypeFootEndNote) {
            if ((subtype == FootEndNoteSubtypeFootnote) || (subtype == FootEndNoteSubtypeEndnote)) {
              if (opener)
                return false;
              else
                return true;
            }
          }
          // Encounters a verse: blocker.
          if (type == StyleTypeVerseNumber)
            return false;
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
              if (opener)
                return false;
              else
                return true;
            }
          }
          // Encounters a verse: blocker.
          if (type == StyleTypeVerseNumber)
            return false;
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
      if (input_embedded)
        return true;
      if (input_opener && !input_embedded) {
        if (type == StyleTypeInlineText) {
          if (embedded) {
            // An embedded inline marker is OK: Road ahead is clear.
            return true;
          } else {
            // It it encounters another non-embedded inline opening marker, that's a blocker.
            if (opener)
              return false;
            // If it finds a matching closing marker: OK.
            if (input_marker == marker) {
              if (!opener)
                return true;
            }
          }
        }
        // The inline text opener encounters a verse: blocker.
        if (type == StyleTypeVerseNumber)
          return false;
        // The inline text opener encounters a paragraph: blocker.
        if (type == StyleTypeStartsParagraph)
          return false;
      }
    }
  }

  // Nothing clearing the way was found, and if it reached the end of the chapter, that's a blocker.
  if (end_chapter_reached)
    return false;
  
  // No blockers found: The road is clear.
  return true;
}


void Editor_Usfm2Html::set_preview ()
{
  m_preview = true;
}


int Editor_Usfm2Html::get_word_level_attributes_id(const bool next)
{
  if (next) {
    m_word_level_attributes_id++;
    // The class name for the Quill editor has defined the "0" as class separator.
    // So make sure that the word-level attribute ID never contains any "0".
    while (std::to_string(m_word_level_attributes_id).find("0") != std::string::npos) {
      m_word_level_attributes_id++;
    }
  }
  return m_word_level_attributes_id;
}


void Editor_Usfm2Html::extract_word_level_attributes()
{
  // The current function is expected to be called on an opening marker
  // that also may contain word-level attributes.
  // See USFM 3.x for details.

  // Check the text following this markup whether it contains word-level attribut(s).
  std::string possible_markup = filter::usfm::peek_text_following_marker (m_markers_and_text, m_markers_and_text_pointer);
  
  // The minumum length of the markup is:
  // 1. The canonical word: At least 1 character.
  // 2. The vertical bar: One character.
  // 3. The attribute: At least character for the default auttribute.
  // If the markup is too short then bail out.
  if (possible_markup.size() < 3)
    return;
  
  // Look for the vertical bar. If it's not there, bail out.
  const size_t bar_position = possible_markup.find("|");
  if (bar_position == std::string::npos)
    return;
  
  // Remove the fragment and store the remainder back into the object.
  m_markers_and_text [m_markers_and_text_pointer + 1] = possible_markup.substr(0, bar_position);
  
  // Remain with the fragment with the word-level attributes.
  // Store the word-level attributes as-is, as pending item, ready for processing.
  std::string attribute_string = possible_markup.substr(bar_position + 1);
  m_pending_word_level_attributes = std::move(attribute_string);
}


void Editor_Usfm2Html::add_word_level_attributes(const std::string id)
{
  // If there's no attributes, bail out.
  if (!m_pending_word_level_attributes)
    return;
  
  // Open a paragraph element for the word-level attributes body.
  pugi::xml_node p_node = m_word_level_attributes_node.append_child ("p");

  // Set the correct class in the paragraph. This links it to the correct text fragment in the text body.
  const std::string class_value {quill_class_prefix_block + id};
  p_node.append_attribute ("class") = class_value.c_str();

  // Add the word-level attributes as text.
  p_node.text ().set (m_pending_word_level_attributes.value().c_str());
}
