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


namespace database::books {


std::vector<book_id> get_ids()
{
    auto&& ids_view = books_table | std::ranges::views::transform(&book_record::id);
    return {ids_view.begin(), ids_view.end()};
}


// Metaprogramming utility.
// Given a pointer-to-member value (like &book_record::english),
// extract just the type of the member it points to (e.g., std::string_view),
// so that type can be used elsewhere as a function parameter type, a return type, etc.
template <typename Struct, typename Member>
// ReSharper disable once CppFunctionIsNotImplemented
static Member member_value_type(Member Struct::*);
// Above is just the declaration, because it is never called at runtime.
// The parameter type, Member Struct::*, is a pointer-to-member declarator.
// It means "a pointer to a member of type Member belonging to class Struct."

template <auto member_ptr>
using member_t = decltype(member_value_type(member_ptr));



template <auto search_member, auto result_member, typename DefaultFn>
[[nodiscard]] static auto lookup_field(const member_t<search_member>& key, DefaultFn&& default_fn)
    -> member_t<result_member>
{
    if (const auto iter = std::ranges::find(books_table, key, search_member);
        iter != std::ranges::cend(books_table))
        return iter->*result_member;
    return std::invoke(std::forward<DefaultFn>(default_fn));
}

book_id get_id_from_english(const std::string_view english) noexcept
{
    auto default_fn = [] { return book_id::_unknown; };
    return lookup_field<&book_record::english, &book_record::id>(english, std::move(default_fn));
}

std::string get_english_from_id(const book_id id)
{
    auto default_fn = [] { return std::string_view{"Unknown"}; };
    return std::string{lookup_field<&book_record::id, &book_record::english>(id, std::move(default_fn))};
}

std::string get_usfm_from_id(const book_id id)
{
    auto default_fn = [] { return std::string_view{"XXX"}; };
    return std::string{lookup_field<&book_record::id, &book_record::usfm>(id, std::move(default_fn))};
}

std::string get_bibleworks_from_id(const book_id id)
{
    auto default_fn = [] { return std::string_view{"Xxx"}; };
    return std::string{lookup_field<&book_record::id, &book_record::bibleworks>(id, std::move(default_fn))};
}

std::string get_osis_from_id(const book_id id)
{
    auto default_fn = [] { return std::string_view{"Unknown"}; };
    return std::string{lookup_field<&book_record::id, &book_record::osis>(id, std::move(default_fn))};
}

book_id get_id_from_usfm(const std::string_view usfm)
{
    auto default_fn = [] { return book_id::_unknown; };
    return lookup_field<&book_record::usfm, &book_record::id>(usfm, std::move(default_fn));
}

book_id get_id_from_osis(const std::string_view osis)
{
    auto default_fn = [] { return book_id::_unknown; };
    return lookup_field<&book_record::osis, &book_record::id>(osis, std::move(default_fn));
}

book_id get_id_from_bibleworks(const std::string_view bibleworks)
{
    auto default_fn = [] { return book_id::_unknown; };
    return lookup_field<&book_record::bibleworks, &book_record::id>(bibleworks, std::move(default_fn));
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
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(std::string{record.english})));
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(std::string{record.osis})));
        // USFM is canonical uppercase: Leave it like that.
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, std::string{record.usfm}));
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(std::string{record.bibleworks})));
        candidates.emplace_back(record.id, filter_diff_character_similarity(text, filter::string::unicode_string_casefold(std::string{record.onlinebible})));
    }

    // Don't sort the entire vector, just take the maximum element.
    const auto best = std::ranges::max_element(candidates, {}, &candidate::similarity);
    return best->id;
}


book_id get_id_from_onlinebible(const std::string_view onlinebible)
{
    auto default_fn = [] { return book_id::_unknown; };
    return lookup_field<&book_record::onlinebible, &book_record::id>(onlinebible, std::move(default_fn));
}

std::string get_onlinebible_from_id(const book_id id)
{
    auto default_fn = [] { return std::string_view{}; };
    return std::string{lookup_field<&book_record::id, &book_record::onlinebible>(id, std::move(default_fn))};
}

short get_order_from_id(const book_id id)
{
    auto default_fn = [] { return static_cast<uint8_t>(0); };
    return lookup_field<&book_record::id, &book_record::order>(id, std::move(default_fn));
}

book_type get_type(const book_id id)
{
    auto default_fn = [] { return book_type::unknown; };
    return lookup_field<&book_record::id, &book_record::type>(id, std::move(default_fn));
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
