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


#include <checks/pairs.h>
#include <filter/string.h>
#include <database/bibles.h>
#include <database/check.h>


void Checks_Pairs::run (const string & bible, int book, int chapter,
                        const map <int, string> & texts,
                        const vector <pair <string, string> > & pairs)
{
  // This holds the opener characters of the pairs which were opened in the text.
  // For example, it may hold the "[".
  vector <int> verses;
  vector <string> opened;
  
  // Containers with the openers and the closers.
  vector <string> openers;
  vector <string> closers;
  for (auto & element : pairs) {
    openers.push_back (element.first);
    closers.push_back (element.second);
  }

  Database_Check database_check;

  // Go through the verses with their texts.
  for (auto & element : texts) {
    int verse = element.first;
    string text = element.second;
    size_t length = unicode_string_length (text);
    for (size_t pos = 0; pos < length; pos++) {
      
      string character = unicode_string_substr (text, pos, 1);
      
      if (in_array (character, openers)) {
        verses.push_back (verse);
        opened.push_back (character);
      }
      
      if (in_array (character, closers)) {
        
        string opener = match (character, pairs);
        bool mismatch = false;
        if (opened.empty ()) {
          mismatch = true;
        } else if (opened.back () == opener) {
          verses.pop_back ();
          opened.pop_back ();
        } else {
          mismatch = true;
        }
        if (mismatch) {
          database_check.recordOutput (bible, book, chapter, verse, "Closing character \"" + character + "\" without its matching opening character \"" + opener + "\"");
        }
      }
    }
  }
  
  // Report unclosed openers.
  for (size_t i = 0; i < verses.size (); i++) {
    int verse = verses [i];
    string opener = opened [i];
    string closer = match (opener, pairs);
    database_check.recordOutput (bible, book, chapter, verse, "Opening character \"" + opener + "\" without its matching closing character \"" + closer + "\"");
  }
}


string Checks_Pairs::match (const string & character, const vector <pair <string, string> > & pairs)
{
  for (auto & element : pairs) {
    if (character == element.first) return element.second;
    if (character == element.second) return element.first;
  }
  return "";
}
