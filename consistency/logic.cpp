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


#include <consistency/logic.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/styles.h>
#include <database/volatile.h>
#include <database/config/bible.h>
#include <resource/logic.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <locale/translate.h>


Consistency_Logic::Consistency_Logic (void * webserver_request_in, int id_in)
{
  webserver_request = webserver_request_in;
  id = id_in;
}


string Consistency_Logic::response ()
{
  // The request.
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  // The resources to display in the Consistency tool.
  vector <string> resources = request->database_config_user()->getConsistencyResources ();
  string bible = access_bible_clamp (webserver_request, request->database_config_user()->getBible ());
  resources.insert (resources.begin (), bible);
  
  // The passages entered in the Consistency tool.
  string s_passages = Database_Volatile::getValue (id, "passages");
  s_passages = filter_string_trim (s_passages);
  vector <string> passages = filter_string_explode (s_passages, '\n');
  
  // The translations entered in the Consistency tool.
  string s_translations = Database_Volatile::getValue (id, "translations");
  s_translations = filter_string_trim (s_translations);
  vector <string> translations = filter_string_explode (s_translations, '\n');
  
  // Contains the response to display.
  vector <string> response;
  
  // Go through the passages interpreting them.
  Passage previousPassage = Passage ("", 1, 1, "1");
  for (auto line : passages) {
    
    // Clean line.
    line = filter_string_trim (line);
    
    // Skip empty line.
    if (line.empty ()) continue;
    
    // Remove verse text remaining with the passage(s) only.
    line = omit_verse_text (line);
    
    vector <string> range_sequence = filter_passage_handle_sequences_ranges (line);
    for (auto line : range_sequence) {
      Passage passage = filter_passage_interpret_passage (previousPassage, line);
      if (passage.book != 0) {
        int book = passage.book;
        int chapter = passage.chapter;
        string verse = passage.verse;
        line = filter_passage_link_for_opening_editor_at (book, chapter, verse);
        line += " ";
        
        // Check whether the chapter identifier has changed for this reference.
        // If so, set a flag so the data can be re-assembled for this verse.
        // If there was no change, then the data can be fetched from the volatile database.
        bool redoPassage = false;
        string passageKey = convert_to_string (book) + "." + convert_to_string (chapter) + "." + verse;
        int currentChapterId = request->database_bibles()->getChapterId (resources [0], book, chapter);
        int storedChapterId = convert_to_int (Database_Volatile::getValue (id, passageKey + ".id"));
        if (currentChapterId != storedChapterId) {
          Database_Volatile::setValue (id, passageKey + ".id", convert_to_string (currentChapterId));
          redoPassage = true;
        }
        
        // Go through each resource.
        for (auto resource : resources) {
          
          // Produce new verse text if the passage is to be redone, or else fetch the existing text.
          string text;
          if (redoPassage) {
            text = verseText (resource, book, chapter, convert_to_int (verse));
            size_t length1 = text.size ();
            if (!translations.empty ()) {
              text = filter_string_markup_words (translations, text);
            }
            size_t length2 = text.size ();
            if (length2 == length1) {
              text.insert (0, "<div style=\"background-color: yellow;\">");
              text.append ("</div>");
            }
            Database_Volatile::setValue (id, passageKey + "." + resource, text);
          } else {
            text = Database_Volatile::getValue (id, passageKey + "." + resource);
          }
          
          // Formatting.
          if (resources.size () > 1) {
            line += "<br>";
          }
          line += text;
        }
        response.push_back (line);
        previousPassage = passage;
      } else {
        response.push_back ("<span class=\"error\">" + translate("Unknown passage") + " " + line + "</span>");
      }
    }
  }
  
  string output;
  for (auto line : response) {
    output += "<div>" + line + "</div>\n";
  }
  return output;
}


string Consistency_Logic::verseText (string resource, int book, int chapter, int verse)
{
  return resource_logic_get_html (webserver_request, resource, book, chapter, verse, false);
}


// This function omits the verse text from a line of text from the search results.
string Consistency_Logic::omit_verse_text (string input)
{
  // Imagine the following $input:
  // 1 Peter 4:17 For the time has come for judgment to begin with the household of God. If it begins first with us, what will happen to those who don’t obey the Good News of God?
  // The purpose of this function is to extract "1 Peter 4:17" from it, and leave the rest out.
  // This is done by leaving out everything after the last numeral.
  size_t length = unicode_string_length (input);
  size_t last_numeral = 0;
  for (size_t i = 0; i < length; i++) {
    string character = unicode_string_substr (input, i, 1);
    if (filter_string_is_numeric (character)) {
      last_numeral = i;
    }
  }
  last_numeral++;
  input = unicode_string_substr (input, 0, last_numeral);
  return input;
}
