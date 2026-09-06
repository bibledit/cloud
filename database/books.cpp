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


#include <database/books.h>
#include <database/booksdata.h>
#include <filter/diff.h>
#include <filter/string.h>
#include <locale/translate.h>


namespace database::books {
std::vector<book_id> get_ids()
{
    auto&& ids_view = books_table | std::ranges::views::transform(&book_record::id);
    return {ids_view.begin(), ids_view.end()};
}


book_id get_id_from_english(const std::string_view english) noexcept
{
    if (const auto iter = std::ranges::find(books_table, english, &book_record::english);
        iter != std::ranges::cend(books_table))
        return iter->id;
    return book_id::_unknown;
}


std::string get_english_from_id(const book_id id)
{
    if (const auto iter = std::ranges::find(books_table, id, &book_record::id);
        iter != std::ranges::cend(books_table))
        return iter->english;
    return translate("Unknown");
}


std::string get_usfm_from_id(const book_id id)
{
    if (const auto iter = std::ranges::find(books_table, id, &book_record::id);
        iter != std::ranges::cend(books_table))
        return iter->usfm;
    return "XXX";
}


std::string get_bibleworks_from_id(const book_id id)
{
    if (const auto iter = std::ranges::find(books_table, id, &book_record::id);
        iter != std::ranges::cend(books_table))
        return iter->bibleworks;
    return "Xxx";
}


std::string get_osis_from_id(const book_id id)
{
    if (const auto iter = std::ranges::find(books_table, id, &book_record::id);
        iter != std::ranges::cend(books_table))
        return iter->osis;
    return translate("Unknown");
}


book_id get_id_from_usfm(const std::string_view usfm)
{
    if (const auto iter = std::ranges::find(books_table, usfm, &book_record::usfm);
        iter != std::ranges::cend(books_table))
        return iter->id;
    return book_id::_unknown;
}


book_id get_id_from_osis(const std::string_view osis)
{
    if (const auto iter = std::ranges::find(books_table, osis, &book_record::osis);
        iter != std::ranges::cend(books_table))
        return iter->id;
    return book_id::_unknown;
}


book_id get_id_from_bibleworks(const std::string_view bibleworks)
{
    if (const auto iter = std::ranges::find(books_table, bibleworks, &book_record::bibleworks);
        iter != std::ranges::cend(books_table))
        return iter->id;
    return book_id::_unknown;
}


// Tries to interpret $text as the name of a Bible book.
// Returns the book's identifier if it succeeds.
// If it fails, it returns 0.

book_id get_id_like_text(const std::string& text)
{
    struct candidate
    {
        book_id id;
        int similarity;
    };
    std::vector<candidate> candidates;
    candidates.reserve(books_table.size() * 5);

    // Go through all known book names and abbreviations.
    // Store how much parameter "text" differs from the known names.
    std::vector<int> ids{};
    std::vector<int> similarities{};
    for (const auto & record : books_table)
    {
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(record.english)));
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(record.osis)));
        // USFM is canonical uppercase: Leave it like that.
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, record.usfm));
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(record.bibleworks)));
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(record.onlinebible)));
    }

    // Don't sort the entire vector, just take the maximum element.
    const auto best = std::ranges::max_element(candidates, {}, &candidate::similarity);
    return best->id;
}


book_id get_id_from_onlinebible(const std::string_view onlinebible)
{
    if (const auto iter = std::ranges::find(books_table, onlinebible, &book_record::onlinebible);
        iter != std::ranges::cend(books_table))
        return iter->id;
    return book_id::_unknown;
}


std::string get_onlinebible_from_id(const book_id id)
{
    if (const auto iter = std::ranges::find(books_table, id, &book_record::id);
        iter != std::ranges::cend(books_table))
        return iter->onlinebible;
    return {};
}


short get_order_from_id(const book_id id)
{
    if (const auto iter = std::ranges::find(books_table, id, &book_record::id);
        iter != std::ranges::cend(books_table))
        return iter->order;
    return 0;
}


book_type get_type(const book_id id)
{
    if (const auto iter = std::ranges::find(books_table, id, &book_record::id);
        iter != std::ranges::cend(books_table))
        return iter->type;
    return book_type::unknown;
}


std::string book_type_to_string(const book_type type)
{
    switch (type)
    {
    case book_type::unknown: return {};
    case book_type::old_testament: return "ot";
    case book_type::new_testament: return "nt";
    case book_type::front_back: return "frontback";
    case book_type::other: return "other";
    case book_type::apocryphal: return "ap";
    default: return {};
    }
}
} // End of namespace.
