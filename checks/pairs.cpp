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


#include <checks/pairs.h>
#include <filter/string.h>
#include <database/check.h>
#include <checks/french.h>
#include <checks/issues.h>


void checks_pairs::run(const std::string& bible, const int book, const int chapter,
                       const std::map<int, std::string>& texts,
                       const std::vector<std::pair<std::string, std::string>>& pairs,
                       const bool french_citation_style)
{
    // This holds the opener characters of the pairs which were opened in the text.
    // For example, it may hold the "[".
    std::vector<int> verses{};
    std::vector<std::string> opened{};

    // Containers with the openers and the closers.
    // If the check on the French citation style is active,
    // skip the French guillemets.
    std::vector<std::string> openers{};
    std::vector<std::string> closers{};
    for (const auto& [opener, closer] : pairs)
    {
        if (french_citation_style and opener == checks_french::left_guillemet) continue;
        if (french_citation_style and opener == checks_french::right_guillemet) continue;
        openers.push_back(opener);
        closers.push_back(closer);
    }

    // Go through the verses with their texts.
    for (const auto& [verse, text] : texts)
    {
        const std::size_t length = filter::string::unicode_string_length(text);
        for (std::size_t pos = 0; pos < length; pos++)
        {
            const std::string character = filter::string::unicode_string_substr(text, pos, 1);

            if (filter::string::in_array(character, openers))
            {
                verses.push_back(verse);
                opened.push_back(character);
            }

            if (filter::string::in_array(character, closers))
            {
                const std::string opener = match(character, pairs);
                bool mismatch = false;
                if (opened.empty())
                {
                    mismatch = true;
                }
                else if (opened.back() == opener)
                {
                    verses.pop_back();
                    opened.pop_back();
                }
                else
                {
                    mismatch = true;
                }
                if (mismatch)
                {
                    std::stringstream msg{};
                    msg << checks::issues::text(checks::issues::issue::closing_character);
                    msg << " " << std::quoted(character) << " ";
                    msg << checks::issues::text(
                        checks::issues::issue::without_its_matching_opening_character);
                    msg << " " << std::quoted(opener);
                    database::check::record_output(bible, book, chapter, verse, msg.str());
                }
            }
        }
    }

    // Report unclosed openers.
    for (std::size_t i = 0; i < verses.size(); i++)
    {
        const int verse = verses.at(i);
        const std::string opener = opened.at(i);
        const std::string closer = match(opener, pairs);
        std::stringstream msg{};
        msg << checks::issues::text(checks::issues::issue::opening_character);
        msg << " " << std::quoted(opener) << " ";
        msg << checks::issues::text(
            checks::issues::issue::without_its_matching_closing_character);
        msg << " " << std::quoted(closer);
        database::check::record_output(bible, book, chapter, verse, msg.str());
    }
}


std::string checks_pairs::match(const std::string& character,
                                const std::vector<std::pair<std::string, std::string>>& pairs)
{
    for (const auto& [opener, closer] : pairs)
    {
        if (character == opener) return closer;
        if (character == closer) return opener;
    }
    return {};
}
