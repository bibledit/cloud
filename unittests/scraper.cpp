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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <resource/external.h>
#include <filter/string.h>
#include <database/versifications.h>
#include <database/books.h>


TEST (scraper, biblegateway)
{
  std::string resource;
  int book;
  std::string text;
  
  resource = "Библия, ревизирано издание (BPB)";
  book = 1; // Genesis.
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("НАЧАЛО НА СВЕТА И ЧОВЕЧЕСТВОТОСътворението 1 В началото Бог сътвори небето и земята.", text);
  
  resource = "21st Century King James Version (KJ21)";
  book = 34; // Nahum.
  text = resource_logic_bible_gateway_get (resource, book, 1, 0);
  EXPECT_EQ ("", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("1 The burden of Nineveh. The book of the vision of Nahum the Elkoshite.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 15);
  EXPECT_EQ ("Behold upon the mountains the feet of Him that bringeth good tidings, that publisheth peace! O Judah, keep thy solemn feasts, perform thy vows; for the wicked shall no more pass through thee; he is utterly cut off.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 16);
  EXPECT_EQ ("", text);
  
  book = 64; // 3 John.
  text = resource_logic_bible_gateway_get (resource, book, 1, 0);
  EXPECT_EQ ("", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("The Elder, Unto the well-beloved Gaius, whom I love in the truth:", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 14);
  EXPECT_EQ ("but I trust I shall shortly see thee, and we shall speak face to face. Peace be to thee. Our friends salute thee. Greet the friends by name.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 15);
  EXPECT_EQ ("", text);
  
  resource = "Expanded Bible (EXB)";
  book = 34; // Nahum.
  text = resource_logic_bible_gateway_get (resource, book, 1, 0);
  EXPECT_EQ ("", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("1 This is the ·message [oracle; burden] for the city of Nineveh [C the capital of the Assyrian empire]. This is the book of the vision of Nahum, ·who was from the town of Elkosh [L the Elkoshite; C of uncertain location].", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 2);
  EXPECT_EQ ("The Lord Is Angry with Nineveh The Lord is a ·jealous [zealous] God [Ex. 20:5; 34:14; Deut. 4:24; 5:9; Josh. 24:19] who ·punishes [avenges]; the Lord ·punishes [avenges] and is filled with ·anger [wrath]. The Lord ·punishes [takes vengeance on] ·those who are against him [his adversaries/enemies], and he ·stays angry with [or vents his wrath against] his enemies.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 11);
  EXPECT_EQ ("Someone has come from ·Nineveh [L you] who ·makes evil plans [plots evil] against the Lord and gives wicked ·advice [counsel; strategy].", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 12);
  EXPECT_EQ ("This is what the Lord says: “Although Assyria ·is strong [or has allies] and has many people, it will be ·defeated [destroyed] and ·brought to an end [pass away]. Although I have ·made you suffer, Judah [L afflicted you], I will ·make you suffer [afflict you] no more.", text);
  
  resource = "J.B. Phillips New Testament (PHILLIPS)";
  book = 63; // 2 John.
  text = resource_logic_bible_gateway_get (resource, book, 1, 3);
  EXPECT_EQ ("This letter comes from the Elder to a certain Christian lady and her children, held in the highest affection not only by me but by all who know the truth. For the truth’s sake (which even now we know and which will be our companion for ever) I wish you, in all love and sincerity, grace, mercy and peace from God the Father and the Lord Jesus Christ, the Father’s Son.", text);
  
  resource = "Living Bible (TLB)";
  book = 40; // Matthew.
  text = resource_logic_bible_gateway_get (resource, book, 1, 17);
  EXPECT_EQ ("These are fourteen[a] of the generations from Abraham to King David; and fourteen from King David’s time to the exile; and fourteen from the exile to Christ.<br>Note: These are fourteen, literally, “So all the generations from Abraham unto David are fourteen.”", text);
  
  resource = "New American Standard Bible (NASB)";
  book = 60; // 1 Peter.
  text = resource_logic_bible_gateway_get (resource, book, 2, 5);
  EXPECT_EQ ("you also, as living stones, [a]are being built up as a spiritual house for a holy priesthood, to offer spiritual sacrifices that are acceptable to God through Jesus Christ.<br>Note: Or allow yourselves to be built up; or build yourselves up", text);
  
  resource = "English Standard Version (ESV)";
  book = 60; // 1 Peter.
  text = resource_logic_bible_gateway_get (resource, book, 3, 7);
  EXPECT_EQ ("Likewise, husbands, live with your wives in an understanding way, showing honor to the woman as the weaker vessel, since they are heirs with you[a] of the grace of life, so that your prayers may not be hindered.<br>Note: Some manuscripts since you are joint heirs", text);
  
  // Test poetry in verses.
  resource = "New American Standard Bible (NASB)";
  book = 54; // 1 Timothy.
  text = resource_logic_bible_gateway_get (resource, book, 3, 16);
  EXPECT_EQ ("Beyond question, great is the mystery of godliness: He who was revealed in the flesh, Was [a]vindicated [b]in the Spirit, Seen by angels, Proclaimed among the nations, Believed on in the world, Taken up in glory.<br>Note: Or justified<br>Note: Or by", text);
}


TEST (scraper, easy_english_bible)
{
  // Test the verse markup finder.
  {
    bool near_passage {false};
    bool at_passage {false};
    std::string paragraph {};
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
    std::string text = resource_logic_easy_english_bible_get (book, 10, 14);
    text = filter::strings::html2text (text);
    EXPECT_EQ ("Verse 14 Again, the writer makes it clear that Jesus died once for all time and for all *sin. It is most important that we know and believe that this is true. All who trust in Jesus, God will make holy. Jesus makes them perfect, that is, all that God intended them to be in his plan. Jesus has done all that God said was necessary.", text);
  }
  
  {
    const int book {58}; // Hebrews.
    std::string text = resource_logic_easy_english_bible_get (book, 8, 8);
    text = filter::strings::html2text (text);
    size_t pos = text.find ("Verse 8 From verse 8 to the end of verse 12 the author copies words from Jeremiah 31:31-34. He uses these words to show that the old agreement is no longer in operation. The new agreement, that Jesus brought, has taken its place. It was not that there was a fault with the old agreement. The fault was with people, because nobody could obey the agreement.");
    EXPECT_EQ (0, pos);
    pos = text.find ("Most agreements are between two persons or groups.");
    EXPECT_EQ (356, pos);
  }
  
  {
    const int book {1}; // Genesis.
    std::string text = resource_logic_easy_english_bible_get (book, 2, 5);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("The plants that man needed for his food were not growing. There were two reasons for this.");
    EXPECT_EQ (8, pos);
  }
  
  {
    const int book {2}; // Exodus.
    std::string text = resource_logic_easy_english_bible_get (book, 20, 9);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("At the beginning of time, God rested on the 7th day.");
    EXPECT_EQ (451, pos);
  }
  
  {
    const int book {5}; // Deuteronomy.
    std::string text = resource_logic_easy_english_bible_get (book, 1, 2);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("The journey from Horeb to Kadesh-Barnea would take 11 days.");
    EXPECT_EQ (9, pos);
  }
  
  {
    const int book {7}; // Judges
    std::string text = resource_logic_easy_english_bible_get (book, 3, 12);
    text = filter::strings::html2text (text);
    const size_t pos = text.find ("There the writer says that God uses those that are nothing.");
    EXPECT_EQ (1679, pos);
  }
  
  {
    const int book {19}; // Psalms
    std::string text = resource_logic_easy_english_bible_get (book, 3, 6);
    text = filter::strings::html2text (text);
    const size_t pos = text.find (" The LORD answered David when he prayed.");
    EXPECT_EQ (36, pos);
  }
  
  {
    const int book {54}; // 1 Timothy.
    std::string text = resource_logic_easy_english_bible_get (book, 5, 3);
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
    const std::vector <int> books = database_versifications.getBooks (filter::strings::english());
    for (auto book : books) {
      //if (book < 10) continue;
      //if (book > 9) continue;
      continue;
      std::cout << book << std::endl;
      int total_passage_counter = 0;
      int empty_passage_counter = 0;
      std::vector <int> chapters = database_versifications.getChapters (filter::strings::english(), book);
      for (auto chapter : chapters) {
        if (chapter == 0) continue;
        std::vector <int> verses = database_versifications.getVerses (filter::strings::english(), book, chapter);
        for (auto verse : verses) {
          if (verse == 0) continue;
          total_passage_counter++;
          std::string text = resource_logic_easy_english_bible_get (book, chapter, verse);
          text = filter::strings::html2text (text);
          if (text.empty()) {
            empty_passage_counter++;
            //cout << filter_passage_display (book, chapter, filter::strings::convert_to_string (verse)) << std::endl;
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
          std::cout << "book " << book << " " << filter_passage_display (book, 1, "1") << std::endl;
          std::cout << "total " << total_passage_counter << std::endl;
          std::cout << "empty " << empty_passage_counter << std::endl;
          break;
      }
    }
  }
}


TEST (scraper, gbs)
{
  std::string resource {};
  int book {};
  std::string text {};
  std::string standard {};
  
  resource = "Statenbijbel GBS";
  book = 58; // Hebrews.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 10, 14);
  standard = "Want met één offerande heeft Hij in eeuwigheid volmaakt degenen die geheiligd worden.";
  EXPECT_EQ (standard, text);
  
  resource = "King James Version GBS";
  book = 58; // Hebrews.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 10, 14);
  standard = "For by one offering he hath perfected for ever them that are sanctified.";
  EXPECT_EQ (standard, text);
  
  resource = "Statenbijbel Plus GBS";
  book = 58; // Hebrews.
  standard = "WANT a 1 de wet, hebbende 2 een schaduw 3 der toekomende goederen, niet 4 het beeld zelf der zaken, kan met 5 dezelfde offeranden die zij alle jaar 6 geduriglijk opofferen, nimmermeer 7 heiligen degenen die 8 daar toegaan. <br> a Kolossenzen 2:17 Welke zijn <sup>s</sup>een schaduw der toekomende dingen, maar <sup>67</sup>het lichaam is van Christus. <br> 1 Namelijk der ceremoniën onder het Oude Testament. <br> 2 Dat is, een ruw ontwerp, gelijk de schilders plegen een beeld, dat zij daarna willen volmaken, eerst met enige linies en schaduwen in het ruw af te tekenen of te ontwerpen. <br> 3 Dat is, der geestelijke en hemelse zaken, die ons in het Nieuwe Testament zouden verworven en medegedeeld worden, dat is, van Christus Zelven en Zijn weldaden. <br> 4 Sommigen verstaan hierdoor de volmaakte wijze van den uitwendigen godsdienst, dien God door Christus in het Nieuwe Testament zou instellen, die van de instellingen des Ouden Testaments verschilden als een schaduw of eerste ontwerp van het volmaakte beeld eniger zaak; gelijk die daarom ook de eerste beginselen of elementen en abc der wereld worden genaamd, Gal. 4:3 , 9 . Doch dewijl de apostel nergens in dit en in het voorgaande hoofdstuk enige tegenstelling maakt tussen den uitwendigen godsdienst des Ouden en des Nieuwen Testaments, maar alleen tussen de schaduwen des Ouden Testaments en Christus Zelven met Zijn offerande en weldaden, die Hij ons verworven heeft, zo wordt van anderen door deze woorden het beeld zelf bekwamelijker de betekende zaak zelve of het evenbeeld verstaan, naar hetwelk deze schaduwen zijn voorgesteld, gelijk wij worden gezegd geschapen te zijn naar Gods beeld of evenbeeld. Welken zin het bijgevoegde woord het beeld zelf der zaken ook vereist, dat is, de zaken zelve in haar volle beeltenis of gedaante, gelijk zij moesten en zouden zijn. <br> 5 Dat is, van enerlei soort, of van eenzelfden aard en natuur. <br> 6 Gr. in gedurigheid of eeuwigheid , dat is, zonder nalaten, zolang dit priesterdom en deze wet moest duren. <br> 7 Of: volmaken , namelijk naar de consciëntie, door het wegnemen der zonde en der schuld der zonde, gelijk verklaard wordt Hebr. 9:9 . <br> 8 Of: tot God gaan , namelijk met hun offeranden.";
  text = resource_external_cloud_fetch_cache_extract (resource, book, 10, 1);
  EXPECT_EQ (standard, text);
  
  resource = "Statenbijbel GBS";
  book = 3; // Leviticus.
  standard = "Wetten aangaande de manier van het vrijwillig brandoffer in den tabernakel te slachten, vs. 1 , enz. Hetwelk moest zijn óf van grootvee, als van runderen, 2 . Of van kleinvee, als van schapen en geiten, 10 . Of van vogels, als van tortelduiven en jonge duiven, 14 .";
  text = resource_external_cloud_fetch_cache_extract (resource, book, 1, 0);
  EXPECT_EQ (standard, text);
}


TEST (scraper, studylight_albert_barnet)
{
  constexpr auto resource {"Albert Barnes' Notes on the Whole Bible (studylight-eng/bnb)"};
  std::string text {};
  
  std::vector <book_id> book_ids = database::books::get_ids ();
  for (auto book_id : book_ids) {
    continue;
    std::cout << static_cast <int>(book_id) << std::endl;
    book_type type = database::books::get_type (book_id);
    if ((type != book_type::old_testament) && (type != book_type::new_testament)) continue;
    [[maybe_unused]]int verse {1};
    if (book_id == book_id::_2_chronicles) verse = 2;
    int book_num = static_cast <int> (book_id);
    text = resource_logic_study_light_get (resource, book_num, 1, 1);
    if (text.empty ()) {
      EXPECT_EQ (database::books::get_english_from_id (book_id) + " should not be empty - book " + std::to_string(static_cast<int>(book_id)), std::string());
    }
  }
  
  constexpr auto isaiah {23};
  text = resource_logic_study_light_get (resource, isaiah, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (361, text.find("parent bird"));
  
  constexpr auto daniel {27};
  text = resource_logic_study_light_get (resource, daniel, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (293, text.find("For yet the vision is for many days"));
  
  constexpr auto first_thessalonians {52};
  text = resource_logic_study_light_get (resource, first_thessalonians, 1, 4);
  text = filter::strings::html2text (text);
  EXPECT_EQ (83, text.find("beloved of God, your election"));
  
  constexpr auto second_thessalonians {53};
  text = resource_logic_study_light_get (resource, second_thessalonians, 1, 4);
  text = filter::strings::html2text (text);
  EXPECT_EQ (354, text.find("You have shownunwavering confidence in God in your afflictions"));
  
  constexpr auto hebrews {58};
  text = resource_logic_study_light_get (resource, hebrews, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (2917, text.find("τους ἁγιαζομενους"));
  EXPECT_EQ (3678, text.find("By one offering Christ hath forever justifiedsuch as are purged or cleansed by it"));
}


TEST (scraper, studylight_thomas_constable)
{
  constexpr auto resource = "Expository Notes of Dr. Thomas Constable (studylight-eng/dcc)";
  constexpr auto hebrews {58};
  std::string text {};
  text = resource_logic_study_light_get (resource, hebrews, 10, 14);
  text = filter::strings::html2text (text);
  EXPECT_EQ (2011, text.find("accomplishment of our high priest"));
  EXPECT_EQ (2485, text.find("distinctive features of the high priestly office of the Son"));
  EXPECT_EQ (3151, text.find("The one sacrifice of Christ"));
  EXPECT_EQ (3471, text.find("the finality of Jesus Christ’s offering"));
  EXPECT_EQ (4232, text.find("whom Jesus Christ has"));
}


TEST (scraper, net_bible)
{
  using ::testing::HasSubstr;
  constexpr int hebrews {58};
  const char* net_bible {resource_external_net_bible_name ()};
  std::string result = resource_external_cloud_fetch_cache_extract (net_bible, hebrews, 2, 3);
  result = filter::strings::html2text (std::move(result));
  EXPECT_THAT(result, Not(HasSubstr("Cloudflare")));
  EXPECT_THAT(result, HasSubstr("It was first communicated through the Lord"));
}


#endif

