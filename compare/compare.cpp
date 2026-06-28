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


#include <access/bible.h>
#include <compare/compare.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/jobs.h>
#include <database/logs.h>
#include <database/usfmresources.h>
#include <database/config/bible.h>
#include <filter/diff.h>
#include <filter/passage.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/text.h>
#include <filter/usfm.h>
#include <locale/translate.h>


// Compare the $bible with another Bible, passed through $compare.
void compare_compare(const std::string& bible, const std::string& compare, const int job_id)
{
    database::logs::log(translate("Comparing Bibles") + " " + bible + " " + translate("and") + " " + compare,
                       roles::consultant);


    const std::string stylesheet = database::config::bible::get_export_stylesheet(bible);


    database_jobs::set_progress(job_id, translate("The Bibles are being compared..."));


    // The results of the comparison. Will be displayed to the user.
    std::vector<std::string> result;
    {
        std::stringstream ss{};
        ss << translate("Bible") << " " << std::quoted(bible) << " " << translate("has been compared with") << " " <<
            std::quoted(compare) << ".";
        result.push_back(std::move(ss).str());
    }
    result.push_back(translate("Additions are in bold.") + " " + translate("Removed words are in strikethrough."));
    result.emplace_back("");


    // Get the combined distinct books in both Bibles / Resources.
    const std::vector<int> bible_books = database::bibles::get_books(bible);
    const std::vector<int> compare_books = database::bibles::get_books(compare);
    const std::vector<int> resource_books = database::usfm_resources::get_books(compare);
    const auto combined_distinct_books = [&]
    {
        std::set<int> book_set;
        book_set.insert(bible_books.cbegin(), bible_books.cend());
        book_set.insert(compare_books.cbegin(), compare_books.cend());
        book_set.insert(resource_books.cbegin(), resource_books.cend());
        std::vector books(book_set.cbegin(), book_set.cend());
        std::ranges::sort(books);
        return books;
    };
    const std::vector books{combined_distinct_books()};


    // Results of comparison of raw USFM.
    std::vector<std::string> raw;


    // Absent books / chapters.
    std::vector<std::string> absent;


    // The new verses as in the $bible.
    std::vector<std::string> new_verses;


    for (const auto& book : books)
    {
        const std::string book_name = database::books::get_english_from_id(static_cast<book_id>(book));
        database_jobs::set_progress(job_id, book_name);


        if (std::ranges::find(bible_books, book) == bible_books.end())
        {
            std::ostringstream ss{};
            ss << translate("Bible") << " " << std::quoted(bible) << " " << translate("does not contain") << " " <<
                book_name << ".";
            absent.push_back(std::move(ss).str());
            continue;
        }

        if (std::ranges::find(compare_books, book) == compare_books.end())
        {
            if (std::ranges::find(resource_books, book) == resource_books.end())
            {
                std::ostringstream ss{};
                ss << translate("Bible/Resource") << " " << std::quoted(compare) << " " << translate("does not contain")
                    << " " << book_name << ".";
                absent.push_back(std::move(ss).str());
                continue;
            }
        }


        // Get the combined distinct chapters in both Bibles / Resources.
        const std::vector<int> bible_chapters = database::bibles::get_chapters(bible, book);
        const std::vector<int> compare_chapters = database::bibles::get_chapters(compare, book);
        const std::vector<int> resource_chapters = database::usfm_resources::get_chapters(compare, book);
        const auto combined_distinct_chapters = [&]
        {
            std::set<int> chapter_set;
            chapter_set.insert(bible_chapters.cbegin(), bible_chapters.cend());
            chapter_set.insert(compare_chapters.cbegin(), compare_chapters.cend());
            chapter_set.insert(resource_chapters.cbegin(), resource_chapters.cend());
            std::vector chapters(chapter_set.cbegin(), chapter_set.cend());
            std::ranges::sort(chapters);
            return chapters;
        };


        for (const auto& chapter : combined_distinct_chapters())
        {
            // Look for, report, and skip missing chapters in the source Bible.
            if (std::ranges::find(bible_chapters, chapter) == bible_chapters.end())
            {
                std::ostringstream ss{};
                ss << translate("Bible") << " " << std::quoted(bible) << " " << translate("does not contain") << " " <<
                    book_name << " " << chapter << ".";
                absent.push_back(std::move(ss).str());
                continue;
            }


            // Look for, report, and skip missing chapters in the comparison USFM data.
            if (std::ranges::find(compare_chapters, chapter) == compare_chapters.end())
            {
                if (std::ranges::find(resource_chapters, chapter) == resource_chapters.end())
                {
                    std::ostringstream ss{};
                    ss << translate("Bible/Resource") << " " << std::quoted(compare) << " " <<
                        translate("does not contain") << " " << book_name << " " << chapter << ".";
                    absent.push_back(std::move(ss).str());
                    continue;
                }
            }


            // Get source and compare USFM, and skip them if they are equal.
            const std::string bible_chapter_usfm = database::bibles::get_chapter(bible, book, chapter);
            std::string compare_chapter_usfm = database::bibles::get_chapter(compare, book, chapter);
            if (compare_chapter_usfm.empty())
                compare_chapter_usfm = database::usfm_resources::get_usfm(compare, book, chapter);
            if (bible_chapter_usfm == compare_chapter_usfm)
                continue;


            // Get the sorted combined set of distinct verses in the chapter of the Bible and of the USFM to compare with.
            const auto combined_distinct_verses = [&]
            {
                const std::vector<int> bible_verse_numbers = filter::usfm::get_verse_numbers(bible_chapter_usfm);
                const std::vector<int> compare_verse_numbers = filter::usfm::get_verse_numbers(compare_chapter_usfm);
                std::set<int> verse_set{};
                verse_set.insert(bible_verse_numbers.cbegin(), bible_verse_numbers.cend());
                verse_set.insert(compare_verse_numbers.cbegin(), compare_verse_numbers.cend());
                std::vector verses(verse_set.cbegin(), verse_set.cend());
                std::ranges::sort(verses);
                return verses;
            };


            for (const int& verse : combined_distinct_verses())
            {
                // Get the USFM of verse of the Bible and comparison USFM, and skip it if both are the same.
                const std::string bible_verse_usfm = filter::usfm::get_verse_text(bible_chapter_usfm, verse);
                const std::string compare_verse_usfm = filter::usfm::get_verse_text(compare_chapter_usfm, verse);
                if (bible_verse_usfm == compare_verse_usfm)
                    continue;

                auto filter_text_bible = Filter_Text(bible);
                auto filter_text_compare = Filter_Text(compare);
                filter_text_bible.html_text_standard = new HtmlText({});
                filter_text_compare.html_text_standard = new HtmlText({});
                filter_text_bible.text_text = new Text_Text();
                filter_text_compare.text_text = new Text_Text();
                filter_text_bible.add_usfm_code(bible_verse_usfm);
                filter_text_compare.add_usfm_code(compare_verse_usfm);
                filter_text_bible.run(stylesheet);
                filter_text_compare.run(stylesheet);
                const std::string bible_html = filter_text_bible.html_text_standard->get_inner_html();
                const std::string compare_html = filter_text_compare.html_text_standard->get_inner_html();
                const std::string bible_text = filter_text_bible.text_text->get();
                if (const std::string compare_text = filter_text_compare.text_text->get();
                    bible_text != compare_text)
                {
                    const std::string modification = filter_diff_diff(compare_text, bible_text);
                    result.push_back(filter_passage_display(book, chapter, std::to_string(verse)) + " " + modification);
                    new_verses.push_back(
                        filter_passage_display(book, chapter, std::to_string(verse)) + " " + bible_text);
                }
                const std::string modification = filter_diff_diff(compare_verse_usfm, bible_verse_usfm);
                raw.push_back(filter_passage_display(book, chapter, std::to_string(verse)) + " " + modification);
            }
        }
    }


    // Add the absent books / chapters to the comparison.
    if (!absent.empty())
    {
        result.emplace_back("");
        result.insert(result.end(), absent.begin(), absent.end());
    }


    // Add any differences in the raw USFM to the comparison.
    if (!raw.empty())
    {
        result.emplace_back("");
        result.insert(result.end(), raw.begin(), raw.end());
    }


    // Add the text of the new verses, as they are in the $bible.
    if (!new_verses.empty())
    {
        result.emplace_back("");
        result.push_back(translate("The texts as they are in the Bible") + " " + bible);
        result.emplace_back("");
        result.insert(result.end(), new_verses.begin(), new_verses.end());
    }


    // Format and store the result of the comparison.
    for (auto& line : result)
    {
        if (line.empty())
            line = "<br>";
        else
        {
            line.insert(0, "<p>");
            line.append("</p>");
        }
    }
    database_jobs::set_result(job_id, filter::string::implode(result, "\n"));


    database::logs::log(translate("Comparison is ready"), roles::consultant);
}
