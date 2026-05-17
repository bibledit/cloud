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


#include <database/logic.h>
#include <database/usfmresources.h>
#include <filter/string.h>
#include <filter/url.h>


// Database resilience: 
// The data is stored as separate files in the filesystem.
// That is resilient enough.


namespace database::usfm_resources {


static std::string main_folder()
{
    return filter_url_create_root_path({database_logic_databases(), "usfmresources"});
}


static std::string resource_folder(const std::string& name)
{
    return filter_url_create_path({main_folder(), name});
}


static std::string book_folder(const std::string& name, const int book)
{
    return filter_url_create_path({resource_folder(name), std::to_string(book)});
}


static std::string chapter_file(const std::string& name, const int book, const int chapter)
{
    return filter_url_create_path({book_folder(name, book), std::to_string(chapter)});
}


std::vector<std::string> get_resources()
{
    return filter_url_scandir(main_folder());
}


void delete_resource(const std::string& name)
{
    const std::string path = resource_folder(name);
    // If a folder: Delete it.
    filter_url_rmdir(path);
    // If a file: Delete it.
    filter_url_unlink(path);
}


void delete_book(const std::string& name, const int book)
{
    const std::string path = book_folder(name, book);
    // If a folder: Delete it.
    filter_url_rmdir(path);
    // If a file: Delete it.
    filter_url_unlink(path);
}


void delete_chapter(const std::string& name, const int book, const int chapter)
{
    filter_url_unlink(chapter_file(name, book, chapter));
}


void store_chapter(const std::string& name, const int book, const int chapter, const std::string& usfm)
{
    const std::string file = chapter_file(name, book, chapter);
    if (const std::string folder = filter_url_dirname(file); not file_or_dir_exists(folder))
        filter_url_mkdir(folder);
    filter_url_file_put_contents(file, usfm);
}


std::vector<int> get_books(const std::string& name)
{
    const std::vector<std::string> files = filter_url_scandir(resource_folder(name));
    std::vector<int> books;
    books.reserve(files.size());
    for (auto& book : files)
        books.push_back(filter::string::convert_to_int(book));
    std::ranges::sort(books);
    return books;
}


std::vector<int> get_chapters(const std::string& name, const int book)
{
    const std::vector<std::string> folders = filter_url_scandir(book_folder(name, book));
    std::vector<int> chapters;
    chapters.reserve(folders.size());
    for (auto& chapter : folders)
        chapters.push_back(filter::string::convert_to_int(chapter));
    std::ranges::sort(chapters);
    return chapters;
}


std::string get_usfm(const std::string& name, const int book, const int chapter)
{
    const std::string file = chapter_file(name, book, chapter);
    const std::string usfm = filter_url_file_get_contents(file);
    return usfm;
}


int get_size(const std::string& name, const int book, const int chapter)
{
    const std::string file = chapter_file(name, book, chapter);
    return filter_url_filesize(file);
}
}