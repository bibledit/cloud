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


#include <checks/usfm.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <database/styles.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <styles/logic.h>
#include <locale/translate.h>


Checks_Usfm::Checks_Usfm (string bible)
{
  Database_Styles database_styles;
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  markersStylesheet = database_styles.getMarkers (stylesheet);
  for (auto marker : markersStylesheet) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    int styleType = style.type;
    int styleSubtype = style.subtype;

    // Find out which markers require an endmarker.
    // And which ones are embeddable.
    bool requiredEndmarker = false;
    bool embeddableMarker = false;
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
      markersRequiringEndmarkers.push_back (marker);
    }
    if (embeddableMarker) {
      embeddableMarkers.push_back (marker);
    }
    
    // Look for the \toc[1-3] markers.
    if (styleType == StyleTypeIdentifier) {
      if (styleSubtype == IdentifierSubtypeLongTOC) longToc1Marker = marker;
      if (styleSubtype == IdentifierSubtypeShortTOC) shortToc2Marker = marker;
      if (styleSubtype == IdentifierSubtypeBookAbbrev) abbrevToc3Marker = marker;
    }
  }
}


void Checks_Usfm::initialize (int book, int chapter)
{
  checkingResults.clear ();
  usfmMarkersAndText.clear ();
  usfmMarkersAndTextPointer = 0;
  bookNumber = book;
  chapterNumber = chapter;
  verseNumber = 0;
  openMatchingMarkers.clear ();
}


void Checks_Usfm::finalize ()
{
  // Check on unclosed markers.
  if (openMatchingMarkers.size () > 0) {
    addResult (translate ("Unclosed markers:") + " " + filter_string_implode (openMatchingMarkers, " "), displayNothing);
  }
}


void Checks_Usfm::check (string usfm)
{
  newLineInUsfm (usfm);
  
  forwardSlash (usfm);

  toc (usfm);

  usfmMarkersAndText = usfm_get_markers_and_text (usfm);
  for (usfmMarkersAndTextPointer = 0; usfmMarkersAndTextPointer < usfmMarkersAndText.size(); usfmMarkersAndTextPointer++) {
    usfmItem = usfmMarkersAndText [usfmMarkersAndTextPointer];
    if (usfm_is_usfm_marker (usfmItem)) {
      
      // Get the current verse number.
      if (usfmItem == "\\v ") {
        string verseCode = usfm_peek_text_following_marker (usfmMarkersAndText, usfmMarkersAndTextPointer);
        verseNumber = convert_to_int (usfm_peek_verse_number (verseCode));
      }
      
      malformedVerseNumber ();
      
      markerInStylesheet ();
      
      malformedId ();
      
      widowBackSlash ();
      
      matchingEndmarker ();
      
      embeddedMarker ();
    }
  }
}


void Checks_Usfm::malformedVerseNumber ()
{
  if (usfmItem == "\\v ") {
    string code = usfm_peek_text_following_marker (usfmMarkersAndText, usfmMarkersAndTextPointer);
    string cleanVerseNumber = usfm_peek_verse_number (code);
    vector <string> v_dirtyVerseNumber = filter_string_explode (code, ' ');
    string dirtyVerseNumber;
    if (!v_dirtyVerseNumber.empty ()) dirtyVerseNumber = v_dirtyVerseNumber [0];
    if (cleanVerseNumber != dirtyVerseNumber) {
      addResult (translate ("Malformed verse number"), displayFull);
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
    addResult (translate ("New line within USFM:") + " " + bit, displayNothing);
  }
}


void Checks_Usfm::markerInStylesheet ()
{
  string marker = usfmItem.substr (1);
  marker = filter_string_trim (marker);
  if (!usfm_is_opening_marker (marker)) {
    if (!marker.empty ()) marker = marker.substr (0, marker.length () - 1);
  }
  if (usfm_is_embedded_marker (marker)) {
    if (!marker.empty ()) marker = marker.substr (1);
  }
  if (marker == "") return;
  if (in_array (marker, markersStylesheet)) return;
  addResult (translate ("Marker not in stylesheet"), Checks_Usfm::displayCurrent);
}


void Checks_Usfm::malformedId ()
{
  string item = usfmItem.substr (0, 3);
  string ide = usfmItem.substr (0, 4);
  if (ide == "\\ide") return;
  if (item == "\\id") {
    string code = usfm_peek_text_following_marker (usfmMarkersAndText, usfmMarkersAndTextPointer);
    string sid =  code.substr (0, 3);
    vector <string> vid = filter_string_explode (code, ' ');
    string id;
    if (!vid.empty ()) id = vid [0];
    int book = Database_Books::getIdFromUsfm (id);
    if (book == 0) {
      addResult (translate ("Unknown ID"), displayFull);
    } else {
      if (unicode_string_uppercase (id) != id) {
        addResult (translate ("ID is not in uppercase"), displayFull);
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
    if (find (markersStylesheet.begin(), markersStylesheet.end(), marker) != markersStylesheet.end ()) {
      addResult (translate ("Forward slash instead of backslash:") + " " + bit, displayNothing);
    }
  }
}


void Checks_Usfm::widowBackSlash ()
{
  string marker = usfmItem;
  marker = filter_string_trim (marker);
  if (marker.length() == 1) {
    addResult (translate ("Widow backslash"), displayCurrent);
  }
}


void Checks_Usfm::matchingEndmarker ()
{
  string marker = usfmItem;
  // Remove the initial backslash, e.g. '\add' becomes 'add'.
  marker = marker.substr (1);
  marker = filter_string_trim (marker);
  bool isOpener = usfm_is_opening_marker (marker);
  if (!isOpener) {
   if (!marker.empty ()) marker = marker.substr (0, marker.length () - 1);
  }
  if (!in_array (marker, markersRequiringEndmarkers)) return;
  if (isOpener) {
    if (in_array (marker, openMatchingMarkers)) {
      addResult (translate ("Repeating opening marker"), Checks_Usfm::displayCurrent);
    } else {
      openMatchingMarkers.push_back (marker);
    }
  } else {
    if (in_array (marker, openMatchingMarkers)) {
      openMatchingMarkers = filter_string_array_diff (openMatchingMarkers, {marker});
    } else {
      addResult (translate ("Closing marker does not match opening marker") + " " + filter_string_implode (openMatchingMarkers, " "), displayCurrent);
    }
  }
}


void Checks_Usfm::embeddedMarker ()
{
  // The marker, e.g. '\add'.
  string marker = usfmItem;

  // Remove the initial backslash, e.g. '\add' becomes 'add'.
  marker = marker.substr (1);
  marker = filter_string_trim (marker);

  bool isOpener = usfm_is_opening_marker (marker);

  // Clean a closing marker, e.g. '\add*' becomes '\add'.
  if (!isOpener) {
    if (!marker.empty ()) marker = marker.substr (0, marker.length () - 1);
  }
  
  // If the marker is not relevant for this check, bail out.
  if (!in_array (marker, embeddableMarkers)) return;
  
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
    if (!in_array (marker, openEmbeddableMarkers)) {
      if (!openEmbeddableMarkers.empty ()) {
        checkEmbedding = true;
      }
      openEmbeddableMarkers.push_back (marker);
    }
  } else {
    if (in_array (marker, openEmbeddableMarkers)) {
      openEmbeddableMarkers = filter_string_array_diff (openEmbeddableMarkers, {marker});
      if (!openEmbeddableMarkers.empty ()) {
        checkEmbedding = true;
      }
    }
  }
  
  if (checkEmbedding) {
    if (marker.substr (0, 1) != "+") {
      addResult (translate ("Embedded marker requires a plus sign"), displayFull);
    }
  }
}


void Checks_Usfm::toc (string usfm)
{
  // Only check the 66 canonical books.
  // Skip any of the other books.
  string type = Database_Books::getType (bookNumber);
  if ((type == "ot") || (type == "nt")) {

    // Check on the presence of the table of contents markers in this chapter.
    bool toc1_present = usfm.find (usfm_get_opening_usfm (longToc1Marker)) != string::npos;
    bool toc2_present = usfm.find (usfm_get_opening_usfm (shortToc2Marker)) != string::npos;
    bool toc3_present = usfm.find (usfm_get_opening_usfm (abbrevToc3Marker)) != string::npos;

    // The markers should be on chapter 0 only.
    if (chapterNumber == 0) {
      // Required: \toc1
      if (!toc1_present) {
        addResult (translate ("The book lacks the marker for the verbose book name:") + " " + usfm_get_opening_usfm (longToc1Marker), displayNothing);
      }
      // Required: \toc2
      if (!toc2_present) {
        addResult (translate ("The book lacks the marker for the short book name:") + " " + usfm_get_opening_usfm (shortToc2Marker), displayNothing);
      }
    } else {
      string msg = translate ("The following marker belongs in chapter 0:") + " ";
      // Required markers.
      if (toc1_present) {
        addResult (msg + usfm_get_opening_usfm (longToc1Marker), displayNothing);
      }
      if (toc2_present) {
        addResult (msg + usfm_get_opening_usfm (shortToc2Marker), displayNothing);
      }
      // Optional markers, but should not be anywhere else except in chapter 0.
      if (toc3_present) {
        addResult (msg + usfm_get_opening_usfm (abbrevToc3Marker), displayNothing);
      }
    }
  }
}


vector <pair<int, string>> Checks_Usfm::getResults ()
{
  return checkingResults;
}


void Checks_Usfm::addResult (string text, int modifier)
{
  string current = usfmItem;
  string next = usfm_peek_text_following_marker (usfmMarkersAndText, usfmMarkersAndTextPointer);
  next = next.substr (0, 20);
  switch (modifier) {
    case displayNothing:
      break;
    case displayCurrent:
      text += ": " + current;
      break;
    case displayNext:
      text += ": " + next;
      break;
    case Checks_Usfm::displayFull:
      text += ": " + current + next;
      break;
  }
  checkingResults.push_back (make_pair (verseNumber, text));
}


/*

 The official usfm.sty from paratext.org shows where each marker occurs under.
 That could be used to do a mechanical check on the position of the various markers.
 But this is not implemented just now
 because it is not the purpose of Bibledit to exactly check every aspect of USFM.
 Rather Bibledit checks some important issues that may occur in daily USFM editing.
 Checking every aspect of USFM is a bit of an overkill.

*/
