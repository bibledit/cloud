/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#include <filter/abbreviations.h>
#include <database/books.h>
#include <filter/string.h>


// This function reads the $data for the Bible book abbreviations.
// It then transforms that data into pairs of <book identifier, abbreviation>.
// It returns them as a vector.
vector <pair <int, string> > filter_abbreviations_read (string data)
{
  vector <pair <int, string> > output;
  vector <string> v_data = filter_string_explode (data, '\n');
  for (string & entry : v_data) {
    if (entry.empty ()) continue;
    entry = filter_string_trim (entry);
    vector <string> v_entry = filter_string_explode (entry, '=');
    if (v_entry.size() != 2) continue;
    string sbook = filter_string_trim (v_entry [0]);
    int book = Database_Books::getIdFromEnglish (sbook);
    if (book == 0) continue;
    string abbrev = filter_string_trim (v_entry [1]);
    if (abbrev == "") continue;
    output.push_back (make_pair (book, abbrev));
  }
  return output;
}


// This function takes the abbreviations stored for a Bible as $data.
// It then sorts them out.
// It adds the Bible books not in the $data.
// It sorts everything in the standard canonical order.
// It presents everything for display to the user for editing and update.
string filter_abbreviations_display (string data)
{
  vector <pair <int, string> > v_data = filter_abbreviations_read (data);
  
  vector <string> v_output;
  
  // Check for books, order them, supply missing ones.
  vector <int> books = Database_Books::getIDs ();
  for (int book : books) {
    bool found = false;
    for (auto element : v_data) {
      int bookId = element.first;
      string abbreviation = element.second;
      if (book == bookId) {
        v_output.push_back (Database_Books::getEnglishFromId (book) + " = " + abbreviation);
        found = true;
      }
    }
    if (!found) {
      v_output.push_back (Database_Books::getEnglishFromId (book) + " = ");
    }
  }
  
  // Get data as a string for display.
  return filter_string_implode (v_output, "\n");
}


// Add $fullname and $abbreviation to the $existing abbreviations.
// Return the result.
string filter_abbreviations_add (string existing, string fullname, string abbreviation)
{
  existing = filter_abbreviations_display (existing);
  existing += "\n" + fullname + " = " + abbreviation;
  return existing;
}


// Sorts the $abbreviations, so that the longer abbrivations come first,
// and the shorter ones follow.
// It returns the sorted vector.
vector <pair <int, string> > filter_abbreviations_sort (vector <pair <int, string> > abbreviations)
{
  vector <string> abbrevs;
  vector <unsigned int> lengths;
  for (auto & element : abbreviations) {
    string abbrev = element.second;
    abbrevs.push_back (abbrev);
    int length = unicode_string_length (abbrev);
    lengths.push_back (length);
  }
  quick_sort (lengths, abbrevs, 0, lengths.size ());
  reverse (abbrevs.begin (), abbrevs.end ());

  vector <pair <int, string> > output;
  for (unsigned int i = 0; i < abbrevs.size (); i++) {
    string abbrev = abbrevs [i];
    for (auto & element : abbreviations) {
      if (element.second == abbrev) {
        int book = element.first;
        output.push_back (make_pair (book, abbrev));
      }
    }
  }
  return output;
}
