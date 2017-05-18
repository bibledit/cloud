/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#include <checks/french.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <database/check.h>
#include <locale/translate.h>


// In French there is a no-break space after the « and before the » ! ? : ;
// The Unicode value for the no-break space is U+00A0.
void Checks_French::spaceBeforeAfterPunctuation (string bible, int book, int chapter, map <int, string> texts)
{
  Database_Check database_check;
  string nbsp = no_break_space_utf8_00a0 ();
  string left_guillemet = "«";
  vector <string> right_punctuation = { "»", "!", "?", ":", ";" };
  for (auto element : texts) {
    int verse = element.first;

    {
      string text = element.second;
      size_t pos = text.find (left_guillemet);
      while (pos != string::npos) {
        text.erase (0, pos + left_guillemet.size ());
        if (text.find (" ") == 0) {
          string message = left_guillemet + " - " + translate ("Should be followed by a no-break space rather than a plain space in French");
          database_check.recordOutput (bible, book, chapter, verse, message);
        } else if (text.find (nbsp) != 0) {
          string message = left_guillemet + " - " + translate ("Should be followed by a no-break space in French");
          database_check.recordOutput (bible, book, chapter, verse, message);
        }
        pos = text.find (left_guillemet);
      }
    }
    
    for (auto punctuation : right_punctuation) {
      string text = element.second;
      size_t pos = text.find (punctuation);
      while (pos != string::npos) {
        if ((pos > 0) && (text.substr (pos - 1, 1) == " ")) {
          string message = punctuation + " - " + translate ("Should be preceded by a no-break space rather than a plain space in French");
          database_check.recordOutput (bible, book, chapter, verse, message);
        } else if ((pos > 2) && (text.substr (pos - 2, 2) != nbsp)) {
          string message = punctuation + " - " + translate ("Should be preceded by a no-break space in French");
          database_check.recordOutput (bible, book, chapter, verse, message);
        }
        text.erase (0, pos + punctuation.size ());
        pos = text.find (punctuation);
      }
    }
    
  }
}
