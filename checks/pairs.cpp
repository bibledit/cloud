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


#include <checks/pairs.h>
#include <filter/string.h>
#include <database/bibles.h>
#include <database/check.h>
#include <locale/translate.h>
#include <checks/french.h>
#include <checks/issues.h>


void checks_pairs::run (const std::string& bible, int book, int chapter,
                        const std::map <int, std::string> & texts,
                        const std::vector <std::pair <std::string, std::string>>& pairs,
                        bool french_citation_style)
{
  // This holds the opener characters of the pairs which were opened in the text.
  // For example, it may hold the "[".
  std::vector <int> verses {};
  std::vector <std::string> opened {};
  
  // Containers with the openers and the closers.
  // If the check on the French citation style is active,
  // skip the French guillemets.
  std::vector <std::string> openers {};
  std::vector <std::string> closers {};
  for (const auto& element : pairs) {
    const std::string opener = element.first;
    if (french_citation_style && (opener == checks_french::left_guillemet ())) continue;
    const std::string closer = element.second;
    if (french_citation_style && (opener == checks_french::right_guillemet ())) continue;
    openers.push_back (opener);
    closers.push_back (closer);
  }

  // Go through the verses with their texts.
  for (const auto & element : texts) {
    int verse = element.first;
    std::string text = element.second;
    size_t length = filter::strings::unicode_string_length (text);
    for (size_t pos = 0; pos < length; pos++) {
      
      const std::string character = filter::strings::unicode_string_substr (text, pos, 1);
      
      if (in_array (character, openers)) {
        verses.push_back (verse);
        opened.push_back (character);
      }
      
      if (in_array (character, closers)) {
        
        const std::string opener = match (character, pairs);
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
          const std::string fragment1 = checks::issues::text(checks::issues::issue::closing_character);
          const std::string fragment2 = checks::issues::text(checks::issues::issue::without_its_matching_opening_character);
          std::stringstream message {};
          message << fragment1 << " " << std::quoted(character) << " " << fragment2 << " " << std::quoted(opener);
          database::check::record_output (bible, book, chapter, verse, message.str());
        }
      }
    }
  }
  
  // Report unclosed openers.
  for (size_t i = 0; i < verses.size (); i++) {
    int verse = verses [i];
    const std::string opener = opened [i];
    const std::string closer = match (opener, pairs);
    const std::string fragment1 = checks::issues::text(checks::issues::issue::opening_character);
    const std::string fragment2 = checks::issues::text(checks::issues::issue::without_its_matching_closing_character);
    std::stringstream message {};
    message << fragment1 << " " << std::quoted(opener) << " " << fragment2 << " " << std::quoted(closer);
    database::check::record_output (bible, book, chapter, verse, message.str());
  }
}


std::string checks_pairs::match (const std::string & character, const std::vector <std::pair <std::string, std::string>>& pairs)
{
  for (const auto& element : pairs) {
    if (character == element.first) return element.second;
    if (character == element.second) return element.first;
  }
  return std::string();
}
