/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


// Compare the $bible with another Bible, passed through $compare.
void compare_compare (const std::string& bible, const std::string& compare, const int job_id)
{
  Database_Logs::log (translate("Comparing Bibles") + " " + bible + " " + translate ("and") + " " + compare, Filter_Roles::consultant ());

  
  Database_Jobs database_jobs = Database_Jobs ();
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();

  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  

  database_jobs.set_progress (job_id, translate("The Bibles are being compared..."));
  

  // The results of the comparison. Will be displayed to the user.
  std::vector <std::string> result;
  {
    std::stringstream ss {};
    ss << translate("Bible") << " " << std::quoted(bible) << " " << translate ("has been compared with") << " " << std::quoted(compare) << ".";
    result.push_back (ss.str());
  }
  result.push_back (translate("Additions are in bold.") + " " + translate ("Removed words are in strikethrough."));
  result.push_back (std::string());
  
  
  // Get the combined distinct books in both Bibles / Resources.
  const std::vector <int> bible_books = database::bibles::get_books (bible);
  const std::vector <int> compare_books = database::bibles::get_books (compare);
  const std::vector <int> resource_books = database_usfmresources.getBooks (compare);
  std::vector <int> books;
  {
    std::set <int> bookset;
    bookset.insert (bible_books.begin(), bible_books.end());
    bookset.insert (compare_books.begin(), compare_books.end());
    bookset.insert (resource_books.begin(), resource_books.end());
    books.assign (bookset.begin(), bookset.end ());
    std::sort (books.begin(), books.end());
  }
  
  
  // Results of comparison of raw USFM.
  std::vector <std::string> raw;
  
  
  // Absent books / chapters.
  std::vector <std::string> absent;
  
  
  // The new verses as in the $bible.
  std::vector <std::string> new_verses;
  
  
  for (const auto& book : books) {

    
    const std::string book_name = database::books::get_english_from_id (static_cast<book_id>(book));
    database_jobs.set_progress (job_id, book_name);
    
    
    if (std::find (bible_books.begin(), bible_books.end(), book) == bible_books.end()) {
      std::stringstream ss {};
      ss << translate("Bible") << " " << std::quoted(bible) << " " << translate ("does not contain") << " " << book_name << ".";
      absent.push_back (ss.str());
      continue;
    }
    
    if (std::find (compare_books.begin(), compare_books.end(), book) == compare_books.end()) {
      if (std::find (resource_books.begin(), resource_books.end(), book) == resource_books.end ()) {
        std::stringstream ss{};
        ss << translate("Bible/Resource") << " " << std::quoted(compare) << " " << translate ("does not contain") << " " << book_name << ".";
        absent.push_back (ss.str());
        continue;
      }
    }
    
    
    // Get the combined distinct chapters in both Bibles / Resources.
    const std::vector <int> bible_chapters = database::bibles::get_chapters (bible, book);
    const std::vector <int> compare_chapters = database::bibles::get_chapters (compare, book);
    const std::vector <int> resource_chapters = database_usfmresources.getChapters (compare, book);
    std::vector <int> chapters;
    {
      std::set <int> chapterset;
      chapterset.insert (bible_chapters.begin(), bible_chapters.end());
      chapterset.insert (compare_chapters.begin(), compare_chapters.end());
      chapterset.insert (resource_chapters.begin(), resource_chapters.end());
      chapters.assign (chapterset.begin(), chapterset.end ());
      std::sort (chapters.begin(), chapters.end());
    }


    for (const auto& chapter : chapters) {

      
      // Look for, report, and skip missing chapters in the source Bible.
      if (std::find (bible_chapters.begin(), bible_chapters.end(), chapter) == bible_chapters.end ()) {
        std::stringstream ss {};
        ss << translate("Bible") << " " << std::quoted(bible) << " " << translate ("does not contain") << " " << book_name << " " << chapter << ".";
        absent.push_back (ss.str());
        continue;
      }

      
      // Look for, report, and skip missing chapters in the comparison USFM data.
      if (std::find (compare_chapters.begin(), compare_chapters.end(), chapter) == compare_chapters.end()) {
        if (std::find (resource_chapters.begin(), resource_chapters.end(), chapter) == resource_chapters.end()) {
          std::stringstream ss {};
          ss << translate("Bible/Resource") << " " << std::quoted(compare) << " " << translate ("does not contain") << " " << book_name << " " << chapter << ".";
          absent.push_back (ss.str());
          continue;
        }
      }
      

      // Get source and compare USFM, and skip them if they are equal.
      const std::string bible_chapter_usfm = database::bibles::get_chapter (bible, book, chapter);
      std::string compare_chapter_usfm = database::bibles::get_chapter (compare, book, chapter);
      if (compare_chapter_usfm.empty()) {
        compare_chapter_usfm = database_usfmresources.getUsfm (compare, book, chapter);
      }
      if (bible_chapter_usfm == compare_chapter_usfm) 
        continue;
      
      
      // Get the sorted combined set of distinct verses in the chapter of the Bible and of the USFM to compare with.
      std::vector <int> verses {};
      {
        const std::vector <int> bible_verse_numbers = filter::usfm::get_verse_numbers (bible_chapter_usfm);
        const std::vector <int> compare_verse_numbers = filter::usfm::get_verse_numbers (compare_chapter_usfm);
        std::set <int> verse_set {};
        verse_set.insert (bible_verse_numbers.begin(), bible_verse_numbers.end());
        verse_set.insert (compare_verse_numbers.begin(), compare_verse_numbers.end());
        verses.assign (verse_set.begin(), verse_set.end ());
        std::sort (verses.begin(), verses.end());
      }
      
      
      for (const int& verse : verses) {
 

        // Get the USFM of verse of the Bible and comparison USFM, and skip it if both are the same.
        const std::string bible_verse_usfm = filter::usfm::get_verse_text (bible_chapter_usfm, verse);
        const std::string compare_verse_usfm = filter::usfm::get_verse_text (compare_chapter_usfm, verse);
        if (bible_verse_usfm == compare_verse_usfm)
          continue;
        
        Filter_Text filter_text_bible = Filter_Text (bible);
        Filter_Text filter_text_compare = Filter_Text (compare);
        filter_text_bible.html_text_standard = new HtmlText (std::string());
        filter_text_compare.html_text_standard = new HtmlText (std::string());
        filter_text_bible.text_text = new Text_Text ();
        filter_text_compare.text_text = new Text_Text ();
        filter_text_bible.add_usfm_code (bible_verse_usfm);
        filter_text_compare.add_usfm_code (compare_verse_usfm);
        filter_text_bible.run (stylesheet);
        filter_text_compare.run (stylesheet);
        const std::string bible_html = filter_text_bible.html_text_standard->get_inner_html ();
        const std::string compare_html = filter_text_compare.html_text_standard->get_inner_html ();
        const std::string bible_text = filter_text_bible.text_text->get ();
        const std::string compare_text = filter_text_compare.text_text->get ();
        if (bible_text != compare_text) {
          const std::string modification = filter_diff_diff (compare_text, bible_text);
          result.push_back (filter_passage_display (book, chapter, std::to_string (verse)) + " " + modification);
          new_verses.push_back (filter_passage_display (book, chapter, std::to_string (verse)) + " " + bible_text);
        }
        const std::string modification = filter_diff_diff (compare_verse_usfm, bible_verse_usfm);
        raw.push_back (filter_passage_display (book, chapter, std::to_string (verse)) + " " + modification);
      }
    }
  }

  
  // Add the absent books / chapters to the comparison.
  if (!absent.empty ()) {
    result.push_back (std::string());
    result.insert (result.end (), absent.begin(), absent.end());
  }

  
  // Add any differences in the raw USFM to the comparison.
  if (!raw.empty ()) {
    result.push_back (std::string());
    result.insert (result.end (), raw.begin(), raw.end());
  }
 
  
  // Add the text of the new verses, as they are in the $bible.
  if (!new_verses.empty ()) {
    result.push_back (std::string());
    result.push_back (translate("The texts as they are in the Bible") + " " + bible);
    result.push_back (std::string());
    result.insert (result.end(), new_verses.begin(), new_verses.end());
  }
  
  
  // Format and store the result of the comparison.
  for (auto& line : result) {
    if (line.empty()) {
      line = "<br>";
    } else {
      line.insert (0, "<p>");
      line.append ("</p>");
    }
  }
  database_jobs.set_result (job_id, filter::strings::implode (result, "\n"));
  
  
  Database_Logs::log (translate("Comparison is ready"), Filter_Roles::consultant ());
}
