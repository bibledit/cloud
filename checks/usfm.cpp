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


#include <checks/usfm.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <database/styles.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/bibleimages.h>
#include <styles/logic.h>
#include <locale/translate.h>


Checks_Usfm::Checks_Usfm (const std::string& bible)
{
  m_stylesheet = database::config::bible::get_export_stylesheet (bible);
  markers_stylesheet = database::styles::get_markers (m_stylesheet);
  for (const stylesv2::Style& style : stylesv2::styles) {
    // Find out which markers require an endmarker.
    // And which markers are embeddable.
    bool required_endmarker {false};
    bool embeddable_marker {false};
    switch (style.type) {
      case stylesv2::Type::book_id:
      case stylesv2::Type::usfm_version:
      case stylesv2::Type::file_encoding:
      case stylesv2::Type::remark:
      case stylesv2::Type::running_header:
        break;
      case stylesv2::Type::long_toc_text:
        long_toc1_marker = style.marker;
        break;
      case stylesv2::Type::short_toc_text:
        short_toc2_marker = style.marker;
        break;
      case stylesv2::Type::book_abbrev:
        abbrev_toc3_marker = style.marker;
        break;
      case stylesv2::Type::introduction_end:
      case stylesv2::Type::title:
      case stylesv2::Type::heading:
      case stylesv2::Type::paragraph:
      case stylesv2::Type::chapter:
      case stylesv2::Type::chapter_label:
      case stylesv2::Type::published_chapter_marker:
        break;
      case stylesv2::Type::alternate_chapter_number:
        required_endmarker = true;
        break;
      case stylesv2::Type::verse:
        break;
      case stylesv2::Type::published_verse_marker:
      case stylesv2::Type::alternate_verse_marker:
        required_endmarker = true;
        break;
      case stylesv2::Type::table_row:
      case stylesv2::Type::table_heading:
      case stylesv2::Type::table_cell:
        break;
      case stylesv2::Type::footnote_wrapper:
      case stylesv2::Type::endnote_wrapper:
        required_endmarker = true;
        break;
      case stylesv2::Type::note_standard_content:
      case stylesv2::Type::note_content:
        break;
      case stylesv2::Type::note_content_with_endmarker:
        required_endmarker = true;
        break;
      case stylesv2::Type::note_paragraph:
        break;
      case stylesv2::Type::crossreference_wrapper:
        required_endmarker = true;
        break;
      case stylesv2::Type::crossreference_standard_content:
      case stylesv2::Type::crossreference_content:
        break;
      case stylesv2::Type::crossreference_content_with_endmarker:
        required_endmarker = true;
        break;
      case stylesv2::Type::character:
        required_endmarker = true;
        embeddable_marker = true;
        break;
      case stylesv2::Type::page_break:
        break;
      case stylesv2::Type::figure:
        required_endmarker = true;
        break;
      case stylesv2::Type::word_list:
        required_endmarker = true;
        embeddable_marker = true;
        break;
      case stylesv2::Type::sidebar_begin:
      case stylesv2::Type::sidebar_end:
      case stylesv2::Type::peripheral:
      case stylesv2::Type::starting_boundary:
      case stylesv2::Type::stopping_boundary:
      case stylesv2::Type::none:
      default:
        break;
    }
    if (required_endmarker) {
      markers_requiring_endmarkers.push_back (style.marker);
    }
    if (embeddable_marker) {
      embeddable_markers.push_back (style.marker);
    }
  }
}


void Checks_Usfm::initialize (int book, int chapter)
{
  checking_results.clear ();
  usfm_markers_and_text.clear ();
  usfm_markers_and_text_pointer = 0;
  book_number = book;
  chapter_number = chapter;
  verse_number = 0;
  open_matching_markers.clear ();
  empty_markup_previous_item.clear();
}


void Checks_Usfm::finalize ()
{
  // Check on unclosed markers.
  if (open_matching_markers.size () > 0) {
    add_result (translate ("Unclosed markers:") + " " + filter::strings::implode (open_matching_markers, " "), display_nothing);
  }
}


void Checks_Usfm::check (const std::string& usfm)
{
  new_line_in_usfm (usfm);
  
  forward_slash (usfm);

  toc (usfm);
  
  usfm_markers_and_text = filter::usfm::get_markers_and_text (usfm);
  for (usfm_markers_and_text_pointer = 0; usfm_markers_and_text_pointer < usfm_markers_and_text.size(); usfm_markers_and_text_pointer++) {
    usfm_item = usfm_markers_and_text [usfm_markers_and_text_pointer];
    if (filter::usfm::is_usfm_marker (usfm_item)) {
      
      // Get the current verse number.
      if (usfm_item == R"(\v )") {
        std::string verseCode = filter::usfm::peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
        verse_number = filter::strings::convert_to_int (filter::usfm::peek_verse_number (verseCode));
      }
      
      malformed_verse_number ();
      
      marker_in_stylesheet ();
      
      malformed_id ();
      
      widow_back_slash ();
      
      matching_endmarker ();
      
      embedded_marker ();
      
      figure ();

    }
    
    empty_markup();
    
    note();

  }
}


void Checks_Usfm::malformed_verse_number ()
{
  if (usfm_item == "\\v ") {
    const std::string code = filter::usfm::peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
    const std::string cleanVerseNumber = filter::usfm::peek_verse_number (code);
    std::vector <std::string> v_dirtyVerseNumber = filter::strings::explode (code, ' ');
    std::string dirtyVerseNumber {};
    if (!v_dirtyVerseNumber.empty ()) dirtyVerseNumber = v_dirtyVerseNumber [0];
    if (cleanVerseNumber != dirtyVerseNumber) {
      add_result (translate ("Malformed verse number"), display_full);
    }
  }
}


void Checks_Usfm::new_line_in_usfm (const std::string& usfm)
{
  size_t position {std::string::npos};
  size_t pos = usfm.find ("\\\n");
  if (pos != std::string::npos) {
    position = pos;
  }
  pos = usfm.find ("\\ \n");
  if (pos != std::string::npos) {
    position = pos;
  }
  if (position != std::string::npos) {
    if (position == 0) position = 1;
    std::string bit = usfm.substr (position - 1, 10);
    bit = filter::strings::replace ("\n", " ", bit);
    add_result (translate ("New line within USFM:") + " " + bit, display_nothing);
  }
}


void Checks_Usfm::marker_in_stylesheet ()
{
  std::string marker = usfm_item.substr (1);
  marker = filter::strings::trim (marker);
  if (!filter::usfm::is_opening_marker (marker)) {
    if (!marker.empty ()) marker = marker.substr (0, marker.length () - 1);
  }
  if (filter::usfm::is_embedded_marker (marker)) {
    if (!marker.empty ()) marker = marker.substr (1);
  }
  if (marker.empty()) return;
  if (in_array (marker, markers_stylesheet)) return;
  add_result (translate ("Marker not in stylesheet"), Checks_Usfm::display_current);
}


void Checks_Usfm::malformed_id ()
{
  const std::string item = usfm_item.substr (0, 3);
  const std::string ide = usfm_item.substr (0, 4);
  if (ide == R"(\ide)") return;
  if (item == R"(\id)") {
    const std::string code = filter::usfm::peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
    const std::string sid = code.substr (0, 3);
    const std::vector <std::string> vid = filter::strings::explode (code, ' ');
    std::string id {};
    if (!vid.empty ()) id = vid [0];
    book_id book = database::books::get_id_from_usfm (id);
    if (book == book_id::_unknown) {
      add_result (translate ("Unknown ID"), display_full);
    } else {
      if (filter::strings::unicode_string_uppercase (id) != id) {
        add_result (translate ("ID is not in uppercase"), display_full);
      }
    }
  }
}


void Checks_Usfm::forward_slash (const std::string& usfm)
{
  const std::string code = filter::strings::replace ("\n", " ", usfm);
  size_t pos = code.find ("/");
  std::string bit {};
  if (pos != std::string::npos) {
    size_t pos2 = code.find (" ", pos);
    if (pos2 != std::string::npos) {
      bit = code.substr (pos, pos2 - pos);
    } else {
      bit = code.substr (pos, 100);
    }
    pos2 = bit.find ("*");
    if (pos2 != std::string::npos) {
      bit = bit.substr (0, pos2);
    }
    const std::string marker = bit.substr (1, 100);
    if (find (markers_stylesheet.begin(), markers_stylesheet.end(), marker) != markers_stylesheet.end ()) {
      add_result (translate ("Forward slash instead of backslash:") + " " + bit, display_nothing);
    }
  }
}


void Checks_Usfm::widow_back_slash ()
{
  std::string marker = usfm_item;
  marker = filter::strings::trim (marker);
  if (marker.length() == 1) {
    add_result (translate ("Widow backslash"), display_current);
  }
}


void Checks_Usfm::matching_endmarker ()
{
  std::string marker = usfm_item;
  // Remove the initial backslash, e.g. '\add' becomes 'add'.
  marker = marker.substr (1);
  marker = filter::strings::trim (marker);
  bool is_opener = filter::usfm::is_opening_marker (marker);
  if (!is_opener) {
    if (!marker.empty ())
      marker.pop_back();
  }
  if (!in_array (marker, markers_requiring_endmarkers))
    return;
  if (is_opener) {
    if (in_array (marker, open_matching_markers)) {
      add_result (translate ("Repeating opening marker"), Checks_Usfm::display_current);
    } else {
      open_matching_markers.push_back (marker);
    }
  } else {
    if (in_array (marker, open_matching_markers)) {
      open_matching_markers = filter::strings::array_diff (open_matching_markers, {marker});
    } else {
      add_result (translate ("Closing marker does not match opening marker") + " " + filter::strings::implode (open_matching_markers, " "), display_current);
    }
  }
}


void Checks_Usfm::embedded_marker ()
{
  // The marker, e.g. '\add'.
  std::string marker = usfm_item;

  // Remove the initial backslash, e.g. '\add' becomes 'add'.
  marker = marker.substr (1);
  marker = filter::strings::trim (marker);

  const bool isOpener = filter::usfm::is_opening_marker (marker);

  // Clean a closing marker, e.g. '\add*' becomes '\add'.
  if (!isOpener) {
    if (!marker.empty ()) marker = marker.substr (0, marker.length () - 1);
  }
  
  // If the marker is not relevant for this check, bail out.
  if (!in_array (marker, embeddable_markers)) return;
  
  // Checking method is as follows:
  // If there's no open embeddable markers, then the '+' sign is not needed.
  // If there's open embeddable markers, and another marker is opened,
  // then the '+' sign is needed.
  // Example USFM:
  // \v 1 This \add is an \+w embedded\+w* marker\add*.
  // See the following URL for more information about embedding markers:
  // https://ubsicap.github.io/usfm/characters/nesting.html
  
  bool checkEmbedding = false;
  if (isOpener) {
    if (!in_array (marker, open_embeddable_markers)) {
      if (!open_embeddable_markers.empty ()) {
        checkEmbedding = true;
      }
      open_embeddable_markers.push_back (marker);
    }
  } else {
    if (in_array (marker, open_embeddable_markers)) {
      open_embeddable_markers = filter::strings::array_diff (open_embeddable_markers, {marker});
      if (!open_embeddable_markers.empty ()) {
        checkEmbedding = true;
      }
    }
  }
  
  if (checkEmbedding) {
    if (marker.substr (0, 1) != "+") {
      add_result (translate ("Embedded marker requires a plus sign"), display_full);
    }
  }
}


void Checks_Usfm::toc (std::string usfm)
{
  // Only check the 66 canonical books.
  // Skip any of the other books.
  const book_type type = database::books::get_type (static_cast<book_id>(book_number));
  if ((type == book_type::old_testament) || (type == book_type::new_testament)) {

    // Check on the presence of the table of contents markers in this chapter.
    bool toc1_present = usfm.find (filter::usfm::get_opening_usfm (long_toc1_marker)) != std::string::npos;
    bool toc2_present = usfm.find (filter::usfm::get_opening_usfm (short_toc2_marker)) != std::string::npos;
    bool toc3_present = usfm.find (filter::usfm::get_opening_usfm (abbrev_toc3_marker)) != std::string::npos;

    // The markers should be on chapter 0 only.
    if (chapter_number == 0) {
      // Required: \toc1
      if (!toc1_present) {
        add_result (translate ("The book lacks the marker for the verbose book name:") + " " + filter::usfm::get_opening_usfm (long_toc1_marker), display_nothing);
      }
      // Required: \toc2
      if (!toc2_present) {
        add_result (translate ("The book lacks the marker for the short book name:") + " " + filter::usfm::get_opening_usfm (short_toc2_marker), display_nothing);
      }
    } else {
      const std::string msg = translate ("The following marker belongs in chapter 0:") + " ";
      // Required markers.
      if (toc1_present) {
        add_result (msg + filter::usfm::get_opening_usfm (long_toc1_marker), display_nothing);
      }
      if (toc2_present) {
        add_result (msg + filter::usfm::get_opening_usfm (short_toc2_marker), display_nothing);
      }
      // Optional markers, but should not be anywhere else except in chapter 0.
      if (toc3_present) {
        add_result (msg + filter::usfm::get_opening_usfm (abbrev_toc3_marker), display_nothing);
      }
    }
  }
}


void Checks_Usfm::figure ()
{
  if (usfm_item == R"(\fig )") {
    std::string usfm = filter::usfm::peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
    std::string caption, alt, src, size, loc, copy, ref;
    filter::usfm::extract_fig (usfm, caption, alt, src, size, loc, copy, ref);
    if (src.empty()) {
      add_result (translate ("Empty figure source:") + " " + usfm, display_nothing);
    } else {
      const std::string image_contents = database::bible_images::get (src);
      if (image_contents.empty()) {
        add_result (translate ("Could not find Bible image:") + " " + src, display_nothing);
      }
    }
    const size_t pos = usfm.find("“");
    if (pos != std::string::npos) {
      add_result (translate ("Unusual quotation mark found:") + " " + usfm, display_nothing);
    }
  }
}


std::vector <std::pair<int, std::string>> Checks_Usfm::get_results ()
{
  return checking_results;
}


void Checks_Usfm::add_result (std::string text, int modifier)
{
  const std::string current = usfm_item;
  std::string next = filter::usfm::peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
  next = next.substr (0, 20);
  switch (modifier) {
    case display_nothing:
      break;
    case display_current:
      text += ": " + current;
      break;
    case display_next:
      text += ": " + next;
      break;
    case display_full:
      text += ": " + current + next;
      break;
    default:
      break;
  }
  checking_results.push_back (std::pair (verse_number, text));
}


// Checks on markup without intervening text.
void Checks_Usfm::empty_markup ()
{
  // Get the current item (markup or text).
  const std::string current_item = usfm_item;

  // Flags that will describe the current item.
  // bool current_is_text = false;
  // bool current_is_usfm = false;
  // bool current_is_opener = false;
  bool current_is_closer {false};
  // bool current_is_embedded = false;

  // Flags that will describe the previous item.
  // bool previous_is_text = false;
  // bool previous_is_usfm = false;
  bool previous_is_opener {false};
  // bool previous_is_closer = false;
  // bool previous_is_embedded = false;

  // Set the above set of flags.
  if (filter::usfm::is_usfm_marker (current_item)) {
    //current_is_usfm = true;
    if (filter::usfm::is_opening_marker(current_item)) {
      //current_is_opener = true;
    }
    else current_is_closer = true;
    //if (usfm_is_embedded_marker(current_item)) current_is_embedded = true;
  } else {
    //current_is_text = true;
  }
  if (filter::usfm::is_usfm_marker (empty_markup_previous_item)) {
    //previous_is_usfm = true;
    if (filter::usfm::is_opening_marker(empty_markup_previous_item)) previous_is_opener = true;
    //else previous_is_closer = true;
    //if (usfm_is_embedded_marker(empty_markup_previous_item)) previous_is_embedded = true;
  } else {
    //previous_is_text = true;
  }

  // Flag the following situation:
  // An opener is followed by a closer without intervening text.
  if (previous_is_opener && current_is_closer) {
    add_result (translate ("Opening markup is followed by closing markup without intervening text:") + " " + empty_markup_previous_item + current_item, Checks_Usfm::display_nothing);
  }

  // Save the current item (markup or text) into the object for next iteration.
  empty_markup_previous_item = current_item;
}


// Flags a otherwise correctly formed footnote or cross reference that is blank
// Such a note has no content but has (all of) the markers.
void Checks_Usfm::note ()
{
  // Flags that describe the current item.
  bool current_is_text {false};
  //bool current_is_usfm = false;
  bool current_is_opener {false};
  bool current_is_closer {false};
  //bool current_is_embedded = false;
  if (filter::usfm::is_usfm_marker (usfm_item)) {
    //current_is_usfm = true;
    if (filter::usfm::is_opening_marker(usfm_item)) current_is_opener = true;
    else current_is_closer = true;
    //if (usfm_is_embedded_marker(usfm_item)) current_is_embedded = true;
  } else {
    current_is_text = true;
  }

  // If the current item is text, then do no further checks on that.
  if (current_is_text)
    return;
  // From here on it is assumed that the current item is USFM, not text.

  // Get the plain marker, e.g. '\f ' becomes "f".
  const std::string current_marker = filter::usfm::get_marker (usfm_item);
  
  // Get this style's properties.
  const stylesv2::Style* stylev2 = database::styles::get_marker_data(m_stylesheet, current_marker);
  
  // Set a flag if this USFM starts a footnote or an endnote or a crossreference.
  // Clear this flag if it ends the note or xref.
  bool note_border_marker {false};
  if (stylev2) {
    if (stylev2->type == stylesv2::Type::footnote_wrapper)
      note_border_marker = true;
    if (stylev2->type == stylesv2::Type::endnote_wrapper)
      note_border_marker = true;
    if (stylev2->type == stylesv2::Type::crossreference_wrapper)
      note_border_marker = true;
  }
  if (note_border_marker) {
    if (current_is_opener) within_note = true;
    if (current_is_closer) within_note = false;
  }

  // If the current location is not within a footnote / endnote / cross reference,
  // then there is nothing to check for.
  if (!within_note)
    return;

  // Get the next item, that is the item following the current item.
  const std::string next_item = filter::usfm::peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
  
  // Flags that describe the next item.
  bool next_is_text {false};
  //bool next_is_usfm = false;
  //bool next_is_opener = false;
  //bool next_is_closer = false;
  //bool next_is_embedded = false;
  if (filter::usfm::is_usfm_marker (next_item)) {
    //next_is_usfm = true;
    //if (usfm_is_opening_marker(next_item)) next_is_opener = true;
    //else next_is_closer = true;
    //if (usfm_is_embedded_marker(next_item)) next_is_embedded = true;
  } else {
    next_is_text = true;
  }

  // Change, e.g. '\f ' to '\f'.
  // Remove the initial backslash, e.g. '\f' becomes 'f'.
  const std::string next_marker = filter::usfm::get_marker(next_item);

  // If the current item is opening markup ...
  if (!current_is_opener) return;
  // ... and not requiring an endmarker ...
  if (in_array(current_marker, markers_requiring_endmarkers)) return;
  // then the following item should be one of these:
  // - a text item
  if (next_is_text) return;
  // - a marker requiring an endmarker, like e.g. \add
  if (in_array(next_marker, markers_requiring_endmarkers)) return;

  // It has not passed the text for a correctly formatted note.
  // So add a message.
  add_result (translate ("This sequence in the note does not look right:") + " " + usfm_item + next_item, display_nothing);
}


/*

 The official usfm.sty from paratext.org shows where each marker occurs under.
 That could be used to do a mechanical check on the position of the various markers.
 But this is not implemented just now
 because it is not the purpose of Bibledit to exactly check every aspect of USFM.
 Rather Bibledit checks some important issues that may occur in daily USFM editing.
 Checking every aspect of USFM is a bit of an overkill.

*/
