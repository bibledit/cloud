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


#include <unittests/studylight.h>
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <filter/string.h>
#include <database/versifications.h>
using namespace std;


void test_easy_english_bible ()
{
  trace_unit_tests (__func__);
  
  // Test the verse markup finder.
  {
    bool near_passage {false};
    bool at_passage {false};
    string paragraph {};
    bool handled {false};

    at_passage = false;
    paragraph = "Verse 13 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 13, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verse 13: Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 13, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verse 14";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 15, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verses 15-17 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 16, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 18, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verse 8-11 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 9, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 12, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verses 3 – 4: DAVID LOOKS TO THE LORD";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 3, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 5, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);
    
    near_passage = false;
    at_passage = false;
    paragraph = "Proverbs chapter 25";
    handled = resource_logic_easy_english_bible_handle_chapter_heading (paragraph, 25, near_passage, at_passage);
    evaluate (__LINE__, __func__, true, handled);
    evaluate (__LINE__, __func__, true, near_passage);
    evaluate (__LINE__, __func__, false, at_passage);
    near_passage = false;
    at_passage = false;
    handled = resource_logic_easy_english_bible_handle_chapter_heading (paragraph, 26, near_passage, at_passage);
    evaluate (__LINE__, __func__, false, handled);
    evaluate (__LINE__, __func__, false, near_passage);
    evaluate (__LINE__, __func__, false, at_passage);

  }
  
  // A couple of tests for text extraction.
  {
    int book = 58; // Hebrews.
    string text = resource_logic_easy_english_bible_get (book, 10, 14);
    text = filter::strings::html2text (text);
    evaluate (__LINE__, __func__, "Verse 14 Again, the writer makes it clear that Jesus died once for all time and for all *sin. It is most important that we know and believe that this is true. All who trust in Jesus, God will make holy. Jesus makes them perfect, that is, all that God intended them to be in his plan. Jesus has done all that God said was necessary.", text);
  }

  {
    int book = 58; // Hebrews.
    string text = resource_logic_easy_english_bible_get (book, 8, 8);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("Verse 8 From verse 8 to the end of verse 12 the author copies words from Jeremiah 31:31-34. He uses these words to show that the old agreement is no longer in operation. The new agreement, that Jesus brought, has taken its place. It was not that there was a fault with the old agreement. The fault was with people, because nobody could obey the agreement.");
    evaluate (__LINE__, __func__, 0, pos);
    pos = text.find ("Most agreements are between two persons or groups.");
    evaluate (__LINE__, __func__, 356, pos);
  }

  {
    int book = 1; // Genesis.
    string text = resource_logic_easy_english_bible_get (book, 2, 5);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("The plants that man needed for his food were not growing. There were two reasons for this.");
    evaluate (__LINE__, __func__, 8, pos);
  }

  {
    int book = 2; // Exodus.
    string text = resource_logic_easy_english_bible_get (book, 20, 9);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("At the beginning of time, God rested on the 7th day.");
    evaluate (__LINE__, __func__, 461, pos);
  }

  {
    int book = 5; // Deuteronomy.
    string text = resource_logic_easy_english_bible_get (book, 1, 2);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("The journey from Horeb to Kadesh-Barnea would take 11 days.");
    evaluate (__LINE__, __func__, 9, pos);
  }

  {
    int book = 7; // Judges
    string text = resource_logic_easy_english_bible_get (book, 3, 12);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("There the writer says that God uses those that are nothing.");
    evaluate (__LINE__, __func__, 1707, pos);
  }

  {
    int book = 19; // Psalms
    string text = resource_logic_easy_english_bible_get (book, 3, 6);
    text = filter::strings::html2text (text);
    size_t pos = text.find (" The LORD answered David when he prayed.");
    evaluate (__LINE__, __func__, 38, pos);
  }

  {
    int book = 54; // 1 Timothy.
    string text = resource_logic_easy_english_bible_get (book, 5, 3);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("Widows need help and care.");
    evaluate (__LINE__, __func__, 8, pos);
  }

  // Unit test that iterates over all possible passages in the Bible.
  // It retrieves text for all verses.
  // For development purposes.
  {
    Database_Versifications database_versifications;
    database_versifications.create ();
    database_versifications.defaults ();
    vector <int> books = database_versifications.getBooks (filter::strings::english());
    for (auto book : books) {
      //if (book < 10) continue;
      //if (book > 9) continue;
      continue;
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
          evaluate (__LINE__, __func__, 1533, total_passage_counter);
          evaluate (__LINE__, __func__, 36, empty_passage_counter);
          break;
        case 2:
          // Exodus
          evaluate (__LINE__, __func__, 1213, total_passage_counter);
          evaluate (__LINE__, __func__, 39, empty_passage_counter);
          break;
        case 3:
          // Leviticus
          evaluate (__LINE__, __func__, 859, total_passage_counter);
          evaluate (__LINE__, __func__, 106, empty_passage_counter);
          break;
        case 4:
          // Numbers
          evaluate (__LINE__, __func__, 1288, total_passage_counter);
          evaluate (__LINE__, __func__, 119, empty_passage_counter);
          break;
        case 5:
          // Deuteronomy
          evaluate (__LINE__, __func__, 959, total_passage_counter);
          evaluate (__LINE__, __func__, 1, empty_passage_counter);
          break;
        case 6:
          // Joshua
          evaluate (__LINE__, __func__, 658, total_passage_counter);
          evaluate (__LINE__, __func__, 91, empty_passage_counter);
          break;
        case 7:
          // Judges
          evaluate (__LINE__, __func__, 618, total_passage_counter);
          evaluate (__LINE__, __func__, 3, empty_passage_counter);
          break;
        case 8:
          // Ruth
          evaluate (__LINE__, __func__, 85, total_passage_counter);
          evaluate (__LINE__, __func__, 4, empty_passage_counter);
          break;
        case 9:
          // 1 Samuel
          evaluate (__LINE__, __func__, 810, total_passage_counter);
          evaluate (__LINE__, __func__, 54, empty_passage_counter);
          break;
        case 10:
          // 2 Samuel
          evaluate (__LINE__, __func__, 695, total_passage_counter);
          evaluate (__LINE__, __func__, 27, empty_passage_counter);
          break;
        case 11:
          // 1 Kings
          evaluate (__LINE__, __func__, 816, total_passage_counter);
          evaluate (__LINE__, __func__, 4, empty_passage_counter);
          break;
        case 12:
          // 2 Kings
          evaluate (__LINE__, __func__, 719, total_passage_counter);
          evaluate (__LINE__, __func__, 17, empty_passage_counter);
          break;
        case 13:
          // 1 Chronicles
          evaluate (__LINE__, __func__, 942, total_passage_counter);
          evaluate (__LINE__, __func__, 114, empty_passage_counter);
          break;
        case 14:
          // 2 Chronicles
          evaluate (__LINE__, __func__, 822, total_passage_counter);
          evaluate (__LINE__, __func__, 85, empty_passage_counter);
          break;
        case 15:
          // Ezra
          evaluate (__LINE__, __func__, 280, total_passage_counter);
          evaluate (__LINE__, __func__, 39, empty_passage_counter);
          break;
        case 16:
          // Nehemiah
          evaluate (__LINE__, __func__, 406, total_passage_counter);
          evaluate (__LINE__, __func__, 389, empty_passage_counter);
          break;
        case 17:
          // Esther
          evaluate (__LINE__, __func__, 167, total_passage_counter);
          evaluate (__LINE__, __func__, 6, empty_passage_counter);
          break;
        case 18:
          // Job
          evaluate (__LINE__, __func__, 1070, total_passage_counter);
          evaluate (__LINE__, __func__, 931, empty_passage_counter);
          break;
        case 19:
          // Psalms
          evaluate (__LINE__, __func__, 2461, total_passage_counter);
          evaluate (__LINE__, __func__, 1350, empty_passage_counter);
          break;
        case 20:
          // Proverbs
          evaluate (__LINE__, __func__, 915, total_passage_counter);
          evaluate (__LINE__, __func__, 410, empty_passage_counter);
          break;
        case 21:
          // Ecclesiastes
          evaluate (__LINE__, __func__, 222, total_passage_counter);
          evaluate (__LINE__, __func__, 8, empty_passage_counter);
          break;
        case 22:
          // Song of Solomon
          evaluate (__LINE__, __func__, 117, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 23:
          // Isaiah
          evaluate (__LINE__, __func__, 1292, total_passage_counter);
          evaluate (__LINE__, __func__, 92, empty_passage_counter);
          break;
        case 24:
          // Jeremiah
          evaluate (__LINE__, __func__, 1364, total_passage_counter);
          evaluate (__LINE__, __func__, 117, empty_passage_counter);
          break;
        case 25:
          // Lamentations
          evaluate (__LINE__, __func__, 154, total_passage_counter);
          evaluate (__LINE__, __func__, 55, empty_passage_counter);
          break;
        case 26:
          // Ezekiel
          evaluate (__LINE__, __func__, 1273, total_passage_counter);
          evaluate (__LINE__, __func__, 120, empty_passage_counter);
          break;
        case 27:
          // Daniel
          evaluate (__LINE__, __func__, 357, total_passage_counter);
          evaluate (__LINE__, __func__, 22, empty_passage_counter);
          break;
        case 28:
          // Hosea
          evaluate (__LINE__, __func__, 197, total_passage_counter);
          evaluate (__LINE__, __func__, 3, empty_passage_counter);
          break;
        case 29:
          // Joel
          evaluate (__LINE__, __func__, 73, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 30:
          // Amos
          evaluate (__LINE__, __func__, 146, total_passage_counter);
          evaluate (__LINE__, __func__, 2, empty_passage_counter);
          break;
        case 31:
          // Obadiah
          evaluate (__LINE__, __func__, 21, total_passage_counter);
          evaluate (__LINE__, __func__, 21, empty_passage_counter);
          break;
        case 32:
          // Jonah
          evaluate (__LINE__, __func__, 48, total_passage_counter);
          evaluate (__LINE__, __func__, 3, empty_passage_counter);
          break;
        case 33:
          // Micah
          evaluate (__LINE__, __func__, 105, total_passage_counter);
          evaluate (__LINE__, __func__, 47, empty_passage_counter);
          break;
        case 34:
          // Nahum
          evaluate (__LINE__, __func__, 47, total_passage_counter);
          evaluate (__LINE__, __func__, 16, empty_passage_counter);
          break;
        case 35:
          // Habakkuk
          evaluate (__LINE__, __func__, 56, total_passage_counter);
          evaluate (__LINE__, __func__, 56, empty_passage_counter);
          break;
        case 36:
          // Zephaniah
          evaluate (__LINE__, __func__, 53, total_passage_counter);
          evaluate (__LINE__, __func__, 53, empty_passage_counter);
          break;
        case 37:
          // Haggai
          evaluate (__LINE__, __func__, 38, total_passage_counter);
          evaluate (__LINE__, __func__, 38, empty_passage_counter);
          break;
        case 38:
          // Zechariah
          evaluate (__LINE__, __func__, 211, total_passage_counter);
          evaluate (__LINE__, __func__, 70, empty_passage_counter);
          break;
        case 39:
          // Malachi
          evaluate (__LINE__, __func__, 55, total_passage_counter);
          evaluate (__LINE__, __func__, 16, empty_passage_counter);
          break;
        case 40:
          // Matthew
          evaluate (__LINE__, __func__, 1071, total_passage_counter);
          evaluate (__LINE__, __func__, 63, empty_passage_counter);
          break;
        case 41:
          // Mark
          evaluate (__LINE__, __func__, 678, total_passage_counter);
          evaluate (__LINE__, __func__, 97, empty_passage_counter);
          break;
        case 42:
          // Luke
          evaluate (__LINE__, __func__, 1151, total_passage_counter);
          evaluate (__LINE__, __func__, 51, empty_passage_counter);
          break;
        case 43:
          // John
          evaluate (__LINE__, __func__, 879, total_passage_counter);
          evaluate (__LINE__, __func__, 100, empty_passage_counter);
          break;
        case 44:
          // Acts
          evaluate (__LINE__, __func__, 1007, total_passage_counter);
          evaluate (__LINE__, __func__, 95, empty_passage_counter);
          break;
        case 45:
          // Romans
          evaluate (__LINE__, __func__, 433, total_passage_counter);
          evaluate (__LINE__, __func__, 45, empty_passage_counter);
          break;
        case 46:
          // 1 Corinthians
          evaluate (__LINE__, __func__, 437, total_passage_counter);
          evaluate (__LINE__, __func__, 64, empty_passage_counter);
          break;
        case 47:
          // 2 Corinthians
          evaluate (__LINE__, __func__, 257, total_passage_counter);
          evaluate (__LINE__, __func__, 2, empty_passage_counter);
          break;
        case 48:
          // Galatians
          evaluate (__LINE__, __func__, 149, total_passage_counter);
          evaluate (__LINE__, __func__, 2, empty_passage_counter);
          break;
        case 49:
          // Ephesians
          evaluate (__LINE__, __func__, 155, total_passage_counter);
          evaluate (__LINE__, __func__, 5, empty_passage_counter);
          break;
        case 50:
          // Philippians
          evaluate (__LINE__, __func__, 104, total_passage_counter);
          evaluate (__LINE__, __func__, 6, empty_passage_counter);
          break;
        case 51:
          // Colossians
          evaluate (__LINE__, __func__, 95, total_passage_counter);
          evaluate (__LINE__, __func__, 8, empty_passage_counter);
          break;
        case 52:
          // 1 Thessalonians
          evaluate (__LINE__, __func__, 89, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 53:
          // 2 Thessalonians
          evaluate (__LINE__, __func__, 47, total_passage_counter);
          evaluate (__LINE__, __func__, 1, empty_passage_counter);
          break;
        case 54:
          // 1 Timothy
          evaluate (__LINE__, __func__, 113, total_passage_counter);
          evaluate (__LINE__, __func__, 4, empty_passage_counter);
          break;
        case 55:
          // 2 Timothy
          evaluate (__LINE__, __func__, 83, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 56:
          // Titus
          evaluate (__LINE__, __func__, 46, total_passage_counter);
          evaluate (__LINE__, __func__, 2, empty_passage_counter);
          break;
        case 57:
          // Philemon
          evaluate (__LINE__, __func__, 25, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 58:
          // Hebrews
          evaluate (__LINE__, __func__, 303, total_passage_counter);
          evaluate (__LINE__, __func__, 23, empty_passage_counter);
          break;
        case 59:
          // James
          evaluate (__LINE__, __func__, 108, total_passage_counter);
          evaluate (__LINE__, __func__, 5, empty_passage_counter);
          break;
        case 60:
          // 1 Peter
          evaluate (__LINE__, __func__, 105, total_passage_counter);
          evaluate (__LINE__, __func__, 37, empty_passage_counter);
          break;
        case 61:
          // 2 Peter
          evaluate (__LINE__, __func__, 61, total_passage_counter);
          evaluate (__LINE__, __func__, 8, empty_passage_counter);
          break;
        case 62:
          // 1 John
          evaluate (__LINE__, __func__, 105, total_passage_counter);
          evaluate (__LINE__, __func__, 2, empty_passage_counter);
          break;
        case 63:
          // 2 John
          evaluate (__LINE__, __func__, 13, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 64:
          // 3 John
          evaluate (__LINE__, __func__, 14, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 65:
          // Jude
          evaluate (__LINE__, __func__, 25, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 66:
          // Revelation
          evaluate (__LINE__, __func__, 404, total_passage_counter);
          evaluate (__LINE__, __func__, 17, empty_passage_counter);
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
