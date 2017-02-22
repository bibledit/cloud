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


#include <manage/hyphenate.h>
#include <filter/string.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/users.h>
#include <database/books.h>
#include <database/privileges.h>
#include <database/config/bible.h>
#include <webserver/request.h>


void manage_hyphenate (string bible, string user)
{
  Database_Bibles database_bibles;


  string inputBible (bible);
  string outputBible = inputBible + "-hyphenated";
  
  
  Database_Logs::log ("Reading Bible " + inputBible + ", adding soft hyphens, putting it into Bible " + outputBible);
  
  
  // Get the two sets of characters as arrays.
  // The /u switch treats the text as UTF8 Unicode.
  vector <string> firstset;
  string s_firstset = Database_Config_Bible::getHyphenationFirstSet (inputBible);
  size_t length = unicode_string_length (s_firstset);
  for (size_t i = 0; i < length; i++) {
    string s = unicode_string_substr (s_firstset, i, 1);
    if (s == " ") continue;
    firstset.push_back (s);
  }
  vector <string> secondset;
  string s_secondset = Database_Config_Bible::getHyphenationSecondSet (inputBible);
  length = unicode_string_length (s_secondset);
  for (size_t i = 0; i < length; i++) {
    string s = unicode_string_substr (s_secondset, i, 1);
    if (s == " ") continue;
    secondset.push_back (s);
  }
 
  
  // Delete and (re)create the hyphenated Bible.
  database_bibles.deleteBible (outputBible);
  Database_Privileges::removeBible (outputBible);
  Database_Config_Bible::remove (outputBible);
  database_bibles.createBible (outputBible);
  Webserver_Request webserver_request;
  if (!access_bible_write (&webserver_request, outputBible, user)) {
    // Only grant access if the user does not yet have it.
    // This avoid assigning the Bible to the user in case no Bible was assigned to anyone,
    // in which case assigning this Bible to the user would possible withdraw privileges from other users.
    Database_Privileges::setBible (user, outputBible, true);
  }
  
  
  // Go through the input Bible's books and chapters.
  vector <int> books = database_bibles.getBooks (inputBible);
  for (auto book : books) {
    Database_Logs::log (Database_Books::getEnglishFromId (book));
    vector <int> chapters = database_bibles.getChapters (inputBible, book);
    for (auto chapter : chapters) {
      string data = database_bibles.getChapter (inputBible, book, chapter);
      data = hyphenate_at_transition (firstset, secondset, data);
      database_bibles.storeChapter (outputBible, book, chapter, data);
    }
  }

  
  Database_Logs::log ("The Bible has been hyphenated");
}


/**
 * This filter inserts soft hyphens in text.
 * It goes through $text character by character.
 * At the transition from any character in $firstset
 * to any character in $secondset, it inserts a soft hyphen.
 * $firstset: vector of characters.
 * $secondset: vector of characters.
 * $text: A string of text to operate on.
 * Returns: The hyphenated text.
 */
string hyphenate_at_transition (vector <string>& firstset, vector <string>& secondset, string text)
{
  // Verify the input.
  if (firstset.empty ()) return text;
  if (secondset.empty ()) return text;
  if (text.empty ()) return text;
  
  // Split the text up into lines and go through each one.
  vector <string> lines = filter_string_explode (text, '\n');
  for (string & line : lines) {
    
    // Split the line up into an array of UTF8 Unicode characters.
    vector <string> characters;
    size_t length = unicode_string_length (line);
    for (size_t i = 0; i < length; i++) {
      string s = unicode_string_substr (line, i, 1);
      characters.push_back (s);
    }
    
    // Processor flags.
    bool previousCharacterWasRelevant = false;
    bool thisCharacterIsRelevant = false;
    bool isUsfm = false;
    
    // Process each character.
    for (unsigned int i = 0; i < characters.size (); i++) {
      
      string character = characters [i];
      
      // Skip USFM marker.
      if (character == "\\") isUsfm = true;
      
      if (!isUsfm) {
        
        // Check whether to insert the soft hyphen here.
        thisCharacterIsRelevant = in_array (character, secondset);
        if ((thisCharacterIsRelevant) && (previousCharacterWasRelevant)) {
          if (!hyphenate_is_near_white_space (characters, i)) {
            characters[i] = soft_hyphen () + character;
          }
        }
        
        // Flag for next iteration.
        previousCharacterWasRelevant = in_array (character, firstset);
      }
      
      if (isUsfm) {
        // Look for the end of the USFM marker.
        if (character == " ") isUsfm = false;
        if (character == "*") isUsfm = false;
      }
    }
    
    // Re-assemble the line from the separate (updated) characters.
    line = filter_string_implode (characters, "");
    
  }
  
  // Assemble the hyphenated text from the separate lines.
  text = filter_string_implode (lines, "\n");
  return text;
}


/**
 * This filter looks whether the offset is near whitespace
 * in the vector of characters.
 * Returns: true or false.
 */
bool hyphenate_is_near_white_space (const vector <string> & characters, int offset)
{
  int start = offset - 2; // The constant for the nearness to the start of the word.
  unsigned int end = offset + 2; // The constant for the nearness to the end of the word.
  if (start < 0) start = 0;
  if (end > characters.size()) end = characters.size();
  for (unsigned int i = start; i < end; i++) {
    if (characters[i] == " ") return true;
  }
  return false;
}


// Another method for hyphenation could be to rely on the routines as used by TeX.
// TeX has hyphenation patterns.
// The 'soul' package can show possible word breaks.
