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


#include <filter/usfm.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/diff.h>
#include <database/books.h>
#include <database/styles.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <styles/logic.h>
#include <webserver/request.h>
#include <bb/logic.h>
#include <locale/translate.h>
#include <developer/logic.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


namespace filter::usfm {


BookChapterData::BookChapterData (int book, int chapter, std::string data)
{
  m_book = book;
  m_chapter = chapter;
  m_data = data;
}


// Returns the string $usfm as one long string.
// $usfm may contain new lines, but the resulting long string won't.
std::string one_string (std::string usfm)
{
  std::string long_string = "";
  std::vector <std::string> usfm_lines = filter::strings::explode (usfm, '\n');
  for (std::string & line : usfm_lines) {
    line = filter::strings::trim (line);
    // Skip empty line.
    if (line != "") {
      // The line will be appended to the output line.
      // If it does not start with a backslash (\), a space is inserted first.
      size_t pos = line.find ("\\");
      if (pos != 0) {
        if (long_string != "") long_string += " ";
      }
      long_string += line;
    }
  }
  return long_string;
}


// Returns the string $code as an array alternating between marker and text.
// Example, input is:   \id GEN
//                      \c 10
//             ...
// Output would be:     array ("\id ", "GEN", "\c ", "10", ...)
// If $code does not start with a marker, this becomes visible in the output too.
std::vector <std::string> get_markers_and_text (std::string code)
{
  std::vector <std::string> markers_and_text;
  code = filter::strings::replace ("\n\\", "\\", code); // New line followed by backslash: leave new line out.
  code = filter::strings::replace ("\n", " ", code); // New line only: change to space, according to the USFM specification.
  // No removal of double spaces, because it would remove an opening marker (which already has its own space), followed by a space.
  code = filter::strings::trim (code);
  while (!code.empty ()) {
    size_t pos = code.find ("\\");
    if (pos == 0) {
      // Marker found.
      // The marker ends
      // - after the first space, or
      // - after the first asterisk (*), or
      // - at the first backslash (\), or
      // - at the end of the string,
      // whichever comes first.
      std::vector <size_t> positions;
      pos = code.find (" ");
      if (pos != std::string::npos) positions.push_back (pos + 1);
      pos = code.find ("*");
      if (pos != std::string::npos) positions.push_back (pos + 1);
      pos = code.find ("\\", 1);
      if (pos != std::string::npos) positions.push_back (pos);
      positions.push_back (code.length());
      sort (positions.begin (), positions.end());
      pos = positions[0];
      std::string marker = code.substr (0, pos);
      markers_and_text.push_back (marker);
      code = code.substr (pos);
    } else {
      // Text found. It ends at the next backslash or at the end of the string.
      pos = code.find ("\\");
      if (pos == std::string::npos) pos = code.length();
      std::string text = code.substr (0, pos);
      markers_and_text.push_back (text);
      code = code.substr (pos);
    }
  }
  return markers_and_text;
}


// Gets the marker from $usfm if it is there, else returns an empty string.
// Examples:
// "\id"    -> "id"
// "\id "   -> "id"
// "\add*"  -> "add"
// "\+add*" -> "add"
std::string get_marker (std::string usfm)
{
  if (usfm.empty ()) return usfm;
  size_t pos = usfm.find ("\\");
  if (pos == 0) {
    // Marker found.
    // Erase backslash.
    usfm = usfm.substr (1);
    // Optionally erase the + embedded marker.
    pos = usfm.find ("+");
    if (pos == 0) usfm = usfm.substr (1);
    // The marker ends
    // - at the first space, or
    // - at the first asterisk (*), or
    // - at the first backslash (\), or
    // - at the end of the string,
    // whichever comes first.
    std::vector <size_t> positions;
    pos = usfm.find (" ");
    if (pos != std::string::npos) positions.push_back (pos);
    pos = usfm.find ("*");
    if (pos != std::string::npos) positions.push_back (pos);
    pos = usfm.find ("\\");
    if (pos != std::string::npos) positions.push_back (pos);
    positions.push_back (usfm.length());
    sort (positions.begin(), positions.end());
    pos = positions[0];
    std::string marker = usfm.substr (0, pos);
    return marker;
  }
  // Text found. No marker.
  return std::string();
}


// This imports USFM $input.
// It takes raw $input,
// and returns a vector with objects with book_number, chapter_number, chapter_data.
std::vector <BookChapterData> usfm_import (std::string input, std::string stylesheet)
{
  std::vector <BookChapterData> result;

  book_id bookid {0};
  int chapter_number {0};
  std::string chapter_data {};

  input = one_string (input);
  std::vector <std::string> markers_and_text = get_markers_and_text (input);
  bool retrieve_book_number_on_next_iteration = false;
  bool retrieve_chapter_number_on_next_iteration = false;

  for (std::string marker_or_text : markers_and_text) {
    if (retrieve_book_number_on_next_iteration) {
      bookid = database::books::get_id_from_usfm (marker_or_text.substr (0, 3));
      chapter_number = 0;
      retrieve_book_number_on_next_iteration = false;
    }
    if (retrieve_chapter_number_on_next_iteration) {
      retrieve_chapter_number_on_next_iteration = false;
      chapter_number = filter::strings::convert_to_int (marker_or_text);
    }
    std::string marker = get_marker (marker_or_text);
    if (!marker.empty()) {
      // USFM marker found.
      bool opener = is_opening_marker (marker_or_text);
      bool store_chapter_data = false;
      if (marker == "id") {
        retrieve_book_number_on_next_iteration = true;
        store_chapter_data = true;
      }
      if (marker == "c") {
        retrieve_chapter_number_on_next_iteration = true;
        store_chapter_data = true;
      }
      if (store_chapter_data) {
        chapter_data = filter::strings::trim (chapter_data);
        if (!chapter_data.empty()) result.push_back ( { static_cast<int>(bookid), chapter_number, chapter_data } );
        chapter_number = 0;
        chapter_data = "";
        store_chapter_data = false;
      }
      Database_Styles database_styles;
      Database_Styles_Item marker_data = database_styles.getMarkerData (stylesheet, marker);
      int type = marker_data.type;
      int subtype = marker_data.subtype;
      // Only opening markers can start on a new line.
      // Closing markers never do.
      if (opener) {
        if (styles_logic_starts_new_line_in_usfm (type, subtype)) {
          chapter_data.append ("\n");
        }
      }
    }
    chapter_data += marker_or_text;
  }
  chapter_data = filter::strings::trim (chapter_data);
  if (!chapter_data.empty()) result.push_back (BookChapterData (static_cast<int>(bookid), chapter_number, chapter_data));
  return result;
}


// Returns an array with the verse numbers found in $usfm.
// It handles a single verse, a range of verses, or a sequence of verses.
// It locates separate whole verse numbers.
// Examples:
// 10
// 10-12b
// 10,11a
// 10,12
std::vector <int> get_verse_numbers (std::string usfm)
{
  std::vector <int> verse_numbers = { 0 };
  std::vector <std::string> markers_and_text = get_markers_and_text (usfm);
  bool extract_verse = false;
  for (std::string marker_or_text : markers_and_text) {
    if (extract_verse) {
      std::string verse = peek_verse_number (marker_or_text);
      // Range of verses.
      if (handle_verse_range (verse, verse_numbers));
      // Sequence of verses.
      else if (handle_verse_sequence (verse, verse_numbers));
      // Single verse.
      else verse_numbers.push_back (filter::strings::convert_to_int (verse));
      extract_verse = false;
    }
    if (marker_or_text.substr (0, 2) == marker_v ()) extract_verse = true;
    std::string va_or_vp = marker_or_text.substr (0, 3);
    if (va_or_vp == marker_va ()) extract_verse = false;
    if (va_or_vp == marker_vp ()) extract_verse = false;
  }
  return verse_numbers;
}


// Returns the chapter numbers found in $usfm.
std::vector <int> get_chapter_numbers (std::string usfm)
{
  std::vector <int> chapter_numbers = { 0 };
  std::vector <std::string> markers_and_text = get_markers_and_text (usfm);
  bool extract_chapter = false;
  for (std::string marker_or_text : markers_and_text) {
    if (extract_chapter) {
      std::string chapter = peek_verse_number (marker_or_text);
      chapter_numbers.push_back (filter::strings::convert_to_int (chapter));
      extract_chapter = false;
    }
    if (marker_or_text.substr (0, 2) == "\\c") {
      extract_chapter = true;
    }
  }
  return chapter_numbers;
}


// Returns the verse numbers in the string of $usfm code at line number $line_number.
std::vector <int> linenumber_to_versenumber (std::string usfm, unsigned int line_number)
{
  std::vector <int> verse_number = {0}; // Initial verse number.
  std::vector <std::string> lines = filter::strings::explode (usfm, '\n');
  for (unsigned int i = 0; i < lines.size(); i++) {
    if (i <= line_number) {
      std::vector <int> verse_numbers = get_verse_numbers (lines[i]);
      if (verse_numbers.size() >= 2) {
        verse_number = filter::strings::array_diff (verse_numbers, {0});
      }
    }
  }
  return verse_number;
}


// Returns the verse numbers in the string of $usfm code at offset $offset.
// Offset is calculated with filter::strings::unicode_string_length to support UTF-8.
std::vector <int> offset_to_versenumber (std::string usfm, unsigned int offset)
{
  size_t totalOffset = 0;
  std::vector <std::string> lines = filter::strings::explode (usfm, '\n');
  for (unsigned i = 0; i < lines.size(); i++) {
    size_t length = filter::strings::unicode_string_length (lines [i]);
    totalOffset += length;
    if (totalOffset >= offset) {
      return linenumber_to_versenumber (usfm, i);
    }
    // Add 1 for new line.
    totalOffset += 1;
  }
  return {0};
}



// Returns the offset within the $usfm code where $verse number starts.
int versenumber_to_offset (std::string usfm, int verse)
{
  // Verse number 0 starts at offset 0.
  if (verse == 0) return 0;
  int totalOffset = 0;
  std::vector <std::string> lines = filter::strings::explode (usfm, '\n');
  for (std::string line : lines) {
    std::vector <int> verses = get_verse_numbers (line);
    for (auto & v : verses) {
      if (v == verse) return totalOffset;
    }
    totalOffset += static_cast<int>( filter::strings::unicode_string_length (line));
    // Add 1 for new line.
    totalOffset += 1;
  }
  return static_cast<int>( filter::strings::unicode_string_length (usfm));
}


// Returns the verse text given a $verse_number and $usfm code.
// Handles combined verses.
std::string get_verse_text (std::string usfm, int verse_number)
{
  std::vector <std::string> result;
  bool hit = (verse_number == 0);

  std::vector <std::string> lines = filter::strings::explode (usfm, '\n');
  for (std::string line : lines) {
    std::vector <int> verses = get_verse_numbers (line);
    if (verse_number == 0) {
      if (verses.size () != 1) hit = false;
      if (hit) result.push_back (line);
    } else {
      if (in_array (verse_number, verses)) {
        // Desired verse found.
        hit = true;
      } else if (verses.size () == 1) {
        // No verse found: No change in situation.
      } else {
        // Outside desired verse.
        hit = false;
      }
      if (hit) result.push_back (line);
    }
  }
  
  // Return the verse text.
  std::string verseText = filter::strings::implode (result, "\n");
  return verseText;
}


// Gets the USFM for the $verse number for a Quill-based verse editor.
// This means that preceding empty paragraphs will be included also.
// And that empty paragraphs at the end will be omitted.
std::string get_verse_text_quill (std::string usfm, int verse)
{
  // Get the raw USFM for the verse, that is, the bit between the \v... markers.
  std::string raw_verse_usfm = get_verse_text (usfm, verse);
  
  // Bail out if empty.
  if (raw_verse_usfm.empty ()) {
    return raw_verse_usfm;
  }

  // Omit new paragraphs at the end.
  // In this context it is taken as opening USFM markers without content.
  std::string verse_usfm (raw_verse_usfm);
  std::vector <std::string> markers_and_text = get_markers_and_text (verse_usfm);
  while (true) {
    if (markers_and_text.empty ()) break;
    std::string code = markers_and_text.back ();
    markers_and_text.pop_back ();
    if (!is_usfm_marker (code)) break;
    if (!is_opening_marker (code)) break;
    verse_usfm.erase (verse_usfm.size () - code.size ());
    verse_usfm = filter::strings::trim (verse_usfm);
    if (verse_usfm.empty ()) break;
  }

  // Bail out if empty USFM for the verse.
  if (verse_usfm.empty ()) {
    return verse_usfm;
  }
  
  // Get the raw USFM for the previous verse for verses greater than 0, in the same way.
  // Any empty paragraphs at the end of the previous verse USFM,
  // add them to the current verse's USFM.
  if (verse) {
    std::string previous_verse_usfm = get_verse_text (usfm, verse - 1);
    // For combined verses: The raw USFM fragments should differ to make sense.
    if (previous_verse_usfm != raw_verse_usfm) {
      if (!previous_verse_usfm.empty ()) {
        markers_and_text = get_markers_and_text (previous_verse_usfm);
        while (true) {
          if (markers_and_text.empty ()) break;
          std::string code = markers_and_text.back ();
          markers_and_text.pop_back ();
          if (!is_usfm_marker (code)) break;
          if (!is_opening_marker (code)) break;
          verse_usfm.insert (0, code + "\n");
        }
      }
    }
  }

  // Done.
  return verse_usfm;
}


// Gets the chapter text given a book of $usfm code, and the $chapter_number.
std::string get_chapter_text (std::string usfm, int chapter_number)
{
  // Empty input: Ready.
  if (usfm.empty ()) return usfm;

  // Remove the part of the USFM that precedes the chapter fragment.
  if (chapter_number) {
    // Normal chapter marker (new line after the number).
    bool found = false;
    std::string marker = get_opening_usfm ("c", false) + std::to_string(chapter_number) + "\n";
    size_t pos = usfm.find (marker);
    // Was the chapter found?
    if (pos != std::string::npos) {
      found = true;
      usfm.erase (0, pos);
    }
    // Unusual chapter marker (space after the number).
    marker = get_opening_usfm ("c", false) + std::to_string(chapter_number) + " ";
    pos = usfm.find (marker);
    if (pos != std::string::npos) {
      found = true;
      usfm.erase (0, pos);
    }
    // Another observed unusual situation: A non-breaking space after the chapter number.
    marker = get_opening_usfm ("c", false) + std::to_string(chapter_number) + filter::strings::non_breaking_space_u00A0 ();
    pos = usfm.find (marker);
    if (pos != std::string::npos) {
      found = true;
      usfm.erase (0, pos);
    }

    // Starting chapter markup not found: Non-existing chapter.
    if (!found) return std::string();
  }

  // Look for any next chapter marker.
  size_t pos = usfm.find (get_opening_usfm ("c", false), 1);
  if (pos != std::string::npos) {
    usfm.erase (pos);
  }
  
  // Clean up.
  usfm = filter::strings::trim (usfm);
  
  // Done.
  return usfm;
}


// Returns the USFM text for a range of verses for the input $usfm code.
// It handles combined verses.
// It ensures that the $exclude_usfm does not make it to the output of the function.
// In case of $quill, it uses a routine optimized for a Quill-based editor.
// This means that empty paragraphs at the end of the extracted USFM fragment are not included.
std::string get_verse_range_text (std::string usfm, int verse_from, int verse_to, const std::string& exclude_usfm, bool quill)
{
  std::vector <std::string> bits;
  std::string previous_usfm;
  for (int vs = verse_from; vs <= verse_to; vs++) {
    std::string verse_usfm;
    if (quill) verse_usfm = get_verse_text_quill (usfm, vs);
    else verse_usfm = get_verse_text (usfm, vs);
    // Do not include repeating USFM in the case of combined verse numbers in the input USFM code.
    if (verse_usfm == previous_usfm) continue;
    if (!verse_usfm.empty () && !previous_usfm.empty ()) {
      if (verse_usfm.find (previous_usfm) != std::string::npos) continue;
      if (previous_usfm.find (verse_usfm) != std::string::npos) continue;
    }
    previous_usfm = verse_usfm;
    // In case of combined verses, the excluded USFM should not be included in the result.
    if (verse_usfm == exclude_usfm) continue;
    if (!verse_usfm.empty () && !exclude_usfm.empty ()) {
      if (verse_usfm.find (exclude_usfm) != std::string::npos) continue;
      if (exclude_usfm.find (verse_usfm) != std::string::npos) continue;
    }
    bits.push_back (verse_usfm);
  }
  usfm = filter::strings::implode (bits, "\n");
  return usfm;
}


// Returns true if the $code contains a USFM marker.
bool is_usfm_marker (std::string code)
{
  if (code.length () < 2) return false;
  if (code.substr (0, 1) == "\\") return true;
  return false;
}


// Returns true if the marker in $usfm is an opening marker.
// Else it returns false.
bool is_opening_marker (std::string usfm)
{
  return usfm.find ("*") == std::string::npos;
}


// Returns true if the marker in $usfm is an embedded marker.
// Else it returns false.
bool is_embedded_marker (std::string usfm)
{
  return usfm.find ( "+") != std::string::npos;
}


// Returns the USFM book identifier.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: if increased by one, it should point to the \id in $usfm.
std::string get_book_identifier (const std::vector <std::string>& usfm, unsigned int pointer)
{
  std::string identifier = "XXX"; // Fallback value.
  if (++pointer < usfm.size ()) {
    identifier = usfm[pointer].substr (0, 3);
  }
  return identifier;
}


// Returns the text that follows a USFM marker.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: should point to the marker in $usfm. It gets increased by one.
std::string get_text_following_marker (const std::vector <std::string>& usfm, unsigned int & pointer)
{
  std::string text = ""; // Fallback value.
  ++pointer;
  if (pointer < usfm.size()) {
    text = usfm [pointer];
  }
  return text;
}


// Returns the text that follows a USFM marker.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: should point to the marker in $usfm. Pointer is left as it is.
std::string peek_text_following_marker (const std::vector <std::string>& usfm, unsigned int pointer)
{
  return get_text_following_marker (usfm, pointer);
}


// Returns the verse number in the $usfm code.
std::string peek_verse_number (std::string usfm)
{
  // Make it robust, even handling cases like \v 1-2“Moi - No space after verse number.
  std::string verseNumber = "";
  size_t usfmStringLength = usfm.length ();
  unsigned int i = 0;
  for (i = 0; i < usfmStringLength; i++) {
    std::string character = usfm.substr (i, 1);
    if (character == "0") continue;
    if (character == "1") continue;
    if (character == "2") continue;
    if (character == "3") continue;
    if (character == "4") continue;
    if (character == "5") continue;
    if (character == "6") continue;
    if (character == "7") continue;
    if (character == "8") continue;
    if (character == "9") continue;
    if (character == ",") continue;
    if (character == "-") continue;
    if (character == "a") continue;
    if (character == "b") continue;
    break;
  }
  verseNumber = usfm.substr (0, i);
  verseNumber = filter::strings::trim (verseNumber);
  return verseNumber;
}


// Takes a marker in the form of text only, like "id" or "add",
// and converts it into opening USFM, like "\id " or "\add ".
// Supports the embedded markup "+".
std::string get_opening_usfm (std::string text, bool embedded)
{
  std::string embed = embedded ? "+" : "";
  return "\\" + embed + text + " ";
}


// Takes a marker in the form of text only, like "add",
// and converts it into closing USFM, like "\add*".
// Supports the embedded markup "+".
std::string get_closing_usfm (std::string text, bool embedded)
{
  std::string embed = embedded ? "+" : "";
  return "\\" + embed + text + "*";
}


// This function compares the $newtext with the $oldtext.
// It returns an empty string if the difference is below the limit set for the Bible.
// It returns a short message specifying the difference if it exceeds that limit.
// It fills $explanation with a longer message in case saving is not safe.
std::string save_is_safe (Webserver_Request& webserver_request,
                          std::string oldtext, std::string newtext, bool chapter, std::string & explanation)
{
  // Two texts are equal: safe.
  if (newtext == oldtext) return std::string();

  const char * explanation1 = "The text was not saved for safety reasons.";
  const char * explanation2 = "Make fewer changes at a time and wait till the editor has saved the text. Or relax the restriction in the editing settings. See menu Settings - Preferences.";
  
  // Allowed percentage difference.
  int allowed_percentage = 0;
  if (chapter)
    allowed_percentage = webserver_request.database_config_user ()->getEditingAllowedDifferenceChapter ();
  else
    allowed_percentage = webserver_request.database_config_user ()->getEditingAllowedDifferenceVerse ();

  // When the verse editor has an empty verse, it should allow for 100% change.
  // Same for the chapter editor, if it has empty verses, allow for a 100% change.
  // This is useful for filling in empty verses.
  if (chapter) {
    if (oldtext.length () < 50) allowed_percentage = 100;
  } else {
    if (oldtext.length () < 10) allowed_percentage = 100;
  }

  // When the new text is longer than the old text, it means the user is typing extra text in the verse.
  // Allow that in all cases.
  if (newtext.length() > oldtext.length()) allowed_percentage = 100;

  // The length of the new text should not differ more than a set percentage from the old text.
  float existingLength = static_cast <float> (oldtext.length());
  float newLength = static_cast <float> (newtext.length ());
  int percentage = static_cast<int> (100 * (newLength - existingLength) / existingLength);
  percentage = abs (percentage);
  if (percentage > 100) percentage = 100;
  if (percentage > allowed_percentage) {
    explanation.append (explanation1);
    explanation.append (" ");
    explanation.append ("The length differs " + std::to_string(percentage) + "% from the existing text.");
    explanation.append (" ");
    explanation.append (explanation2);
    Database_Logs::log (explanation + "\n" + newtext);
    return translate ("Text length differs too much");
  }
  
  // The new text should be at least a set percentage similar to the old text.
  if (chapter) {
    // For larger texts, work at the word level, for much better performance.
    // The time it takes to calculate the difference was measured:
    // character level: 327421 microseconds.
    // word level: 489 microseconds.
    // Doing it at the word level is more than 650 times faster.
    percentage = filter_diff_word_similarity (oldtext, newtext);
  } else {
    // For shorter texts, work at the character level, for better accuracy.
    percentage = filter_diff_character_similarity (oldtext, newtext);
  }
  if (percentage < (100 - allowed_percentage)) {
    explanation.append (explanation1);
    explanation.append (" ");
    explanation.append ("The new text is " + std::to_string(percentage) + "% similar to the existing text.");
    explanation.append (" ");
    explanation.append (explanation2);
    Database_Logs::log (explanation + "\n" + newtext);
    return translate ("Text content differs too much");
  }
  
  // Safety checks have passed.
  return std::string();
}


// Function to safely store a chapter.
// It saves the chapter if the new USFM does not differ too much from the existing USFM.
// On success it returns an empty string.
// On failure it returns the reason of the failure.
// This function proves useful in cases that the text in the Bible editor gets corrupted
// due to human error.
// It also is useful in cases where the session is deleted from the server,
// where the text in the editors would get corrupted.
// It also is useful in view of an unstable connection between browser and server, to prevent data corruption.
std::string safely_store_chapter (Webserver_Request& webserver_request,
                                  std::string bible, int book, int chapter, std::string usfm, std::string & explanation)
{
  // Existing chapter contents.
  std::string existing = database::bibles::get_chapter (bible, book, chapter);
  
  // Bail out if the existing chapter equals the USFM to be saved.
  if (usfm == existing) return std::string();
  
  // Safety check.
  std::string message = save_is_safe (webserver_request, existing, usfm, true, explanation);
  if (!message.empty ()) return message;

  // Record the change in the journal.
  const std::string& user = webserver_request.session_logic ()->get_username ();
  bible_logic::log_change (bible, book, chapter, usfm, user, translate ("Saving chapter"), false);
  
  // Safety checks have passed: Save chapter.
  bible_logic::store_chapter (bible, book, chapter, usfm);
  return std::string();
}


// Function to safely store a verse.
// It saves the verse if the new USFM does not differ too much from the existing USFM.
// On success it returns an empty message.
// On failure it returns a message that specifies the reason why it could not be saved.
// This function proves useful in cases that the text in the Bible editor gets corrupted due to human error.
// It also is useful in cases where the session is deleted from the server,
 // where the text in the editors would get corrupted.
// It also is useful in view of an unstable connection between browser and server, to prevent data corruption.
// It handles combined verses.
std::string safely_store_verse (Webserver_Request& webserver_request,
                                std::string bible, int book, int chapter, int verse, std::string usfm,
                                std::string & explanation, bool quill)
{
  usfm = filter::strings::trim (usfm);

  // Check that the USFM to be saved is for the correct verse.
  std::vector <int> save_verses = get_verse_numbers (usfm);
  if ((verse != 0) && !save_verses.empty ()) {
    save_verses.erase (save_verses.begin());
  }
  if (save_verses.empty ()) {
    explanation = "The USFM contains no verse information";
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Missing verse number");
  }
  if (!in_array (verse, save_verses)) {
    std::vector <std::string> vss;
    for (auto vs : save_verses) vss.push_back (std::to_string(vs));
    explanation = "The USFM contains verse(s) " + filter::strings::implode (vss, " ") + " while it wants to save to verse " + std::to_string (verse);
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Verse mismatch");
  }

  // Get the existing chapter USFM.
  std::string chapter_usfm = database::bibles::get_chapter (bible, book, chapter);
  
  // Get the existing USFM fragment for the verse to save.
  std::string existing_verse_usfm;
  if (quill) existing_verse_usfm = get_verse_text_quill (chapter_usfm, verse);
  else existing_verse_usfm = get_verse_text (chapter_usfm, verse);
  existing_verse_usfm = filter::strings::trim (existing_verse_usfm);

  // Check that there is a match between the existing verse numbers and the verse numbers to save.
  std::vector <int> existing_verses = get_verse_numbers (existing_verse_usfm);
  save_verses = get_verse_numbers (usfm);
  bool verses_match = true;
  if (save_verses.size () == existing_verses.size ()) {
    for (unsigned int i = 0; i < save_verses.size (); i++) {
      if (save_verses [i] != existing_verses [i])
        verses_match = false;
    }
  } else {
    verses_match = false;
  }
  if (!verses_match) {
    std::vector <std::string> existing, save;
    for (auto vs : existing_verses) existing.push_back (std::to_string(vs));
    for (auto vs : save_verses) save.push_back (std::to_string(vs));
    explanation = "The USFM contains verse(s) " + filter::strings::implode (save, " ") + " which would overwrite a fragment that contains verse(s) " + filter::strings::implode (existing, " ");
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Cannot overwrite another verse");
  }

  // Bail out if the new USFM is the same as the existing.
  if (usfm == existing_verse_usfm) {
    return std::string();
  }

  // Check maximum difference between new and existing USFM.
  std::string message = save_is_safe (webserver_request, existing_verse_usfm, usfm, false, explanation);
  if (!message.empty ()) return message;
  
  // Store the new verse USFM in the existing chapter USFM.
  size_t pos = chapter_usfm.find (existing_verse_usfm);
  size_t length = existing_verse_usfm.length ();
  if (pos == std::string::npos) {
    explanation = "Cannot find the exact location in the chapter where to save this USFM fragment";
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Doesn't know where to save");
  }
  chapter_usfm.erase (pos, length);
  chapter_usfm.insert (pos, usfm);

  // Record the change in the journal.
  const std::string& user = webserver_request.session_logic ()->get_username ();
  bible_logic::log_change (bible, book, chapter, chapter_usfm, user, translate ("Saving verse"), false);
  
  // Safety checks have passed: Save chapter.
  bible_logic::store_chapter (bible, book, chapter, chapter_usfm);

  // Done: OK.
  return std::string();
}


// Returns whether $usfm contains one or more empty verses.
bool contains_empty_verses (std::string usfm)
{
  usfm = filter::strings::replace ("\n", "", usfm);
  if (usfm.empty ()) return false;
  for (int i = 0; i <= 9; i++) {
    usfm = filter::strings::replace (std::to_string(i), "", usfm);
  }
  if (usfm.empty ()) return false;
  usfm = filter::strings::replace (" ", "", usfm);
  if (usfm.empty ()) return false;
  size_t pos = usfm.find ("\\v\\v");
  if (pos != std::string::npos) return true;
  pos = usfm.find ("\\v \\v");
  if (pos != std::string::npos) return true;
  pos = usfm.find_last_of (marker_v ());
  if (pos == usfm.length () - 1) return true;
  return false;
}


// This looks at the $fragment, whether it's a range of verses.
// If so, it puts the all of the verses in $verses, and returns true.
bool handle_verse_range (std::string verse, std::vector <int> & verses)
{
  if (verse.find ("-") != std::string::npos) {
    size_t position;
    position = verse.find ("-");
    std::string start_range, end_range;
    start_range = verse.substr (0, position);
    verse.erase (0, ++position);
    end_range = verse;
    int start_verse_i = filter::strings::convert_to_int(filter::strings::number_in_string(start_range));
    int end_verse_i = filter::strings::convert_to_int(filter::strings::number_in_string(end_range));
    for (int i = start_verse_i; i <= end_verse_i; i++) {
      if (i == start_verse_i)
        verses.push_back (filter::strings::convert_to_int (start_range));
      else if (i == end_verse_i)
        verses.push_back (filter::strings::convert_to_int (end_range));
      else
        verses.push_back (i);
    }
    return true;
  }
  return false;
}


// This looks at the $fragment, whether it's a sequence of verses.
// If so, it puts the all of the verses in $verses, and returns true.
bool handle_verse_sequence (std::string verse, std::vector <int> & verses)
{
  if (verse.find (",") != std::string::npos) {
    int iterations = 0;
    do {
      // In case of an unusual range formation, do not hang.
      iterations++;
      if (iterations > 50) {
        break;
      }
      size_t position = verse.find (",");
      std::string vs;
      if (position == std::string::npos) {
        vs = verse;
        verse.clear ();
      } else {
        vs = verse.substr (0, position);
        verse.erase(0, ++position);
      }
      verses.push_back (filter::strings::convert_to_int (vs));
    } while (!verse.empty());
    return true;
  }
  return false;
}


const char * marker_v ()
{
  return R"(\v)";
}


const char * marker_va ()
{
  return R"(\va)";
}


const char * marker_vp ()
{
  return R"(\vp)";
}


// Find and remove the word-level attributes.
// https://ubsicap.github.io/usfm/attributes/index.html
// Example: \+w Lord|strong="H3068"\+w*
// It will dispose of e.g. this: |strong="H3068"
// It handles the default attribute: \w gracious|grace\w*
void remove_word_level_attributes (const std::string& marker,
                                   std::vector <std::string> & container, unsigned int & pointer)
{
  // USFM 3.0 has four markers providing attributes.
  // https://ubsicap.github.io/usfm/attributes/index.html.
  // Deal with those only, and don't deal with any others.
  // Note that the \fig markup is handled elsewhere in this class.
  if ((marker != "w") && (marker != "rb") && (marker != "xt")) return;
  
  // Check the text following this markup whether it contains word-level attributes.
  std::string possible_markup = filter::usfm::peek_text_following_marker (container, pointer);
  
  // If the markup is too short to contain the required characters, then bail out.
  if (possible_markup.length() < 4) return;
  
  // Look for the vertical bar. If it's not there, bail out.
  size_t bar_position = possible_markup.find("|");
  if (bar_position == std::string::npos) return;
  
  // Remove the fragment and store the remainder back into the object.
  possible_markup.erase(bar_position);
  container [pointer + 1] = possible_markup;
}


// This extracts the attributs for the "fig" markup.
// It supports USFM 3.x.
// https://ubsicap.github.io/usfm/characters/index.html#fig-fig
// That means it is backwards compatible with USFM 1/2:
// \fig DESC|FILE|SIZE|LOC|COPY|CAP|REF\fig*
std::string extract_fig (std::string usfm, std::string & caption, std::string & alt, std::string& src, std::string& size, std::string& loc, std::string& copy, std::string& ref)
{
  // The resulting USFM where the \fig markup has been removed from.
  std::string usfm_out;
  
  // The string passed in the $usfm variable may contain the \fig..\fig* markup.
  // Or it may omit those.
  // Handle both cases: Get the USFM fragment within the \fig...\fig* markup.
  std::string marker = "fig";
  
  // If the opener is there, it means the \fig markup could be there.
  std::string opener = get_opening_usfm (marker);
  size_t pos1 = usfm.find (opener);
  if (pos1 != std::string::npos) {
    usfm_out.append(usfm.substr(0, pos1));
    usfm.erase (0, pos1 + opener.length());
    // Erase the \fig* closing markup.
    std::string closer = get_closing_usfm(marker);
    size_t pos2 = usfm.find(closer);
    if (pos2 != std::string::npos) {
      usfm_out.append(usfm.substr(pos2 + closer.length()));
      usfm.erase(pos2);
    }
  } else {
    usfm_out.assign(usfm);
  }
  
  // Split the bit of USFM between the \fig...\fig* markup on the vertical bar.
  std::vector<std::string> bits = filter::strings::explode(usfm, '|');

  // Clear the variables that will contain the extracted information.
  caption.clear();
  alt.clear();
  src.clear();
  size.clear();
  loc.clear();
  copy.clear();
  ref.clear();
  
  // Handle a situation that there are 7 bits of information.
  // That is the situation as used in USFM 1/2.x
  // \fig DESC|FILE|SIZE|LOC|COPY|CAP|REF\fig*
  if (bits.size() == 7) {
    alt = bits[0];
    src = bits[1];
    size = bits[2];
    loc = bits[3];
    copy = bits[4];
    caption = bits[5];
    ref = bits[6];
  }

  // Handle the situation that there are two bits of information.
  // This is when there is one vertical bar.
  // This is the situation of USFM 3.x.
  // https://ubsicap.github.io/usfm/characters/index.html#fig-fig
  if (bits.size() == 2) {
    caption = bits[0];
    std::string xml = "<fig " + bits[1] + " ></fig>";
    pugi::xml_document document;
    document.load_string (xml.c_str(), pugi::parse_ws_pcdata_single);
    pugi::xml_node node = document.first_child ();
    alt = node.attribute ("alt").value ();
    src = node.attribute ("src").value ();
    size = node.attribute ("size").value ();
    loc = node.attribute ("loc").value ();
    copy = node.attribute ("copy").value ();
    ref = node.attribute ("ref").value ();
  }

  // The resulting USFM with the figure markup removed.
  return usfm_out;
}


// Returns true if the marker is a standard "q." marker.
bool is_standard_q_poetry (const std::string& marker)
{
  if (marker == "q") return true;
  if (marker == "q1") return true;
  if (marker == "q2") return true;
  if (marker == "q3") return true;
  return false;
}


} // End of namespace.
