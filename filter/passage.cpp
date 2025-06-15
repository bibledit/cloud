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


#include <filter/passage.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <database/books.h>
#include <database/bibles.h>
#include <database/config/bible.h>
#include <locale/translate.h>
#include <assets/view.h>
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


Passage::Passage ()
{
  m_book = 0;
  m_chapter = 0;
}


Passage::Passage (std::string bible, int book, int chapter, std::string verse)
{
  m_bible = bible;
  m_book = book;
  m_chapter = chapter;
  m_verse = verse;
}


bool Passage::equal (Passage & passage)
{
  if (m_bible != passage.m_bible) return false;
  if (m_book != passage.m_book) return false;
  if (m_chapter != passage.m_chapter) return false;
  if (m_verse != passage.m_verse) return false;
  return true;
}


// This method converts the passage of the object into text, like e.g. so:
// "hexadecimal Bible _1_2_3".
// First the hexadecimal Bible comes, then the book identifier, then the chapter number, and finally the verse number.
std::string Passage::encode () const
{
  std::string text;
  // The encoded passage can be used as an attribute in the HTML DOM.
  // Therefore it will be encoded such that any Bible name will be acceptable as an attribute in the DOM.
  text.append (filter::strings::bin2hex (m_bible));
  text.append ("_");
  text.append (std::to_string (m_book));
  text.append ("_");
  text.append (std::to_string (m_chapter));
  text.append ("_");
  text.append (m_verse);
  if (m_verse.empty()) text.append ("0");
  return text;
}


// This method converts encoded text into a passage.
// The text is in the format as its complementary function, "encode", produces.
Passage Passage::decode (const std::string& encoded)
{
  Passage passage;
  std::vector <std::string> bits = filter::strings::explode (encoded, '_');
  if (!bits.empty ()) {
    std::string verse = bits.back ();
    if (!verse.empty ()) passage.m_verse = verse;
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    std::string chapter = bits.back ();
    if (!chapter.empty()) passage.m_chapter = filter::strings::convert_to_int (chapter);
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    std::string book = bits.back ();
    if (!book.empty()) passage.m_book = filter::strings::convert_to_int (book);
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    passage.m_bible = filter::strings::hex2bin (bits.back ());
    bits.pop_back ();
  }
  return passage;
}


std::string filter_passage_display (int book, int chapter, std::string verse)
{
  std::string display;
  display.append (translate (database::books::get_english_from_id (static_cast<book_id>(book)).c_str()));
  display.append (" ");
  display.append (std::to_string (chapter));
  if (!verse.empty ()) display.append (":" + verse);
  return display;
}


// Returns the display string for the $passages as one line.
std::string filter_passage_display_inline (std::vector <Passage> passages)
{
  std::string display;
  for (Passage & passage : passages) {
    if (!display.empty()) display.append (" | ");
    display.append (filter_passage_display (passage.m_book, passage.m_chapter, passage.m_verse));
  }
  return display;
}


// Returns the display string for the $passages as several lines.
std::string filter_passage_display_multiline (std::vector <Passage> passages)
{
  std::string display;
  for (Passage & passage : passages) {
    display.append (filter_passage_display (passage.m_book, passage.m_chapter, passage.m_verse));
    display.append ("\n");
  }
  return display;
}


// This function converts $passage to an integer, so that passages can be compared or stored.
int filter_passage_to_integer (Passage passage)
{
  return 1000000 * passage.m_book + 1000 * passage.m_chapter + filter::strings::convert_to_int (passage.m_verse);
}


// This converts the $integer, created above, to a passage.
Passage filter_integer_to_passage (int integer)
{
  const int book = static_cast<int> (round (integer / 1000000));
  integer -= book * 1000000;
  const int chapter = static_cast<int> (round (integer / 1000));
  integer -= chapter * 1000;
  const std::string verse = std::to_string (integer);
  return Passage ("", book, chapter, verse);
}


// This filter takes $books as a string,
// and looks whether it can be interpreted as a valid book in any way.
// It returns a valid book identifier,
// or the unknown enum in case no book could be interpreted.
book_id filter_passage_interpret_book_v2 (std::string book)
{
  book = filter::strings::trim (book);
  
  // Recognise the USFM book abbreviations.
  {
    book_id identifier = database::books::get_id_from_usfm (book);
    if (identifier != book_id::_unknown) return identifier;
  }

  // Recognize the BibleWorks book abbreviations.
  {
    book_id identifier = database::books::get_id_from_bibleworks (book);
    if (identifier != book_id::_unknown) return identifier;
  }

  // Handle names from BibleWorks when copying the verse list to the clipboard.
  // These are not handled elsewhere.
  if (book == "Cant") return book_id::_song_of_solomon;
  if (book == "Mk") return book_id::_mark;
  if (book == "Lk") return book_id::_luke;
  if (book == "Jn") return book_id::_john;
  if (book == "1 Jn") return book_id::_1_john;
  if (book == "2 Jn") return book_id::_2_john;
  if (book == "3 Jn") return book_id::_3_john;

  // Recognize names like "I Peter", where the "I" can also be "II" or "III".
  // Do the longest ones first.
  book = filter::strings::replace ("III ", "3 ", book);
  book = filter::strings::replace ("II ", "2 ", book);
  book = filter::strings::replace ("I ", "1 ", book);
  
  // Do case folding, i.e., work with lower case only.
  book = filter::strings::unicode_string_casefold (book);
  
  // Remove any spaces from the book name and try with that too.
  std::string nospacebook = filter::strings::replace (" ", "", book);

  // Store all of the available IDs locally.
  std::vector <book_id> bookids = database::books::get_ids ();
  
  // Check on names entered like "Genesis" or "1 Corinthians", the full English name.
  // A bug was discovered so that "Judges" was interpreted as "Jude",
  // because of the three letters "Jud".
  // Solved by checking on full English name first.
  // In general, do exact matching first before moving on to similarity matching.
  // Compare with the translation to Bibledit's language too.
  for (auto identifier : bookids) {
    std::string english = database::books::get_english_from_id(identifier);
    if (english.empty()) continue;
    if (book == filter::strings::unicode_string_casefold(english)) return identifier;
    
    if (nospacebook == filter::strings::unicode_string_casefold(english)) return identifier;
    
    std::string localized = translate(english);
    if (localized.empty()) continue;
    
    if (book == filter::strings::unicode_string_casefold(localized)) return identifier;
    
    if (nospacebook == filter::strings::unicode_string_casefold(localized)) return identifier;
  }
  
  // Try the OSIS abbreviations.
  for (auto identifier : bookids) {
    std::string osis = database::books::get_osis_from_id(identifier);
    if (osis.empty()) continue;
    if (book == filter::strings::unicode_string_casefold(osis)) return identifier;
    if (nospacebook == filter::strings::unicode_string_casefold(osis)) return identifier;
    std::string localized = translate(osis);
    if (localized.empty()) continue;
    if (book == filter::strings::unicode_string_casefold(localized)) return identifier;
    if (nospacebook == filter::strings::unicode_string_casefold(localized)) return identifier;
  }
  
  // Try the abbreviations of BibleWorks.
  for (auto identifier : bookids) {
    std::string bibleworks = database::books::get_bibleworks_from_id(identifier);
    if (bibleworks.empty()) continue;
    if (book == filter::strings::unicode_string_casefold(bibleworks)) return identifier;
    if (nospacebook == filter::strings::unicode_string_casefold(bibleworks)) return identifier;
    std::string localized = translate(bibleworks);
    if (localized.empty()) continue;
    if (book == filter::strings::unicode_string_casefold(localized)) return identifier;
    if (nospacebook == filter::strings::unicode_string_casefold(localized)) return identifier;
  }
  
  // Try the abbreviations of the Online Bible.
  for (auto identifier : bookids) {
    std::string onlinebible = database::books::get_onlinebible_from_id(identifier);
    if (onlinebible.empty()) continue;
    if (book == filter::strings::unicode_string_casefold(onlinebible)) return identifier;
    if (nospacebook == filter::strings::unicode_string_casefold(onlinebible)) return identifier;
    std::string localized = translate(onlinebible);
    if (localized.empty()) continue;
    if (book == filter::strings::unicode_string_casefold(localized)) return identifier;
    if (nospacebook == filter::strings::unicode_string_casefold(localized)) return identifier;
  }
  
  // Do a case-insensitive search in the books database for something like the book given.
  {
    book_id identifier = database::books::get_id_like_text (book);
    if (identifier != book_id::_unknown) return identifier;
  }
  
  // Sorry, no book found.
  return book_id::_unknown;
}


std::string filter_passage_clean_passage (std::string text)
{
  // Trim text.
  text = filter::strings::trim (text);
  // As references could be, e.g.: Genesis 10.2, or Genesis 10:2,
  // it needs to convert a the full stop and the colon to a space.
  text = filter::strings::replace (".", " ", text);
  text = filter::strings::replace (":", " ", text);
  // Change double spaces into single ones.
  text = filter::strings::collapse_whitespace (text);
  // Trim again.
  text = filter::strings::trim (text);
  // Result.
  return text;
}


// Takes the passage in $text, and explodes it into book, chapter, verse.
// The book is the numerical identifier, not the string, e.g.,
// it would not return "Genesis", but identifier 1.
Passage filter_passage_explode_passage (std::string text)
{
  text = filter_passage_clean_passage (text);
  // Cut the text in its parts.
  std::vector <std::string> bits = filter::strings::explode (text, ' ');
  // Defaults to empty passage.
  Passage passage;
  // Take the bits.
  if (!bits.empty ()) {
    std::string verse = bits.back ();
    if (!verse.empty ()) passage.m_verse = verse;
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    std::string chapter = bits.back ();    
    if (!chapter.empty()) passage.m_chapter = filter::strings::convert_to_int (chapter);
    bits.pop_back ();
  }
  std::string book = filter::strings::implode (bits, " ");
  if (!book.empty()) {
    book_id bk = filter_passage_interpret_book_v2 (book);
    passage.m_book = static_cast<int>(bk);
  }
  // Return the result.
  return passage;
}


// Takes the passage in $rawPassage, and tries to interpret it.
// The following situations can occur:
// - Only book given, e.g. "Genesis".
// - One number given, e.g. "10".
// - Two numbers given, e.g. "1 2".
// - Book and one number given, e.g. "Exodus 10".
// - Book and two numbers given, e.g. "Song of Solomon 2 3".
// It deals with these situations.
// If needed, it bases the interpreted passage on $currentPassage.
Passage filter_passage_interpret_passage (Passage currentPassage, std::string rawPassage)
{
  rawPassage = filter_passage_clean_passage (rawPassage);

  // Create an array with the bits of the raw input.
  std::vector <std::string> input = filter::strings::explode (rawPassage, ' ');

  // Go through the array from verse to chapter to book.
  // Check how many numerals it has after the book part.
  std::vector <int> numerals;
  std::string book = "";
  std::vector <std::string> invertedInput (input.begin(), input.end ());
  reverse (invertedInput.begin (), invertedInput.end());
  for (std::string & bit : invertedInput) {
    int integer = filter::strings::convert_to_int (bit);
    if (bit == std::to_string (integer)) {
      numerals.push_back (integer);
      input.pop_back ();
    } else {
      book = filter::strings::implode (input, " ");
      break;
    }
  }

  // Deal with: Only book given, e.g. "Genesis".
  if ((book != "") && (numerals.size () == 0)) {
    return filter_passage_explode_passage (book + " 1 1");
  }

  // Deal with: One number given, e.g. "10".
  else if ((book == "") && (numerals.size () == 1)) {
    int bk = currentPassage.m_book;
    int chapter = currentPassage.m_chapter;
    std::string verse = std::to_string (numerals [0]);
    Passage passage = filter_passage_explode_passage ("Unknown " + std::to_string (chapter) + " " + verse);
    passage.m_book = bk;
    return passage;
  }

  // Deal with: Two numbers given, e.g. "1 2".
  else if ((book == "") && (numerals.size () == 2)) {
    int bk = currentPassage.m_book;
    int chapter = numerals [1];
    std::string verse = std::to_string (numerals [0]);
    Passage passage = filter_passage_explode_passage ("Unknown " + std::to_string (chapter) + " " + verse);
    passage.m_book = bk;
    return passage;
  }

  // Deal with: Book and one number given, e.g. "Exodus 10".
  else if ((book != "") && (numerals.size () == 1)) {
    int chapter = numerals [0];
    return filter_passage_explode_passage (book + " " + std::to_string (chapter) + " 1");
  }

  // Deal with: Book and two numbers given, e.g. "Song of Solomon 2 3".
  else if ((book != "") && (numerals.size () == 2)) {
    return filter_passage_explode_passage (rawPassage);
  }

  // Give up.
  return currentPassage;
}


// This deals with sequences and ranges of verses, like the following:
// Exod. 37:4-5, 14-15, 27-28
// It puts each verse on a separate line.
std::vector <std::string> filter_passage_handle_sequences_ranges (const std::string& passage)
{
  // A passage like Exod. 37:4-5, 14-15, 27-28 will be cut at the comma.
  // The resulting array contains the following:
  // Exod. 37:4-5
  // 14-15
  // 27-28
  // It implies that the first sequence has book and chapter.
  std::vector <std::string> sequences = filter::strings::explode (passage, ',');
  for (std::string & line : sequences) line = filter::strings::trim (line);


  // Store output lines.
  std::vector <std::string> output;

  // Cut the passages at the hyphen.
  for (unsigned int offset = 0; offset < sequences.size(); offset++) {
    std::string sequence = sequences [offset];
    std::vector <std::string> range = filter::strings::explode (sequence, '-');
    if (range.size () == 1) {
      output.push_back (filter::strings::trim (range [0]));
    } else {
      std::string start = filter::strings::trim (range [0]);
      output.push_back (start);
      if (offset == 0) {
        // Since the first bit contains book / chapter / verse,
        // extract the verse number.
        start = std::string (start.rbegin(), start.rend());
        start = std::to_string (filter::strings::convert_to_int (start));
        start = std::string (start.rbegin(), start.rend());
      }
      unsigned int end = static_cast <unsigned> (filter::strings::convert_to_int (filter::strings::trim (range [1])));
      for (size_t i = static_cast<size_t> (filter::strings::convert_to_int (start) + 1); i <= end; i++) {
        output.push_back (std::to_string (i));
      }
    }
  }

  // Result.
  return output;
}


void filter_passage_link_for_opening_editor_at (pugi::xml_node& node, int book, int chapter, std::string verse)
{
  const std::string display = filter_passage_display (book, chapter, verse);
  const Passage passage = Passage ("", book, chapter, verse);
  const std::string numeric = std::to_string (filter_passage_to_integer (passage));
  pugi::xml_node a_node = node.append_child ("a");
  a_node.append_attribute("class") = "starteditor";
  a_node.append_attribute("href") = "nothing";
  a_node.append_attribute("passage") = numeric.c_str();
  a_node.text().set(display.c_str());
  pugi::xml_node span_node = node.append_child("span");
  span_node.text().set(" ");
}


std::string filter_passage_link_for_opening_editor_at (int book, int chapter, std::string verse)
{
  pugi::xml_document document;
  filter_passage_link_for_opening_editor_at(document, book, chapter, verse);
  std::stringstream output;
  document.print (output, "", pugi::format_raw);
  return output.str ();
}


// A Bible has a standard order for the books, and it can have their books in a custom order.
// This function returns either the standard order, or a custom order in case it is available for the $bible.
std::vector <int> filter_passage_get_ordered_books (const std::string& bible)
{
  // The available books from the Bible.
  std::vector <int> projectbooks = database::bibles::get_books (bible);

  // The book order from the settings, if any.
  std::string s_orderedbooks = database::config::bible::get_book_order (bible);
  std::vector <std::string> vs_orderedbooks = filter::strings::explode (s_orderedbooks, ' ');

  // Keep books available in the Bible.
  std::vector <int> orderedbooks;
  for (std::string & book : vs_orderedbooks) {
    int bk = filter::strings::convert_to_int (book);
    if (find (projectbooks.begin(), projectbooks.end(), bk) != projectbooks.end()) {
      orderedbooks.push_back (bk);
    }
  }
  
  // Books in the Bible but not in the settings: Add them to the end.
  for (int book : projectbooks) {
    if (find (orderedbooks.begin(), orderedbooks.end(), book) == orderedbooks.end()) {
      orderedbooks.push_back (book);
    }
  }

  return orderedbooks;
}


