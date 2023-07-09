/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/studylight.h>
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <filter/string.h>
#include <database/versifications.h>
using namespace std;


TEST (easy_english_bible, logic)
{
  // Test the verse markup finder.
  {
    bool near_passage {false};
    bool at_passage {false};
    string paragraph {};
    bool handled {false};

    at_passage = false;
    paragraph = "Verse 13 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 13, at_passage);
    EXPECT_EQ (true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    EXPECT_EQ (false, at_passage);

    at_passage = false;
    paragraph = "Verse 13: Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 13, at_passage);
    EXPECT_EQ (true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    EXPECT_EQ (false, at_passage);

    at_passage = false;
    paragraph = "Verse 14";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    EXPECT_EQ (true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 15, at_passage);
    EXPECT_EQ (false, at_passage);

    at_passage = false;
    paragraph = "Verses 15-17 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 16, at_passage);
    EXPECT_EQ (true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 18, at_passage);
    EXPECT_EQ (false, at_passage);

    at_passage = false;
    paragraph = "Verse 8-11 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 9, at_passage);
    EXPECT_EQ (true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 12, at_passage);
    EXPECT_EQ (false, at_passage);

    at_passage = false;
    paragraph = "Verses 3 – 4: DAVID LOOKS TO THE LORD";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 3, at_passage);
    EXPECT_EQ (true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 5, at_passage);
    EXPECT_EQ (false, at_passage);
    
    near_passage = false;
    at_passage = false;
    paragraph = "Proverbs chapter 25";
    handled = resource_logic_easy_english_bible_handle_chapter_heading (paragraph, 25, near_passage, at_passage);
    EXPECT_EQ (true, handled);
    EXPECT_EQ (true, near_passage);
    EXPECT_EQ (false, at_passage);
    near_passage = false;
    at_passage = false;
    handled = resource_logic_easy_english_bible_handle_chapter_heading (paragraph, 26, near_passage, at_passage);
    EXPECT_EQ (false, handled);
    EXPECT_EQ (false, near_passage);
    EXPECT_EQ (false, at_passage);

  }
  
  // A couple of tests for text extraction.
  {
    const int book {58}; // Hebrews.
    string text = resource_logic_easy_english_bible_get (book, 10, 14);
    text = filter::strings::html2text (text);
    EXPECT_EQ ("Verse 14 Again, the writer makes it clear that Jesus died once for all time and for all *sin. It is most important that we know and believe that this is true. All who trust in Jesus, God will make holy. Jesus makes them perfect, that is, all that God intended them to be in his plan. Jesus has done all that God said was necessary.", text);
  }

  {
    const int book {58}; // Hebrews.
    string text = resource_logic_easy_english_bible_get (book, 8, 8);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("Verse 8 From verse 8 to the end of verse 12 the author copies words from Jeremiah 31:31-34. He uses these words to show that the old agreement is no longer in operation. The new agreement, that Jesus brought, has taken its place. It was not that there was a fault with the old agreement. The fault was with people, because nobody could obey the agreement.");
    EXPECT_EQ (0, pos);
    pos = text.find ("Most agreements are between two persons or groups.");
    EXPECT_EQ (356, pos);
  }

  {
    const int book {1}; // Genesis.
    string text = resource_logic_easy_english_bible_get (book, 2, 5);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("The plants that man needed for his food were not growing. There were two reasons for this.");
    EXPECT_EQ (8, pos);
  }

  {
    const int book {2}; // Exodus.
    string text = resource_logic_easy_english_bible_get (book, 20, 9);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("At the beginning of time, God rested on the 7th day.");
    EXPECT_EQ (451, pos);
  }

  {
    const int book {5}; // Deuteronomy.
    string text = resource_logic_easy_english_bible_get (book, 1, 2);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("The journey from Horeb to Kadesh-Barnea would take 11 days.");
    EXPECT_EQ (9, pos);
  }

  {
    const int book {7}; // Judges
    string text = resource_logic_easy_english_bible_get (book, 3, 12);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("There the writer says that God uses those that are nothing.");
    EXPECT_EQ (1679, pos);
  }

  {
    const int book {19}; // Psalms
    string text = resource_logic_easy_english_bible_get (book, 3, 6);
    text = filter::strings::html2text (text);
    const size_t pos = text.find (" The LORD answered David when he prayed.");
    EXPECT_EQ (36, pos);
  }

  {
    const int book {54}; // 1 Timothy.
    string text = resource_logic_easy_english_bible_get (book, 5, 3);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("Widows need help and care.");
    EXPECT_EQ (8, pos);
  }

  // Unit test that iterates over all possible passages in the Bible.
  // It retrieves text for all verses.
  // For development purposes.
  {
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    const vector <int> books = database_versifications.getBooks (filter::strings::english());
    for (auto book : books) {
      //if (book < 10) continue;
      //if (book > 9) continue;
      continue;
      std::cout << book << std::endl;
      int total_passage_counter = 0;
      int empty_passage_counter = 0;
      vector <int> chapters = database_versifications.getChapters (filter::strings::english(), book);
      for (auto chapter : chapters) {
        if (chapter == 0) continue;
        vector <int> verses = database_versifications.getVerses (filter::strings::english(), book, chapter);
        for (auto verse : verses) {
          if (verse == 0) continue;
          total_passage_counter++;
          string text = resource_logic_easy_english_bible_get (book, chapter, verse);
          text = filter::strings::html2text (text);
          if (text.empty()) {
            empty_passage_counter++;
            //cout << filter_passage_display (book, chapter, filter::strings::convert_to_string (verse)) << endl;
          }
        }
      }
      // Evaluate the total passages and the empty passages per book.
      switch (book) {
        case 1:
          // Genesis
          EXPECT_EQ (1533, total_passage_counter);
          EXPECT_EQ (36, empty_passage_counter);
          break;
        case 2:
          // Exodus
          EXPECT_EQ (1213, total_passage_counter);
          EXPECT_EQ (39, empty_passage_counter);
          break;
        case 3:
          // Leviticus
          EXPECT_EQ (859, total_passage_counter);
          EXPECT_EQ (106, empty_passage_counter);
          break;
        case 4:
          // Numbers
          EXPECT_EQ (1288, total_passage_counter);
          EXPECT_EQ (119, empty_passage_counter);
          break;
        case 5:
          // Deuteronomy
          EXPECT_EQ (959, total_passage_counter);
          EXPECT_EQ (1, empty_passage_counter);
          break;
        case 6:
          // Joshua
          EXPECT_EQ (658, total_passage_counter);
          EXPECT_EQ (91, empty_passage_counter);
          break;
        case 7:
          // Judges
          EXPECT_EQ (618, total_passage_counter);
          EXPECT_EQ (3, empty_passage_counter);
          break;
        case 8:
          // Ruth
          EXPECT_EQ (85, total_passage_counter);
          EXPECT_EQ (4, empty_passage_counter);
          break;
        case 9:
          // 1 Samuel
          EXPECT_EQ (810, total_passage_counter);
          EXPECT_EQ (54, empty_passage_counter);
          break;
        case 10:
          // 2 Samuel
          EXPECT_EQ (695, total_passage_counter);
          EXPECT_EQ (27, empty_passage_counter);
          break;
        case 11:
          // 1 Kings
          EXPECT_EQ (816, total_passage_counter);
          EXPECT_EQ (4, empty_passage_counter);
          break;
        case 12:
          // 2 Kings
          EXPECT_EQ (719, total_passage_counter);
          EXPECT_EQ (17, empty_passage_counter);
          break;
        case 13:
          // 1 Chronicles
          EXPECT_EQ (942, total_passage_counter);
          EXPECT_EQ (114, empty_passage_counter);
          break;
        case 14:
          // 2 Chronicles
          EXPECT_EQ (822, total_passage_counter);
          EXPECT_EQ (85, empty_passage_counter);
          break;
        case 15:
          // Ezra
          EXPECT_EQ (280, total_passage_counter);
          EXPECT_EQ (39, empty_passage_counter);
          break;
        case 16:
          // Nehemiah
          EXPECT_EQ (406, total_passage_counter);
          EXPECT_EQ (389, empty_passage_counter);
          break;
        case 17:
          // Esther
          EXPECT_EQ (167, total_passage_counter);
          EXPECT_EQ (6, empty_passage_counter);
          break;
        case 18:
          // Job
          EXPECT_EQ (1070, total_passage_counter);
          EXPECT_EQ (931, empty_passage_counter);
          break;
        case 19:
          // Psalms
          EXPECT_EQ (2461, total_passage_counter);
          EXPECT_EQ (1350, empty_passage_counter);
          break;
        case 20:
          // Proverbs
          EXPECT_EQ (915, total_passage_counter);
          EXPECT_EQ (410, empty_passage_counter);
          break;
        case 21:
          // Ecclesiastes
          EXPECT_EQ (222, total_passage_counter);
          EXPECT_EQ (8, empty_passage_counter);
          break;
        case 22:
          // Song of Solomon
          EXPECT_EQ (117, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 23:
          // Isaiah
          EXPECT_EQ (1292, total_passage_counter);
          EXPECT_EQ (92, empty_passage_counter);
          break;
        case 24:
          // Jeremiah
          EXPECT_EQ (1364, total_passage_counter);
          EXPECT_EQ (117, empty_passage_counter);
          break;
        case 25:
          // Lamentations
          EXPECT_EQ (154, total_passage_counter);
          EXPECT_EQ (55, empty_passage_counter);
          break;
        case 26:
          // Ezekiel
          EXPECT_EQ (1273, total_passage_counter);
          EXPECT_EQ (120, empty_passage_counter);
          break;
        case 27:
          // Daniel
          EXPECT_EQ (357, total_passage_counter);
          EXPECT_EQ (22, empty_passage_counter);
          break;
        case 28:
          // Hosea
          EXPECT_EQ (197, total_passage_counter);
          EXPECT_EQ (3, empty_passage_counter);
          break;
        case 29:
          // Joel
          EXPECT_EQ (73, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 30:
          // Amos
          EXPECT_EQ (146, total_passage_counter);
          EXPECT_EQ (2, empty_passage_counter);
          break;
        case 31:
          // Obadiah
          EXPECT_EQ (21, total_passage_counter);
          EXPECT_EQ (21, empty_passage_counter);
          break;
        case 32:
          // Jonah
          EXPECT_EQ (48, total_passage_counter);
          EXPECT_EQ (3, empty_passage_counter);
          break;
        case 33:
          // Micah
          EXPECT_EQ (105, total_passage_counter);
          EXPECT_EQ (47, empty_passage_counter);
          break;
        case 34:
          // Nahum
          EXPECT_EQ (47, total_passage_counter);
          EXPECT_EQ (16, empty_passage_counter);
          break;
        case 35:
          // Habakkuk
          EXPECT_EQ (56, total_passage_counter);
          EXPECT_EQ (56, empty_passage_counter);
          break;
        case 36:
          // Zephaniah
          EXPECT_EQ (53, total_passage_counter);
          EXPECT_EQ (53, empty_passage_counter);
          break;
        case 37:
          // Haggai
          EXPECT_EQ (38, total_passage_counter);
          EXPECT_EQ (38, empty_passage_counter);
          break;
        case 38:
          // Zechariah
          EXPECT_EQ (211, total_passage_counter);
          EXPECT_EQ (70, empty_passage_counter);
          break;
        case 39:
          // Malachi
          EXPECT_EQ (55, total_passage_counter);
          EXPECT_EQ (16, empty_passage_counter);
          break;
        case 40:
          // Matthew
          EXPECT_EQ (1071, total_passage_counter);
          EXPECT_EQ (63, empty_passage_counter);
          break;
        case 41:
          // Mark
          EXPECT_EQ (678, total_passage_counter);
          EXPECT_EQ (97, empty_passage_counter);
          break;
        case 42:
          // Luke
          EXPECT_EQ (1151, total_passage_counter);
          EXPECT_EQ (51, empty_passage_counter);
          break;
        case 43:
          // John
          EXPECT_EQ (879, total_passage_counter);
          EXPECT_EQ (100, empty_passage_counter);
          break;
        case 44:
          // Acts
          EXPECT_EQ (1007, total_passage_counter);
          EXPECT_EQ (95, empty_passage_counter);
          break;
        case 45:
          // Romans
          EXPECT_EQ (433, total_passage_counter);
          EXPECT_EQ (45, empty_passage_counter);
          break;
        case 46:
          // 1 Corinthians
          EXPECT_EQ (437, total_passage_counter);
          EXPECT_EQ (64, empty_passage_counter);
          break;
        case 47:
          // 2 Corinthians
          EXPECT_EQ (257, total_passage_counter);
          EXPECT_EQ (2, empty_passage_counter);
          break;
        case 48:
          // Galatians
          EXPECT_EQ (149, total_passage_counter);
          EXPECT_EQ (2, empty_passage_counter);
          break;
        case 49:
          // Ephesians
          EXPECT_EQ (155, total_passage_counter);
          EXPECT_EQ (5, empty_passage_counter);
          break;
        case 50:
          // Philippians
          EXPECT_EQ (104, total_passage_counter);
          EXPECT_EQ (6, empty_passage_counter);
          break;
        case 51:
          // Colossians
          EXPECT_EQ (95, total_passage_counter);
          EXPECT_EQ (8, empty_passage_counter);
          break;
        case 52:
          // 1 Thessalonians
          EXPECT_EQ (89, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 53:
          // 2 Thessalonians
          EXPECT_EQ (47, total_passage_counter);
          EXPECT_EQ (1, empty_passage_counter);
          break;
        case 54:
          // 1 Timothy
          EXPECT_EQ (113, total_passage_counter);
          EXPECT_EQ (4, empty_passage_counter);
          break;
        case 55:
          // 2 Timothy
          EXPECT_EQ (83, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 56:
          // Titus
          EXPECT_EQ (46, total_passage_counter);
          EXPECT_EQ (2, empty_passage_counter);
          break;
        case 57:
          // Philemon
          EXPECT_EQ (25, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 58:
          // Hebrews
          EXPECT_EQ (303, total_passage_counter);
          EXPECT_EQ (23, empty_passage_counter);
          break;
        case 59:
          // James
          EXPECT_EQ (108, total_passage_counter);
          EXPECT_EQ (5, empty_passage_counter);
          break;
        case 60:
          // 1 Peter
          EXPECT_EQ (105, total_passage_counter);
          EXPECT_EQ (37, empty_passage_counter);
          break;
        case 61:
          // 2 Peter
          EXPECT_EQ (61, total_passage_counter);
          EXPECT_EQ (8, empty_passage_counter);
          break;
        case 62:
          // 1 John
          EXPECT_EQ (105, total_passage_counter);
          EXPECT_EQ (2, empty_passage_counter);
          break;
        case 63:
          // 2 John
          EXPECT_EQ (13, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 64:
          // 3 John
          EXPECT_EQ (14, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 65:
          // Jude
          EXPECT_EQ (25, total_passage_counter);
          EXPECT_EQ (0, empty_passage_counter);
          break;
        case 66:
          // Revelation
          EXPECT_EQ (404, total_passage_counter);
          EXPECT_EQ (17, empty_passage_counter);
          break;
        default:
          cout << "book " << book << " " << filter_passage_display (book, 1, "1") << endl;
          cout << "total " << total_passage_counter << endl;
          cout << "empty " << empty_passage_counter << endl;
          break;
      }
    }
  }
}

#endif

