/*
Copyright (©) 2003-2020 Teus Benschop.

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


BookChapterData::BookChapterData (int book_in, int chapter_in, string data_in)
{
  book = book_in;
  chapter = chapter_in;
  data = data_in;
}


// Returns the string $usfm as one long string.
// $usfm may contain new lines, but the resulting long string won't.
string usfm_one_string (string usfm)
{
  string long_string = "";
  vector <string> usfm_lines = filter_string_explode (usfm, '\n');
  for (string & line : usfm_lines) {
    line = filter_string_trim (line);
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
vector <string> usfm_get_markers_and_text (string code)
{
  vector <string> markers_and_text;
  code = filter_string_str_replace ("\n\\", "\\", code); // New line followed by backslash: leave new line out.
  code = filter_string_str_replace ("\n", " ", code); // New line only: change to space, according to the USFM specification.
  // No removal of double spaces, because it would remove an opening marker (which already has its own space), followed by a space.
  code = filter_string_trim (code);
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
      vector <size_t> positions;
      pos = code.find (" ");
      if (pos != string::npos) positions.push_back (pos + 1);
      pos = code.find ("*");
      if (pos != string::npos) positions.push_back (pos + 1);
      pos = code.find ("\\", 1);
      if (pos != string::npos) positions.push_back (pos);
      positions.push_back (code.length());
      sort (positions.begin (), positions.end());
      pos = positions[0];
      string marker = code.substr (0, pos);
      markers_and_text.push_back (marker);
      code = code.substr (pos);
    } else {
      // Text found. It ends at the next backslash or at the end of the string.
      pos = code.find ("\\");
      if (pos == string::npos) pos = code.length();
      string text = code.substr (0, pos);
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
string usfm_get_marker (string usfm)
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
    vector <size_t> positions;
    pos = usfm.find (" ");
    if (pos != string::npos) positions.push_back (pos);
    pos = usfm.find ("*");
    if (pos != string::npos) positions.push_back (pos);
    pos = usfm.find ("\\");
    if (pos != string::npos) positions.push_back (pos);
    positions.push_back (usfm.length());
    sort (positions.begin(), positions.end());
    pos = positions[0];
    string marker = usfm.substr (0, pos);
    return marker;
  }
  // Text found. No marker.
  return "";
}


// This imports USFM $input.
// It takes raw $input,
// and returns a vector with objects with book_number, chapter_number, chapter_data.
vector <BookChapterData> usfm_import (string input, string stylesheet)
{
  vector <BookChapterData> result;

  int book_number = 0;
  int chapter_number = 0;
  string chapter_data = "";

  input = usfm_one_string (input);
  vector <string> markers_and_text = usfm_get_markers_and_text (input);
  bool retrieve_book_number_on_next_iteration = false;
  bool retrieve_chapter_number_on_next_iteration = false;

  for (string marker_or_text : markers_and_text) {
    if (retrieve_book_number_on_next_iteration) {
      book_number = Database_Books::getIdFromUsfm (marker_or_text.substr (0, 3));
      chapter_number = 0;
      retrieve_book_number_on_next_iteration = false;
    }
    if (retrieve_chapter_number_on_next_iteration) {
      retrieve_chapter_number_on_next_iteration = false;
      chapter_number = convert_to_int (marker_or_text);
    }
    string marker = usfm_get_marker (marker_or_text);
    if (!marker.empty()) {
      // USFM marker found.
      bool opener = usfm_is_opening_marker (marker_or_text);
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
        chapter_data = filter_string_trim (chapter_data);
        if (chapter_data != "") result.push_back ( { book_number, chapter_number, chapter_data } );
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
  chapter_data = filter_string_trim (chapter_data);
  if (chapter_data != "") result.push_back (BookChapterData (book_number, chapter_number, chapter_data));
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
vector <int> usfm_get_verse_numbers (string usfm)
{
  vector <int> verse_numbers = { 0 };
  vector <string> markers_and_text = usfm_get_markers_and_text (usfm);
  bool extract_verse = false;
  for (string marker_or_text : markers_and_text) {
    if (extract_verse) {
      string verse = usfm_peek_verse_number (marker_or_text);
      // Range of verses.
      if (usfm_handle_verse_range (verse, verse_numbers));
      // Sequence of verses.
      else if (usfm_handle_verse_sequence (verse, verse_numbers));
      // Single verse.
      else verse_numbers.push_back (convert_to_int (verse));
      extract_verse = false;
    }
    if (marker_or_text.substr (0, 2) == usfm_marker_v ()) extract_verse = true;
    string va_or_vp = marker_or_text.substr (0, 3);
    if (va_or_vp == usfm_marker_va ()) extract_verse = false;
    if (va_or_vp == usfm_marker_vp ()) extract_verse = false;
  }
  return verse_numbers;
}


// Returns the chapter numbers found in $usfm.
vector <int> usfm_get_chapter_numbers (string usfm)
{
  vector <int> chapter_numbers = { 0 };
  vector <string> markers_and_text = usfm_get_markers_and_text (usfm);
  bool extract_chapter = false;
  for (string marker_or_text : markers_and_text) {
    if (extract_chapter) {
      string chapter = usfm_peek_verse_number (marker_or_text);
      chapter_numbers.push_back (convert_to_int (chapter));
      extract_chapter = false;
    }
    if (marker_or_text.substr (0, 2) == "\\c") {
      extract_chapter = true;
    }
  }
  return chapter_numbers;
}


// Returns the verse numbers in the string of $usfm code at line number $line_number.
vector <int> usfm_linenumber_to_versenumber (string usfm, unsigned int line_number)
{
  vector <int> verse_number = {0}; // Initial verse number.
  vector <string> lines = filter_string_explode (usfm, '\n');
  for (unsigned int i = 0; i < lines.size(); i++) {
    if (i <= line_number) {
      vector <int> verse_numbers = usfm_get_verse_numbers (lines[i]);
      if (verse_numbers.size() >= 2) {
        verse_number = filter_string_array_diff (verse_numbers, {0});
      }
    }
  }
  return verse_number;
}


// Returns the verse numbers in the string of $usfm code at offset $offset.
// Offset is calculated with unicode_string_length to support UTF-8.
vector <int> usfm_offset_to_versenumber (string usfm, unsigned int offset)
{
  unsigned int totalOffset = 0;
  vector <string> lines = filter_string_explode (usfm, '\n');
  for (unsigned int i = 0; i < lines.size(); i++) {
    int length = unicode_string_length (lines [i]);
    totalOffset += length;
    if (totalOffset >= offset) {
      return usfm_linenumber_to_versenumber (usfm, i);
    }
    // Add 1 for new line.
    totalOffset += 1;
  }
  return {0};
}



// Returns the offset within the $usfm code where $verse number starts.
int usfm_versenumber_to_offset (string usfm, int verse)
{
  // Verse number 0 starts at offset 0.
  if (verse == 0) return 0;
  int totalOffset = 0;
  vector <string> lines = filter_string_explode (usfm, '\n');
  for (string line : lines) {
    vector <int> verses = usfm_get_verse_numbers (line);
    for (auto & v : verses) {
      if (v == verse) return totalOffset;
    }
    totalOffset += unicode_string_length (line);
    // Add 1 for new line.
    totalOffset += 1;
  }
  return unicode_string_length (usfm);
}


// Returns the verse text given a $verse_number and $usfm code.
// Handles combined verses.
string usfm_get_verse_text (string usfm, int verse_number)
{
  vector <string> result;
  bool hit = (verse_number == 0);

  vector <string> lines = filter_string_explode (usfm, '\n');
  for (string line : lines) {
    vector <int> verses = usfm_get_verse_numbers (line);
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
  string verseText = filter_string_implode (result, "\n");
  return verseText;
}


// Gets the USFM for the $verse number for a Quill-based verse editor.
// This means that preceding empty paragraphs will be included also.
// And that empty paragraphs at the end will be omitted.
string usfm_get_verse_text_quill (string usfm, int verse)
{
  // Get the raw USFM for the verse, that is, the bit between the \v... markers.
  string raw_verse_usfm = usfm_get_verse_text (usfm, verse);
  
  // Bail out if empty.
  if (raw_verse_usfm.empty ()) {
    return raw_verse_usfm;
  }

  // Omit new paragraphs at the end.
  // In this context it is taken as opening USFM markers without content.
  string verse_usfm (raw_verse_usfm);
  vector <string> markers_and_text = usfm_get_markers_and_text (verse_usfm);
  while (true) {
    if (markers_and_text.empty ()) break;
    string code = markers_and_text.back ();
    markers_and_text.pop_back ();
    if (!usfm_is_usfm_marker (code)) break;
    if (!usfm_is_opening_marker (code)) break;
    verse_usfm.erase (verse_usfm.size () - code.size ());
    verse_usfm = filter_string_trim (verse_usfm);
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
    string previous_verse_usfm = usfm_get_verse_text (usfm, verse - 1);
    // For combined verses: The raw USFM fragments should differ to make sense.
    if (previous_verse_usfm != raw_verse_usfm) {
      if (!previous_verse_usfm.empty ()) {
        markers_and_text = usfm_get_markers_and_text (previous_verse_usfm);
        while (true) {
          if (markers_and_text.empty ()) break;
          string code = markers_and_text.back ();
          markers_and_text.pop_back ();
          if (!usfm_is_usfm_marker (code)) break;
          if (!usfm_is_opening_marker (code)) break;
          verse_usfm.insert (0, code + "\n");
        }
      }
    }
  }

  // Done.
  return verse_usfm;
}


// Gets the chapter text given a book of $usfm code, and the $chapter_number.
string usfm_get_chapter_text (string usfm, int chapter_number)
{
  // Empty input: Ready.
  if (usfm.empty ()) return usfm;

  // Remove the part of the USFM that precedes the chapter fragment.
  if (chapter_number) {
    // Normal chapter marker (new line after the number).
    bool found = false;
    string marker = usfm_get_opening_usfm ("c", false) + convert_to_string (chapter_number) + "\n";
    size_t pos = usfm.find (marker);
    // Was the chapter found?
    if (pos != string::npos) {
      found = true;
      usfm.erase (0, pos);
    }
    // Unusual chapter marker (space after the number).
    marker = usfm_get_opening_usfm ("c", false) + convert_to_string (chapter_number) + " ";
    pos = usfm.find (marker);
    if (pos != string::npos) {
      found = true;
      usfm.erase (0, pos);
    }
    // Another observed unusual situation: A non-breaking space after the chapter number.
    marker = usfm_get_opening_usfm ("c", false) + convert_to_string (chapter_number) + non_breaking_space_u00A0 ();
    pos = usfm.find (marker);
    if (pos != string::npos) {
      found = true;
      usfm.erase (0, pos);
    }

    // Starting chapter markup not found: Non-existing chapter.
    if (!found) return "";
  }

  // Look for any next chapter marker.
  size_t pos = usfm.find (usfm_get_opening_usfm ("c", false), 1);
  if (pos != string::npos) {
    usfm.erase (pos);
  }
  
  // Clean up.
  usfm = filter_string_trim (usfm);
  
  // Done.
  return usfm;
}


// Returns the USFM text for a range of verses for the input $usfm code.
// It handles combined verses.
// It ensures that the $exclude_usfm does not make it to the output of the function.
// In case of $quill, it uses a routine optimized for a Quill-based editor.
// This means that empty paragraphs at the end of the extracted USFM fragment are not included.
string usfm_get_verse_range_text (string usfm, int verse_from, int verse_to, const string& exclude_usfm, bool quill)
{
  vector <string> bits;
  string previous_usfm;
  for (int vs = verse_from; vs <= verse_to; vs++) {
    string verse_usfm;
    if (quill) verse_usfm = usfm_get_verse_text_quill (usfm, vs);
    else verse_usfm = usfm_get_verse_text (usfm, vs);
    // Do not include repeating USFM in the case of combined verse numbers in the input USFM code.
    if (verse_usfm == previous_usfm) continue;
    if (!verse_usfm.empty () && !previous_usfm.empty ()) {
      if (verse_usfm.find (previous_usfm) != string::npos) continue;
      if (previous_usfm.find (verse_usfm) != string::npos) continue;
    }
    previous_usfm = verse_usfm;
    // In case of combined verses, the excluded USFM should not be included in the result.
    if (verse_usfm == exclude_usfm) continue;
    if (!verse_usfm.empty () && !exclude_usfm.empty ()) {
      if (verse_usfm.find (exclude_usfm) != string::npos) continue;
      if (exclude_usfm.find (verse_usfm) != string::npos) continue;
    }
    bits.push_back (verse_usfm);
  }
  usfm = filter_string_implode (bits, "\n");
  return usfm;
}


// Returns true if the $code contains a USFM marker.
bool usfm_is_usfm_marker (string code)
{
  if (code.length () < 2) return false;
  if (code.substr (0, 1) == "\\") return true;
  return false;
}


// Returns true if the marker in $usfm is an opening marker.
// Else it returns false.
bool usfm_is_opening_marker (string usfm)
{
  return usfm.find ("*") == string::npos;
}


// Returns true if the marker in $usfm is an embedded marker.
// Else it returns false.
bool usfm_is_embedded_marker (string usfm)
{
  return usfm.find ( "+") != string::npos;
}


// Returns the USFM book identifier.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: if increased by one, it should point to the \id in $usfm.
string usfm_get_book_identifier (const vector <string>& usfm, unsigned int pointer)
{
  string identifier = "XXX"; // Fallback value.
  if (++pointer < usfm.size ()) {
    identifier = usfm[pointer].substr (0, 3);
  }
  return identifier;
}


// Returns the text that follows a USFM marker.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: should point to the marker in $usfm. It gets increased by one.
string usfm_get_text_following_marker (const vector <string>& usfm, unsigned int & pointer)
{
  string text = ""; // Fallback value.
  ++pointer;
  if (pointer < usfm.size()) {
    text = usfm [pointer];
  }
  return text;
}


// Returns the text that follows a USFM marker.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: should point to the marker in $usfm. Pointer is left as it is.
string usfm_peek_text_following_marker (const vector <string>& usfm, unsigned int pointer)
{
  return usfm_get_text_following_marker (usfm, pointer);
}


// Returns the verse number in the $usfm code.
string usfm_peek_verse_number (string usfm)
{
  // Make it robust, even handling cases like \v 1-2“Moi - No space after verse number.
  string verseNumber = "";
  size_t usfmStringLength = usfm.length ();
  unsigned int i = 0;
  for (i = 0; i < usfmStringLength; i++) {
    string character = usfm.substr (i, 1);
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
  verseNumber = filter_string_trim (verseNumber);
  return verseNumber;
}


// Takes a marker in the form of text only, like "id" or "add",
// and converts it into opening USFM, like "\id " or "\add ".
// Supports the embedded markup "+".
string usfm_get_opening_usfm (string text, bool embedded)
{
  string embed = embedded ? "+" : "";
  return "\\" + embed + text + " ";
}


// Takes a marker in the form of text only, like "add",
// and converts it into closing USFM, like "\add*".
// Supports the embedded markup "+".
string usfm_get_closing_usfm (string text, bool embedded)
{
  string embed = embedded ? "+" : "";
  return "\\" + embed + text + "*";
}


// This function compares the $newtext with the $oldtext.
// It returns an empty string if the difference is below the limit set for the Bible.
// It returns a short message specifying the difference if it exceeds that limit.
// It fills $explanation with a longer message in case saving is not safe.
string usfm_save_is_safe (void * webserver_request, string oldtext, string newtext, bool chapter, string & explanation)
{
  // Two texts are equal: safe.
  if (newtext == oldtext) return "";

  Webserver_Request * request = (Webserver_Request *) webserver_request;

  const char * explanation1 = "The text was not saved for safety reasons.";
  const char * explanation2 = "Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Preferences.";
  
  // Allowed percentage difference.
  int allowed_percentage = request->database_config_user ()->getEditingAllowedDifferenceVerse ();
  if (chapter) allowed_percentage = request->database_config_user ()->getEditingAllowedDifferenceChapter ();

  // When the verse editor has an empty verse, it should allow for 100% change.
  // This is useful for filling in empty verses.
  if (!chapter) {
    if (oldtext.length () < 10) allowed_percentage = 100;
  }
  
  // The length of the new text should not differ more than a set percentage from the old text.
  float existingLength = oldtext.length();
  float newLength = newtext.length ();
  int percentage = 100 * (newLength - existingLength) / existingLength;
  percentage = abs (percentage);
  if (percentage > 100) percentage = 100;
  if (percentage > allowed_percentage) {
    explanation.append (explanation1);
    explanation.append (" ");
    explanation.append ("The length differs " + convert_to_string (percentage) + "% from the existing text.");
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
    explanation.append ("The new text is " + convert_to_string (percentage) + "% similar to the existing text.");
    explanation.append (" ");
    explanation.append (explanation2);
    Database_Logs::log (explanation + "\n" + newtext);
    return translate ("Text content differs too much");
  }
  
  // Safety checks have passed.
  return "";
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
string usfm_safely_store_chapter (void * webserver_request,
                                  string bible, int book, int chapter, string usfm, string & explanation)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  // Existing chapter contents.
  string existing = request->database_bibles()->getChapter (bible, book, chapter);
  
  // Bail out if the existing chapter equals the USFM to be saved.
  if (usfm == existing) return "";
  
  // Safety check.
  string message = usfm_save_is_safe (webserver_request, existing, usfm, true, explanation);
  if (!message.empty ()) return message;

  // Record the change in the journal.
  string user = request->session_logic ()->currentUser ();
  bible_logic_log_change (bible, book, chapter, usfm, user, translate ("Saving chapter"), false);
  
  // Safety checks have passed: Save chapter.
  bible_logic_store_chapter (bible, book, chapter, usfm);
  return "";
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
string usfm_safely_store_verse (void * webserver_request,
                                string bible, int book, int chapter, int verse, string usfm,
                                string & explanation, bool quill)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  usfm = filter_string_trim (usfm);

  // Check that the USFM to be saved is for the correct verse.
  vector <int> save_verses = usfm_get_verse_numbers (usfm);
  if ((verse != 0) && !save_verses.empty ()) {
    save_verses.erase (save_verses.begin());
  }
  if (save_verses.empty ()) {
    explanation = "The USFM contains no verse information";
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Missing verse number");
  }
  if (!in_array (verse, save_verses)) {
    vector <string> vss;
    for (auto vs : save_verses) vss.push_back (convert_to_string (vs));
    explanation = "The USFM contains verse(s) " + filter_string_implode (vss, " ") + " while it wants to save to verse " + convert_to_string (verse);
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Verse mismatch");
  }

  // Get the existing chapter USFM.
  string chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  // Get the existing USFM fragment for the verse to save.
  string existing_verse_usfm;
  if (quill) existing_verse_usfm = usfm_get_verse_text_quill (chapter_usfm, verse);
  else existing_verse_usfm = usfm_get_verse_text (chapter_usfm, verse);
  existing_verse_usfm = filter_string_trim (existing_verse_usfm);

  // Check that there is a match between the existing verse numbers and the verse numbers to save.
  vector <int> existing_verses = usfm_get_verse_numbers (existing_verse_usfm);
  save_verses = usfm_get_verse_numbers (usfm);
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
    vector <string> existing, save;
    for (auto vs : existing_verses) existing.push_back (convert_to_string (vs));
    for (auto vs : save_verses) save.push_back (convert_to_string (vs));
    explanation = "The USFM contains verse(s) " + filter_string_implode (save, " ") + " which would overwrite a fragment that contains verse(s) " + filter_string_implode (existing, " ");
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Cannot overwrite another verse");
  }

  // Bail out if the new USFM is the same as the existing.
  if (usfm == existing_verse_usfm) {
    return "";
  }

  // Check maximum difference between new and existing USFM.
  string message = usfm_save_is_safe (webserver_request, existing_verse_usfm, usfm, false, explanation);
  if (!message.empty ()) return message;
  
  // Store the new verse USFM in the existing chapter USFM.
  size_t pos = chapter_usfm.find (existing_verse_usfm);
  size_t length = existing_verse_usfm.length ();
  if (pos == string::npos) {
    explanation = "Cannot find the exact location in the chapter where to save this USFM fragment";
    Database_Logs::log (explanation + ": " + usfm);
    return translate ("Doesn't know where to save");
  }
  chapter_usfm.erase (pos, length);
  chapter_usfm.insert (pos, usfm);

  // Record the change in the journal.
  string user = request->session_logic ()->currentUser ();
  bible_logic_log_change (bible, book, chapter, chapter_usfm, user, translate ("Saving verse"), false);
  
  // Safety checks have passed: Save chapter.
  bible_logic_store_chapter (bible, book, chapter, chapter_usfm);

  // Done: OK.
  return "";
}


// Returns whether $usfm contains one or more empty verses.
bool usfm_contains_empty_verses (string usfm)
{
  usfm = filter_string_str_replace ("\n", "", usfm);
  if (usfm.empty ()) return false;
  for (int i = 0; i <= 9; i++) {
    usfm = filter_string_str_replace (convert_to_string (i), "", usfm);
  }
  if (usfm.empty ()) return false;
  usfm = filter_string_str_replace (" ", "", usfm);
  if (usfm.empty ()) return false;
  size_t pos = usfm.find ("\\v\\v");
  if (pos != string::npos) return true;
  pos = usfm.find ("\\v \\v");
  if (pos != string::npos) return true;
  pos = usfm.find_last_of (usfm_marker_v ());
  if (pos == usfm.length () - 1) return true;
  return false;
}


// This looks at the $fragment, whether it's a range of verses.
// If so, it puts the all of the verses in $verses, and returns true.
bool usfm_handle_verse_range (string verse, vector <int> & verses)
{
  if (verse.find ("-") != string::npos) {
    size_t position;
    position = verse.find ("-");
    string start_range, end_range;
    start_range = verse.substr (0, position);
    verse.erase (0, ++position);
    end_range = verse;
    unsigned int start_verse_i = convert_to_int(number_in_string(start_range));
    unsigned int end_verse_i = convert_to_int(number_in_string(end_range));
    for (unsigned int i = start_verse_i; i <= end_verse_i; i++) {
      if (i == start_verse_i)
        verses.push_back (convert_to_int (start_range));
      else if (i == end_verse_i)
        verses.push_back (convert_to_int (end_range));
      else
        verses.push_back (i);
    }
    return true;
  }
  return false;
}


// This looks at the $fragment, whether it's a sequence of verses.
// If so, it puts the all of the verses in $verses, and returns true.
bool usfm_handle_verse_sequence (string verse, vector <int> & verses)
{
  if (verse.find (",") != string::npos) {
    int iterations = 0;
    do {
      // In case of an unusual range formation, do not hang.
      iterations++;
      if (iterations > 50) {
        break;
      }
      size_t position = verse.find (",");
      string vs;
      if (position == string::npos) {
        vs = verse;
        verse.clear ();
      } else {
        vs = verse.substr (0, position);
        verse.erase(0, ++position);
      }
      verses.push_back (convert_to_int (vs));
    } while (!verse.empty());
    return true;
  }
  return false;
}


const char * usfm_marker_v ()
{
  return R"(\v)";
}


const char * usfm_marker_va ()
{
  return R"(\va)";
}


const char * usfm_marker_vp ()
{
  return R"(\vp)";
}


// This removes the word level attributes from $usfm.
// See https://ubsicap.github.io/usfm/attributes/index.html
// Within a character marker span,
// an attributes list is separated from the text content by a vertical bar |.
// Attributes are listed as pairs of name and corresponding value using the syntax:
//   attribute = "value".
// Example:
//   \w gracious|lemma="grace"\w*
string usfm_remove_word_level_attributes (string usfm)
{
  // Check there is a vertical bar at all in the input USFM.
  // If it's not there, then the function is ready without much ado.
  if (usfm.find ("|") != string::npos) {

    // Flag whether a replacement was made.
    bool keep_going = false;
    
    // In USFM 3.0 there's two character markers that support word level attributes:
    vector <string> supported_character_markers = { "w", "fig" };
    for (auto & marker : supported_character_markers) {

      // Support multiple replacements.
      size_t last_pos = 0;
      do {
      
        // Set flag.
        keep_going = false;

        // The opener should be there.
        size_t opener_pos = usfm.find (usfm_get_opening_usfm (marker), last_pos);
        if (opener_pos == string::npos) continue;
        last_pos = opener_pos + 1;

        // The closer should be there too.
        size_t closer_pos = usfm.find (usfm_get_closing_usfm (marker), last_pos);
        if (closer_pos == string::npos) continue;

        // The vertical bar should be between the opener and closer.
        size_t bar_pos = usfm.find ("|", last_pos);
        if (bar_pos == string::npos) continue;
        if (bar_pos < opener_pos) continue;
        
        // There may be situations without the vertical bar.
        if (bar_pos < closer_pos) {
          // Remove the word level attribute.
          usfm.erase (bar_pos, closer_pos - bar_pos);
        }

        // Set flag.
        keep_going = true;

      } while (keep_going);
    }
  }
  
  // Done.
  return usfm;
}
