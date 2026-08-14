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


#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>
// ReSharper disable once CppUnusedIncludeDirective
#include "database/books.h"
#include "database/booksdata.h"
#include "styles/logic.h"
namespace stylesv2 {
// ReSharper disable once CppUnusedIncludeDirective
#include "styles/definitions.hpp"
}


static std::string file_get_contents(const std::string& filename)
{
    std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    const std::streamoff filesize = ifs.tellg();
    if (filesize == 0) return std::string();
    ifs.seekg(0, std::ios::beg);
    std::vector<char> bytes(static_cast<unsigned>(filesize));
    ifs.read(&bytes[0], static_cast<unsigned>(filesize));
    return std::string(&bytes[0], static_cast<unsigned>(filesize));
}


static void file_put_contents(const std::string& filename, const std::string& contents)
{
    std::ofstream file;
    file.open(filename, std::ios::binary | std::ios::trunc);
    file << contents;
    file.close();
}


static std::vector<std::string> explode(const std::string& value, const char delimiter)
{
    std::vector<std::string> result{};
    std::istringstream iss(value);
    for (std::string token; std::getline(iss, token, delimiter);)
    {
        result.push_back(std::move(token));
    }
    return result;
}


static std::string implode(const std::vector<std::string>& values, const std::string& delimiter)
{
    std::string full{};
    for (auto iterator = values.begin(); iterator != values.end(); ++iterator)
    {
        full += *iterator;
        if (iterator != values.end() - 1) full += delimiter;
    }
    return full;
}


static std::string str_replace(const std::string& search, const std::string& replace, std::string subject)
{
    size_t off_position = subject.find(search);
    while (off_position != std::string::npos)
    {
        subject.replace(off_position, search.length(), replace);
        off_position = subject.find(search, off_position + replace.length());
    }
    return subject;
}


int main()
{
    // Read all html files to process.
    std::string contents = file_get_contents("i18n.html");
    const std::vector<std::string> files = explode(contents, '\n');
    std::cout << "Processing " << files.size() << " html files" << std::endl;

    // Store the translatable strings.
    std::vector<std::string> translatables{};

    // Go over all html files.
    for (const auto& file : files)
    {
        // Read the html.
        contents = file_get_contents(file);

        // Clean up the "translate" (gettext) calls.
        contents = str_replace("translate (", "translate(", contents);

        // Gettext markup.
        constexpr std::string_view gettext_open = R"(translate(")";
        constexpr std::string_view gettext_close = R"("))";

        // Limit gettext iterations.
        int iterations {0};

        // Start processing variables by locating the first one.
        size_t position = contents.find(gettext_open);

        // Iterate through the contents till all gettext calls have been dealt with.
        while (position != std::string::npos and iterations < 1000)
        {
            iterations++;

            // Remove the gettext opener.
            contents.erase(position, gettext_open.length());

            // Position where the gettext call ends.
            if (size_t pos = contents.find(gettext_close, position); pos != std::string::npos)
            {
                // Take the gettext closer out.
                contents.erase(pos, gettext_close.length());

                // The English string.
                std::string english = contents.substr(position, pos - position);

                // If the English string is empty, don't store it.
                if (english.empty()) continue;

                // Wrap it in calls recognizable as gettext calls, and store it.
                english.insert(0, "translate(\"");
                english.append("\")");
                translatables.push_back(english);
            }

            // Next gettext call.
            position = contents.find(gettext_open);
        }
    }

    // Go over all USFM styles to internationalize them.
    for (const auto& style : stylesv2::styles)
    {
        const auto translate = [&translatables](std::string english)
        {
            if (!english.empty())
            {
                english.insert(0, "translate(\"");
                english.append("\")");
                translatables.push_back(english);
            }
        };
        translate(style.name);
        translate(style.info);
    }

    // Go over all Bible books to internationalize them.
    for (unsigned int i = 0; i < std::size(books_table); ++i)
    {
        if (std::string english = books_table[i].english; not english.empty())
        {
            english.insert(0, "translate(\"");
            english.append("\")");
            translatables.push_back(english);
        }
        if (std::string osis = books_table[i].osis; not osis.empty())
        {
            osis.insert(0, "translate(\"");
            osis.append("\")");
            translatables.push_back(osis);
        }
        if (std::string bible_works = books_table[i].bibleworks; not bible_works.empty())
        {
            bible_works.insert(0, "translate(\"");
            bible_works.append("\")");
            translatables.push_back(bible_works);
        }
    }

    // Store translatable strings.
    contents = implode(translatables, "\n");
    file_put_contents("translatables.cpp", contents);

    return 0;
}
