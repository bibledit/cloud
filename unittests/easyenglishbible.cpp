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


#include <unittests/studylight.h>
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <filter/string.h>
#include <database/versifications.h>


void test_easy_english_bible ()
{
  trace_unit_tests (__func__);
  
  // A couple of tests for text extraction.
  
  {
    int book = 58; // Hebrews.
    string text = resource_logic_easy_english_bible_get (book, 10, 14);
    text = filter_string_html2text (text);
    evaluate (__LINE__, __func__, "Verse 14 Again, the writer makes it clear that Jesus died once for all time and for all *sin. It is most important that we know and believe that this is true. All who trust in Jesus, God will make holy. Jesus makes them perfect, that is, all that God intended them to be in his plan. Jesus has done all that God said was necessary.", text);
  }

  {
    int book = 58; // Hebrews.
    string text = resource_logic_easy_english_bible_get (book, 8, 8);
    text = filter_string_html2text (text);
    size_t pos = text.find ("Verse 8 From verse 8 to the end of verse 12 the author copies words from Jeremiah 31:31-34. He uses these words to show that the old agreement is no longer in operation. The new agreement, that Jesus brought, has taken its place. It was not that there was a fault with the old agreement. The fault was with people, because nobody could obey the agreement.");
    evaluate (__LINE__, __func__, 0, pos);
    pos = text.find ("Most agreements are between two persons or groups. They both agree to do all that it requires of them. The new agreement is not like that. No mere *human can make an agreement with God. Here it says, I will make a new agreement. This means that God himself will arrange the new agreement. He will also make a way for it to achieve its purpose. The old agreement was with the peoples of *Israel and *Judah, and the new agreement will be for them. It will not only be for them, but for all who trust in Jesus.");
    evaluate (__LINE__, __func__, 356, pos);
  }

  // Unit test that iterates over all possible passages in the Bible.
  // It retrieves text for all verses.
  // For development purposes.
  {
    return; // Todo
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    vector <int> books = database_versifications.getBooks (english());
    for (auto book : books) {
      vector <Passage> empty_passages;
      int total_passage_counter = 0;
      int empty_passage_counter = 0;
      vector <int> chapters = database_versifications.getChapters (english(), book);
      for (auto chapter : chapters) {
        if (chapter == 0) continue;
        cout << chapter << endl; // Todo
        vector <int> verses = database_versifications.getVerses (english(), book, chapter);
        for (auto verse : verses) {
          if (verse == 0) continue;
          total_passage_counter++;
          string text = resource_logic_easy_english_bible_get (book, chapter, verse);
          text = filter_string_html2text (text);
          if (text.empty()) {
            empty_passage_counter++;
            empty_passages.push_back (Passage ("", book, chapter, convert_to_string(verse)));
          }
        }
      }
      for (auto passage : empty_passages) {
        cout << filter_passage_display_inline ({passage}) << endl;
      }
    }
  }

  
  
}
