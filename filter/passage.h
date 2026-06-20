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


#pragma once

#include <config/libraries.h>

// Forward declaration.
enum class book_id;

namespace pugi { class xml_node; }

struct Passage
{
    explicit Passage() = default;
    explicit Passage(std::string bible, int book, int chapter, std::string verse);
    std::string m_bible{};
    int m_book{0};
    int m_chapter{0};
    std::string m_verse{};
    [[nodiscard]] std::string encode() const;
    static Passage decode(const std::string& encoded);
    constexpr auto operator<=>(const Passage&) const noexcept = default;
};

std::string filter_passage_display(int book, int chapter, const std::string& verse);
std::string filter_passage_display_inline(const std::vector<Passage>& passages);
std::string filter_passage_display_multiline(const std::vector<Passage>& passages);
int filter_passage_to_integer(const Passage& passage);
Passage filter_integer_to_passage(int integer);
book_id filter_passage_interpret_book(std::string book);
std::string filter_passage_clean_passage(std::string text);
Passage filter_passage_explode_passage(std::string text);
Passage filter_passage_interpret_passage(Passage current_passage, std::string raw_passage);
std::vector<std::string> filter_passage_handle_sequences_ranges(const std::string& passage);
void filter_passage_link_for_opening_editor_at(pugi::xml_node& node, int book, int chapter, const std::string& verse);
std::string filter_passage_link_for_opening_editor_at(int book, int chapter, const std::string& verse);
std::vector<int> filter_passage_get_ordered_books(const std::string& bible);
