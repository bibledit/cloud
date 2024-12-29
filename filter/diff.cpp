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


#include <filter/diff.h>
#include <filter/string.h>
#include <filter/text.h>
#include <filter/usfm.h>
#include <filter/url.h>
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <dtl/dtl.hpp>
#pragma GCC diagnostic pop
#include <webserver/request.h>
#include <database/modifications.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <html/text.h>
#include <text/text.h>
#include <locale/translate.h>
#include <developer/logic.h>


static std::mutex filter_diff_mutex;


// This filter returns the diff of two input strings.
// $oldstring: The old string for input.
// $newstring: The new string for input.
// The function returns the differences marked.
// If the containers for $removals and $additions are given,
// they will be filled with the appropriate text fragments.
std::string filter_diff_diff (std::string oldstring, std::string newstring,
                              std::vector <std::string> * removals,
                              std::vector <std::string> * additions)
{
  // Save the new lines.
  std::string newline = " newline_newline_newline ";
  oldstring = filter::strings::replace ("\n", newline, oldstring);
  newstring = filter::strings::replace ("\n", newline, newstring);
  
  // Split the input up into words.
  // It compares with word granularity.
  std::vector <std::string> old_sequence = filter::strings::explode (oldstring, ' ');
  std::vector <std::string> new_sequence = filter::strings::explode (newstring, ' ');
  
  // See issue https://github.com/bibledit/cloud/issues/419
  // It is unclear at this time whether the code below
  // to find the differences between texts, is thread-safe.
  // So just to be sure, a mutex is placed around it.
  filter_diff_mutex.lock();

  // Run the diff engine.
  dtl::Diff <std::string> diff (old_sequence, new_sequence);
  diff.compose();
  
  // Get the shortest edit distance.
  std::stringstream result;
  diff.printSES (result);

  filter_diff_mutex.unlock();
  
  // Add html markup for bold and strikethrough.
  std::vector <std::string> output = filter::strings::explode (result.str (), '\n');
  for (auto & line : output) {
    if (line.empty ()) continue;
    char indicator = line.front ();
    line.erase (0, 1);
    if (indicator == '+') {
      if (additions) additions->push_back (line);
      line.insert (0, R"(<span style="font-weight: bold;"> )");
      line.append (" </span>");
    }
    if (indicator == '-') {
      if (removals) removals->push_back(line);
      line.insert (0, R"(<span style="text-decoration: line-through;"> )");
      line.append (" </span>");
    }
  }
  
  // Resulting html.
  std::string html = filter::strings::implode (output, " ");
  
  // Restore the new lines.
  html = filter::strings::replace (filter::strings::trim (newline), "\n", html);
  
  return html;
}


// This filter returns the diff of two input vector<std::string>'s.
// $old: The old vector<std::string> for input.
// $new: The new vector<std::string> for input.
//
// The function produces information,
// that if applied to the old input, will produce the new input.
// This information consists of positions
// for addition or deletion operators,
// and if an addition, which content to add.
//
// Most UTF-8 characters in common use fit within two bytes when encoded in UTF-16.
// Javascript works with UTF-16.
// Also the Quilljs editor works with UTF-16.
// The positions in the Quill editor are influenced by
// whether the character is represented by 2 bytes in UTF-16, or by 4 bytes.
// A 2-byte UTF-16 character when inserted increases the position by 1.
// A 4-byte UTF-16 character when inserted increases the position by 2.
// When deleting a 4-byte UTF-16 character, the Quill API deletes 2 positions.
//
// The C++ server works with UTF-8.
// So there is a need for some translation in positios between UTF-8 in C++ and UTF-16 in Javascript.
// This function gives the positions as related to UTF-16.
// That means that characters that fit in 2-byte UTF-16 give their positions as 1.
// Those that fit in 4-byte UTF-16 give their positions as 2.
// Each differing character is given a size of 1 or 2 accordingly.
void filter_diff_diff_utf16 (const std::vector <std::string> & oldinput, const std::vector <std::string> & newinput,
                             std::vector <int> & positions,
                             std::vector <int> & sizes,
                             std::vector <bool> & additions,
                             std::vector <std::string> & content,
                             int & new_line_diff_count)
{
  // Clear anything from the output containers just to be sure.
  positions.clear();
  sizes.clear();
  additions.clear();
  content.clear();
  
  // Start with zero changes in a new line.
  new_line_diff_count = 0;
  
  // The sequences to compare.
  std::vector <std::string> old_sequence = oldinput;
  std::vector <std::string> new_sequence = newinput;

  // Save the new lines.
  std::string newline = "_newline_";
  for (auto & s : old_sequence) {
    s = filter::strings::replace ("\n", newline, s);
  }
  for (auto & s : new_sequence) {
    s = filter::strings::replace ("\n", newline, s);
  }

  // Run the diff engine.
  dtl::Diff <std::string> diff (old_sequence, new_sequence);
  diff.compose();
  
  // Get the shortest edit distance.
  std::stringstream result;
  diff.printSES (result);

  // Convert the new line place holder back to the original new line.
  std::vector <std::string> differences = filter::strings::explode (result.str (), '\n');
  for (auto & s : differences) {
    s = filter::strings::replace (newline, "\n", s);
  }

  // Convert the additions and deletions to a change set.
  int position = 0;
  for (auto & line : differences) {
    if (line.empty ()) continue;
    char indicator = line.front ();
    line.erase (0, 1);
    // Get the size of the character in UTF-16, whether 1 or 2.
    std::string utf8 = filter::strings::unicode_string_substr (line, 0, 1);
    std::u16string utf16 = filter::strings::convert_to_u16string (utf8);
    size_t size = utf16.length();
    if (indicator == '+') {
      // Something to be inserted into the old sequence to get at the new sequence.
      positions.push_back(position);
      sizes.push_back(static_cast<int> (size));
      additions.push_back(true);
      content.push_back(line);
      // Something was inserted.
      // So increase the position to point to the next offset in the sequence from where to proceed.
      position += static_cast<int>(size);
      // Check on number of changes in paragraphs.
      if (line.substr(0, 1) == "\n") new_line_diff_count++;
    }
    else if (indicator == '-') {
      // Something to be deleted at the given position.
      positions.push_back(position);
      sizes.push_back(static_cast<int> (size));
      additions.push_back(false);
      content.push_back(line);
      // Something was deleted.
      // So the position will remain to point to the same offset in the sequence from where to proceed.
      // Check on number of changes in paragraphs.
      if (line.substr(0, 1) == "\n") new_line_diff_count++;
    }
    else {
      // No difference.
      // Increase the position of the subsequent edit
      // with the amount of 16-bits code points of the current text bit in UTF-16.
      position += static_cast<int>(size);
    }
  }
}


// This calculates the similarity between the old and new strings.
// It works at the character level.
// It returns the similarity as a percentage.
// 100% means that the text is completely similar.
// And 0% means that the text is completely different.
// The output ranges from 0 to 100%.
int filter_diff_character_similarity (std::string oldstring, std::string newstring)
{
  try {
   
    // Split the input up into unicode characers.
    std::vector <std::string> old_sequence;
    std::vector <std::string> new_sequence;
    size_t oldlength = oldstring.size();
    for (size_t i = 0; i < oldlength; i++) {
      old_sequence.push_back (oldstring.substr (i, 1));
    }
    size_t newlength = newstring.size();
    for (size_t i = 0; i < newlength; i++) {
      new_sequence.push_back (newstring.substr (i, 1));
    }

    // See issue https://github.com/bibledit/cloud/issues/419
    // It is unclear at this time whether the code below
    // to find the differences between texts, is thread-safe.
    // So just to be sure, a mutex is placed around it.
    filter_diff_mutex.lock();

    // Run the diff engine.
    dtl::Diff <std::string> diff (old_sequence, new_sequence);
    diff.compose();
    
    // Get the shortest edit distance.
    std::stringstream result;
    diff.printSES (result);

    filter_diff_mutex.unlock();
    
    // Calculate the total elements compared, and the total differences found.
    int element_count = 0;
    int similar_count = 0;
    std::vector <std::string> output = filter::strings::explode (result.str(), '\n');
    for (auto & line : output) {
      if (line.empty ()) continue;
      element_count++;
      char indicator = line.front ();
      if (indicator == ' ') similar_count++;
    }
    
    // Calculate the percentage similarity.
    int percentage = static_cast<int> (round (100 * (static_cast<float>(similar_count) / static_cast<float>(element_count))));
    return percentage;
    
  } catch (...) {
  }
  // An exception was raised.
  // Usually related to invalid UTF-8.
  // Act as if there's no similarity at all.
  return 0;
}


// This calculates the similarity between the old and new strings.
// It works at the word level.
// It returns the similarity as a percentage.
// 100% means that the text is completely similar.
// And 0% means that the text is completely different.
// The output ranges from 0 to 100%.
int filter_diff_word_similarity (std::string oldstring, std::string newstring)
{
  // Split the input up into words separated by spaces.
  std::vector <std::string> old_sequence;
  std::vector <std::string> new_sequence;
  oldstring = filter::strings::replace ("\n", " ", oldstring);
  newstring = filter::strings::replace ("\n", " ", newstring);
  old_sequence = filter::strings::explode (oldstring, ' ');
  new_sequence = filter::strings::explode (newstring, ' ');
  
  // See issue https://github.com/bibledit/cloud/issues/419
  // It is unclear at this time whether the code below
  // to find the differences between texts, is thread-safe.
  // So just to be sure, a mutex is placed around it.
  filter_diff_mutex.lock();

  // Run the diff engine.
  dtl::Diff <std::string> diff (old_sequence, new_sequence);
  diff.compose();
  
  // Get the shortest edit distance.
  std::stringstream result;
  diff.printSES (result);

  filter_diff_mutex.unlock();
  
  // Calculate the total elements compared, and the total differences found.
  int element_count = 0;
  int similar_count = 0;
  std::vector <std::string> output = filter::strings::explode (result.str(), '\n');
  for (auto & line : output) {
    if (line.empty ()) continue;
    element_count++;
    char indicator = line.front ();
    if (indicator == ' ') similar_count++;
  }
  
  // Calculate the percentage similarity.
  int percentage = static_cast<int> (round (100 * (static_cast<float>(similar_count) / static_cast<float>(element_count))));
  return percentage;
}


// This filter produces files in USFM, html and text format.
// The text files are to be used for showing the differences between them.
// The files contain all verses that differ.
// $bible: The Bible to go through.
// $directory: The existing directory where to put the files.
// Two files are created: verses_old.usfm and verses_new.usfm.
// The book chapter.verse precede each verse.
void filter_diff_produce_verse_level (std::string bible, std::string directory)
{
  Webserver_Request request;
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);
  
  std::vector <std::string> old_vs_usfm;
  std::vector <std::string> new_vs_usfm;
  
  Filter_Text filter_text_old = Filter_Text (bible);
  filter_text_old.html_text_standard = new HtmlText (translate("Bible"));
  filter_text_old.text_text = new Text_Text ();
  Filter_Text filter_text_new = Filter_Text (bible);
  filter_text_new.html_text_standard = new HtmlText (translate("Bible"));
  filter_text_new.text_text = new Text_Text ();
  
  std::vector <int> books = database::modifications::getTeamDiffBooks (bible);
  for (auto book : books) {
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    std::vector <int> chapters = database::modifications::getTeamDiffChapters (bible, book);
    for (auto chapter : chapters) {
      // Go through the combined verse numbers in the old and new chapter.
      std::string old_chapter_usfm = database::modifications::getTeamDiff (bible, book, chapter);
      std::string new_chapter_usfm = database::bibles::get_chapter (bible, book, chapter);
      std::vector <int> old_verse_numbers = filter::usfm::get_verse_numbers (old_chapter_usfm);
      std::vector <int> new_verse_numbers = filter::usfm::get_verse_numbers (new_chapter_usfm);
      std::vector <int> verses = old_verse_numbers;
      verses.insert (verses.end (), new_verse_numbers.begin (), new_verse_numbers.end ());
      verses = filter::strings::array_unique (verses);
      sort (verses.begin(), verses.end());
      for (auto verse : verses) {
        std::string old_verse_text = filter::usfm::get_verse_text (old_chapter_usfm, verse);
        std::string new_verse_text = filter::usfm::get_verse_text (new_chapter_usfm, verse);
        if (old_verse_text != new_verse_text) {
          std::string usfmCode = "\\p " + bookname + " " + std::to_string(chapter) + "." + std::to_string(verse) + ": " + old_verse_text;
          old_vs_usfm.push_back (usfmCode);
          filter_text_old.add_usfm_code (usfmCode);
          usfmCode = "\\p " + bookname + " " + std::to_string(chapter) + "." + std::to_string(verse) + ": " + new_verse_text;
          new_vs_usfm.push_back (usfmCode);
          filter_text_new.add_usfm_code (usfmCode);
        }
      }
    }
  }
  
  filter_url_file_put_contents (filter_url_create_path ({directory, "verses_old.usfm"}), filter::strings::implode (old_vs_usfm, "\n"));
  filter_url_file_put_contents (filter_url_create_path ({directory, "verses_new.usfm"}), filter::strings::implode (new_vs_usfm, "\n"));
  filter_text_old.run (stylesheet);
  filter_text_new.run (stylesheet);
  filter_text_old.html_text_standard->save (filter_url_create_path ({directory, "verses_old.html"}));
  filter_text_new.html_text_standard->save (filter_url_create_path ({directory, "verses_new.html"}));
  filter_text_old.text_text->save (filter_url_create_path ({directory, "verses_old.txt"}));
  filter_text_new.text_text->save (filter_url_create_path ({directory, "verses_new.txt"}));
}


/**
 * This filter runs a diff.
 * $oldfile: The name of the old file for input.
 * $newfile: The name of the new file for input.
 * $outputfile: The name of the output file
 */
void filter_diff_run_file (std::string oldfile, std::string newfile, std::string outputfile)
{
  std::string oldstring = filter_url_file_get_contents (oldfile);
  std::string newstring = filter_url_file_get_contents (newfile);

  std::string differences = filter_diff_diff (oldstring, newstring);
  
  std::vector <std::string> lines = filter::strings::explode (differences, '\n');
  for (auto & line : lines) {
    line = "<p>" + line + "</p>";
  }
  differences = filter::strings::implode (lines, "\n");
  
  filter_url_file_put_contents (outputfile, differences);
}
