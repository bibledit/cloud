/*
Copyright (©) 2003-2016 Teus Benschop.

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


Passage::Passage ()
{
  book = 0;
  chapter = 0;
}


Passage::Passage (string bible_in, int book_in, int chapter_in, string verse_in)
{
  bible = bible_in;
  book = book_in;
  chapter = chapter_in;
  verse = verse_in;
}


bool Passage::equal (Passage & passage)
{
  if (bible != passage.bible) return false;
  if (book != passage.book) return false;
  if (chapter != passage.chapter) return false;
  if (verse != passage.verse) return false;
  return true;
}


// This method converts the passage of the object into text, like e.g. so:
// "hexadecimal Bible _1_2_3".
// First the hexadecimal Bible comes, then the book identifier, then the chapter number, and finally the verse number.
string Passage::encode ()
{
  string text;
  // The encoded passage can be used as an attribute in the HTML DOM.
  // Therefore it will be encoded such that any Bible name will be acceptable as an attribute in the DOM.
  text.append (bin2hex (bible));
  text.append ("_");
  text.append (convert_to_string (book));
  text.append ("_");
  text.append (convert_to_string (chapter));
  text.append ("_");
  text.append (verse);
  if (verse.empty()) text.append ("0");
  return text;
}


// This method converts encoded text into a passage.
// The text is in the format as its complementary function, "encode", produces.
Passage Passage::decode (const string& encoded)
{
  Passage passage;
  vector <string> bits = filter_string_explode (encoded, '_');
  if (!bits.empty ()) {
    string verse = bits.back ();
    if (!verse.empty ()) passage.verse = verse;
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    string chapter = bits.back ();
    if (!chapter.empty()) passage.chapter = convert_to_int (chapter);
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    string book = bits.back ();
    if (!book.empty()) passage.book = convert_to_int (book);
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    passage.bible = hex2bin (bits.back ());
    bits.pop_back ();
  }
  return passage;
}


string filter_passage_display (int book, int chapter, string verse)
{
  string display;
  display.append (translate (Database_Books::getEnglishFromId (book).c_str()));
  display.append (" ");
  display.append (convert_to_string (chapter));
  if (!verse.empty ()) display.append (":" + verse);
  return display;
}


// Returns the display string for the $passages as one line.
string filter_passage_display_inline (vector <Passage> passages)
{
  string display;
  for (Passage & passage : passages) {
    if (display != "") display.append (" | ");
    display.append (filter_passage_display (passage.book, passage.chapter, passage.verse));
  }
  return display;
}



// Returns the display string for the $passages as several lines.
string filter_passage_display_multiline (vector <Passage> passages)
{
  string display;
  for (Passage & passage : passages) {
    display.append (filter_passage_display (passage.book, passage.chapter, passage.verse));
    display.append ("\n");
  }
  return display;
}


// This function converts $passage to an integer, so that passages can be compared or stored.
int filter_passage_to_integer (Passage passage)
{
  return 1000000 * passage.book + 1000 * passage.chapter + convert_to_int (passage.verse);
}


// This converts the $integer, created above, to a passage.
Passage filter_integer_to_passage (int integer)
{
  int book = round (integer / 1000000);
  integer -= book * 1000000;
  int chapter = round (integer / 1000);
  integer -= chapter * 1000;
  string verse = convert_to_string (integer);
  return Passage ("", book, chapter, verse);
}


// This filter takes $books as a string,
// and looks whether it can be interpreted as a valid book in any way.
// It returns a valid book identifier, or 0 in case no book could be interpreted.
int filter_passage_interpret_book (string book)
{
  book = filter_string_trim (book);
  int identifier = 0;

  // Recognize names like "I Peter", where the "I" can also be "II" or "III".
  // Do the longest ones first.
  book = filter_string_str_replace ("III ", "3 ", book);
  book = filter_string_str_replace ("II ", "2 ", book);
  book = filter_string_str_replace ("I ", "1 ", book);

  // Check on names entered like "Genesis" or "1 Corinthians", the full English name.
  // A bug was discovered so that "Judges" was interpreted as "Jude", because
  // of the three letters "Jud". Solved by checking on full English name first.
  identifier = Database_Books::getIdFromEnglish (book);
  if (identifier) {
    return identifier;
  }

  // Recognise the USFM book abbreviations.
  identifier = Database_Books::getIdFromUsfm (book);
  if (identifier) return identifier;

  // Try the OSIS abbreviations.
  identifier = Database_Books::getIdFromOsis (book);
  if (identifier) return identifier;

  // Try the abbreviations of BibleWorks.
  identifier = Database_Books::getIdFromBibleworks (book);
  if (identifier) return identifier;

  // Handle names from BibleWorks when copying the verse list to the clipboard.
  // These are not handled elsewhere.
  if (book == "Cant") return 22;
  if (book == "Mk") return 41;
  if (book == "Lk") return 42;
  if (book == "Jn") return 43;
  if (book == "1 Jn") return 62;
  if (book == "2 Jn") return 63;
  if (book == "3 Jn") return 64;

  // Try the abbreviations of the Online Bible.
  identifier = Database_Books::getIdFromOnlinebible (book);
  if (identifier) return identifier;

  // Do a case-insensitive search in the books database for something like the book given.
  identifier = Database_Books::getIdLikeText (book);
  if (identifier) return identifier;
  
  // Remove any spaces from the book name, and try again.
  book = filter_string_str_replace (" ", "", book);
  identifier = Database_Books::getIdFromOsis (book);
  if (identifier) return identifier;
  identifier = Database_Books::getIdFromBibleworks (book);
  if (identifier) return identifier;
  identifier = Database_Books::getIdFromOnlinebible (book);
  if (identifier) return identifier;
  identifier = Database_Books::getIdLikeText (book);
  if (identifier) return identifier;
  
  // Last effort to find a book.
  identifier = Database_Books::getIdLastEffort (book);
  if (identifier) return identifier;

  // Sorry, no book found.
  return 0;
}


string filter_passage_clean_passage (string text)
{
  // Trim text.
  text = filter_string_trim (text);
  // As references could be, e.g.: Genesis 10.2, or Genesis 10:2,
  // it needs to convert a the full stop and the colon to a space.
  text = filter_string_str_replace (".", " ", text);
  text = filter_string_str_replace (":", " ", text);
  // Change double spaces into single ones.
  text = filter_string_str_replace ("  ", " ", text);
  text = filter_string_str_replace ("  ", " ", text);
  // Trim again.
  text = filter_string_trim (text);
  // Result.
  return text;
}


// Takes the passage in $text, and explodes it into book, chapter, verse.
// The book is the numerical identifier, not the string, e.g.,
// it would not return "Genesis", but identifier 1.
Passage filter_passage_explode_passage (string text)
{
  text = filter_passage_clean_passage (text);
  // Cut the text in its parts.
  vector <string> bits = filter_string_explode (text, ' ');
  // Defaults to empty passage.
  Passage passage;
  // Take the bits.
  if (!bits.empty ()) {
    string verse = bits.back ();
    if (!verse.empty ()) passage.verse = verse;
    bits.pop_back ();
  }
  if (!bits.empty ()) {
    string chapter = bits.back ();    
    if (!chapter.empty()) passage.chapter = convert_to_int (chapter);
    bits.pop_back ();
  }
  string book = filter_string_implode (bits, " ");
  if (!book.empty()) {
    int bk = filter_passage_interpret_book (book);
    passage.book = bk;
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
Passage filter_passage_interpret_passage (Passage currentPassage, string rawPassage)
{
  rawPassage = filter_passage_clean_passage (rawPassage);

  // Create an array with the bits of the raw input.
  vector <string> input = filter_string_explode (rawPassage, ' ');

  // Go through the array from verse to chapter to book.
  // Check how many numerals it has after the book part.
  vector <int> numerals;
  string book = "";
  vector <string> invertedInput (input.begin(), input.end ());
  reverse (invertedInput.begin (), invertedInput.end());
  for (string & bit : invertedInput) {
    int integer = convert_to_int (bit);
    if (bit == convert_to_string (integer)) {
      numerals.push_back (integer);
      input.pop_back ();
    } else {
      book = filter_string_implode (input, " ");
      break;
    }
  }

  // Deal with: Only book given, e.g. "Genesis".
  if ((book != "") && (numerals.size () == 0)) {
    return filter_passage_explode_passage (book + " 1 1");
  }

  // Deal with: One number given, e.g. "10".
  else if ((book == "") && (numerals.size () == 1)) {
    int bk = currentPassage.book;
    int chapter = currentPassage.chapter;
    string verse = convert_to_string (numerals [0]);
    Passage passage = filter_passage_explode_passage ("Unknown " + convert_to_string (chapter) + " " + verse);
    passage.book = bk;
    return passage;
  }

  // Deal with: Two numbers given, e.g. "1 2".
  else if ((book == "") && (numerals.size () == 2)) {
    int bk = currentPassage.book;
    int chapter = numerals [1];
    string verse = convert_to_string (numerals [0]);
    Passage passage = filter_passage_explode_passage ("Unknown " + convert_to_string (chapter) + " " + verse);
    passage.book = bk;
    return passage;
  }

  // Deal with: Book and one number given, e.g. "Exodus 10".
  else if ((book != "") && (numerals.size () == 1)) {
    int chapter = numerals [0];
    return filter_passage_explode_passage (book + " " + convert_to_string (chapter) + " 1");
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
vector <string> filter_passage_handle_sequences_ranges (const string& passage)
{
  // A passage like Exod. 37:4-5, 14-15, 27-28 will be cut at the comma.
  // The resulting array contains the following:
  // Exod. 37:4-5
  // 14-15
  // 27-28
  // It implies that the first sequence has book and chapter.
  vector <string> sequences = filter_string_explode (passage, ',');
  for (string & line : sequences) line = filter_string_trim (line);


  // Store output lines.
  vector <string> output;

  // Cut the passages at the hyphen.
  for (unsigned int offset = 0; offset < sequences.size(); offset++) {
    string sequence = sequences [offset];
    vector <string> range = filter_string_explode (sequence, '-');
    if (range.size () == 1) {
      output.push_back (filter_string_trim (range [0]));
    } else {
      string start = filter_string_trim (range [0]);
      output.push_back (start);
      if (offset == 0) {
        // Since the first bit contains book / chapter / verse,
        // extract the verse number.
        start = string (start.rbegin(), start.rend());
        start = convert_to_string (convert_to_int (start));
        start = string (start.rbegin(), start.rend());
      }
      unsigned int end = convert_to_int (filter_string_trim (range [1]));
      for (size_t i = convert_to_int (start) + 1; i <= end; i++) {
        output.push_back (convert_to_string (i));
      }
    }
  }

  // Result.
  return output;
}


string filter_passage_link_for_opening_editor_at (int book, int chapter, string verse)
{
  string display = filter_passage_display (book, chapter, verse);
  Passage passage = Passage ("", book, chapter, verse);
  string numeric = convert_to_string (filter_passage_to_integer (passage));
  string link = "<a class=\"starteditor\" href=\"" + numeric + "\">" + display + "</a> <span></span>";
  return link;
}


// A Bible has a standard order for the books, and it can have their books in a custom order.
// This function returns either the standard order, or a custom order in case it is available for the $bible.
vector <int> filter_passage_get_ordered_books (const string& bible)
{
  Database_Bibles database_bibles;

  // The available books from the Bible.
  vector <int> projectbooks = database_bibles.getBooks (bible);

  // The book order from the settings, if any.
  string s_orderedbooks = Database_Config_Bible::getBookOrder (bible);
  vector <string> vs_orderedbooks = filter_string_explode (s_orderedbooks, ' ');

  // Keep books available in the Bible.
  vector <int> orderedbooks;
  for (string & book : vs_orderedbooks) {
    int bk = convert_to_int (book);
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


