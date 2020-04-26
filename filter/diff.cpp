/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


// This filter returns the diff of two input strngs.
// $oldstring: The old string for input.
// $newstring: The new string for input.
// The function returns the differences marked.
string filter_diff_diff (string oldstring, string newstring)
{
  // Type definitions for the diff template engine.
  typedef string elem;
  typedef vector <string> sequence;

  // Save the new lines.
  string newline = " newline_newline_newline ";
  oldstring = filter_string_str_replace ("\n", newline, oldstring);
  newstring = filter_string_str_replace ("\n", newline, newstring);
  
  // Split the input up into words.
  // It compares with word granularity.
  sequence oldvector = filter_string_explode (oldstring, ' ');
  sequence newvector = filter_string_explode (newstring, ' ');
  
  // Run the diff engine.
  Diff <elem> d (oldvector, newvector);
  d.compose();
  
  // Get the shortest edit distance.
  stringstream result;
  d.printSES (result);
  
  // Add html markup for bold and strikethrough.
  vector <string> output = filter_string_explode (result.str (), '\n');
  for (auto & line : output) {
    if (line.empty ()) continue;
    char indicator = line.front ();
    line.erase (0, 1);
    if (indicator == '+') {
      line.insert (0, "<span style=\"font-weight: bold;\"> ");
      line.append (" </span>");
    }
    if (indicator == '-') {
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


// This calculates the similarity between the old and new strings.
// It works at the character level.
// It returns the similarity as a percentage.
// 100% means that the text is completely similar.
// And 0% means that the text is completely different.
// The output ranges from 0 to 100%.
int filter_diff_character_similarity (string oldstring, string newstring)
{
  try {

    // Type definitions for the diff template engine.
    typedef string elem;
    typedef vector <string> sequence;
    
    // Split the input up into unicode characers.
    sequence oldvector;
    sequence newvector;
    size_t oldlength = oldstring.size();
    for (size_t i = 0; i < oldlength; i++) {
      oldvector.push_back (oldstring.substr (i, 1));
    }
    size_t newlength = newstring.size();
    for (size_t i = 0; i < newlength; i++) {
      newvector.push_back (newstring.substr (i, 1));
    }

    // Run the diff engine.
    Diff <elem> d (oldvector, newvector);
    d.compose();
    
    // Get the shortest edit distance.
    stringstream result;
    d.printSES (result);
    
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
  // Type definitions for the diff template engine.
  typedef string elem;
  typedef vector <string> sequence;
  
  // Split the input up into words separated by spaces.
  sequence oldvector;
  sequence newvector;
  oldstring = filter_string_str_replace ("\n", " ", oldstring);
  newstring = filter_string_str_replace ("\n", " ", newstring);
  oldvector = filter_string_explode (oldstring, ' ');
  newvector = filter_string_explode (newstring, ' ');
  
  // Run the diff engine.
  Diff <elem> d (oldvector, newvector);
  d.compose();
  
  // Get the shortest edit distance.
  stringstream result;
  d.printSES (result);
  
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
          filter_text_old.addUsfmCode (usfmCode);
          usfmCode = "\\p " + bookname + " " + convert_to_string (chapter) + "." + convert_to_string (verse) + ": " + new_verse_text;
          new_vs_usfm.push_back (usfmCode);
          filter_text_new.addUsfmCode (usfmCode);
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
