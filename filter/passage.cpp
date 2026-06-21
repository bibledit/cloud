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


#include <filter/passage.h>
#include <filter/string.h>
#include <database/books.h>
#include <database/bibles.h>
#include <database/config/bible.h>
#include <locale/translate.h>
#include <pugixml/include.h>


Passage::Passage(std::string bible, const int book, const int chapter, std::string verse)
    : m_bible(std::move(bible)), m_book(book), m_chapter(chapter), m_verse(std::move(verse))
{
}


std::string filter_passage_display(const int book, const int chapter, const std::string& verse)
{
    std::string display;
    display.append(translate(database::books::get_english_from_id(static_cast<book_id>(book))));
    display.append(" ");
    display.append(std::to_string(chapter));
    if (!verse.empty()) display.append(":" + verse);
    return display;
}


// Returns the display string for the $passages as one line.
std::string filter_passage_display_inline(const std::vector<Passage>& passages)
{
    std::string display;
    std::ranges::for_each(passages, [&display](const auto& passage)
    {
        if (!display.empty()) display.append(" | ");
        display.append(filter_passage_display(passage.book(), passage.chapter(), passage.verse()));
    });
    return display;
}


// Returns the display string for the $passages as several lines.
std::string filter_passage_display_multiline(const std::vector<Passage>& passages)
{
    std::string display;
    std::ranges::for_each(passages, [&display](const auto& passage)
    {
        display.append(filter_passage_display(passage.book(), passage.chapter(), passage.verse()));
        display.append("\n");
    });
    return display;
}


// This function converts $passage to an integer, so that passages can be compared or stored.
int filter_passage_to_integer(const Passage& passage)
{
    return 1000000 * passage.book() + 1000 * passage.chapter() + filter::string::convert_to_int(passage.verse());
}


// This converts the $integer, created above, to a passage.
Passage filter_integer_to_passage(int integer)
{
    const int book = static_cast<int>(round(integer / 1000000));
    integer -= book * 1000000;
    const int chapter = static_cast<int>(round(integer / 1000));
    integer -= chapter * 1000;
    const std::string verse = std::to_string(integer);
    return Passage("", book, chapter, verse);
}


// This filter takes $books as a string,
// and looks whether it can be interpreted as a valid book in any way.
// It returns a valid book identifier,
// or the unknown enum in case no book could be interpreted.
book_id filter_passage_interpret_book(std::string book)
{
    book = filter::string::trim(book);

    // Recognize the USFM book abbreviations.
    {
        if (const book_id identifier = database::books::get_id_from_usfm(book);
            identifier != book_id::_unknown)
            return identifier;
    }

    // Recognize the BibleWorks book abbreviations.
    {
        if (const book_id identifier = database::books::get_id_from_bibleworks(book);
            identifier != book_id::_unknown)
            return identifier;
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
    book = filter::string::replace("III ", "3 ", book);
    book = filter::string::replace("II ", "2 ", book);
    book = filter::string::replace("I ", "1 ", book);

    // Do case folding, i.e., work with lower case only.
    book = filter::string::unicode_string_casefold(book);

    // Remove any spaces from the book name and try with that too.
    std::string nospacebook = filter::string::replace(" ", "", book);

    // Store all the available IDs locally.
    std::vector<book_id> book_ids = database::books::get_ids();

    // Check on names entered like "Genesis" or "1 Corinthians", the full English name.
    // A bug was discovered so that "Judges" was interpreted as "Jude",
    // because of the three letters "Jud".
    // Solved by checking on full English name first.
    // In general, do exact matching first before moving on to similarity matching.
    // Compare with the translation to Bibledit's language too.
    for (auto identifier : book_ids)
    {
        std::string english = database::books::get_english_from_id(identifier);
        if (english.empty()) continue;
        if (book == filter::string::unicode_string_casefold(english)) return identifier;

        if (nospacebook == filter::string::unicode_string_casefold(english)) return identifier;

        std::string localized = translate(english);
        if (localized.empty()) continue;

        if (book == filter::string::unicode_string_casefold(localized)) return identifier;

        if (nospacebook == filter::string::unicode_string_casefold(localized)) return identifier;
    }

    // Try the OSIS abbreviations.
    for (auto identifier : book_ids)
    {
        std::string osis = database::books::get_osis_from_id(identifier);
        if (osis.empty()) continue;
        if (book == filter::string::unicode_string_casefold(osis)) return identifier;
        if (nospacebook == filter::string::unicode_string_casefold(osis)) return identifier;
        std::string localized = translate(osis);
        if (localized.empty()) continue;
        if (book == filter::string::unicode_string_casefold(localized)) return identifier;
        if (nospacebook == filter::string::unicode_string_casefold(localized)) return identifier;
    }

    // Try the abbreviations of BibleWorks.
    for (const auto identifier : book_ids)
    {
        std::string bibleworks = database::books::get_bibleworks_from_id(identifier);
        if (bibleworks.empty()) continue;
        if (book == filter::string::unicode_string_casefold(bibleworks)) return identifier;
        if (nospacebook == filter::string::unicode_string_casefold(bibleworks)) return identifier;
        std::string localized = translate(bibleworks);
        if (localized.empty()) continue;
        if (book == filter::string::unicode_string_casefold(localized)) return identifier;
        if (nospacebook == filter::string::unicode_string_casefold(localized)) return identifier;
    }

    // Try the abbreviations of the Online Bible.
    for (auto identifier : book_ids)
    {
        std::string onlinebible = database::books::get_onlinebible_from_id(identifier);
        if (onlinebible.empty()) continue;
        if (book == filter::string::unicode_string_casefold(onlinebible)) return identifier;
        if (nospacebook == filter::string::unicode_string_casefold(onlinebible)) return identifier;
        std::string localized = translate(onlinebible);
        if (localized.empty()) continue;
        if (book == filter::string::unicode_string_casefold(localized)) return identifier;
        if (nospacebook == filter::string::unicode_string_casefold(localized)) return identifier;
    }

    // Do a case-insensitive search in the books database for something like the book given.
    if (const book_id identifier = database::books::get_id_like_text(book);
        identifier != book_id::_unknown)
        return identifier;

    // Sorry, no book found.
    return book_id::_unknown;
}


std::string filter_passage_clean_passage(std::string text)
{
    // Trim text.
    text = filter::string::trim(text);
    // As references could be, e.g.: Genesis 10.2, or Genesis 10:2,
    // it needs to convert the full stop and the colon to a space.
    text = filter::string::replace(".", " ", text);
    text = filter::string::replace(":", " ", text);
    // Change double spaces into single ones.
    text = filter::string::collapse_whitespace(text);
    // Trim again.
    text = filter::string::trim(text);
    // Result.
    return text;
}


// Takes the passage in $text, and explodes it into book, chapter, verse.
// The book is the numerical identifier, not the string, e.g.,
// it would not return "Genesis", but identifier 1.
Passage filter_passage_explode_passage(std::string text)
{
    text = filter_passage_clean_passage(text);
    // Cut the text in its parts.
    std::vector<std::string> bits = filter::string::explode(text, ' ');
    // Defaults to empty passage.
    Passage passage;
    // Take the bits.
    if (not bits.empty()) {
        if (const std::string& verse = bits.back(); not verse.empty())
            passage.verse(verse);
        bits.pop_back();
    }
    if (not bits.empty()) {
        if (const std::string& chapter = bits.back(); not chapter.empty())
            passage.chapter(filter::string::convert_to_int(chapter));
        bits.pop_back();
    }
    if (const std::string book = filter::string::implode(bits, " "); not book.empty())
    {
        const book_id bk = filter_passage_interpret_book(book);
        passage.book(static_cast<int>(bk));
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
Passage filter_passage_interpret_passage(Passage current_passage, std::string raw_passage)
{
    raw_passage = filter_passage_clean_passage(raw_passage);

    // Create an array with the bits of the raw input.
    std::vector<std::string> input = filter::string::explode(raw_passage, ' ');

    // Go through the array from verse to chapter to book.
    // Check how many numerals it has after the book part.
    std::vector<int> numerals;
    std::string book;
    std::vector inverted_input(input.begin(), input.end());
    std::ranges::reverse(inverted_input);
    for (std::string& bit : inverted_input)
    {
        if (const auto integer = filter::string::convert_to_int(bit); bit == std::to_string(integer))
        {
            numerals.push_back(integer);
            input.pop_back();
        }
        else
        {
            book = filter::string::implode(input, " ");
            break;
        }
    }

    // Deal with: Only book given, e.g. "Genesis".
    if (not book.empty() and numerals.empty())
        return filter_passage_explode_passage(book + " 1 1");

    // Deal with: One number given, e.g. "10".
    if (book.empty() and numerals.size() == 1)
    {
        int bk = current_passage.book();
        int chapter = current_passage.chapter();
        std::string verse = std::to_string(numerals[0]);
        Passage passage = filter_passage_explode_passage("Unknown " + std::to_string(chapter) + " " + verse);
        passage.book(bk);
        return passage;
    }

    // Deal with: Two numbers given, e.g. "1 2".
    if (book.empty() and numerals.size() == 2)
    {
        int bk = current_passage.book();
        int chapter = numerals[1];
        std::string verse = std::to_string(numerals[0]);
        Passage passage = filter_passage_explode_passage("Unknown " + std::to_string(chapter) + " " + verse);
        passage.book(bk);
        return passage;
    }

    // Deal with: Book and one number given, e.g. "Exodus 10".
    if (not book.empty() and numerals.size() == 1)
    {
        const int chapter = numerals.front();
        return filter_passage_explode_passage(book + " " + std::to_string(chapter) + " 1");
    }

    // Deal with: Book and two numbers given, e.g. "Song of Solomon 2 3".
    if (not book.empty() and numerals.size() == 2)
    {
        return filter_passage_explode_passage(raw_passage);
    }

    // Give up.
    return current_passage;
}


// This deals with sequences and ranges of verses, like the following:
// Exod. 37:4-5, 14-15, 27-28
// It puts each verse on a separate line.
std::vector<std::string> filter_passage_handle_sequences_ranges(const std::string& passage)
{
    // A passage like Exod. 37:4-5, 14-15, 27-28 will be cut at the comma.
    // The resulting array contains the following:
    // Exod. 37:4-5
    // 14-15
    // 27-28
    // It implies that the first sequence has book and chapter.
    std::vector<std::string> sequences = filter::string::explode(passage, ',');
    for (std::string& line : sequences) line = filter::string::trim(line);


    // Store output lines.
    std::vector<std::string> output;

    // Cut the passages at the hyphen.
    for (unsigned int offset = 0; offset < sequences.size(); offset++)
    {
        const std::string& sequence = sequences.at(offset);
        if (const std::vector<std::string> range = filter::string::explode(sequence, '-'); range.size() == 1)
            output.push_back(filter::string::trim(range.at(0)));
        else
        {
            std::string start = filter::string::trim(range[0]);
            output.push_back(start);
            if (offset == 0)
            {
                // Since the first bit contains book / chapter / verse,
                // extract the verse number.
                start = std::string(start.rbegin(), start.rend());
                start = std::to_string(filter::string::convert_to_int(start));
                start = std::string(start.rbegin(), start.rend());
            }
            const auto end = filter::string::convert_to_int(filter::string::trim(range[1]));
            for (auto i = filter::string::convert_to_int(start) + 1; i <= end; i++)
            {
                output.push_back(std::to_string(i));
            }
        }
    }

    // Result.
    return output;
}


void filter_passage_link_for_opening_editor_at(pugi::xml_node& node, const int book, const int chapter, const std::string& verse)
{
    const std::string display = filter_passage_display(book, chapter, verse);
    const auto passage = Passage("", book, chapter, verse);
    const std::string numeric = std::to_string(filter_passage_to_integer(passage));
    pugi::xml_node a_node = node.append_child("a");
    a_node.append_attribute("class") = "starteditor";
    a_node.append_attribute("href") = "nothing";
    a_node.append_attribute("passage") = numeric.c_str();
    a_node.text().set(display.c_str());
    const pugi::xml_node span_node = node.append_child("span");
    span_node.text().set(" ");
}


std::string filter_passage_link_for_opening_editor_at(const int book, const int chapter, const std::string& verse)
{
    pugi::xml_document document;
    filter_passage_link_for_opening_editor_at(document, book, chapter, verse);
    std::stringstream output;
    document.print(output, "", pugi::format_raw);
    return output.str();
}


// A Bible has a standard order for the books, and it can have their books in a custom order.
// This function returns either the standard order, or a custom order in case it is available for the $bible.
std::vector<int> filter_passage_get_ordered_books(const std::string& bible)
{
    // The available books from the Bible.
    const std::vector project_books = database::bibles::get_books(bible);

    // The book order from the settings, if any.
    const std::string s_ordered_books = database::config::bible::get_book_order(bible);
    const std::vector vs_ordered_books = filter::string::explode(s_ordered_books, ' ');

    // Keep books available in the Bible.
    std::vector<int> ordered_books;
    std::ranges::for_each(vs_ordered_books, [&](const std::string& book)
    {
        if (const auto bk = filter::string::convert_to_int(book);
            std::ranges::find(project_books, bk) != project_books.end())
            ordered_books.push_back(bk);
    });

    // Books in the Bible but not in the settings: Add them to the end.
    std::ranges::for_each(project_books, [&](const int book)
    {
        if (std::ranges::find(ordered_books, book) == ordered_books.end())
            ordered_books.push_back(book);
    });

    return ordered_books;
}


// This method converts the passage into text, like e.g. so:
// "hexadecimal Bible _1_2_3".
// First the hexadecimal Bible comes, then the book identifier, then the chapter number, and finally the verse number.
std::string filter_passage_encode(const Passage& passage)
{
    std::string text;
    // The encoded passage can be used as an attribute in the HTML DOM.
    // Therefore, it will be encoded such that any Bible name will be acceptable as an attribute in the DOM.
    text.append(filter::string::bin2hex(passage.bible()));
    text.append("_");
    text.append(std::to_string(passage.book()));
    text.append("_");
    text.append(std::to_string(passage.chapter()));
    text.append("_");
    text.append(passage.verse());
    if (passage.verse().empty())
        text.append("0");
    return text;
}





// This method converts encoded text into a passage.
// The text is in the format as its complementary function, "encode", produces.
Passage filter_passage_decode(const std::string& encoded)
{
    Passage passage;
    std::vector<std::string> bits = filter::string::explode(encoded, '_');
    if (not bits.empty())
    {
        if (const std::string& verse = bits.back(); not verse.empty())
            passage.verse(verse);
        bits.pop_back();
    }
    if (not bits.empty())
    {
        if (const std::string& chapter = bits.back(); not chapter.empty())
            passage.chapter(filter::string::convert_to_int(chapter));
        bits.pop_back();
    }
    if (not bits.empty())
    {
        if (const std::string& book = bits.back(); !book.empty())
            passage.book(filter::string::convert_to_int(book));
        bits.pop_back();
    }
    if (not bits.empty())
    {
        passage.bible(filter::string::hex2bin(bits.back()));
        bits.pop_back();
    }
    return passage;
}


