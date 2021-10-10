/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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
#include <dtl/dtl.hpp>
#include <webserver/request.h>
#include <database/modifications.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <html/text.h>
#include <text/text.h>
#include <locale/translate.h>
#include <developer/logic.h>


using dtl::Diff;


static mutex filter_diff_mutex;


// This filter returns the diff of two input strings.
// $oldstring: The old string for input.
// $newstring: The new string for input.
// The function returns the differences marked.
// If the containers for $removals and $additions are given,
// they will be filled with the appropriate text fragments.
string filter_diff_diff (string oldstring, string newstring,
                         vector <string> * removals,
                         vector <string> * additions)
{
  // Save the new lines.
  string newline = " newline_newline_newline ";
  oldstring = filter_string_str_replace ("\n", newline, oldstring);
  newstring = filter_string_str_replace ("\n", newline, newstring);
  
  // Split the input up into words.
  // It compares with word granularity.
  vector <string> old_sequence = filter_string_explode (oldstring, ' ');
  vector <string> new_sequence = filter_string_explode (newstring, ' ');
  
  // See issue https://github.com/bibledit/cloud/issues/419
  // It is unclear at this time whether the code below
  // to find the differences between texts, is thread-safe.
  // So just to be sure, a mutex is placed around it.
  filter_diff_mutex.lock();

  // Run the diff engine.
  Diff <string> diff (old_sequence, new_sequence);
  diff.compose();
  
  // Get the shortest edit distance.
  stringstream result;
  diff.printSES (result);

  filter_diff_mutex.unlock();
  
  // Add html markup for bold and strikethrough.
  vector <string> output = filter_string_explode (result.str (), '\n');
  for (auto & line : output) {
    if (line.empty ()) continue;
    char indicator = line.front ();
    line.erase (0, 1);
    if (indicator == '+') {
      if (additions) additions->push_back (line);
      line.insert (0, "<span style=\"font-weight: bold;\"> ");
      line.append (" </span>");
    }
    if (indicator == '-') {
      if (removals) removals->push_back(line);
      line.insert (0, "<span style=\"text-decoration: line-through;\"> ");
      line.append (" </span>");
    }
  }
  
  // Resulting html.
  string html = filter_string_implode (output, " ");
  
  // Restore the new lines.
  html = filter_string_str_replace (filter_string_trim (newline), "\n", html);
  
  return html;
}


// This filter returns the diff of two input vector<string>'s.
// $old: The old vector<string> for input.
// $new: The new vector<string> for input.
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
void filter_diff_diff_utf16 (const vector<string> & oldinput, const vector<string> & newinput,
                             vector <int> & positions,
                             vector <int> & sizes,
                             vector <bool> & additions,
                             vector <string> & content,
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
  vector <string> old_sequence = oldinput;
  vector <string> new_sequence = newinput;

  // Save the new lines.
  string newline = "_newline_";
  for (auto & s : old_sequence) {
    s = filter_string_str_replace ("\n", newline, s);
  }
  for (auto & s : new_sequence) {
    s = filter_string_str_replace ("\n", newline, s);
  }

  // Run the diff engine.
  Diff <string> diff (old_sequence, new_sequence);
  diff.compose();
  
  // Get the shortest edit distance.
  stringstream result;
  diff.printSES (result);

  // Convert the new line place holder back to the original new line.
  vector <string> differences = filter_string_explode (result.str (), '\n');
  for (auto & s : differences) {
    s = filter_string_str_replace (newline, "\n", s);
  }

  // Convert the additions and deletions to a change set.
  int position = 0;
  for (auto & line : differences) {
    if (line.empty ()) continue;
    char indicator = line.front ();
    line.erase (0, 1);
    // Get the size of the character in UTF-16, whether 1 or 2.
    string utf8 = unicode_string_substr (line, 0, 1);
    u16string utf16 = convert_to_u16string (utf8);
    size_t size = utf16.length();
    if (indicator == '+') {
      // Something to be inserted into the old sequence to get at the new sequence.
      positions.push_back(position);
      sizes.push_back((int)size);
      additions.push_back(true);
      content.push_back(line);
      // Something was inserted.
      // So increase the position to point to the next offset in the sequence from where to proceed.
      position += size;
      // Check on number of changes in paragraphs.
      if (line.substr(0, 1) == "\n") new_line_diff_count++;
    }
    else if (indicator == '-') {
      // Something to be deleted at the given position.
      positions.push_back(position);
      sizes.push_back((int)size);
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
      position += size;
    }
  }
}


// This calculates the similarity between the old and new strings.
// It works at the character level.
// It returns the similarity as a percentage.
// 100% means that the text is completely similar.
// And 0% means that the text is completely different.
// The output ranges from 0 to 100%.
int filter_diff_character_similarity (string oldstring, string newstring)
{
  try {
   
    // Split the input up into unicode characers.
    vector <string> old_sequence;
    vector <string> new_sequence;
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
    Diff <string> diff (old_sequence, new_sequence);
    diff.compose();
    
    // Get the shortest edit distance.
    stringstream result;
    diff.printSES (result);

    filter_diff_mutex.unlock();
    
    // Calculate the total elements compared, and the total differences found.
    int element_count = 0;
    int similar_count = 0;
    vector <string> output = filter_string_explode (result.str(), '\n');
    for (auto & line : output) {
      if (line.empty ()) continue;
      element_count++;
      char indicator = line.front ();
      if (indicator == ' ') similar_count++;
    }
    
    // Calculate the percentage similarity.
    int percentage = round (100 * ((float) similar_count / (float) element_count));
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
int filter_diff_word_similarity (string oldstring, string newstring)
{
  // Split the input up into words separated by spaces.
  vector <string> old_sequence;
  vector <string> new_sequence;
  oldstring = filter_string_str_replace ("\n", " ", oldstring);
  newstring = filter_string_str_replace ("\n", " ", newstring);
  old_sequence = filter_string_explode (oldstring, ' ');
  new_sequence = filter_string_explode (newstring, ' ');
  
  // See issue https://github.com/bibledit/cloud/issues/419
  // It is unclear at this time whether the code below
  // to find the differences between texts, is thread-safe.
  // So just to be sure, a mutex is placed around it.
  filter_diff_mutex.lock();

  // Run the diff engine.
  Diff <string> diff (old_sequence, new_sequence);
  diff.compose();
  
  // Get the shortest edit distance.
  stringstream result;
  diff.printSES (result);

  filter_diff_mutex.unlock();
  
  // Calculate the total elements compared, and the total differences found.
  int element_count = 0;
  int similar_count = 0;
  vector <string> output = filter_string_explode (result.str(), '\n');
  for (auto & line : output) {
    if (line.empty ()) continue;
    element_count++;
    char indicator = line.front ();
    if (indicator == ' ') similar_count++;
  }
  
  // Calculate the percentage similarity.
  int percentage = round (100 * ((float) similar_count / (float) element_count));
  return percentage;
}


// This filter produces files in USFM, html and text format.
// The text files are to be used for showing the differences between them.
// The files contain all verses that differ.
// $bible: The Bible to go through.
// $directory: The existing directory where to put the files.
// Two files are created: verses_old.usfm and verses_new.usfm.
// The book chapter.verse precede each verse.
void filter_diff_produce_verse_level (string bible, string directory)
{
  Webserver_Request request;
  Database_Modifications database_modifications;
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  vector <string> old_vs_usfm;
  vector <string> new_vs_usfm;
  
  Filter_Text filter_text_old = Filter_Text (bible);
  filter_text_old.html_text_standard = new Html_Text (translate("Bible"));
  filter_text_old.text_text = new Text_Text ();
  Filter_Text filter_text_new = Filter_Text (bible);
  filter_text_new.html_text_standard = new Html_Text (translate("Bible"));
  filter_text_new.text_text = new Text_Text ();
  
  vector <int> books = database_modifications.getTeamDiffBooks (bible);
  for (auto book : books) {
    string bookname = Database_Books::getEnglishFromId (book);
    vector <int> chapters = database_modifications.getTeamDiffChapters (bible, book);
    for (auto chapter : chapters) {
      // Go through the combined verse numbers in the old and new chapter.
      string old_chapter_usfm = database_modifications.getTeamDiff (bible, book, chapter);
      string new_chapter_usfm = request.database_bibles()->getChapter (bible, book, chapter);
      vector <int> old_verse_numbers = usfm_get_verse_numbers (old_chapter_usfm);
      vector <int> new_verse_numbers = usfm_get_verse_numbers (new_chapter_usfm);
      vector <int> verses = old_verse_numbers;
      verses.insert (verses.end (), new_verse_numbers.begin (), new_verse_numbers.end ());
      verses = array_unique (verses);
      sort (verses.begin(), verses.end());
      for (auto verse : verses) {
        string old_verse_text = usfm_get_verse_text (old_chapter_usfm, verse);
        string new_verse_text = usfm_get_verse_text (new_chapter_usfm, verse);
        if (old_verse_text != new_verse_text) {
          string usfmCode = "\\p " + bookname + " " + convert_to_string (chapter) + "." + convert_to_string (verse) + ": " + old_verse_text;
          old_vs_usfm.push_back (usfmCode);
          filter_text_old.add_usfm_code (usfmCode);
          usfmCode = "\\p " + bookname + " " + convert_to_string (chapter) + "." + convert_to_string (verse) + ": " + new_verse_text;
          new_vs_usfm.push_back (usfmCode);
          filter_text_new.add_usfm_code (usfmCode);
        }
      }
    }
  }
  
  filter_url_file_put_contents (filter_url_create_path (directory, "verses_old.usfm"), filter_string_implode (old_vs_usfm, "\n"));
  filter_url_file_put_contents (filter_url_create_path (directory, "verses_new.usfm"), filter_string_implode (new_vs_usfm, "\n"));
  filter_text_old.run (stylesheet);
  filter_text_new.run (stylesheet);
  filter_text_old.html_text_standard->save (filter_url_create_path (directory, "verses_old.html"));
  filter_text_new.html_text_standard->save (filter_url_create_path (directory, "verses_new.html"));
  filter_text_old.text_text->save (filter_url_create_path (directory, "verses_old.txt"));
  filter_text_new.text_text->save (filter_url_create_path (directory, "verses_new.txt"));
}


/**
 * This filter runs a diff.
 * $oldfile: The name of the old file for input.
 * $newfile: The name of the new file for input.
 * $outputfile: The name of the output file
 */
void filter_diff_run_file (string oldfile, string newfile, string outputfile)
{
  string oldstring = filter_url_file_get_contents (oldfile);
  string newstring = filter_url_file_get_contents (newfile);

  string differences = filter_diff_diff (oldstring, newstring);
  
  vector <string> lines = filter_string_explode (differences, '\n');
  for (auto & line : lines) {
    line = "<p>" + line + "</p>";
  }
  differences = filter_string_implode (lines, "\n");
  
  filter_url_file_put_contents (outputfile, differences);
}
