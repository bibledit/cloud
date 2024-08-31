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
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <filter/passage.h>
#include <database/config/bible.h>
#include <database/bibles.h>
#include <database/state.h>
#include <database/books.h>


TEST (filter, passage)
{
  {
    Passage passage;
    EXPECT_EQ ("", passage.m_bible);
    EXPECT_EQ (0, passage.m_book);
    EXPECT_EQ (0, passage.m_chapter);
    EXPECT_EQ ("", passage.m_verse);
    passage = Passage ("bible", 1, 2, "3");
    EXPECT_EQ ("bible", passage.m_bible);
    EXPECT_EQ (1, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("3", passage.m_verse);
    EXPECT_EQ (true, passage.equal (passage));
    Passage passage2 = Passage ("bible", 1, 2, "4");
    EXPECT_EQ (false, passage.equal (passage2));
  }
  
  // Encoding and decoding passages.
  {
    Passage input = Passage ("עברית", 1, 2, "3");
    std::string encoded = input.encode ();
    Passage output = Passage::decode (encoded);
    EXPECT_EQ (true, input.equal (output));
    
    input = Passage ("ελληνικά", 5, 4, "0");
    encoded = input.encode ();
    output = Passage::decode (encoded);
    EXPECT_EQ (true, input.equal (output));
    
    input = Passage ("Sample .!_ Bible", 99, 999, "9999");
    encoded = input.encode ();
    output = Passage::decode (encoded);
    EXPECT_EQ (true, input.equal (output));
  }
  
  // Passage display.
  {
    EXPECT_EQ ("Genesis 10:2", filter_passage_display (1, 10, "2"));
    EXPECT_EQ ("Exodus 11:2", filter_passage_display (2, 11, "2"));
  }

  // Inline passage display.
  {
    EXPECT_EQ ("Genesis 10:2", filter_passage_display_inline ({Passage ("", 1, 10, "2")}));
    EXPECT_EQ ("", filter_passage_display_inline ({}));
    EXPECT_EQ ("Genesis 10:2 | Exodus 777:777", filter_passage_display_inline ({ Passage ("", 1, 10, "2"), Passage ("", 2, 777, "777")}));
    EXPECT_EQ ("Unknown 10:2 | Exodus 777:777", filter_passage_display_inline ({ Passage ("", -1, 10, "2"), Passage ("", 2, 777, "777")}));
  }

  // Passage display multiline.
  {
    EXPECT_EQ ("Genesis 10:2\n", filter_passage_display_multiline ({Passage ("", 1, 10, "2")}));
    EXPECT_EQ ("", filter_passage_display_inline ({}));
    EXPECT_EQ ("Genesis 10:2\nExodus 777:777\n", filter_passage_display_multiline ({ Passage ("", 1, 10, "2"), Passage ("", 2, 777, "777")}));
    EXPECT_EQ ("Unknown 10:2\nExodus 777:777\n", filter_passage_display_multiline ({ Passage ("", -1, 10, "2"), Passage ("", 2, 777, "777")}));
  }
  
  // Passage to integer.
  {
    Passage standard = Passage ("", 6, 4, "2");
    int numeric = filter_passage_to_integer (standard);
    Passage passage = filter_integer_to_passage (numeric);
    EXPECT_EQ (true, standard.equal (passage));
  }

  // Interpret book English.
  {
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("Genesis")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1 Corinthians")));
    EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("Song of Solomon")));
    EXPECT_EQ (static_cast<int>(book_id::_1_peter), static_cast<int>(filter_passage_interpret_book_v2 ("I Peter")));
    EXPECT_EQ (static_cast<int>(book_id::_2_john), static_cast<int>(filter_passage_interpret_book_v2 ("II John")));
    EXPECT_EQ (static_cast<int>(book_id::_3_john), static_cast<int>(filter_passage_interpret_book_v2 ("III John")));
  }

  // Interpret book USFM.
  {
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("GEN")));
    EXPECT_EQ (static_cast<int>(book_id::_front_matter), static_cast<int>(filter_passage_interpret_book_v2 ("FRT")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1CO")));
  }
  
  // Interpret book OSIS.
  {
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("Gen")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1Cor")));
    EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("Song")));
  }
  
  // Interpret book BibleWorks.
  {
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("Gen")));
    EXPECT_EQ (static_cast<int>(book_id::_exodus), static_cast<int>(filter_passage_interpret_book_v2 ("Exod")));
    EXPECT_EQ (static_cast<int>(book_id::_leviticus), static_cast<int>(filter_passage_interpret_book_v2 ("Lev")));
    EXPECT_EQ (static_cast<int>(book_id::_numbers), static_cast<int>(filter_passage_interpret_book_v2 ("Num")));
    EXPECT_EQ (static_cast<int>(book_id::_deuteronomy), static_cast<int>(filter_passage_interpret_book_v2 ("Deut")));
    EXPECT_EQ (static_cast<int>(book_id::_joshua), static_cast<int>(filter_passage_interpret_book_v2 ("Jos")));
    EXPECT_EQ (static_cast<int>(book_id::_judges), static_cast<int>(filter_passage_interpret_book_v2 ("Jdg")));
    EXPECT_EQ (static_cast<int>(book_id::_ruth), static_cast<int>(filter_passage_interpret_book_v2 ("Ruth")));
    EXPECT_EQ (static_cast<int>(book_id::_1_samuel), static_cast<int>(filter_passage_interpret_book_v2 ("1 Sam")));
    EXPECT_EQ (static_cast<int>(book_id::_2_samuel), static_cast<int>(filter_passage_interpret_book_v2 ("2 Sam")));
    EXPECT_EQ (static_cast<int>(book_id::_1_kings), static_cast<int>(filter_passage_interpret_book_v2 ("1 Ki")));
    EXPECT_EQ (static_cast<int>(book_id::_2_kings), static_cast<int>(filter_passage_interpret_book_v2 ("2 Ki")));
    EXPECT_EQ (static_cast<int>(book_id::_1_chronicles), static_cast<int>(filter_passage_interpret_book_v2 ("1 Chr")));
    EXPECT_EQ (static_cast<int>(book_id::_2_chronicles), static_cast<int>(filter_passage_interpret_book_v2 ("2 Chr")));
    EXPECT_EQ (static_cast<int>(book_id::_ezra), static_cast<int>(filter_passage_interpret_book_v2 ("Ezr")));
    EXPECT_EQ (static_cast<int>(book_id::_nehemiah), static_cast<int>(filter_passage_interpret_book_v2 ("Neh")));
    EXPECT_EQ (static_cast<int>(book_id::_esther), static_cast<int>(filter_passage_interpret_book_v2 ("Est")));
    EXPECT_EQ (static_cast<int>(book_id::_job), static_cast<int>(filter_passage_interpret_book_v2 ("Job")));
    EXPECT_EQ (static_cast<int>(book_id::_psalms), static_cast<int>(filter_passage_interpret_book_v2 ("Ps")));
    EXPECT_EQ (static_cast<int>(book_id::_proverbs), static_cast<int>(filter_passage_interpret_book_v2 ("Prov")));
    EXPECT_EQ (static_cast<int>(book_id::_ecclesiastes), static_cast<int>(filter_passage_interpret_book_v2 ("Eccl")));
    EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("Sol")));
    EXPECT_EQ (static_cast<int>(book_id::_isaiah), static_cast<int>(filter_passage_interpret_book_v2 ("Isa")));
    EXPECT_EQ (static_cast<int>(book_id::_jeremiah), static_cast<int>(filter_passage_interpret_book_v2 ("Jer")));
    EXPECT_EQ (static_cast<int>(book_id::_lamentations), static_cast<int>(filter_passage_interpret_book_v2 ("Lam")));
    EXPECT_EQ (static_cast<int>(book_id::_ezekiel), static_cast<int>(filter_passage_interpret_book_v2 ("Ezek")));
    EXPECT_EQ (static_cast<int>(book_id::_daniel), static_cast<int>(filter_passage_interpret_book_v2 ("Dan")));
    EXPECT_EQ (static_cast<int>(book_id::_hosea), static_cast<int>(filter_passage_interpret_book_v2 ("Hos")));
    EXPECT_EQ (static_cast<int>(book_id::_joel), static_cast<int>(filter_passage_interpret_book_v2 ("Joel")));
    EXPECT_EQ (static_cast<int>(book_id::_amos), static_cast<int>(filter_passage_interpret_book_v2 ("Amos")));
    EXPECT_EQ (static_cast<int>(book_id::_obadiah), static_cast<int>(filter_passage_interpret_book_v2 ("Obad")));
    EXPECT_EQ (static_cast<int>(book_id::_jonah), static_cast<int>(filter_passage_interpret_book_v2 ("Jon")));
    EXPECT_EQ (static_cast<int>(book_id::_micah), static_cast<int>(filter_passage_interpret_book_v2 ("Mic")));
    EXPECT_EQ (static_cast<int>(book_id::_nahum), static_cast<int>(filter_passage_interpret_book_v2 ("Nah")));
    EXPECT_EQ (static_cast<int>(book_id::_habakkuk), static_cast<int>(filter_passage_interpret_book_v2 ("Hab")));
    EXPECT_EQ (static_cast<int>(book_id::_zephaniah), static_cast<int>(filter_passage_interpret_book_v2 ("Zeph")));
    EXPECT_EQ (static_cast<int>(book_id::_haggai), static_cast<int>(filter_passage_interpret_book_v2 ("Hag")));
    EXPECT_EQ (static_cast<int>(book_id::_zechariah), static_cast<int>(filter_passage_interpret_book_v2 ("Zech")));
    EXPECT_EQ (static_cast<int>(book_id::_malachi), static_cast<int>(filter_passage_interpret_book_v2 ("Mal")));
    EXPECT_EQ (static_cast<int>(book_id::_matthew), static_cast<int>(filter_passage_interpret_book_v2 ("Matt")));
    EXPECT_EQ (static_cast<int>(book_id::_mark), static_cast<int>(filter_passage_interpret_book_v2 ("Mk")));
    EXPECT_EQ (static_cast<int>(book_id::_luke), static_cast<int>(filter_passage_interpret_book_v2 ("Lk")));
    EXPECT_EQ (static_cast<int>(book_id::_john), static_cast<int>(filter_passage_interpret_book_v2 ("Jn")));
    EXPECT_EQ (static_cast<int>(book_id::_acts), static_cast<int>(filter_passage_interpret_book_v2 ("Acts")));
    EXPECT_EQ (static_cast<int>(book_id::_romans), static_cast<int>(filter_passage_interpret_book_v2 ("Rom")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1 Co")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1Co")));
    EXPECT_EQ (static_cast<int>(book_id::_2_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("2 Co")));
    EXPECT_EQ (static_cast<int>(book_id::_galatians), static_cast<int>(filter_passage_interpret_book_v2 ("Gal")));
    EXPECT_EQ (static_cast<int>(book_id::_ephesians), static_cast<int>(filter_passage_interpret_book_v2 ("Eph")));
    EXPECT_EQ (static_cast<int>(book_id::_philippians), static_cast<int>(filter_passage_interpret_book_v2 ("Phil")));
    EXPECT_EQ (static_cast<int>(book_id::_colossians), static_cast<int>(filter_passage_interpret_book_v2 ("Col")));
    EXPECT_EQ (static_cast<int>(book_id::_1_thessalonians), static_cast<int>(filter_passage_interpret_book_v2 ("1 Thess")));
    EXPECT_EQ (static_cast<int>(book_id::_2_thessalonians), static_cast<int>(filter_passage_interpret_book_v2 ("2 Thess")));
    EXPECT_EQ (static_cast<int>(book_id::_1_timothy), static_cast<int>(filter_passage_interpret_book_v2 ("1 Tim")));
    EXPECT_EQ (static_cast<int>(book_id::_2_timothy), static_cast<int>(filter_passage_interpret_book_v2 ("2 Tim")));
    EXPECT_EQ (static_cast<int>(book_id::_titus), static_cast<int>(filter_passage_interpret_book_v2 ("Tit")));
    EXPECT_EQ (static_cast<int>(book_id::_philemon), static_cast<int>(filter_passage_interpret_book_v2 ("Phlm")));
    EXPECT_EQ (static_cast<int>(book_id::_hebrews), static_cast<int>(filter_passage_interpret_book_v2 ("Heb")));
    EXPECT_EQ (static_cast<int>(book_id::_james), static_cast<int>(filter_passage_interpret_book_v2 ("Jas")));
    EXPECT_EQ (static_cast<int>(book_id::_1_peter), static_cast<int>(filter_passage_interpret_book_v2 ("1 Pet")));
    EXPECT_EQ (static_cast<int>(book_id::_2_peter), static_cast<int>(filter_passage_interpret_book_v2 ("2 Pet")));
    EXPECT_EQ (static_cast<int>(book_id::_1_john), static_cast<int>(filter_passage_interpret_book_v2 ("1 Jn")));
    EXPECT_EQ (static_cast<int>(book_id::_2_john), static_cast<int>(filter_passage_interpret_book_v2 ("2 Jn")));
    EXPECT_EQ (static_cast<int>(book_id::_3_john), static_cast<int>(filter_passage_interpret_book_v2 ("3 Jn")));
    EXPECT_EQ (static_cast<int>(book_id::_jude), static_cast<int>(filter_passage_interpret_book_v2 ("Jude")));
    EXPECT_EQ (static_cast<int>(book_id::_revelation), static_cast<int>(filter_passage_interpret_book_v2 ("Rev")));
  }

  // Interpret book Online Bible.
  {
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("Ge")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1Co")));
    EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("So")));
  }

  // Interpret book partial names.
  {
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("G")));
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("g")));
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("ge")));
    EXPECT_EQ (static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("gene")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1 Cori")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1 cori")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1 corint")));
    EXPECT_EQ (static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1cor")));
    EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("song")));
    EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("song of")));
    EXPECT_EQ (static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("song of sol")));
    EXPECT_EQ (static_cast<int>(book_id::_1_kings), static_cast<int>(filter_passage_interpret_book_v2 ("1ki")));
  }

  // Explode passage.
  {
    Passage passage = filter_passage_explode_passage ("Genesis 2:2");
    EXPECT_EQ (1, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("2", passage.m_verse);
    
    passage = filter_passage_explode_passage ("1 Corinth. 2:2");
    EXPECT_EQ (46, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("2", passage.m_verse);
    
    passage = filter_passage_explode_passage ("Song of Sol. 2:2");
    EXPECT_EQ (22, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("2", passage.m_verse);
    
    passage = filter_passage_explode_passage ("Revelation 2:2");
    EXPECT_EQ (66, passage.m_book);
    EXPECT_EQ (2, passage.m_chapter);
    EXPECT_EQ ("2", passage.m_verse);
  }

  // Interpret passage.
  {
    Passage currentPassage = Passage ("", 2, 4, "6");
    
    Passage standard = Passage ("", 1, 2, "3");
    Passage output = filter_passage_interpret_passage (currentPassage, " Genesis 2 3");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = Passage ("", 46, 2, "3");
    output = filter_passage_interpret_passage (currentPassage, "1  Corinthians 2:3");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = Passage ("", 22, 2, "3");
    output = filter_passage_interpret_passage (currentPassage, "Song of Solomon 2.3");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = currentPassage;
    output = filter_passage_interpret_passage (currentPassage, "");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = Passage ("", 1, 1, "1");
    output = filter_passage_interpret_passage (currentPassage, "Genesis Exodus");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = Passage ("", 2, 1, "1");
    output = filter_passage_interpret_passage (currentPassage, "Exodus");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = Passage ("", 2, 4, "11");
    output = filter_passage_interpret_passage (currentPassage, "11");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = Passage ("", 2, 11, "14");
    output = filter_passage_interpret_passage (currentPassage, "11 14");
    EXPECT_EQ (true, standard.equal (output));
    
    standard = Passage ("", 22, 2, "1");
    output = filter_passage_interpret_passage (currentPassage, "Song of Solomon 2");
    EXPECT_EQ (true, standard.equal (output));
  }

  // Sequence and range none.
  {
    std::vector <std::string> standard = {"Exod. 30:4"};
    std::vector <std::string> output = filter_passage_handle_sequences_ranges ("Exod. 30:4");
    EXPECT_EQ (standard, output);
  }

  // Sequence.
  {
    std::vector <std::string> standard = {"Exod. 37:5", "14", "28"};
    std::vector <std::string> output = filter_passage_handle_sequences_ranges ("Exod. 37:5, 14, 28");
    EXPECT_EQ (standard, output);
  }

  // Range.,
  {
    std::vector <std::string> standard = {"Exod. 37:5", "14", "15", "16"};
    std::vector <std::string> output = filter_passage_handle_sequences_ranges ("Exod. 37:5, 14 - 16");
    EXPECT_EQ (standard, output);
  }

  // Sequence and range.
  {
    std::vector <std::string> standard = {"Exod. 37:4", "5", "14", "15", "27", "28", "30", "40"};
    std::vector <std::string> output = filter_passage_handle_sequences_ranges ("Exod. 37:4 - 5, 14 - 15, 27 - 28, 30, 40");
    EXPECT_EQ (standard, output);
  }
  
  // Sequence and range.
  {
    std::vector <std::string> standard = {"Exod. 25:13", "14", "27", "28"};
    std::vector <std::string> output = filter_passage_handle_sequences_ranges ("Exod. 25:13-14, 27-28");
    EXPECT_EQ (standard, output);
  }

  // Test ordered books.
  {
    refresh_sandbox (true);
    Database_State::create ();
    
    std::string bible = "php unit";
    
    // No ordering.
    database::bibles::create_bible (bible);
    database::bibles::store_chapter (bible, 1, 1, "data");
    database::bibles::store_chapter (bible, 2, 1, "data");
    database::bibles::store_chapter (bible, 3, 1, "data");
    database::bibles::store_chapter (bible, 4, 1, "data");
    database::bibles::store_chapter (bible, 5, 1, "data");
    std::vector <int> books = filter_passage_get_ordered_books (bible);
    EXPECT_EQ ((std::vector <int>{1, 2, 3, 4, 5}), books);
    
    // Existing books re-ordered.
    database::config::bible::set_book_order (bible, "1 3 2 5 4");
    books = filter_passage_get_ordered_books (bible);
    EXPECT_EQ ((std::vector <int>{1, 3, 2, 5, 4}), books);
    
    // Some books ordered, and Bible has extra books: These are to be added to the end.
    database::config::bible::set_book_order (bible, "1 3 2");
    books = filter_passage_get_ordered_books (bible);
    EXPECT_EQ ((std::vector <int>{1, 3, 2, 4, 5}), books);
    
    // More books ordered than in Bible: Remove the extra ones.
    database::config::bible::set_book_order (bible, "1 3 2 5 4 6");
    books = filter_passage_get_ordered_books (bible);
    EXPECT_EQ ((std::vector <int>{1, 3, 2, 5, 4}), books);
  }

  refresh_sandbox (true);
}

#endif

