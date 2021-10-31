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


Checks_Usfm::Checks_Usfm (string bible)
{
  Database_Styles database_styles;
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  markers_stylesheet = database_styles.getMarkers (stylesheet);
  for (auto marker : markers_stylesheet) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    int styleType = style.type;
    int styleSubtype = style.subtype;

    // Find out which markers require an endmarker.
    // And which markers are embeddable.
    bool requiredEndmarker = false;
    bool embeddableMarker = false;
    if (styleType == StyleTypeIdentifier) {
      if (styleSubtype == IdentifierSubtypePublishedVerseMarker) {
        requiredEndmarker = true;
      }
    }
    if (styleType == StyleTypeFootEndNote) {
      if ((styleSubtype == FootEndNoteSubtypeFootnote) || (styleSubtype == FootEndNoteSubtypeEndnote)) {
        requiredEndmarker = true;
      }
    }
    if (styleType == StyleTypeCrossreference) {
      if (styleSubtype == CrossreferenceSubtypeCrossreference) {
        requiredEndmarker = true;
      }
    }
    if (styleType == StyleTypeInlineText) {
      requiredEndmarker = true;
      embeddableMarker = true;
    }
    if (styleType == StyleTypeWordlistElement) {
      requiredEndmarker = true;
      embeddableMarker = true;
    }
    if (requiredEndmarker) {
      markers_requiring_endmarkers.push_back (marker);
    }
    if (embeddableMarker) {
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
}


void Checks_Usfm::finalize ()
{
  // Check on unclosed markers.
  if (open_matching_markers.size () > 0) {
    addResult (translate ("Unclosed markers:") + " " + filter_string_implode (open_matching_markers, " "), display_nothing);
  }
}


void Checks_Usfm::check (string usfm)
{
  newLineInUsfm (usfm);
  
  forwardSlash (usfm);

  toc (usfm);
  
  usfm_markers_and_text = usfm_get_markers_and_text (usfm);
  for (usfm_markers_and_text_pointer = 0; usfm_markers_and_text_pointer < usfm_markers_and_text.size(); usfm_markers_and_text_pointer++) {
    usfm_item = usfm_markers_and_text [usfm_markers_and_text_pointer];
    if (usfm_is_usfm_marker (usfm_item)) {
      
      // Get the current verse number.
      if (usfm_item == "\\v ") {
        string verseCode = usfm_peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
        verse_number = convert_to_int (usfm_peek_verse_number (verseCode));
      }
      
      malformedVerseNumber ();
      
      markerInStylesheet ();
      
      malformedId ();
      
      widowBackSlash ();
      
      matchingEndmarker ();
      
      embeddedMarker ();
      
      figure ();

    }
  }
}


void Checks_Usfm::malformedVerseNumber ()
{
  if (usfm_item == "\\v ") {
    string code = usfm_peek_text_following_marker (usfm_markers_and_text, usfm_markers_and_text_pointer);
    string cleanVerseNumber = usfm_peek_verse_number (code);
    vector <string> v_dirtyVerseNumber = filter_string_explode (code, ' ');
    string dirtyVerseNumber;
    if (!v_dirtyVerseNumber.empty ()) dirtyVerseNumber = v_dirtyVerseNumber [0];
    if (cleanVerseNumber != dirtyVerseNumber) {
      addResult (translate ("Malformed verse number"), display_full);
    }
  }
}


void Checks_Usfm::newLineInUsfm (string usfm)
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
    addResult (translate ("New line within USFM:") + " " + bit, display_nothing);
  }
}


void Checks_Usfm::markerInStylesheet ()
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
  addResult (translate ("Marker not in stylesheet"), Checks_Usfm::display_current);
}


void Checks_Usfm::malformedId ()
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
      addResult (translate ("Unknown ID"), display_full);
    } else {
      if (unicode_string_uppercase (id) != id) {
        addResult (translate ("ID is not in uppercase"), display_full);
      }
    }
  }
}


void Checks_Usfm::forwardSlash (string usfm)
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
      addResult (translate ("Forward slash instead of backslash:") + " " + bit, display_nothing);
    }
  }
}


void Checks_Usfm::widowBackSlash ()
{
  string marker = usfm_item;
  marker = filter_string_trim (marker);
  if (marker.length() == 1) {
    addResult (translate ("Widow backslash"), display_current);
  }
}


void Checks_Usfm::matchingEndmarker ()
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
      addResult (translate ("Repeating opening marker"), Checks_Usfm::display_current);
    } else {
      open_matching_markers.push_back (marker);
    }
  } else {
    if (in_array (marker, open_matching_markers)) {
      open_matching_markers = filter_string_array_diff (open_matching_markers, {marker});
    } else {
      addResult (translate ("Closing marker does not match opening marker") + " " + filter_string_implode (open_matching_markers, " "), display_current);
    }
  }
}


void Checks_Usfm::embeddedMarker ()
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
      addResult (translate ("Embedded marker requires a plus sign"), display_full);
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
        addResult (translate ("The book lacks the marker for the verbose book name:") + " " + usfm_get_opening_usfm (long_toc1_marker), display_nothing);
      }
      // Required: \toc2
      if (!toc2_present) {
        addResult (translate ("The book lacks the marker for the short book name:") + " " + usfm_get_opening_usfm (short_toc2_marker), display_nothing);
      }
    } else {
      string msg = translate ("The following marker belongs in chapter 0:") + " ";
      // Required markers.
      if (toc1_present) {
        addResult (msg + usfm_get_opening_usfm (long_toc1_marker), display_nothing);
      }
      if (toc2_present) {
        addResult (msg + usfm_get_opening_usfm (short_toc2_marker), display_nothing);
      }
      // Optional markers, but should not be anywhere else except in chapter 0.
      if (toc3_present) {
        addResult (msg + usfm_get_opening_usfm (abbrev_toc3_marker), display_nothing);
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
      addResult (translate ("Empty figure source:") + " " + usfm, display_nothing);
    } else {
      Database_BibleImages database_bibleimages;
      string image_contents = database_bibleimages.get (src);
      if (image_contents.empty()) {
        addResult (translate ("Could not find Bible image:") + " " + src, display_nothing);
      }
    }
    size_t pos = usfm.find("“");
    if (pos != string::npos) {
      addResult (translate ("Unusual quotation mark found:") + " " + usfm, display_nothing);
    }
  }
}


vector <pair<int, string>> Checks_Usfm::get_results ()
{
  return checking_results;
}


void Checks_Usfm::addResult (string text, int modifier)
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
  checking_results.push_back (make_pair (verse_number, text));
}


/*

 The official usfm.sty from paratext.org shows where each marker occurs under.
 That could be used to do a mechanical check on the position of the various markers.
 But this is not implemented just now
 because it is not the purpose of Bibledit to exactly check every aspect of USFM.
 Rather Bibledit checks some important issues that may occur in daily USFM editing.
 Checking every aspect of USFM is a bit of an overkill.

*/
