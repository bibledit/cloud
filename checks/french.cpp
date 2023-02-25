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


#include <checks/french.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <locale/translate.h>
using namespace std;


// In French there is a no-break space after the « and before the » ! ? : ;
// The Unicode value for the no-break space is U+00A0.
// Another type of non-breaking space will be acceptable too.
void checks_french::space_before_after_punctuation (const string & bible, int book, int chapter,
                                                    const map <int, string> & texts)
{
  Database_Check database_check {};
  string nbsp = non_breaking_space_u00A0 ();
  string nnbsp = narrow_non_breaking_space_u202F ();
  vector <string> right_punctuation = { right_guillemet(), "!", "?", ":", ";" };
  for (const auto & element : texts) {
    int verse = element.first;

    {
      string text = element.second;
      size_t pos = text.find (left_guillemet ());
      while (pos != string::npos) {
        text.erase (0, pos + left_guillemet ().size ());
        bool space_follows = text.find (" ") == 0;
        bool nbsp_follows = text.find (nbsp) == 0;
        bool nnbsp_follows = text.find (nnbsp) == 0;
        if (space_follows) {
          string message = left_guillemet () + " - " + translate ("Should be followed by a no-break space rather than a plain space in French");
          database_check.recordOutput (bible, book, chapter, verse, message);
        } else if (!nbsp_follows && !nnbsp_follows)  {
          string message = left_guillemet () + " - " + translate ("Should be followed by a no-break space in French");
          database_check.recordOutput (bible, book, chapter, verse, message);
        }
        pos = text.find (left_guillemet ());
      }
    }
    
    for (const auto & punctuation : right_punctuation) {
      string text = element.second;
      // The location of this punctuation character.
      size_t pos = unicode_string_strpos (text, punctuation);
      while (pos != string::npos) {
        if (pos > 0) {
          string preceding_character = unicode_string_substr (text, pos - 1, 1);
          if (preceding_character == " ") {
            string message = punctuation + " - " + translate ("Should be preceded by a no-break space rather than a plain space in French");
            database_check.recordOutput (bible, book, chapter, verse, message);
          }
          else if (preceding_character == nbsp) { /* This is OK. */ }
          else if (preceding_character == nnbsp) { /* This is OK. */ }
          else {
            string message = punctuation + " - " + translate ("Should be preceded by a no-break space in French");
            database_check.recordOutput (bible, book, chapter, verse, message);
          }
        }
        // Prepare for next iteration.
        text = unicode_string_substr (text, pos + 1, unicode_string_length (text) - pos - 1);
        pos = unicode_string_strpos (text, punctuation);
      }
    }
    
  }
}


// In French, if a citation starts with "«", all subsequent paragraphs within that citation, may begin with a new «.
// Example:
// « This is the text of the citation.
// « This is a new paragraph, and it ends the citation ».
// This checks on that style.
void checks_french::citation_style (const string & bible, int book, int chapter,
                                    const vector <map <int, string>> & verses_paragraphs)
{
  Database_Check database_check {};

  // Store the state of the previous paragraph.
  // It indicates whether any citation was left open at the end of the paragraph.
  bool previous_paragraph_open_citation {false};
  
  // Iterate over the paragraphs.
  for (unsigned int paragraph_counter = 0; paragraph_counter < verses_paragraphs.size (); paragraph_counter++) {
    
    // Container with verse numbers as the keys, plus the text of the whole paragraph.
    const map <int, string> verses_paragraph = verses_paragraphs [paragraph_counter];
    
    // Skip empty containers.
    if (verses_paragraph.empty ()) continue;

    // If this the first paragraph in the chapter, leave it as it is.
    // If it is not the first paragraph, and if the previous paragraph left an open citation,
    // this new paragraph should start with the French citation marker.
    if (paragraph_counter) {
      if (previous_paragraph_open_citation) {
        int verse = verses_paragraph.begin()->first;
        string text = verses_paragraph.begin()->second;
        if (!text.empty ()) {
          string character = unicode_string_substr (text, 0, 1);
          if (character != left_guillemet ()) {
            string message = translate ("The previous paragraph contains a citation not closed with a » therefore the current paragraph is expected to start with a « to continue that citation in French");
            database_check.recordOutput (bible, book, chapter, verse, message);
          }
        }
      }
    }

    // Count the number of left and right guillements in the paragraph.
    int last_verse {0};
    string paragraph {};
    for (const auto & element : verses_paragraph) {
      paragraph.append (element.second);
      last_verse = element.first;
    }
    int opener_count {0};
    filter_string_str_replace (left_guillemet (), "", paragraph, &opener_count);
    int closer_count {0};
    filter_string_str_replace (right_guillemet (), "", paragraph, &closer_count);

    // Determine whether the current paragraph opens a citation and does not close it.
    previous_paragraph_open_citation = (opener_count > closer_count);
    
    // Whether there's too many left guillements.
    if (opener_count > (closer_count + 1)) {
      string message = translate ("The paragraph contains more left guillements than needed");
      database_check.recordOutput (bible, book, chapter, last_verse, message);
    }
    
    // Whether there's too many right guillements.
    if (closer_count > opener_count) {
      string message = translate ("The paragraph contains more right guillements than needed");
      database_check.recordOutput (bible, book, chapter, last_verse, message);
    }
  }
}


string checks_french::left_guillemet ()
{
  return "«";
}


string checks_french::right_guillemet ()
{
  return "»";
}
