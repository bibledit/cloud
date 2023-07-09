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
#include <unittests/utilities.h>
#include <database/state.h>
#include <database/bibles.h>
#include <search/logic.h>
using namespace std;


void test_search_setup ()
{
  string standardUSFM1 =  "\\c 1\n"
                          "\\p\n"
                          "\\v 1 Text of the first verse.\n"
                          "\\v 2 \\add Text of the second \\add*verse.\n"
                          "\\s Heading\n"
                          "\\p\n"
                          "\\v 3 Text of the 3rd third verse.\n"
                          "\\v 4 Text of the 4th \\add fourth\\add* verse.\n"
                          "\\v 5 Text of the 5th fifth verse is this: Verse five ✆.\n"
                          "\\v 6 Text of the 6th sixth verse ✆.\n"
                          "\\v 7 Text of the seventh verse with some UTF-8 characters: ✆ ❼ ሯ.\n"
                          "\\v 8 Verse eight ✆ ❼ ሯ.\n"
                          "\\v 9 Verse nine nine.\n"
                          "\\v 10 خدا بود و کلمه کلمه خدا بود. در ابتدا کلمه بود و کلمه نزد.\n";
  string standardUSFM2 =  "\\c 1\n"
                          "\\p\n"
                          "\\v 3 And he said.\n";
  string standardUSFM3 =  "\\c 3  \n"
                          "\\s1 Manusia pertama berdosa karena tidak mentaati Allah.\n"
                          "\\p\n"
                          "\\v 1 Ular adalah binatang yang paling licik diantara semua binatang buas yang ALLAH ciptajkan. Ular bertanya kepada perempuan itu, “Apakah benar Allah berkata kepada kalian, ‘Jangan memakan satu buah pun dari semua pohon yang ada di taman ini?’ ’’\n"
                          "\\v 2-3 Perempuan itu menjawab, “ALLAH melarang kami menyentuh apa lagi memakan buah yang berada di tengah-tengah taman. Bila kami melanggar larangannya, maka kami akan mati! Tetapi untuk semua buah yang lain kami boleh memakannya.”\n"
                          "\\v 4,5 Ular berkata kepada perempuan itu,”Tentu saja kamu tidak akan mati. ALLAH mengatakan hal itu karena tahu kalau kamu makan buah dari pohon yang berada di tengah taman itu, kamu akan memahami sesuatu yang baru yaitu mata dan pikiranmu akan terbuka dan kamu akan menjadi sama seperti Allah. Kamu akan mengetahui apa yang baik yang boleh dilakukan dan yang jahat, yang tidak boleh dilakukan.\n"
                          "\\v 6 Perempuan itu melihat bahwa pohon itu menghasilkan buah yang sangat indah dan enak untuk dimakan. Maka dia menginginkannya karena mengira, akan menjadi perempuan yang bijaksana. Lalu, dipetiklah beberapa buah dan dimakannya. Kemudian, dia memberikan beberapa buah juga kepada suaminya dan suaminya juga memakannya.\n";
  Database_State::create ();
  Database_Bibles database_bibles;
  database_bibles.createBible ("phpunit");
  database_bibles.storeChapter ("phpunit", 2, 3, standardUSFM1);
  database_bibles.createBible ("phpunit2");
  database_bibles.storeChapter ("phpunit2", 4, 5, standardUSFM2);
  database_bibles.createBible ("phpunit3");
  database_bibles.storeChapter ("phpunit3", 6, 7, standardUSFM3);
}


TEST (search, logic)
{
  // Test updating search fields.
  {
    refresh_sandbox (true);
    test_search_setup ();
    search_logic_index_chapter ("phpunit", 2, 3);
  }

  // Test searching and getting Bible passage
  {
    refresh_sandbox (true);
    test_search_setup ();
    vector <Passage> passages = search_logic_search_text ("sixth", {"phpunit"});
    EXPECT_EQ (1, static_cast <int> (passages.size()));
    if (!passages.empty ()) {
      EXPECT_EQ ("phpunit", passages[0].m_bible);
      EXPECT_EQ (2, passages[0].m_book);
      EXPECT_EQ (3, passages[0].m_chapter);
      EXPECT_EQ ("6", passages[0].m_verse);
    }
  }

  // Search in combined verses.
  {
    refresh_sandbox (true);
    test_search_setup ();
    vector <Passage> passages = search_logic_search_text ("ALLAH", {"phpunit3"});
    EXPECT_EQ (4, static_cast <int> (passages.size()));
    if (passages.size () == 4) {
      EXPECT_EQ ("phpunit3", passages[0].m_bible);
      EXPECT_EQ (6, passages[1].m_book);
      EXPECT_EQ (7, passages[2].m_chapter);
      EXPECT_EQ ("0", passages[0].m_verse);
      EXPECT_EQ ("1", passages[1].m_verse);
      EXPECT_EQ ("2", passages[2].m_verse);
      EXPECT_EQ ("4", passages[3].m_verse);
    }
  }

  // Test search Bible
  {
    refresh_sandbox (true);
    test_search_setup ();
    vector <Passage> passages = search_logic_search_bible_text ("phpunit", "sixth");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    passages = search_logic_search_bible_text ("phpunit2", "sixth");
    EXPECT_EQ (0, static_cast<int>(passages.size ()));
    passages = search_logic_search_bible_text ("phpunit2", "said");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
  }
  
  // Test search Bible case sensitive.
  {
    refresh_sandbox (true);
    test_search_setup ();
    vector <Passage> passages = search_logic_search_bible_text_case_sensitive ("phpunit", "Verse");
    EXPECT_EQ (3, static_cast<int>(passages.size ()));
    passages = search_logic_search_bible_text_case_sensitive ("phpunit", "sixth");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
    passages = search_logic_search_bible_text_case_sensitive ("phpunit2", "said");
    EXPECT_EQ (1, static_cast<int>(passages.size ()));
  }

  // Searching in USFM.
  {
    refresh_sandbox (true);
    test_search_setup ();
    vector <Passage> passages = search_logic_search_bible_usfm ("phpunit", "\\Add");
    EXPECT_EQ (2, static_cast<int>(passages.size ()));
  }
  
  // Searching in USFM case-sensitive.
  {
    refresh_sandbox (true);
    test_search_setup ();
    vector <Passage> passages = search_logic_search_bible_usfm_case_sensitive ("phpunit", "\\Add");
    EXPECT_EQ (0, static_cast<int>(passages.size ()));
    passages = search_logic_search_bible_usfm_case_sensitive ("phpunit", "\\add");
    EXPECT_EQ (2, static_cast<int>(passages.size ()));
  }
  
  // Test getting Bible verse text.
  {
    refresh_sandbox (true);
    test_search_setup ();
    // Plain.
    string text = search_logic_get_bible_verse_text ("phpunit", 2, 3, 5);
    EXPECT_EQ ("Text of the 5th fifth verse is this: Verse five ✆.", text);
    // USFM.
    text = search_logic_get_bible_verse_usfm ("phpunit", 2, 3, 5);
    EXPECT_EQ ("\\v 5 Text of the 5th fifth verse is this: Verse five ✆.", text);
  }
  
  // Test deleting a bible or book or chapter.
  {
    refresh_sandbox (true);
    
    test_search_setup ();
    
    vector <Passage> passages = search_logic_search_bible_text ("phpunit", "e");
    EXPECT_EQ (10, static_cast <int> (passages.size()));
    search_logic_delete_bible ("phpunit");
    passages = search_logic_search_bible_text ("phpunit", "e");
    EXPECT_EQ (0, static_cast <int> (passages.size()));
    
    test_search_setup ();

    search_logic_delete_book ("phpunit", 3);
    passages = search_logic_search_bible_text ("phpunit", "e");
    EXPECT_EQ (10, static_cast <int> (passages.size()));
    search_logic_delete_book ("phpunit", 2);
    passages = search_logic_search_bible_text ("phpunit", "e");
    EXPECT_EQ (0, static_cast <int> (passages.size()));
    
    test_search_setup ();

    search_logic_delete_chapter ("phpunit", 3, 3);
    passages = search_logic_search_bible_text ("phpunit", "e");
    EXPECT_EQ (10, static_cast <int> (passages.size()));
    search_logic_delete_chapter ("phpunit", 2, 3);
    passages = search_logic_search_bible_text ("phpunit", "e");
    EXPECT_EQ (0, static_cast <int> (passages.size()));
  }
  
  // Test total verse count in Bible.
  {
    refresh_sandbox (true);
    test_search_setup ();
    int count = search_logic_get_verse_count ("phpunit");
    EXPECT_EQ (11, count);
  }
}

#endif

