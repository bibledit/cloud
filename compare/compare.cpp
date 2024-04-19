/*
 Copyright (©) 2003-2024 Teus Benschop.
 
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


#include <compare/compare.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/diff.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <tasks/logic.h>
#include <database/jobs.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/usfmresources.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <jobs/index.h>


void compare_compare (std::string bible, std::string compare, int jobId)
{
  Database_Logs::log (translate("Comparing Bibles") + " " + bible + " " + translate ("and") + " " + compare, Filter_Roles::consultant ());

  
  Database_Jobs database_jobs = Database_Jobs ();
  Database_Bibles database_bibles;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();

  const std::string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  

  database_jobs.set_progress (jobId, translate("The Bibles are being compared..."));
  

  // The results of the comparison. Will be displayed to the user.
  std::vector <std::string> result;
  result.push_back (translate("Bible") + " '" + bible + "' " + translate ("has been compared with") + " '" + compare + "'.");
  result.push_back (translate("Additions are in bold.") + " " + translate ("Removed words are in strikethrough."));
  result.push_back ("");
  
  
  // Get the combined books in both Bibles / Resources.
  std::vector <int> bibleBooks = database_bibles.get_books (bible);
  std::vector <int> compareBooks = database_bibles.get_books (compare);
  std::vector <int> resourceBooks = database_usfmresources.getBooks (compare);
  std::vector <int> books;
  {
    std::set <int> bookset;
    bookset.insert (bibleBooks.begin(), bibleBooks.end());
    bookset.insert (compareBooks.begin(), compareBooks.end());
    bookset.insert (resourceBooks.begin(), resourceBooks.end());
    books.assign (bookset.begin(), bookset.end ());
    std::sort (books.begin(), books.end());
  }
  
  
  // Results of comparison of raw USFM.
  std::vector <std::string> raw;
  
  
  // Absent books / chapters.
  std::vector <std::string> absent;
  
  
  // The new verses as in the $bible.
  std::vector <std::string> new_verses;
  
  
  for (auto & book : books) {

    
    std::string bookName = database::books::get_english_from_id (static_cast<book_id>(book));
    database_jobs.set_progress (jobId, bookName);
    
    
    if (find (bibleBooks.begin(), bibleBooks.end(), book) == bibleBooks.end()) {
      absent.push_back (translate("Bible") + " '" + bible + "' " + translate ("does not contain") + " " + bookName + ".");
      continue;
    }
    
    if (find (compareBooks.begin(), compareBooks.end(), book) == compareBooks.end()) {
      if (find (resourceBooks.begin(), resourceBooks.end(), book) == resourceBooks.end ()) {
        absent.push_back (translate("Bible/Resource") + " '" + compare + "' " + translate ("does not contain") + " " + bookName + ".");
        continue;
      }
    }
    
    
    // Get the combined chapters in both Bibles / Resources.
    std::vector <int> bibleChapters = database_bibles.get_chapters (bible, book);
    std::vector <int> compareChapters = database_bibles.get_chapters (compare, book);
    std::vector <int> resourceChapters = database_usfmresources.getChapters (compare, book);
    std::vector <int> chapters;
    {
      std::set <int> chapterset;
      chapterset.insert (bibleChapters.begin(), bibleChapters.end());
      chapterset.insert (compareChapters.begin(), compareChapters.end());
      chapterset.insert (resourceChapters.begin(), resourceChapters.end());
      chapters.assign (chapterset.begin(), chapterset.end ());
      std::sort (chapters.begin(), chapters.end());
    }


    for (const auto & chapter : chapters) {

      
      // Look for, report, and skip missing chapters in the source Bible.
      if (find (bibleChapters.begin(), bibleChapters.end(), chapter) == bibleChapters.end ()) {
        absent.push_back (translate("Bible") + " '" + bible + "' " + translate ("does not contain") + " " + bookName + " " + filter::strings::convert_to_string (chapter) + ".");
        continue;
      }

      
      // Look for, report, and skip missing chapters in the comparison USFM data.
      if (find (compareChapters.begin(), compareChapters.end(), chapter) == compareChapters.end()) {
        if (find (resourceChapters.begin(), resourceChapters.end(), chapter) == resourceChapters.end()) {
          absent.push_back (translate("Bible/Resource") + " '" + compare + "' " + translate ("does not contain") + " " + bookName + " " + filter::strings::convert_to_string (chapter) + ".");
          continue;
        }
      }
      

      // Get source and compare USFM, and skip them if they are equal.
      std::string bible_chapter_usfm = database_bibles.get_chapter (bible, book, chapter);
      std::string compare_chapter_usfm = database_bibles.get_chapter (compare, book, chapter);
      if (compare_chapter_usfm == "") {
        compare_chapter_usfm = database_usfmresources.getUsfm (compare, book, chapter);
      }
      if (bible_chapter_usfm == compare_chapter_usfm) continue;
      
      
      // Get the combined set of verses in the chapter of the Bible and of the USFM to compare with.
      std::vector <int> bible_verse_numbers = filter::usfm::get_verse_numbers (bible_chapter_usfm);
      std::vector <int> compare_verse_numbers = filter::usfm::get_verse_numbers (compare_chapter_usfm);
      std::vector <int> verses;
      {
        std::set <int> verseset;
        verseset.insert (bible_verse_numbers.begin(), bible_verse_numbers.end());
        verseset.insert (compare_verse_numbers.begin(), compare_verse_numbers.end());
        verses.assign (verseset.begin(), verseset.end ());
        std::sort (verses.begin(), verses.end());
      }
      
      
      for (const int & verse : verses) {
 

        // Get the USFM of verse of the Bible and comparison USFM, and skip it if both are the same.
        std::string bible_verse_usfm = filter::usfm::get_verse_text (bible_chapter_usfm, verse);
        std::string compare_verse_usfm = filter::usfm::get_verse_text (compare_chapter_usfm, verse);
        if (bible_verse_usfm == compare_verse_usfm) continue;
        
        Filter_Text filter_text_bible = Filter_Text (bible);
        Filter_Text filter_text_compare = Filter_Text (compare);
        filter_text_bible.html_text_standard = new HtmlText ("");
        filter_text_compare.html_text_standard = new HtmlText ("");
        filter_text_bible.text_text = new Text_Text ();
        filter_text_compare.text_text = new Text_Text ();
        filter_text_bible.add_usfm_code (bible_verse_usfm);
        filter_text_compare.add_usfm_code (compare_verse_usfm);
        filter_text_bible.run (stylesheet);
        filter_text_compare.run (stylesheet);
        std::string bible_html = filter_text_bible.html_text_standard->get_inner_html ();
        std::string compare_html = filter_text_compare.html_text_standard->get_inner_html ();
        std::string bible_text = filter_text_bible.text_text->get ();
        std::string compare_text = filter_text_compare.text_text->get ();
        if (bible_text != compare_text) {
          std::string modification = filter_diff_diff (compare_text, bible_text);
          result.push_back (filter_passage_display (book, chapter, filter::strings::convert_to_string (verse)) + " " + modification);
          new_verses.push_back (filter_passage_display (book, chapter, filter::strings::convert_to_string (verse)) + " " + bible_text);
        }
        std::string modification = filter_diff_diff (compare_verse_usfm, bible_verse_usfm);
        raw.push_back (filter_passage_display (book, chapter, filter::strings::convert_to_string (verse)) + " " + modification);
      }
    }
  }

  
  // Add the absent books / chapters to the comparison.
  if (!absent.empty ()) {
    result.push_back ("");
    result.insert (result.end (), absent.begin(), absent.end());
  }

  
  // Add any differences in the raw USFM to the comparison.
  if (!raw.empty ()) {
    result.push_back ("");
    result.insert (result.end (), raw.begin(), raw.end());
  }
 
  
  // Add the text of the new verses, as they are in the $bible.
  if (!new_verses.empty ()) {
    result.push_back ("");
    result.push_back (translate("The texts as they are in the Bible") + " " + bible);
    result.push_back ("");
    result.insert (result.end(), new_verses.begin(), new_verses.end());
  }
  
  
  // Format and store the result of the comparison.
  for (auto & line : result) {
    if (line == "") {
      line = "<br>";
    } else {
      line.insert (0, "<p>");
      line.append ("</p>");
    }
  }
  database_jobs.set_result (jobId, filter::strings::implode (result, "\n"));
  
  
  Database_Logs::log (translate("Comparison is ready"), Filter_Roles::consultant ());
}
