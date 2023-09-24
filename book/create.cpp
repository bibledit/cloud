/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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
using namespace std;


// Creates book template with ID $book in Bible $bible.
// If a $chapter is given instead of -1, it creates that chapter only.
// If the $chapter is -1, it creates all chapters within that book.
bool book_create (const string & bible, const book_id book, const int chapter, vector <string> & feedback)
{
  Database_Bibles database_bibles {};
  Database_Versifications database_versifications {};

  vector <string> bibles = database_bibles.get_bibles ();
  if (!in_array (bible, bibles)) {
    feedback.push_back (translate("Bible bible does not exist: Cannot create book"));
    return false;
  }
  if (book == book_id::_unknown) {
    feedback.push_back (translate("Invalid book while creating a book template"));
    return false;
  }
  
  // The chapters that have been created.
  vector <int> chapters_created {};
  
  // The USFM created.
  string data {};
  
  // Chapter 0.
  if (chapter <=  0) {
    data  = "\\id "    + database::books::get_usfm_from_id(book)     + "\n";
    data += "\\h "     + database::books::get_english_from_id (book) + "\n";
    data += "\\toc2 "  + database::books::get_english_from_id (book) + "\n";
    bible_logic::store_chapter (bible, static_cast<int>(book), 0, data);
    chapters_created.push_back (0);
  }
  
  
  // Subsequent chapters.
  string versification = Database_Config_Bible::getVersificationSystem (bible);
  vector <Passage> versification_data = database_versifications.getBooksChaptersVerses (versification);
  for (const auto & row : versification_data) {
    if (book == static_cast<book_id>(row.m_book)) {
      int ch = row.m_chapter;
      int verse = filter::strings::convert_to_int (row.m_verse);
      if ((chapter < 0) || (chapter == ch)) {
        data  = "\\c " + filter::strings::convert_to_string (ch) + "\n";
        data += "\\p\n";
        for (int i = 1; i <= verse; i++) {
          data += "\\v " + filter::strings::convert_to_string (i) + "\n";
        }
        bible_logic::store_chapter (bible, static_cast<int>(book), ch, data);
        chapters_created.push_back (ch);
      }
    }
  }
  
  // Done.
  if (chapters_created.size () == 0) {
    feedback.push_back (translate("No chapters have been created"));
    return false;
  }
  string created;
  for (auto & chapter_created : chapters_created) {
    if (!created.empty ()) created.append (" ");
    created.append (filter::strings::convert_to_string (chapter_created));
  }
  feedback.push_back (translate("The following chapters have been created:") + " " + created);
  return true;
}
