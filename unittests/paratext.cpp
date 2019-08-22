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


#include <unittests/paratext.h>
#include <unittests/utilities.h>
#include <paratext/logic.h>
#include <filter/string.h>


void test_paratext () // Todo
{
  trace_unit_tests (__func__);

  refresh_sandbox (true);

  // If Bibledit has the chapter, and Paratext does not, take the Bibledit chapter.
  {
    string ancestor;
    string bibledit = R"(
\c 1
\s header
\p
\v 1 verse 1.
\v 2 verse 2.
    )";
    string paratext;
    vector <string> messages;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages);
    evaluate (__LINE__, __func__, bibledit, result);
    evaluate (__LINE__, __func__, 1, messages.size ());
  }
  
  // If Paratext has the chapter, and Bibledit does not, take the Paratext chapter.
  {
    string ancestor;
    string bibledit;
    string paratext = R"(
\c 1
\s header
\p
\v 1 verse 1.
\v 2 verse 2.
    )";
    vector <string> messages;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages);
    evaluate (__LINE__, __func__, paratext, result);
    evaluate (__LINE__, __func__, 1, messages.size ());
  }
  
  // Bibledit and Paratext are the same: Do nothing.
  {
    string ancestor;
    string bibledit = R"(
\c 1
\s header
\p
\v 1 verse 1.
\v 2 verse 2.
    )";
    string paratext (bibledit);
    vector <string> messages;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages);
    evaluate (__LINE__, __func__, "", result);
    evaluate (__LINE__, __func__, 0, messages.size ());
  }
  
  // Normal merge operation between Bibledit and Paratext data.
  {
    string bibledit = R"(
\c 1
\s header
\p
\v 1 bibledit 1.
\v 2 bibledit 2.
    )";
    string paratext = R"(
\c 1
\s header
\p
\v 1 paratext 1.
\v 2 paratext 2.
    )";
    bibledit = filter_string_trim (bibledit);
    paratext = filter_string_trim (paratext);
    {
      // Test it takes the changes from Paratext.
      string ancestor (bibledit);
      vector <string> messages;
      string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages);
      evaluate (__LINE__, __func__, paratext, result);
      evaluate (__LINE__, __func__, 1, messages.size ());
      if (messages.size() == 1) {
        evaluate (__LINE__, __func__, "Chapter merged", messages[0]);
      }
    }
    {
      // Test that it takes the changes from Bibledit.
      string ancestor (paratext);
      vector <string> messages;
      string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages);
      evaluate (__LINE__, __func__, bibledit, result);
      evaluate (__LINE__, __func__, 1, messages.size ());
      if (messages.size() == 1) {
        evaluate (__LINE__, __func__, "Chapter merged", messages[0]);
      }
    }
  }
  
}
