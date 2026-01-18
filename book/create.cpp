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


#include <book/create.h>
#include <database/bibles.h>
#include <database/versifications.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <bb/logic.h>
#include <filter/string.h>


// Creates book template with ID $book in Bible $bible.
// If a $chapter is given instead of -1, it creates that chapter only.
// If the $chapter is -1, it creates all chapters within that book.
bool book_create (const std::string & bible, const book_id book, const int chapter,
                  std::vector<std::string>& feedback)
{
  Database_Versifications database_versifications {};

  if (const auto bibles = database::bibles::get_bibles();
      std::ranges::find(bibles, bible) == bibles.cend()) {
    feedback.push_back (translate("This Bible does not exist: Cannot create book"));
    return false;
  }
  if (book == book_id::_unknown) {
    feedback.push_back (translate("Invalid book while creating a book template"));
    return false;
  }
  
  // The chapters that have been created.
  std::vector<int> chapters_created {};
  
  
  // Chapter 0.
  if (chapter <= 0) {
    std::string data {};
    data += R"(\id )"    + database::books::get_usfm_from_id   (book) + "\n";
    data += R"(\h )"     + database::books::get_english_from_id(book) + "\n";
    data += R"(\toc2 )"  + database::books::get_english_from_id(book) + "\n";
    bible_logic::store_chapter (bible, static_cast<int>(book), 0, data);
    chapters_created.push_back(0);
  }
  
  
  // Subsequent chapters.
  const std::string versification = database::config::bible::get_versification_system(bible);
  const std::vector<Passage> versification_data = database_versifications.get_books_chapters_verses(versification);
  const auto book_filter = [book](const auto& row) {
    return book == static_cast<book_id>(row.m_book);
  };
  for (const auto& row : versification_data | std::views::filter(book_filter)) {
    const int ch = row.m_chapter;
    const int verse = filter::string::convert_to_int (row.m_verse);
    if ((chapter < 0) or (chapter == ch)) {
      std::string data {};
      data += R"(\c )" + std::to_string(ch) + "\n";
      data += R"(\p)" "\n";
      for (int i = 1; i <= verse; i++) {
        data += R"(\v )" "" + std::to_string (i) + "\n";
      }
      bible_logic::store_chapter (bible, static_cast<int>(book), ch, data);
      chapters_created.push_back (ch);
    }
  }

  
  // Done.
  if (chapters_created.empty()) {
    feedback.push_back(translate("No chapters have been created"));
    return false;
  }
  const auto filter = [](const std::string& existing, const int chapter) {
    return existing + " " + std::to_string(chapter);
  };
  const std::string created = std::accumulate(chapters_created.cbegin(), chapters_created.cend(), std::string(), filter);
  feedback.push_back(translate("The following chapters have been created:") + created);
  return true;
}
