/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <checks/sentences.h>
#include <webserver/request.h>
#include <filter/string.h>
#include <locale/translate.h>
using namespace std;


void Checks_Sentences::enter_capitals (const string & capitals)
{
  m_capitals = filter_string_explode (capitals, ' ');
}


void Checks_Sentences::enter_small_letters (const string & small_letters)
{
  m_small_letters = filter_string_explode (small_letters, ' ');
}


void Checks_Sentences::enter_end_marks (const string & end_marks)
{
  m_end_marks = filter_string_explode (end_marks, ' ');
}


void Checks_Sentences::enter_center_marks (const string & center_marks)
{
  m_center_marks = filter_string_explode (center_marks, ' ');
}


void Checks_Sentences::enter_disregards (const string & disregards)
{
  m_disregards = filter_string_explode (disregards, ' ');
}


void Checks_Sentences::enter_names (string names)
{
  m_names.clear ();
  names = filter_string_str_replace ("\n", " ", names);
  vector <string> names2 = filter_string_explode (names, ' ');
  for (auto name : names2) {
    if (!name.empty()) {
      // Limit the length to the left of the suffix in the test.
      name = unicode_string_substr (name, 0, 11);
      m_names.push_back (name);
    }
  }
}


void Checks_Sentences::initialize ()
{
  current_position = 0;
  space_position = 0;
  capital_position = 0;
  small_letter_position = 0;
  end_mark_position = 0;
  center_mark_position = 0;
  punctuation_mark_position = 0;
  previous_mark_position = 0;
  checking_results.clear ();
  full_text.clear ();
}


void Checks_Sentences::check (const map <int, string> & texts)
{
  vector <int> verse_numbers {};
  vector <string> characters {};
  int iterations {0};
  for (const auto & element : texts) {
    int verse = element.first;
    string text = element.second;
    // For the second and subsequent verse_numbers, add a space to the text,
    // because this is what is supposed to happen in USFM.
    if (iterations > 0) {
      verse_numbers.push_back (verse);
      characters.push_back (" ");
      full_text += " ";
    }
    // Split the UTF-8 text into characters and add them to the arrays of verse_numbers / characters.
    size_t count = unicode_string_length (text);
    for (size_t i = 0; i < count; i++) {
      character = unicode_string_substr (text, i, 1);
      // Skip characters to be disregarded.
      if (in_array (character, m_disregards)) continue;
      // Store verse numbers and characters.
      verse_numbers.push_back (verse);
      characters.push_back (character);
      full_text += character;
    }
    // Next iteration.
    iterations++;
  }
  
  // Go through the characters.
  for (size_t i = 0; i < characters.size (); i++) {
    // Store current verse number in the object.
    verse_number = verse_numbers [i];
    // Get the current character.
    character = characters [i];
    // Analyze the character.
    analyze_characters ();
    // Run the checks.
    check_unknown_character ();
    check_character ();
  }
}


void Checks_Sentences::check_character ()
{
  // Handle a capital after a comma: ... He said, Go ...
  if (this->is_capital)
    if (this->space_position > 0)
      if (this->current_position == this->space_position + 1)
        if (this->current_position == this->center_mark_position + 2)
          this->add_result (translate ("Capital follows mid-sentence punctuation mark"), Checks_Sentences::skip_names);
  
  
  // Handle a small letter straight after mid-sentence punctuation: ... He said,go ...
  if (this->is_small_letter)
    if (this->center_mark_position > 0)
      if (this->current_position == this->center_mark_position + 1)
        this->add_result (translate ("Small letter follows straight after a mid-sentence punctuation mark"), Checks_Sentences::display_context);
  
  
  // Handle a capital straight after mid-sentence punctuation: ... He said,Go ...
  if (this->is_capital)
    if (this->center_mark_position > 0)
      if (this->current_position == this->center_mark_position + 1)
        this->add_result (translate ("Capital follows straight after a mid-sentence punctuation mark"), Checks_Sentences::display_context);
  
  
  // Handle small letter or capital straight after end-sentence punctuation: He said.Go. // He said.go.
  if ((this->is_small_letter) || (this->is_capital))
    if (this->end_mark_position > 0)
      if (this->current_position == this->end_mark_position + 1)
        this->add_result (translate ("A letter follows straight after an end-sentence punctuation mark"), Checks_Sentences::display_context);
  
  
  // Handle case of no capital after end-sentence punctuation: He did that. he went.
  if (this->is_small_letter)
    if (this->end_mark_position > 0)
      if (this->current_position == this->end_mark_position + 2)
        this->add_result (translate ("No capital after an end-sentence punctuation mark"), Checks_Sentences::display_context);
  
  
  // Handle two punctuation marks in sequence.
  if (this->is_end_mark || this->is_center_mark)
    if (this->current_position == this->previous_mark_position + 1)
      this->add_result (translate ("Two punctuation marks in sequence"), Checks_Sentences::display_context);
  
}


// Checks paragraphs of text whether they are start and end with correct capitalization and punctuation.
// $paragraph_start_markers: The USFM markers that started the new paragraphs.
// $within_sentence_paragraph_markers:
// The USFM markers that start paragraphs that do not need to start with the correct capitalization.
// Usually such markers are poetic markers like \q1 and so on.
// $verses_paragraphs: The entire paragraphs, with verse number as their keys.
void Checks_Sentences::paragraphs (const vector <string> & paragraph_start_markers,
                                   const vector <string> & within_sentence_paragraph_markers,
                                   const vector <map <int, string>> & verses_paragraphs)
{
  // Iterate over the paragraphs.
  for (unsigned int p = 0; p < verses_paragraphs.size (); p++) {
    
    // Container with verse numbers and the whole paragraph.
    const map <int, string> & verses_paragraph = verses_paragraphs [p];
    
    // Skip empty container.
    if (verses_paragraph.empty ()) continue;

    // Get the first character of the paragraph.
    int verse = verses_paragraph.begin()->first;
    string character2 = verses_paragraph.begin()->second;
    if (!character2.empty ()) {
      character2 = unicode_string_substr (character2, 0, 1);
    }
    
    // Check that the paragraph starts with a capital.
    is_capital = in_array (character2, m_capitals);
    if (!is_capital) {
      const string & paragraph_marker = paragraph_start_markers [p];
      if (!in_array (paragraph_marker, within_sentence_paragraph_markers)) {
        string context = verses_paragraph.begin()->second;
        context = unicode_string_substr (context, 0, 15);
        checking_results.push_back (pair (verse, translate ("Paragraph does not start with a capital:") + " " + context));
      }
    }
    
    // Get the last two characters of the paragraph.
    verse = verses_paragraph.rbegin()->first;
    character2 = verses_paragraph.rbegin()->second;
    if (!character2.empty ()) {
      size_t length = unicode_string_length (character2);
      character2 = unicode_string_substr (character2, length - 1, 1);
    }
    string previous_character = verses_paragraph.rbegin()->second;
    if (!previous_character.empty ()) {
      size_t length = unicode_string_length (character2);
      if (length >= 2) {
        previous_character = unicode_string_substr (previous_character, length - 2, 1);
      } else {
        previous_character.clear ();
      }
    }
    
    // Check that the paragraph ends with correct punctuation.
    is_end_mark = in_array (character2, this->m_end_marks) || in_array (previous_character, this->m_end_marks);
    if (!is_end_mark) {
      // If the next paragraph starts with a marker that indicates it should not necessarily be capitalized,
      // then the current paragraph may have punctuation that would be incorrect otherwise.
      string next_paragraph_marker {};
      size_t p2 = p + 1;
      if (p2 < paragraph_start_markers.size ()) {
        next_paragraph_marker = paragraph_start_markers [p2];
      }
      if (next_paragraph_marker.empty () || (!in_array (next_paragraph_marker, within_sentence_paragraph_markers))) {
        string context = verses_paragraph.rbegin()->second;
        const size_t length = unicode_string_length (character2);
        if (length >= 15) {
          context = unicode_string_substr (context, length - 15, 15);
        }
        checking_results.push_back (pair (verse, translate ("Paragraph does not end with an end marker:") + " " + context));
      }
    }
    
  }
}


vector <pair<int, string>> Checks_Sentences::get_results ()
{
  return checking_results;
}


void Checks_Sentences::add_result (string text, int modifier)
{
  // Get previous and next text fragment.
  int start = current_position - 25;
  if (start < 0) start = 0;
  string previousFragment = unicode_string_substr (full_text, static_cast <size_t> (start), static_cast <size_t> (current_position - start - 1));
  int iterations {5};
  while (iterations) {
    const size_t pos = previousFragment.find (" ");
    if (pos != string::npos) {
      if ((previousFragment.length () - pos) > 10) {
        previousFragment.erase (0, pos + 1);
      }
    }
    iterations--;
  }
  string nextFragment = unicode_string_substr (full_text, static_cast <size_t> (current_position), 25);
  while (nextFragment.length () > 10) {
    const size_t pos = nextFragment.rfind (" ");
    if (pos == string::npos) nextFragment.erase (nextFragment.length () - 1, 1);
    else nextFragment.erase (pos);
  }
  // Check whether the result can be skipped due to a name being involved.
  if (modifier == skip_names) {
    string haystack = character + nextFragment;
    for (auto name : m_names) {
      if (haystack.find (name) == 0) return;
    }
  }
  // Assemble text for checking result.
  if (modifier == display_character_only) {
    text += ": " + character;
  }
  if ((modifier == display_context) || (modifier == skip_names)) {
    text += ": " + previousFragment + character + nextFragment;
  }
  // Store checking result.
  checking_results.push_back (pair (verse_number, text));
}


void Checks_Sentences::check_unknown_character ()
{
  if (is_space) return;
  if (is_capital) return;
  if (is_small_letter) return;
  if (is_end_mark) return;
  if (is_center_mark) return;
  add_result (translate ("Unknown character"), Checks_Sentences::display_character_only);
}


void Checks_Sentences::analyze_characters ()
{
  current_position++;
  
  is_space = (character == " ");
  if (is_space) {
    space_position = current_position;
  }
  
  is_capital = in_array (character, m_capitals);
  if (is_capital) {
    capital_position = current_position;
  }
  
  is_small_letter = in_array (character, m_small_letters);
  if (is_small_letter) {
    small_letter_position = current_position;
  }
  
  is_end_mark = in_array (character, m_end_marks);
  if (is_end_mark) {
    end_mark_position = current_position;
    previous_mark_position = punctuation_mark_position;
    punctuation_mark_position = current_position;
  }
  
  is_center_mark = in_array (character, m_center_marks);
  if (is_center_mark) {
    center_mark_position = current_position;
    previous_mark_position = punctuation_mark_position;
    punctuation_mark_position = current_position;
  }
}
