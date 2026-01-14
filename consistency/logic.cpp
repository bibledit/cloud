/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <database/temporal.h>
#include <database/config/bible.h>
#include <resource/logic.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <locale/translate.h>


Consistency_Logic::Consistency_Logic (Webserver_Request& webserver_request, int id) :
m_webserver_request (webserver_request), m_id (id)
{
}


std::string Consistency_Logic::response ()
{
  // The resources to display in the Consistency tool.
  std::vector <std::string> resources = m_webserver_request.database_config_user()->get_consistency_resources ();
  std::string bible = access_bible::clamp (m_webserver_request, m_webserver_request.database_config_user()->get_bible ());
  resources.insert (resources.begin (), bible);
  
  // The passages entered in the Consistency tool.
  std::string s_passages = database::temporal::get_value (m_id, "passages");
  s_passages = filter::string::trim (s_passages);
  std::vector <std::string> passages = filter::string::explode (s_passages, '\n');
  
  // The translations entered in the Consistency tool.
  std::string s_translations = database::temporal::get_value (m_id, "translations");
  s_translations = filter::string::trim (s_translations);
  std::vector <std::string> translations = filter::string::explode (s_translations, '\n');
  
  // Contains the response to display.
  std::vector <std::string> response;
  
  // Go through the passages interpreting them.
  Passage previousPassage = Passage ("", 1, 1, "1");
  for (auto line : passages) {
    
    // Clean line.
    line = filter::string::trim (line);
    
    // Skip empty line.
    if (line.empty ()) continue;
    
    // Remove verse text remaining with the passage(s) only.
    line = omit_verse_text (line);
    
    std::vector <std::string> range_sequence = filter_passage_handle_sequences_ranges (line);
    for (auto line2 : range_sequence) {
      Passage passage = filter_passage_interpret_passage (previousPassage, line2);
      if (passage.m_book != 0) {
        int book = passage.m_book;
        int chapter = passage.m_chapter;
        std::string verse = passage.m_verse;
        line2 = filter_passage_link_for_opening_editor_at (book, chapter, verse);
        line2 += " ";
        
        // Check whether the chapter identifier has changed for this reference.
        // If so, set a flag so the data can be re-assembled for this verse.
        // If there was no change, then the data can be fetched from the volatile database.
        bool redoPassage = false;
        std::string passageKey = std::to_string (book) + "." + std::to_string (chapter) + "." + verse;
        int currentChapterId = database::bibles::get_chapter_id (resources [0], book, chapter);
        int storedChapterId = filter::string::convert_to_int (database::temporal::get_value (m_id, passageKey + ".id"));
        if (currentChapterId != storedChapterId) {
          database::temporal::set_value (m_id, passageKey + ".id", std::to_string (currentChapterId));
          redoPassage = true;
        }
        
        // Go through each resource.
        for (auto resource : resources) {
          
          // Produce new verse text if the passage is to be redone, or else fetch the existing text.
          std::string text;
          if (redoPassage) {
            text = verseText (resource, book, chapter, filter::string::convert_to_int (verse));
            size_t length1 = text.size ();
            if (!translations.empty ()) {
              text = filter::string::markup_words (translations, text);
            }
            size_t length2 = text.size ();
            if (length2 == length1) {
              text.insert (0, R"(<div style="background-color: yellow;">)");
              text.append ("</div>");
            }
            database::temporal::set_value (m_id, passageKey + "." + resource, text);
          } else {
            text = database::temporal::get_value (m_id, passageKey + "." + resource);
          }
          
          // Formatting.
          if (resources.size () > 1) {
            line2 += "<br>";
          }
          line2 += text;
        }
        response.push_back (line2);
        previousPassage = passage;
      } else {
        response.push_back (R"(<span class="error">)" + translate("Unknown passage") + " " + line2 + "</span>");
      }
    }
  }
  
  std::string output;
  for (auto line : response) {
    output += "<div>" + line + "</div>\n";
  }
  return output;
}


std::string Consistency_Logic::verseText (std::string resource, int book, int chapter, int verse)
{
  return resource_logic_get_html (m_webserver_request, resource, book, chapter, verse, false);
}


// This function omits the verse text from a line of text from the search results.
std::string Consistency_Logic::omit_verse_text (std::string input)
{
  // Imagine the following $input:
  // 1 Peter 4:17 For the time has come for judgment to begin with the household of God. If it begins first with us, what will happen to those who don’t obey the Good News of God?
  // The purpose of this function is to extract "1 Peter 4:17" from it, and leave the rest out.
  // This is done by leaving out everything after the last numeral.
  size_t length = filter::string::unicode_string_length (input);
  size_t last_numeral = 0;
  for (size_t i = 0; i < length; i++) {
    std::string character = filter::string::unicode_string_substr (input, i, 1);
    if (filter::string::is_numeric (character)) {
      last_numeral = i;
    }
  }
  last_numeral++;
  input = filter::string::unicode_string_substr (input, 0, last_numeral);
  return input;
}
