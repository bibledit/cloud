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
    vector <Merge_Conflict> conflicts;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
    evaluate (__LINE__, __func__, bibledit, result);
    evaluate (__LINE__, __func__, 1, messages.size ());
    evaluate (__LINE__, __func__, 0, conflicts.size ());
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
    vector <Merge_Conflict> conflicts;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
    evaluate (__LINE__, __func__, paratext, result);
    evaluate (__LINE__, __func__, 1, messages.size ());
    evaluate (__LINE__, __func__, 0, conflicts.size ());
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
    vector <Merge_Conflict> conflicts;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
    evaluate (__LINE__, __func__, "", result);
    evaluate (__LINE__, __func__, 0, messages.size ());
    evaluate (__LINE__, __func__, 0, conflicts.size ());
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
      vector <Merge_Conflict> conflicts;
      string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
      evaluate (__LINE__, __func__, paratext, result);
      evaluate (__LINE__, __func__, 1, messages.size ());
      if (messages.size() == 1) {
        evaluate (__LINE__, __func__, "Chapter merged", messages[0]);
      }
      evaluate (__LINE__, __func__, 0, conflicts.size ());
    }
    {
      // Test that it takes the changes from Bibledit.
      string ancestor (paratext);
      vector <string> messages;
      vector <Merge_Conflict> conflicts;
      string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
      evaluate (__LINE__, __func__, bibledit, result);
      evaluate (__LINE__, __func__, 1, messages.size ());
      if (messages.size() == 1) {
        evaluate (__LINE__, __func__, "Chapter merged", messages[0]);
      }
      evaluate (__LINE__, __func__, 0, conflicts.size ());
    }
  }
  
  // No merging due to missing parent data.
  {
    string ancestor;
    string bibledit = R"(
\c 1
\s header
\p
\v 1 bibledit.
\v 2 bibledit.
    )";
    string paratext = R"(
\c 1
\s header
\p
\v 1 paratext.
\v 2 paratext.
    )";
    vector <string> messages;
    vector <Merge_Conflict> conflicts;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
    evaluate (__LINE__, __func__, "", result);
    evaluate (__LINE__, __func__, 1, messages.size ());
    if (messages.size() == 1) {
      evaluate (__LINE__, __func__, "Cannot merge chapter due to missing parent data", messages[0]);
    }
    evaluate (__LINE__, __func__, 0, conflicts.size ());
  }

  // Merge conflict.
  {
    string ancestor = R"(
\c 1
\s header
\p
\v 1 verse.
\v 2 verse.
    )";
    string bibledit = R"(
\c 1
\s header
\p
\v 1 bibledit.
\v 2 bibledit.
    )";
    string paratext = R"(
\c 1
\s header
\p
\v 1 paratext.
\v 2 paratext.
    )";
    vector <string> messages;
    vector <Merge_Conflict> conflicts;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
    evaluate (__LINE__, __func__, filter_string_trim (paratext), result);
    evaluate (__LINE__, __func__, 1, messages.size ());
    if (messages.size() == 1) {
      evaluate (__LINE__, __func__, "Chapter merged", messages[0]);
    }
    evaluate (__LINE__, __func__, 3, conflicts.size ());
    if (conflicts.size() == 3) {
      evaluate (__LINE__, __func__, R"(\v 1 paratext.)", conflicts[0].result);
      evaluate (__LINE__, __func__, "Failed to merge: The existing text was kept", conflicts[0].subject);
      evaluate (__LINE__, __func__, R"(\v 2 paratext.)", conflicts[1].result);
      evaluate (__LINE__, __func__, "Failed to merge: The existing text was kept", conflicts[1].subject);
      evaluate (__LINE__, __func__, filter_string_trim (paratext), conflicts[2].result);
      evaluate (__LINE__, __func__, "Failed to merge: The existing text was kept", conflicts[1].subject);
    }
  }
  
  // Test it takes a larger Bibledit chapter rather than a smaller Paratext chapter.
  {
    string ancestor;
    string bibledit = R"(
\c 1
\s header
\p
\v 1 In the beginning was the Word, and the Word was with God, and the Word was God.
\v 2 The same was in the beginning with God.
\v 3 All things were made through him. Without him, nothing was made that has been made.
\v 4 In him was life, and the life was the light of men.
\v 5 The light shines in the darkness, and the darkness hasn’t overcome \f + \fr 1:5  \ft The word translated “overcome” (κατέλαβεν) can also be translated “comprehended.” It refers to getting a grip on an enemy to defeat him. \f* it.
\v 6 There came a man, sent from God, whose name was John.
\v 7 The same came as a witness, that he might testify about the light, that all might believe through him.
\v 8 He was not the light, but was sent that he might testify about the light.
\v 9 The true light that enlightens everyone was coming into the world.
\p
\v 10 He was in the world, and the world was made through him, and the world didn’t recognize him.
\v 11 He came to his own, and those who were his own didn’t receive him.
\v 12 But as many as received him, to them he gave the right to become God’s children, to those who believe in his name:
    )";
    string paratext = R"(
\c 1
\s header
\p
\v 1
\v 2
\v 3
\v 4
\v 5
\v 6
\v 7
\v 8
\v 9
\v 10
\v 11
\v 12
    )";
    vector <string> messages;
    vector <Merge_Conflict> conflicts;
    string result = Paratext_Logic::synchronize (ancestor, bibledit, paratext, messages, conflicts);
    evaluate (__LINE__, __func__, filter_string_trim (bibledit), filter_string_trim (result));
    evaluate (__LINE__, __func__, 1, messages.size ());
    if (messages.size() == 1) {
      evaluate (__LINE__, __func__, "Copy larger Bibledit chapter to smaller Paratext chapter", messages[0]);
    }
    evaluate (__LINE__, __func__, 0, conflicts.size ());
  }

}
