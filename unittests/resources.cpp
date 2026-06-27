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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wcharacter-conversion"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop
#include <database/userresources.h>
#include <database/usfmresources.h>
#include <resource/external.h>
#include <unittests/utilities.h>


TEST(database, resources)
{
    refresh_sandbox(false);
    const std::vector<std::string> names = resource_external_names();
    const auto count = std::ranges::count(names, "Statenbijbel GBS");
    EXPECT_EQ(count, 1);
}


TEST(database, usfmresources)
{
    // Empty
    {
        refresh_sandbox(false);
        std::vector<std::string> resources = database::usfm_resources::get_resources();
        const auto count = std::ranges::count(resources, "bibledit");
        EXPECT_EQ(count, 0);
    }
    // Names
    {
        refresh_sandbox(true);
        database::usfm_resources::store_chapter("bibledit", 2, 3, "");
        std::vector<std::string> resources = database::usfm_resources::get_resources();
        const auto count = std::ranges::count(resources, "bibledit");
        EXPECT_EQ(count, 1);
    }
    // Store Get Chapter
    {
        refresh_sandbox(true);
        database::usfm_resources::store_chapter("bibledit", 2, 3, "usfm");
        std::string usfm = database::usfm_resources::get_usfm("bibledit", 2, 3);
        EXPECT_EQ("usfm", usfm);
        usfm = database::usfm_resources::get_usfm("bibledit", 2, 4);
        EXPECT_TRUE(usfm.empty());
    }
    // Books
    {
        refresh_sandbox(true);
        database::usfm_resources::store_chapter("bibledit", 2, 3, "usfm");
        database::usfm_resources::store_chapter("bibledit", 3, 4, "usfm");
        database::usfm_resources::store_chapter("bibledit", 1, 1, "usfm");
        database::usfm_resources::store_chapter("bibledit", 1, 2, "usfm");
        const std::vector<int> books = database::usfm_resources::get_books("bibledit");
        const std::vector standard{1, 2, 3};
        EXPECT_EQ(standard, books);
    }
    // Chapters
    {
        refresh_sandbox(true);
        database::usfm_resources::store_chapter("bibledit", 2, 3, "usfm");
        database::usfm_resources::store_chapter("bibledit", 3, 4, "usfm");
        database::usfm_resources::store_chapter("bibledit", 1, 1, "usfm");
        database::usfm_resources::store_chapter("bibledit", 1, 2, "usfm");
        std::vector<int> chapters = database::usfm_resources::get_chapters("bibledit", 1);
        const std::vector standard{1, 2};
        EXPECT_EQ(standard, chapters);
        chapters = database::usfm_resources::get_chapters("bibledit", 2);
        EXPECT_EQ(std::vector <int>{3}, chapters);
    }
    // Sizes
    {
        refresh_sandbox(true);

        int size = database::usfm_resources::get_size("bibledit", 2, 3);
        EXPECT_EQ(0, size);

        database::usfm_resources::store_chapter("bibledit", 2, 3, "usfm");
        size = database::usfm_resources::get_size("bibledit", 2, 3);
        EXPECT_EQ(4, size);

        database::usfm_resources::store_chapter("bibledit", 2, 3, "chapter");
        size = database::usfm_resources::get_size("bibledit", 2, 3);
        EXPECT_EQ(7, size);
    }
    // Delete Book
    {
        refresh_sandbox(true);
        database::usfm_resources::store_chapter("bibledit", 2, 3, "usfm");
        std::vector<int> books = database::usfm_resources::get_books("bibledit");
        EXPECT_EQ(std::vector{2}, books);
        database::usfm_resources::delete_book("bibledit", 2);
        books = database::usfm_resources::get_books("bibledit");
        EXPECT_EQ(std::vector <int>{}, books);
    }
}


TEST(database, userresources)
{
    refresh_sandbox(false);

    std::vector<std::string> names;
    std::string name = "unit//test";
    std::string url = "https://website.org/resources/<book>/<chapter>/<verse>.html";
    int book = 99;
    std::string abbrev = "Book 99";
    std::string fragment;
    std::string value;
    std::vector<std::string> specialnames = {"abc\\def:ghi", name};

    names = Database_UserResources::names();
    EXPECT_EQ(std::vector <std::string>{}, names);

    Database_UserResources::url(name, url);
    value = Database_UserResources::url(name);
    EXPECT_EQ(url, value);

    for (auto special_name : specialnames)
    {
        Database_UserResources::url(special_name, special_name + url);
    }
    names = Database_UserResources::names();
    EXPECT_EQ(specialnames, names);

    for (auto special_name : specialnames)
    {
        Database_UserResources::remove(special_name);
    }
    names = Database_UserResources::names();
    EXPECT_EQ(std::vector <std::string>{}, names);

    Database_UserResources::book(name, book, abbrev);
    fragment = Database_UserResources::book(name, book);
    EXPECT_EQ(abbrev, fragment);

    fragment = Database_UserResources::book(name + "x", book);
    EXPECT_EQ(std::string(), fragment);

    fragment = Database_UserResources::book(name, book + 1);
    EXPECT_EQ(std::string(), fragment);
}

#endif

