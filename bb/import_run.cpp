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


#include <bb/import_run.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/logs.h>
#include <database/books.h>
#include <bb/logic.h>
#include <styles/logic.h>


void bible_import_run (std::string location, const std::string& bible, int book, int chapter)
{
  Database_Logs::log ("Importing Bible data from location " + location + " into Bible " + bible);

  const std::string folder = filter_archive_uncompress (location);
  if (!folder.empty ()) location = folder;
  std::vector <std::string> files {};
  if (filter_url_is_dir (location)) {
    filter_url_recursive_scandir (location, files);
  } else {
    files.push_back (location);
  }
  
  for (const auto & file : files) {
    if (filter_url_is_dir (file)) continue;
    Database_Logs::log ("Examining file for import: " + file);
    std::string success_message {};
    std::string error_message {};
    const std::string data = filter_url_file_get_contents (file);
    if (!data.empty()) {
      if (filter::strings::unicode_string_is_valid (data)) {
        
        // Check whether this is USFM data.
        bool id = data.find ("\\id ") != std::string::npos;
        bool c = data.find ("\\c ") != std::string::npos;
        bool xml = data.find ("<?xml") != std::string::npos;
        if (!xml) {
          if (id || c) {
            bible_import_usfm (data, bible, book, chapter);
          } else {
            bible_import_text (data, bible, book, chapter);
          }
        } else {
          Database_Logs::log ("The file seems to be an XML file.", true);
        }
      } else {
        Database_Logs::log ("The file does not contain valid Unicode UTF-8 text.", true);
      }
    } else {
      Database_Logs::log ("Nothing was imported.", true);
    }
  }
  
  Database_Logs::log ("Import Bible data has finished");
}


// Import the USFM in $data into $bible.
void bible_import_usfm (const std::string& data, const std::string& bible, int book, int chapter)
{
  (void) book;
  (void) chapter;
  const std::string stylesheet = styles_logic_standard_sheet ();
  const std::vector <filter::usfm::BookChapterData> book_chapter_text = filter::usfm::usfm_import (data, stylesheet);
  for (const auto& data2 : book_chapter_text) {
    const int book_number = data2.m_book;
    const int chapter_number = data2.m_chapter;
    const std::string chapter_data = data2.m_data;
    if (book_number > 0) {
      bible_logic::store_chapter (bible, book_number, chapter_number, chapter_data);
      const std::string book_name = database::books::get_usfm_from_id (static_cast<book_id>(book_number));
      Database_Logs::log ("Imported " + book_name + " " + std::to_string (chapter_number));
    } else {
      Database_Logs::log ("Could not import this data: " + chapter_data.substr (0, 1000));
    }
  }
}


// Import raw $text into $bible $book $chapter.
void bible_import_text (const std::string& text, const std::string& bible, int book, int chapter)
{
  // Consecutive discoveries.
  bool discoveries_passed {true};
  
  // Split the input text into separate lines.
  std::vector <std::string> lines = filter::strings::explode (text, '\n');
  
  // Go through the lines.
  for (size_t i = 0; i < lines.size(); i++) {
    
    // Trim the line.
    lines[i] = filter::strings::trim (lines[i]);
    
    // Skip empty line.
    if (lines[i].empty())
      continue;
    
    // Remove chapter markup.
    if (lines[i].find("\\c") != std::string::npos) {
      lines[i].clear();
      continue;
    }
    
    // Skip line starting with a backslash. The rationale is that this line already has markup.
    if (lines[i].substr(0, 1) == R"(\)")
      continue;
    
    // If the line is a number on its own, and the number agrees with the chapter number
    // that was set, it silently removes this line. But if it differs, an error comes up.
    if (discoveries_passed) {
      if (filter::strings::number_in_string(lines[i]) == lines[i]) {
        const int number = filter::strings::convert_to_int (filter::strings::number_in_string (lines[i]));
        if (number == chapter) {
          lines[i].clear();
          continue;
        }
        const std::string msg = "The line that contains " + lines[i] + " looks like a chapter number, but the number differs from the chapter that was set";
        Database_Logs::log (msg);
        discoveries_passed = false;
      }
    }
    
    // If the line has no number in it,
    // and it ends with some type of punctuation,
    // it is considered a a normal paragraph.
    // If no punctuation at the end, it is a section heading.
    if (discoveries_passed) {
      if (filter::strings::number_in_string(lines[i]).empty()) {
        const std::string last_character = lines[i].substr(lines[i].length() -1, 1);
        if (filter::strings::unicode_string_is_punctuation (last_character)) {
          lines[i].insert(0, "\\p ");
        } else {
          lines[i].insert(0, "\\s ");
        }
        continue;
      }
    }
    
    // If a number is found in the line, then this is considered a verse number.
    // The first time a number is found, a \p is prefixed.
    bool paragraph_open = false;
    if (discoveries_passed) {
      std::string output {};
      std::string number = filter::strings::number_in_string(lines[i]);
      // Setting for having the number only at the start of the line.
      bool treat_as_normal_paragraph {false};
      bool verses_at_start {true};
      if (verses_at_start) {
        if (lines[i].find (number) != 0) {
          number.clear();
          treat_as_normal_paragraph = true;
        }
      }
      if (treat_as_normal_paragraph) {
        
        // Normal paragraph.
        lines[i].insert(0, "\\p ");
        
      } else {
        
        // Find all verse numbers.
        while (!number.empty()) {
          if (!paragraph_open) {
            output.append("\\p");
            paragraph_open = true;
          }
          size_t pos = lines[i].find(number);
          if (pos > 0) {
            output.append(" " + lines[i].substr(0, pos));
            lines[i].erase(0, pos);
          }
          output.append ("\n\\v ");
          output.append (number);
          output.append (" ");
          lines[i].erase (0, number.length());
          lines[i] = filter::strings::trim (lines[i]);
          number = filter::strings::number_in_string(lines[i]);
          // Setting for discovering only first number in a paragraph.
          if (verses_at_start) {
            number.clear();
          }
        }
        
      }
      // Store line.
      output.append(lines[i]);
      lines[i] = output;
    }
    
  }
  
  // Make one block of text.
  std::string newtext {};
  for (unsigned int i = 0; i < lines.size(); i++) {
    if (lines[i].empty())
      continue;
    lines[i] = filter::strings::collapse_whitespace (lines[i]);
    lines[i] = filter::strings::replace (" \n", "\n", lines[i]);
    newtext.append(lines[i]);
    newtext.append("\n");
  }
  
  // If no chapter marker is found, insert it at the top.
  if (newtext.find("\\c") == std::string::npos) {
    newtext.insert(0, "\\c " + std::to_string(chapter) + "\n");
  }

  // Import the text as USFM.
  bible_logic::store_chapter (bible, book, chapter, newtext);
  const std::string book_name = database::books::get_usfm_from_id (static_cast<book_id>(book));
  Database_Logs::log ("Imported " + book_name + " " + std::to_string (chapter) + ": " + text);
}
