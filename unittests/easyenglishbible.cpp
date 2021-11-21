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

  {
    int book = 1; // Genesis.
    string text = resource_logic_easy_english_bible_get (book, 2, 5);
    text = filter_string_html2text (text);
    size_t pos = text.find ("The plants that man needed for his food were not growing. There were two reasons for this.");
    evaluate (__LINE__, __func__, 8, pos);
  }

  {
    int book = 2; // Exodus.
    string text = resource_logic_easy_english_bible_get (book, 20, 9);
    text = filter_string_html2text (text);
    size_t pos = text.find ("At the beginning of time, God rested on the 7th day.");
    evaluate (__LINE__, __func__, 1511, pos);
  }

  {
    int book = 5; // Deuteronomy.
    string text = resource_logic_easy_english_bible_get (book, 1, 2);
    text = filter_string_html2text (text);
    size_t pos = text.find ("The journey from Horeb to Kadesh-Barnea would take 11 days.");
    evaluate (__LINE__, __func__, 9, pos);
  }

  {
    int book = 7; // Judges
    string text = resource_logic_easy_english_bible_get (book, 3, 12);
    text = filter_string_html2text (text);
    size_t pos = text.find ("There the writer says that God uses those that are nothing.");
    evaluate (__LINE__, __func__, 1693, pos);
  }

  {
    int book = 19; // Psalms
    string text = resource_logic_easy_english_bible_get (book, 3, 6);
    text = filter_string_html2text (text);
    cout << text << endl; // Todo
    size_t pos = text.find ("There the writer says that God uses those that are nothing.");
    evaluate (__LINE__, __func__, 1693, pos);
  }

  {
    int book = 54; // 1 Timothy.
    string text = resource_logic_easy_english_bible_get (book, 5, 3);
    text = filter_string_html2text (text);
    size_t pos = text.find ("Widows need help and care.");
    evaluate (__LINE__, __func__, 8, pos);
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
      if (book <= 0) continue;
      vector <Passage> empty_passages;
      int total_passage_counter = 0;
      int empty_passage_counter = 0;
      vector <int> chapters = database_versifications.getChapters (english(), book);
      for (auto chapter : chapters) {
        if (chapter == 0) continue;
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
      // Evaluate the total passages and the empty passages per book.
      switch (book) {
        case 1:
          // Genesis
          evaluate (__LINE__, __func__, 1533, total_passage_counter);
          evaluate (__LINE__, __func__, 23, empty_passage_counter);
          break;
        case 2:
          // Exodus
          evaluate (__LINE__, __func__, 1213, total_passage_counter);
          evaluate (__LINE__, __func__, 10, empty_passage_counter);
          break;
        case 3:
          // Leviticus
          evaluate (__LINE__, __func__, 859, total_passage_counter);
          evaluate (__LINE__, __func__, 106, empty_passage_counter);
          break;
        case 4:
          // Numbers
          evaluate (__LINE__, __func__, 1288, total_passage_counter);
          evaluate (__LINE__, __func__, 37, empty_passage_counter);
          break;
        case 5:
          // Deuteronomy
          evaluate (__LINE__, __func__, 959, total_passage_counter);
          evaluate (__LINE__, __func__, 1, empty_passage_counter);
          break;
        case 6:
          // Joshua
          evaluate (__LINE__, __func__, 658, total_passage_counter);
          evaluate (__LINE__, __func__, 77, empty_passage_counter);
          break;
        case 7:
          // Judges
          evaluate (__LINE__, __func__, 618, total_passage_counter);
          evaluate (__LINE__, __func__, 3, empty_passage_counter);
          break;
        case 8:
          // Ruth
          evaluate (__LINE__, __func__, 85, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 9:
          // 1 Samuel
          evaluate (__LINE__, __func__, 810, total_passage_counter);
          evaluate (__LINE__, __func__, 54, empty_passage_counter);
          break;
        case 10:
          // 2 Samuel
          evaluate (__LINE__, __func__, 695, total_passage_counter);
          evaluate (__LINE__, __func__, 31, empty_passage_counter);
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
          evaluate (__LINE__, __func__, 29, empty_passage_counter);
          break;
        case 14:
          // 2 Chronicles
          evaluate (__LINE__, __func__, 822, total_passage_counter);
          evaluate (__LINE__, __func__, 7, empty_passage_counter);
          break;
        case 15:
          // Ezra
          evaluate (__LINE__, __func__, 280, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 16:
          // Nehemiah
          evaluate (__LINE__, __func__, 406, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 17:
          // Esther
          evaluate (__LINE__, __func__, 167, total_passage_counter);
          evaluate (__LINE__, __func__, 0, empty_passage_counter);
          break;
        case 18:
          // Job
          evaluate (__LINE__, __func__, 1070, total_passage_counter);
          evaluate (__LINE__, __func__, 15, empty_passage_counter);
          break;
        case 19:
          break;
        case 0:
          break;
          //      book 19 Psalms 1:1
          //      total 2461
          //      empty 2461
          //      ^[[15~book 20 Proverbs 1:1
          //      total 915
          //      empty 866
          //      book 21 Ecclesiastes 1:1
          //      total 222
          //      empty 4
          //      book 22 Song of Solomon 1:1
          //      total 117
          //      empty 0
          //      book 23 Isaiah 1:1
          //      total 1292
          //      empty 37
          //      book 24 Jeremiah 1:1
          //      total 1364
          //      empty 21
          //      book 25 Lamentations 1:1
          //      total 154
          //      empty 53
          //      book 26 Ezekiel 1:1
          //      total 1273
          //      empty 31
          //      book 27 Daniel 1:1
          //      total 357
          //      empty 2
          //      book 28 Hosea 1:1
          //      total 197
          //      empty 3
          //      book 29 Joel 1:1
          //      total 73
          //      empty 0
          //      book 30 Amos 1:1
          //      total 146
          //      empty 2
          //      book 31 Obadiah 1:1
          //      total 21
          //      empty 21
          //      book 32 Jonah 1:1
          //      total 48
          //      empty 1
          //      book 33 Micah 1:1
          //      total 105
          //      empty 0
          //      book 34 Nahum 1:1
          //      total 47
          //      empty 13
          //      book 35 Habakkuk 1:1
          //      total 56
          //      empty 56
          //      book 36 Zephaniah 1:1
          //      total 53
          //      empty 53
          //      book 37 Haggai 1:1
          //      total 38
          //      empty 38
          //      book 38 Zechariah 1:1
          //      total 211
          //      empty 70
          //      book 39 Malachi 1:1
          //      total 55
          //      empty 16
          //      book 40 Matthew 1:1
          //      total 1071
          //      empty 1
          //      book 41 Mark 1:1
          //      total 678
          //      empty 83
          //      book 42 Luke 1:1
          //      total 1151
          //      empty 12
          //      book 43 John 1:1
          //      total 879
          //      empty 22
          //      book 44 Acts 1:1
          //      total 1007
          //      empty 24
          //      book 45 Romans 1:1
          //      total 433
          //      empty 7
          //      book 46 1 Corinthians 1:1
          //      total 437
          //      empty 45
          //      book 47 2 Corinthians 1:1
          //      total 257
          //      empty 2
          //      book 48 Galatians 1:1
          //      total 149
          //      empty 0
          //      book 49 Ephesians 1:1
          //      total 155
          //      empty 1
          //      book 50 Philippians 1:1
          //      total 104
          //      empty 20
          //      book 51 Colossians 1:1
          //      total 95
          //      empty 8
          //      book 52 1 Thessalonians 1:1
          //      total 89
          //      empty 0
          //      book 53 2 Thessalonians 1:1
          //      total 47
          //      empty 1
          //      book 54 1 Timothy 1:1
          //      total 113
          //      empty 2
          //      book 55 2 Timothy 1:1
          //      total 83
          //      empty 0
          //      book 56 Titus 1:1
          //      total 46
          //      empty 0
          //      book 57 Philemon 1:1
          //      total 25
          //      empty 25
          //      book 58 Hebrews 1:1
          //      total 303
          //      empty 17
          //      book 59 James 1:1
          //      total 108
          //      empty 3
          //      book 60 1 Peter 1:1
          //      total 105
          //      empty 0
          //      book 61 2 Peter 1:1
          //      total 61
          //      empty 0
          //      book 62 1 John 1:1
          //      total 105
          //      empty 0
          //      book 63 2 John 1:1
          //      total 13
          //      empty 13
          //      book 64 3 John 1:1
          //      total 14
          //      empty 14
          //      book 65 Jude 1:1
          //      total 25
          //      empty 25
          //      book 66 Revelation 1:1
          //      total 404
          //      empty 3
        default:
          cout << "book " << book << " " << filter_passage_display (book, 1, "1") << endl;
          cout << "total " << total_passage_counter << endl; // Todo
          cout << "empty " << empty_passage_counter << endl; // Todo
          break;
      }
//      for (auto passage : empty_passages) {
//        cout << filter_passage_display_inline ({passage}) << endl;
//      }
//      return; // Todo
    }
  }

  
  
}
