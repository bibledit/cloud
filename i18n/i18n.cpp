/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


using namespace std;


#include "../database/stylesdata.h"
#include "../database/booksdata.h"


string file_get_contents (string filename)
{
  ifstream ifs(filename.c_str(), ios::in | ios::binary | ios::ate);
  streamoff filesize = ifs.tellg();
  if (filesize == 0) return "";
  ifs.seekg(0, ios::beg);
  vector <char> bytes((int)filesize);
  ifs.read(&bytes[0], (int)filesize);
  return string(&bytes[0], (int)filesize);
}


void file_put_contents (string filename, string contents)
{
  ofstream file;
  file.open(filename, ios::binary | ios::trunc);
  file << contents;
  file.close ();
}


vector <string> explode (string value, char delimiter)
{
  vector <string> result;
  istringstream iss (value);
  for (string token; getline (iss, token, delimiter); )
  {
    result.push_back (move (token));
  }
  return result;
}


string implode (vector <string>& values, string delimiter)
{
  string full;
  for (vector<string>::iterator it = values.begin (); it != values.end (); ++it)
  {
    full += (*it);
    if (it != values.end ()-1) full += delimiter;
  }
  return full;
}


string str_replace (string search, string replace, string subject)
{
  size_t offposition = subject.find (search);
  while (offposition != string::npos) {
    subject.replace (offposition, search.length (), replace);
    offposition = subject.find (search, offposition + replace.length ());
  }
  return subject;
}


int main ()
{
  // Read all html files to process.
  string contents = file_get_contents ("i18n.html");
  vector <string> files = explode (contents, '\n');
  cout << "Processing " << files.size () << " html files" << endl;
  
  // Store the translatable strings.
  vector <string> translatables;
  
  // Go over all html files.
  for (auto file : files) {
   
    // Read the html.
    contents = file_get_contents (file);
    
    // Clean up the "translate" (gettext) calls.
    contents = str_replace ("translate (", "translate(", contents);

    // Gettext markup.
    string gettextopen = "translate(\"";
    string gettextclose = "\")";
    
    // Limit gettext iterations.
    int iterations = 0;
    
    // Start processing variables by locating the first one.
    size_t position = contents.find (gettextopen);
    
    // Iterate through the contents till all gettext calls have been dealt with.
    while ((position != string::npos) && (iterations < 1000)) {
      iterations++;
      
      // Remove the gettext opener.
      contents.erase (position, gettextopen.length());
      
      // Position where the gettext call ends.
      size_t pos = contents.find (gettextclose, position);
      if (pos != string::npos) {
        
        // Take the gettext closer out.
        contents.erase (pos, gettextclose.length());
        
        // The English string.
        string english = contents.substr (position, pos - position);
        
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
    string english = styles_table[i].name;
    english.insert (0, "translate(\"");
    english.append ("\")");
    translatables.push_back (english);
    english = styles_table[i].info;
    english.insert (0, "translate(\"");
    english.append ("\")");
    translatables.push_back (english);
  }

  // Go over all Bible books to internationalize them.
  unsigned int books_data_count = sizeof (books_table) / sizeof (*books_table);
  for (unsigned int i = 0; i < books_data_count; i++) {
    string english = books_table[i].english;
    english.insert (0, "translate(\"");
    english.append ("\")");
    translatables.push_back (english);
  }
  
  // Store translatable strings.
  contents = implode (translatables, "\n");
  file_put_contents ("translatables.cpp", contents);

  return 0;
}
