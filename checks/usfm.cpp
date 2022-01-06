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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
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
#include <database/styles.h>


Checks_Usfm::Checks_Usfm (string bible)
{
  Database_Styles database_styles;
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  markers_stylesheet = database_styles.getMarkers (stylesheet);
  for (auto marker : markers_stylesheet) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    style_items [marker] = style;
    int styleType = style.type;
    int styleSubtype = style.subtype;

    // Find out which markers require an endmarker.
    // And which markers are embeddable.
    bool required_endmarker = false;
    bool embeddable_marker = false;
    if (styleType == StyleTypeIdentifier) {
      if (styleSubtype == IdentifierSubtypePublishedVerseMarker) {
        required_endmarker = true;
      }
    }
    if (styleType == StyleTypeFootEndNote) {
      if ((styleSubtype == FootEndNoteSubtypeFootnote) || (styleSubtype == FootEndNoteSubtypeEndnote)) {
        required_endmarker = true;
      }
    }
    if (styleType == StyleTypeCrossreference) {
      if (styleSubtype == CrossreferenceSubtypeCrossreference) {
        required_endmarker = true;
      }
    }
    if (styleType == StyleTypeInlineText) {
      required_endmarker = true;
      embeddable_marker = true;
    }
    if (styleType == StyleTypeWordlistElement) {
      required_endmarker = true;
      embeddable_marker = true;
    }
    if (required_endmarker) {
      markers_requiring_endmarkers.push_back (marker);
    }
    if (embeddable_marker) {
      embeddable_markers.push_back (marker);
    }
    
    // Look for the \toc[1-3] markers.
    if (styleType == StyleTypeIdentifier) {
      if (styleSubtype == IdentifierSubtypeLongTOC) long_toc1_marker = marker;
      if (styleSubtype == IdentifierSubtypeShortTOC) short_toc2_marker = marker;
      if (styleSubtype == IdentifierSubtypeBookAbbrev) abbrev_toc3_marker = marker;
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
    add_result (translate ("Unclosed markers:") + " " + filter_string_implode (open_matching_markers, " "), display_nothing);
  }
}


void Checks_Usfm::check (string usfm)
{
  new_line_in_usfm (usfm);
  
  forward_slash (usfm);

  toc (usfm);
  
  usfm_markers_and_text = usfm_get_markers_and_text (usfm);
  for (usfm_markers_and_text_pointer = 0; usfm_markers_and_text_pointer < usfm_markers_and_text.size(); usfm_markers_and_text_pointer++) {
    usfm_item = usfm_markers_and_text [usfm_markers_and_text_pointer];
    if (usfm_is_usfm_marker (usfm_item)) {
      
      // Get the current verse number.
      if (usfm_item == R"(\v )") {
        string verseCode = usfm_peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
        verse_number = convert_to_int (usfm_peek_verse_number (verseCode));
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
    string code = usfm_peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
    string cleanVerseNumber = usfm_peek_verse_number (code);
    vector <string> v_dirtyVerseNumber = filter_string_explode (code, ' ');
    string dirtyVerseNumber;
    if (!v_dirtyVerseNumber.empty ()) dirtyVerseNumber = v_dirtyVerseNumber [0];
    if (cleanVerseNumber != dirtyVerseNumber) {
      add_result (translate ("Malformed verse number"), display_full);
    }
  }
}


void Checks_Usfm::new_line_in_usfm (string usfm)
{
  size_t position = string::npos;
  size_t pos = usfm.find ("\\\n");
  if (pos != string::npos) {
    position = pos;
  }
  pos = usfm.find ("\\ \n");
  if (pos != string::npos) {
    position = pos;
  }
  if (position != string::npos) {
    if (position == 0) position = 1;
    string bit = usfm.substr (position - 1, 10);
    bit = filter_string_str_replace ("\n", " ", bit);
    add_result (translate ("New line within USFM:") + " " + bit, display_nothing);
  }
}


void Checks_Usfm::marker_in_stylesheet ()
{
  string marker = usfm_item.substr (1);
  marker = filter_string_trim (marker);
  if (!usfm_is_opening_marker (marker)) {
    if (!marker.empty ()) marker = marker.substr (0, marker.length () - 1);
  }
  if (usfm_is_embedded_marker (marker)) {
    if (!marker.empty ()) marker = marker.substr (1);
  }
  if (marker == "") return;
  if (in_array (marker, markers_stylesheet)) return;
  add_result (translate ("Marker not in stylesheet"), Checks_Usfm::display_current);
}


void Checks_Usfm::malformed_id ()
{
  string item = usfm_item.substr (0, 3);
  string ide = usfm_item.substr (0, 4);
  if (ide == "\\ide") return;
  if (item == "\\id") {
    string code = usfm_peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
    string sid =  code.substr (0, 3);
    vector <string> vid = filter_string_explode (code, ' ');
    string id;
    if (!vid.empty ()) id = vid [0];
    int book = Database_Books::getIdFromUsfm (id);
    if (book == 0) {
      add_result (translate ("Unknown ID"), display_full);
    } else {
      if (unicode_string_uppercase (id) != id) {
        add_result (translate ("ID is not in uppercase"), display_full);
      }
    }
  }
}


void Checks_Usfm::forward_slash (string usfm)
{
  string code = filter_string_str_replace ("\n", " ", usfm);
  size_t pos = code.find ("/");
  string bit;
  if (pos != string::npos) {
    size_t pos2 = code.find (" ", pos);
    if (pos2 != string::npos) {
      bit = code.substr (pos, pos2 - pos);
    } else {
      bit = code.substr (pos, 100);
    }
    pos2 = bit.find ("*");
    if (pos2 != string::npos) {
      bit = bit.substr (0, pos2);
    }
    string marker = bit.substr (1, 100);
    if (find (markers_stylesheet.begin(), markers_stylesheet.end(), marker) != markers_stylesheet.end ()) {
      add_result (translate ("Forward slash instead of backslash:") + " " + bit, display_nothing);
    }
  }
}


void Checks_Usfm::widow_back_slash ()
{
  string marker = usfm_item;
  marker = filter_string_trim (marker);
  if (marker.length() == 1) {
    add_result (translate ("Widow backslash"), display_current);
  }
}


void Checks_Usfm::matching_endmarker ()
{
  string marker = usfm_item;
  // Remove the initial backslash, e.g. '\add' becomes 'add'.
  marker = marker.substr (1);
  marker = filter_string_trim (marker);
  bool isOpener = usfm_is_opening_marker (marker);
  if (!isOpener) {
   if (!marker.empty ()) marker = marker.substr (0, marker.length () - 1);
  }
  if (!in_array (marker, markers_requiring_endmarkers)) return;
  if (isOpener) {
    if (in_array (marker, open_matching_markers)) {
      add_result (translate ("Repeating opening marker"), Checks_Usfm::display_current);
    } else {
      open_matching_markers.push_back (marker);
    }
  } else {
    if (in_array (marker, open_matching_markers)) {
      open_matching_markers = filter_string_array_diff (open_matching_markers, {marker});
    } else {
      add_result (translate ("Closing marker does not match opening marker") + " " + filter_string_implode (open_matching_markers, " "), display_current);
    }
  }
}


void Checks_Usfm::embedded_marker ()
{
  // The marker, e.g. '\add'.
  string marker = usfm_item;

  // Remove the initial backslash, e.g. '\add' becomes 'add'.
  marker = marker.substr (1);
  marker = filter_string_trim (marker);

  bool isOpener = usfm_is_opening_marker (marker);

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
      open_embeddable_markers = filter_string_array_diff (open_embeddable_markers, {marker});
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


void Checks_Usfm::toc (string usfm)
{
  // Only check the 66 canonical books.
  // Skip any of the other books.
  string type = Database_Books::getType (book_number);
  if ((type == "ot") || (type == "nt")) {

    // Check on the presence of the table of contents markers in this chapter.
    bool toc1_present = usfm.find (usfm_get_opening_usfm (long_toc1_marker)) != string::npos;
    bool toc2_present = usfm.find (usfm_get_opening_usfm (short_toc2_marker)) != string::npos;
    bool toc3_present = usfm.find (usfm_get_opening_usfm (abbrev_toc3_marker)) != string::npos;

    // The markers should be on chapter 0 only.
    if (chapter_number == 0) {
      // Required: \toc1
      if (!toc1_present) {
        add_result (translate ("The book lacks the marker for the verbose book name:") + " " + usfm_get_opening_usfm (long_toc1_marker), display_nothing);
      }
      // Required: \toc2
      if (!toc2_present) {
        add_result (translate ("The book lacks the marker for the short book name:") + " " + usfm_get_opening_usfm (short_toc2_marker), display_nothing);
      }
    } else {
      string msg = translate ("The following marker belongs in chapter 0:") + " ";
      // Required markers.
      if (toc1_present) {
        add_result (msg + usfm_get_opening_usfm (long_toc1_marker), display_nothing);
      }
      if (toc2_present) {
        add_result (msg + usfm_get_opening_usfm (short_toc2_marker), display_nothing);
      }
      // Optional markers, but should not be anywhere else except in chapter 0.
      if (toc3_present) {
        add_result (msg + usfm_get_opening_usfm (abbrev_toc3_marker), display_nothing);
      }
    }
  }
}


void Checks_Usfm::figure ()
{
  if (usfm_item == R"(\fig )") {
    string usfm = usfm_peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
    string caption, alt, src, size, loc, copy, ref;
    usfm_extract_fig (usfm, caption, alt, src, size, loc, copy, ref);
    if (src.empty()) {
      add_result (translate ("Empty figure source:") + " " + usfm, display_nothing);
    } else {
      Database_BibleImages database_bibleimages;
      string image_contents = database_bibleimages.get (src);
      if (image_contents.empty()) {
        add_result (translate ("Could not find Bible image:") + " " + src, display_nothing);
      }
    }
    size_t pos = usfm.find("“");
    if (pos != string::npos) {
      add_result (translate ("Unusual quotation mark found:") + " " + usfm, display_nothing);
    }
  }
}


vector <pair<int, string>> Checks_Usfm::get_results ()
{
  return checking_results;
}


void Checks_Usfm::add_result (string text, int modifier)
{
  string current = usfm_item;
  string next = usfm_peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
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
    case Checks_Usfm::display_full:
      text += ": " + current + next;
      break;
  }
  checking_results.push_back (pair (verse_number, text));
}


// Checks on markup without intervening text.
void Checks_Usfm::empty_markup ()
{
  // Get the current item (markup or text).
  string current_item = usfm_item;

  // Flags that will describe the current item.
  // bool current_is_text = false;
  // bool current_is_usfm = false;
  // bool current_is_opener = false;
  bool current_is_closer = false;
  // bool current_is_embedded = false;

  // Flags that will describe the previous item.
  // bool previous_is_text = false;
  // bool previous_is_usfm = false;
  bool previous_is_opener = false;
  // bool previous_is_closer = false;
  // bool previous_is_embedded = false;

  // Set the above set of flags.
  if (usfm_is_usfm_marker (current_item)) {
    //current_is_usfm = true;
    if (usfm_is_opening_marker(current_item)) {
      //current_is_opener = true;
    }
    else current_is_closer = true;
    //if (usfm_is_embedded_marker(current_item)) current_is_embedded = true;
  } else {
    //current_is_text = true;
  }
  if (usfm_is_usfm_marker (empty_markup_previous_item)) {
    //previous_is_usfm = true;
    if (usfm_is_opening_marker(empty_markup_previous_item)) previous_is_opener = true;
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
  bool current_is_text = false;
  //bool current_is_usfm = false;
  bool current_is_opener = false;
  bool current_is_closer = false;
  //bool current_is_embedded = false;
  if (usfm_is_usfm_marker (usfm_item)) {
    //current_is_usfm = true;
    if (usfm_is_opening_marker(usfm_item)) current_is_opener = true;
    else current_is_closer = true;
    //if (usfm_is_embedded_marker(usfm_item)) current_is_embedded = true;
  } else {
    current_is_text = true;
  }

  // If the current item is text, then do no further checks on that.
  if (current_is_text) return;
  // From here on it is assumed that the current item is USFM, not text.

  // Get the plain marker, e.g. '\f ' becomes "f".
  string current_marker = usfm_get_marker (usfm_item);
  
  // Get this style's properties.
  Database_Styles_Item style = style_items [current_marker];
  
  // Set a flag if this USFM starts a footnote or an endnote or a crossreference.
  // Clear this flag if it ends the note or xref.
  bool note_border_marker = false;
  if (style.type == StyleTypeFootEndNote) {
    if (style.subtype == FootEndNoteSubtypeFootnote) note_border_marker = true;
    if (style.subtype == FootEndNoteSubtypeEndnote) note_border_marker = true;
  }
  if (style.type == StyleTypeCrossreference) {
    if (style.subtype == CrossreferenceSubtypeCrossreference) note_border_marker = true;
  }
  if (note_border_marker) {
    if (current_is_opener) within_note = true;
    if (current_is_closer) within_note = false;
  }

  // If the current location is not within a footnote / endnote / cross reference,
  // then there is nothing to check for.
  if (!within_note) return;

  // Get the next item, that is the item following the current item.
  string next_item = usfm_peek_text_following_marker (usfm_markers_and_text,
                                                      usfm_markers_and_text_pointer);
  
  // Flags that describe the next item.
  bool next_is_text = false;
  //bool next_is_usfm = false;
  //bool next_is_opener = false;
  //bool next_is_closer = false;
  //bool next_is_embedded = false;
  if (usfm_is_usfm_marker (next_item)) {
    //next_is_usfm = true;
    //if (usfm_is_opening_marker(next_item)) next_is_opener = true;
    //else next_is_closer = true;
    //if (usfm_is_embedded_marker(next_item)) next_is_embedded = true;
  } else {
    next_is_text = true;
  }

  // Change, e.g. '\f ' to '\f'.
  // Remove the initial backslash, e.g. '\f' becomes 'f'.
  string next_marker = usfm_get_marker(next_item);

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
