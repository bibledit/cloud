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


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <set>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <thread>
#include <cmath>
#include <mutex>
#include <numeric>
#include <random>
#include <limits>


#include "database/stylesdata.h"
#include "database/books.h"
#include "database/booksdata.h"


std::string file_get_contents (const std::string& filename)
{
  std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
  std::streamoff filesize = ifs.tellg();
  if (filesize == 0) return std::string();
  ifs.seekg(0, std::ios::beg);
  std::vector <char> bytes((int)filesize);
  ifs.read(&bytes[0], (int)filesize);
  return std::string(&bytes[0], (int)filesize);
}


void file_put_contents (const std::string& filename, const std::string& contents)
{
  std::ofstream file;
  file.open(filename, std::ios::binary | std::ios::trunc);
  file << contents;
  file.close ();
}


std::vector <std::string> explode (const std::string& value, char delimiter)
{
  std::vector <std::string> result {};
  std::istringstream iss (value);
  for (std::string token; std::getline (iss, token, delimiter); )
  {
    result.push_back (std::move (token));
  }
  return result;
}


std::string implode (const std::vector <std::string>& values, const std::string delimiter)
{
  std::string full {};
  for (auto iterator = values.begin (); iterator != values.end (); ++iterator)
  {
    full += (*iterator);
    if (iterator != values.end ()-1) full += delimiter;
  }
  return full;
}


std::string str_replace (const std::string& search, const std::string& replace, std::string subject)
{
  size_t offposition = subject.find (search);
  while (offposition != std::string::npos) {
    subject.replace (offposition, search.length (), replace);
    offposition = subject.find (search, offposition + replace.length ());
  }
  return subject;
}


int main ()
{
  // Read all html files to process.
  std::string contents = file_get_contents ("i18n.html");
  const std::vector <std::string> files = explode (contents, '\n');
  std::cout << "Processing " << files.size () << " html files" << std::endl;
  
  // Store the translatable strings.
  std::vector <std::string> translatables {};
  
  // Go over all html files.
  for (const auto& file : files) {
   
    // Read the html.
    contents = file_get_contents (file);
    
    // Clean up the "translate" (gettext) calls.
    contents = str_replace ("translate (", "translate(", contents);

    // Gettext markup.
    const std::string gettextopen = R"(translate(")";
    const std::string gettextclose = R"("))";
    
    // Limit gettext iterations.
    int iterations = 0;
    
    // Start processing variables by locating the first one.
    size_t position = contents.find (gettextopen);
    
    // Iterate through the contents till all gettext calls have been dealt with.
    while ((position != std::string::npos) && (iterations < 1000)) {
      iterations++;
      
      // Remove the gettext opener.
      contents.erase (position, gettextopen.length());
      
      // Position where the gettext call ends.
      size_t pos = contents.find (gettextclose, position);
      if (pos != std::string::npos) {
        
        // Take the gettext closer out.
        contents.erase (pos, gettextclose.length());
        
        // The English string.
        std::string english = contents.substr (position, pos - position);
        
        // If the English string is empty, don't store it.
        if (english.empty()) continue;
        
        // Wrap it in calls recognizable as gettext calls, and store it.
        english.insert (0, "translate(\"");
        english.append ("\")");
        translatables.push_back (english);
      }

      // Next gettext call.
      position = contents.find (gettextopen);
    }
  }

  // Go over all USFM styles to internationalize them.
  unsigned int styles_data_count = sizeof (styles_table) / sizeof (*styles_table);
  for (unsigned int i = 0; i < styles_data_count; i++) {
    std::string english = styles_table[i].name;
    if (!english.empty()) {
      english.insert (0, "translate(\"");
      english.append ("\")");
      translatables.push_back (english);
    }
    english = styles_table[i].info;
    if (!english.empty()) {
      english.insert (0, "translate(\"");
      english.append ("\")");
      translatables.push_back (english);
    }
  }

  // Go over all Bible books to internationalize them.
  unsigned int books_data_count = sizeof (books_table) / sizeof (*books_table);
  for (unsigned int i = 0; i < books_data_count; i++) {
    std::string english = books_table[i].english;
    if (!english.empty()) {
      english.insert (0, "translate(\"");
      english.append ("\")");
      translatables.push_back (english);
    }
    std::string osis = books_table[i].osis;
    if (!osis.empty()) {
      osis.insert (0, "translate(\"");
      osis.append ("\")");
      translatables.push_back (osis);
    }
    std::string bibleworks = books_table[i].bibleworks;
    if (!bibleworks.empty ()) {
      bibleworks.insert (0, "translate(\"");
      bibleworks.append ("\")");
      translatables.push_back (bibleworks);
    }
  }
  
  // Store translatable strings.
  contents = implode (translatables, "\n");
  file_put_contents ("translatables.cpp", contents);

  return 0;
}
