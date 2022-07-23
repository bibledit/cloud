/*
Copyright (©) 2003-2022 Teus Benschop.

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


#include <unittests/passage.h>
#include <unittests/utilities.h>
#include <filter/passage.h>
#include <database/config/bible.h>
#include <database/bibles.h>
#include <database/state.h>


void test_passage ()
{
  trace_unit_tests (__func__);

  {
    Passage passage;
    evaluate (__LINE__, __func__, "", passage.m_bible);
    evaluate (__LINE__, __func__, 0, passage.m_book);
    evaluate (__LINE__, __func__, 0, passage.m_chapter);
    evaluate (__LINE__, __func__, "", passage.m_verse);
    passage = Passage ("bible", 1, 2, "3");
    evaluate (__LINE__, __func__, "bible", passage.m_bible);
    evaluate (__LINE__, __func__, 1, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "3", passage.m_verse);
    evaluate (__LINE__, __func__, true, passage.equal (passage));
    Passage passage2 = Passage ("bible", 1, 2, "4");
    evaluate (__LINE__, __func__, false, passage.equal (passage2));
  }
  
  // Encoding and decoding passages.
  {
    Passage input = Passage ("עברית", 1, 2, "3");
    string encoded = input.encode ();
    Passage output = Passage::decode (encoded);
    evaluate (__LINE__, __func__, true, input.equal (output));
    
    input = Passage ("ελληνικά", 5, 4, "0");
    encoded = input.encode ();
    output = Passage::decode (encoded);
    evaluate (__LINE__, __func__, true, input.equal (output));
    
    input = Passage ("Sample .!_ Bible", 99, 999, "9999");
    encoded = input.encode ();
    output = Passage::decode (encoded);
    evaluate (__LINE__, __func__, true, input.equal (output));
  }
  
  // Passage display.
  {
    evaluate (__LINE__, __func__, "Genesis 10:2", filter_passage_display (1, 10, "2"));
    evaluate (__LINE__, __func__, "Exodus 11:2", filter_passage_display (2, 11, "2"));
  }

  // Inline passage display.
  {
    evaluate (__LINE__, __func__, "Genesis 10:2", filter_passage_display_inline ({Passage ("", 1, 10, "2")}));
    evaluate (__LINE__, __func__, "", filter_passage_display_inline ({}));
    evaluate (__LINE__, __func__, "Genesis 10:2 | Exodus 777:777", filter_passage_display_inline ({ Passage ("", 1, 10, "2"), Passage ("", 2, 777, "777")}));
    evaluate (__LINE__, __func__, "Unknown 10:2 | Exodus 777:777", filter_passage_display_inline ({ Passage ("", -1, 10, "2"), Passage ("", 2, 777, "777")}));
  }

  // Passage display multiline.
  {
    evaluate (__LINE__, __func__, "Genesis 10:2\n", filter_passage_display_multiline ({Passage ("", 1, 10, "2")}));
    evaluate (__LINE__, __func__, "", filter_passage_display_inline ({}));
    evaluate (__LINE__, __func__, "Genesis 10:2\nExodus 777:777\n", filter_passage_display_multiline ({ Passage ("", 1, 10, "2"), Passage ("", 2, 777, "777")}));
    evaluate (__LINE__, __func__, "Unknown 10:2\nExodus 777:777\n", filter_passage_display_multiline ({ Passage ("", -1, 10, "2"), Passage ("", 2, 777, "777")}));
  }
  
  // Passage to integer.
  {
    Passage standard = Passage ("", 6, 4, "2");
    int numeric = filter_passage_to_integer (standard);
    Passage passage = filter_integer_to_passage (numeric);
    evaluate (__LINE__, __func__, true, standard.equal (passage));
  }

  // Interpret book English.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("Genesis"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1 Corinthians"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book ("Song of Solomon"));
    evaluate (__LINE__, __func__, 60, filter_passage_interpret_book ("I Peter"));
    evaluate (__LINE__, __func__, 63, filter_passage_interpret_book ("II John"));
    evaluate (__LINE__, __func__, 64, filter_passage_interpret_book ("III John"));
  }

  // Interpret book USFM.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("GEN"));
    evaluate (__LINE__, __func__, 8, filter_passage_interpret_book ("FRT"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1CO"));
  }
  
  // Interpret book OSIS.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("Gen"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1Cor"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book ("Song"));
  }
  
  // Interpret book BibleWorks.
  {
    evaluate (__LINE__, __func__, 1,  filter_passage_interpret_book ("Gen"));
    evaluate (__LINE__, __func__, 2,  filter_passage_interpret_book ("Exod"));
    evaluate (__LINE__, __func__, 3,  filter_passage_interpret_book ("Lev"));
    evaluate (__LINE__, __func__, 4,  filter_passage_interpret_book ("Num"));
    evaluate (__LINE__, __func__, 5,  filter_passage_interpret_book ("Deut"));
    evaluate (__LINE__, __func__, 6,  filter_passage_interpret_book ("Jos"));
    evaluate (__LINE__, __func__, 7,  filter_passage_interpret_book ("Jdg"));
    evaluate (__LINE__, __func__, 8,  filter_passage_interpret_book ("Ruth"));
    evaluate (__LINE__, __func__, 9,  filter_passage_interpret_book ("1 Sam"));
    evaluate (__LINE__, __func__, 10, filter_passage_interpret_book ("2 Sam"));
    evaluate (__LINE__, __func__, 11, filter_passage_interpret_book ("1 Ki"));
    evaluate (__LINE__, __func__, 12, filter_passage_interpret_book ("2 Ki"));
    evaluate (__LINE__, __func__, 13, filter_passage_interpret_book ("1 Chr"));
    evaluate (__LINE__, __func__, 14, filter_passage_interpret_book ("2 Chr"));
    evaluate (__LINE__, __func__, 15, filter_passage_interpret_book ("Ezr"));
    evaluate (__LINE__, __func__, 16, filter_passage_interpret_book ("Neh"));
    evaluate (__LINE__, __func__, 17, filter_passage_interpret_book ("Est"));
    evaluate (__LINE__, __func__, 18, filter_passage_interpret_book ("Job"));
    evaluate (__LINE__, __func__, 19, filter_passage_interpret_book ("Ps"));
    evaluate (__LINE__, __func__, 20, filter_passage_interpret_book ("Prov"));
    evaluate (__LINE__, __func__, 21, filter_passage_interpret_book ("Eccl"));
    evaluate (__LINE__, __func__, 27, filter_passage_interpret_book ("Cant"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book ("Sol"));
    evaluate (__LINE__, __func__, 23, filter_passage_interpret_book ("Isa"));
    evaluate (__LINE__, __func__, 24, filter_passage_interpret_book ("Jer"));
    evaluate (__LINE__, __func__, 25, filter_passage_interpret_book ("Lam"));
    evaluate (__LINE__, __func__, 26, filter_passage_interpret_book ("Ezek"));
    evaluate (__LINE__, __func__, 27, filter_passage_interpret_book ("Dan"));
    evaluate (__LINE__, __func__, 28, filter_passage_interpret_book ("Hos"));
    evaluate (__LINE__, __func__, 29, filter_passage_interpret_book ("Joel"));
    evaluate (__LINE__, __func__, 30, filter_passage_interpret_book ("Amos"));
    evaluate (__LINE__, __func__, 31, filter_passage_interpret_book ("Obad"));
    evaluate (__LINE__, __func__, 32, filter_passage_interpret_book ("Jon"));
    evaluate (__LINE__, __func__, 33, filter_passage_interpret_book ("Mic"));
    evaluate (__LINE__, __func__, 34, filter_passage_interpret_book ("Nah"));
    evaluate (__LINE__, __func__, 35, filter_passage_interpret_book ("Hab"));
    evaluate (__LINE__, __func__, 36, filter_passage_interpret_book ("Zeph"));
    evaluate (__LINE__, __func__, 37, filter_passage_interpret_book ("Hag"));
    evaluate (__LINE__, __func__, 38, filter_passage_interpret_book ("Zech"));
    evaluate (__LINE__, __func__, 39, filter_passage_interpret_book ("Mal"));
    evaluate (__LINE__, __func__, 40, filter_passage_interpret_book ("Matt"));
    evaluate (__LINE__, __func__, 41, filter_passage_interpret_book ("Mk"));
    evaluate (__LINE__, __func__, 42, filter_passage_interpret_book ("Lk"));
    evaluate (__LINE__, __func__, 32, filter_passage_interpret_book ("Jn"));
    evaluate (__LINE__, __func__, 44, filter_passage_interpret_book ("Acts"));
    evaluate (__LINE__, __func__, 45, filter_passage_interpret_book ("Rom"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1 Co"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1Co"));
    evaluate (__LINE__, __func__, 47, filter_passage_interpret_book ("2 Co"));
    evaluate (__LINE__, __func__, 48, filter_passage_interpret_book ("Gal"));
    evaluate (__LINE__, __func__, 49, filter_passage_interpret_book ("Eph"));
    evaluate (__LINE__, __func__, 50, filter_passage_interpret_book ("Phil"));
    evaluate (__LINE__, __func__, 51, filter_passage_interpret_book ("Col"));
    evaluate (__LINE__, __func__, 52, filter_passage_interpret_book ("1 Thess"));
    evaluate (__LINE__, __func__, 53, filter_passage_interpret_book ("2 Thess"));
    evaluate (__LINE__, __func__, 54, filter_passage_interpret_book ("1 Tim"));
    evaluate (__LINE__, __func__, 55, filter_passage_interpret_book ("2 Tim"));
    evaluate (__LINE__, __func__, 56, filter_passage_interpret_book ("Tit"));
    evaluate (__LINE__, __func__, 57, filter_passage_interpret_book ("Phlm"));
    evaluate (__LINE__, __func__, 58, filter_passage_interpret_book ("Heb"));
    evaluate (__LINE__, __func__, 59, filter_passage_interpret_book ("Jas"));
    evaluate (__LINE__, __func__, 60, filter_passage_interpret_book ("1 Pet"));
    evaluate (__LINE__, __func__, 61, filter_passage_interpret_book ("2 Pet"));
    evaluate (__LINE__, __func__, 62, filter_passage_interpret_book ("1 Jn"));
    evaluate (__LINE__, __func__, 63, filter_passage_interpret_book ("2 Jn"));
    evaluate (__LINE__, __func__, 64, filter_passage_interpret_book ("3 Jn"));
    evaluate (__LINE__, __func__, 65, filter_passage_interpret_book ("Jude"));
    evaluate (__LINE__, __func__, 66, filter_passage_interpret_book ("Rev"));
  }

  // Interpret book Online Bible.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("Ge"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1Co"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book ("So"));
  }

  // Interpret book partial names.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("G"));
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("g"));
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("ge"));
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book ("gene"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1 Cori"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1 cori"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1 corint"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book ("1cor"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book ("song"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book ("song of"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book ("song of sol"));
    evaluate (__LINE__, __func__, 11, filter_passage_interpret_book ("1ki"));
  }

  // Explode passage.
  {
    Passage passage = filter_passage_explode_passage ("Genesis 2:2");
    evaluate (__LINE__, __func__, 1, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "2", passage.m_verse);
    
    passage = filter_passage_explode_passage ("1 Corinth. 2:2");
    evaluate (__LINE__, __func__, 46, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "2", passage.m_verse);
    
    passage = filter_passage_explode_passage ("Song of Sol. 2:2");
    evaluate (__LINE__, __func__, 22, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "2", passage.m_verse);
    
    passage = filter_passage_explode_passage ("Revelation 2:2");
    evaluate (__LINE__, __func__, 66, passage.m_book);
    evaluate (__LINE__, __func__, 2, passage.m_chapter);
    evaluate (__LINE__, __func__, "2", passage.m_verse);
  }

  // Interpret passage.
  {
    Passage currentPassage = Passage ("", 2, 4, "6");
    
    Passage standard = Passage ("", 1, 2, "3");
    Passage output = filter_passage_interpret_passage (currentPassage, " Genesis 2 3");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = Passage ("", 46, 2, "3");
    output = filter_passage_interpret_passage (currentPassage, "1  Corinthians 2:3");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = Passage ("", 22, 2, "3");
    output = filter_passage_interpret_passage (currentPassage, "Song of Solomon 2.3");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = currentPassage;
    output = filter_passage_interpret_passage (currentPassage, "");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = Passage ("", 1, 1, "1");
    output = filter_passage_interpret_passage (currentPassage, "Genesis Exodus");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = Passage ("", 2, 1, "1");
    output = filter_passage_interpret_passage (currentPassage, "Exodus");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = Passage ("", 2, 4, "11");
    output = filter_passage_interpret_passage (currentPassage, "11");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = Passage ("", 2, 11, "14");
    output = filter_passage_interpret_passage (currentPassage, "11 14");
    evaluate (__LINE__, __func__, true, standard.equal (output));
    
    standard = Passage ("", 22, 2, "1");
    output = filter_passage_interpret_passage (currentPassage, "Song of Solomon 2");
    evaluate (__LINE__, __func__, true, standard.equal (output));
  }

  // Sequence and range none.
  {
    vector <string> standard = {"Exod. 30:4"};
    vector <string> output = filter_passage_handle_sequences_ranges ("Exod. 30:4");
    evaluate (__LINE__, __func__, standard, output);
  }

  // Sequence.
  {
    vector <string> standard = {"Exod. 37:5", "14", "28"};
    vector <string> output = filter_passage_handle_sequences_ranges ("Exod. 37:5, 14, 28");
    evaluate (__LINE__, __func__, standard, output);
  }

  // Range.,
  {
    vector <string> standard = {"Exod. 37:5", "14", "15", "16"};
    vector <string> output = filter_passage_handle_sequences_ranges ("Exod. 37:5, 14 - 16");
    evaluate (__LINE__, __func__, standard, output);
  }

  // Sequence and range.
  {
    vector <string> standard = {"Exod. 37:4", "5", "14", "15", "27", "28", "30", "40"};
    vector <string> output = filter_passage_handle_sequences_ranges ("Exod. 37:4 - 5, 14 - 15, 27 - 28, 30, 40");
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Sequence and range.
  {
    vector <string> standard = {"Exod. 25:13", "14", "27", "28"};
    vector <string> output = filter_passage_handle_sequences_ranges ("Exod. 25:13-14, 27-28");
    evaluate (__LINE__, __func__, standard, output);
  }

  // Test ordered books.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    string bible = "php unit";
    
    // No ordering.
    database_bibles.createBible (bible);
    database_bibles.storeChapter (bible, 1, 1, "data");
    database_bibles.storeChapter (bible, 2, 1, "data");
    database_bibles.storeChapter (bible, 3, 1, "data");
    database_bibles.storeChapter (bible, 4, 1, "data");
    database_bibles.storeChapter (bible, 5, 1, "data");
    vector <int> books = filter_passage_get_ordered_books (bible);
    evaluate (__LINE__, __func__, {1, 2, 3, 4, 5}, books);
    
    // Existing books re-ordered.
    Database_Config_Bible::setBookOrder (bible, "1 3 2 5 4");
    books = filter_passage_get_ordered_books (bible);
    evaluate (__LINE__, __func__, {1, 3, 2, 5, 4}, books);
    
    // Some books ordered, and Bible has extra books: These are to be added to the end.
    Database_Config_Bible::setBookOrder (bible, "1 3 2");
    books = filter_passage_get_ordered_books (bible);
    evaluate (__LINE__, __func__, {1, 3, 2, 4, 5}, books);
    
    // More books ordered than in Bible: Remove the extra ones.
    Database_Config_Bible::setBookOrder (bible, "1 3 2 5 4 6");
    books = filter_passage_get_ordered_books (bible);
    evaluate (__LINE__, __func__, {1, 3, 2, 5, 4}, books);
  }

  refresh_sandbox (true);
}
