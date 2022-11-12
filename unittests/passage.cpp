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
#include <database/books.h>
using namespace std;


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
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("Genesis"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1 Corinthians"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("Genesis")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1 Corinthians")));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book_v1 ("Song of Solomon"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("Song of Solomon")));
    evaluate (__LINE__, __func__, 60, filter_passage_interpret_book_v1 ("I Peter"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_peter), static_cast<int>(filter_passage_interpret_book_v2 ("I Peter")));
    evaluate (__LINE__, __func__, 63, filter_passage_interpret_book_v1 ("II John"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_john), static_cast<int>(filter_passage_interpret_book_v2 ("II John")));
    evaluate (__LINE__, __func__, 64, filter_passage_interpret_book_v1 ("III John"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_3_john), static_cast<int>(filter_passage_interpret_book_v2 ("III John")));
  }

  // Interpret book USFM.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("GEN"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("GEN")));
    evaluate (__LINE__, __func__, 8, filter_passage_interpret_book_v1 ("FRT"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_front_matter), static_cast<int>(filter_passage_interpret_book_v2 ("FRT")));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1CO"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1CO")));
  }
  
  // Interpret book OSIS.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("Gen"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("Gen")));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1Cor"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1Cor")));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book_v1 ("Song"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("Song")));
  }
  
  // Interpret book BibleWorks.
  {
    evaluate (__LINE__, __func__, 1,  filter_passage_interpret_book_v1 ("Gen"));
    evaluate (__LINE__, __func__, 2,  filter_passage_interpret_book_v1 ("Exod"));
    evaluate (__LINE__, __func__, 3,  filter_passage_interpret_book_v1 ("Lev"));
    evaluate (__LINE__, __func__, 4,  filter_passage_interpret_book_v1 ("Num"));
    evaluate (__LINE__, __func__, 5,  filter_passage_interpret_book_v1 ("Deut"));
    evaluate (__LINE__, __func__, 6,  filter_passage_interpret_book_v1 ("Jos"));
    evaluate (__LINE__, __func__, 7,  filter_passage_interpret_book_v1 ("Jdg"));
    evaluate (__LINE__, __func__, 8,  filter_passage_interpret_book_v1 ("Ruth"));
    evaluate (__LINE__, __func__, 9,  filter_passage_interpret_book_v1 ("1 Sam"));
    evaluate (__LINE__, __func__, 10, filter_passage_interpret_book_v1 ("2 Sam"));
    evaluate (__LINE__, __func__, 11, filter_passage_interpret_book_v1 ("1 Ki"));
    evaluate (__LINE__, __func__, 12, filter_passage_interpret_book_v1 ("2 Ki"));
    evaluate (__LINE__, __func__, 13, filter_passage_interpret_book_v1 ("1 Chr"));
    evaluate (__LINE__, __func__, 14, filter_passage_interpret_book_v1 ("2 Chr"));
    evaluate (__LINE__, __func__, 15, filter_passage_interpret_book_v1 ("Ezr"));
    evaluate (__LINE__, __func__, 16, filter_passage_interpret_book_v1 ("Neh"));
    evaluate (__LINE__, __func__, 17, filter_passage_interpret_book_v1 ("Est"));
    evaluate (__LINE__, __func__, 18, filter_passage_interpret_book_v1 ("Job"));
    evaluate (__LINE__, __func__, 19, filter_passage_interpret_book_v1 ("Ps"));
    evaluate (__LINE__, __func__, 20, filter_passage_interpret_book_v1 ("Prov"));
    evaluate (__LINE__, __func__, 21, filter_passage_interpret_book_v1 ("Eccl"));
    evaluate (__LINE__, __func__, 27, filter_passage_interpret_book_v1 ("Cant"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book_v1 ("Sol"));
    evaluate (__LINE__, __func__, 23, filter_passage_interpret_book_v1 ("Isa"));
    evaluate (__LINE__, __func__, 24, filter_passage_interpret_book_v1 ("Jer"));
    evaluate (__LINE__, __func__, 25, filter_passage_interpret_book_v1 ("Lam"));
    evaluate (__LINE__, __func__, 26, filter_passage_interpret_book_v1 ("Ezek"));
    evaluate (__LINE__, __func__, 27, filter_passage_interpret_book_v1 ("Dan"));
    evaluate (__LINE__, __func__, 28, filter_passage_interpret_book_v1 ("Hos"));
    evaluate (__LINE__, __func__, 29, filter_passage_interpret_book_v1 ("Joel"));
    evaluate (__LINE__, __func__, 30, filter_passage_interpret_book_v1 ("Amos"));
    evaluate (__LINE__, __func__, 31, filter_passage_interpret_book_v1 ("Obad"));
    evaluate (__LINE__, __func__, 32, filter_passage_interpret_book_v1 ("Jon"));
    evaluate (__LINE__, __func__, 33, filter_passage_interpret_book_v1 ("Mic"));
    evaluate (__LINE__, __func__, 34, filter_passage_interpret_book_v1 ("Nah"));
    evaluate (__LINE__, __func__, 35, filter_passage_interpret_book_v1 ("Hab"));
    evaluate (__LINE__, __func__, 36, filter_passage_interpret_book_v1 ("Zeph"));
    evaluate (__LINE__, __func__, 37, filter_passage_interpret_book_v1 ("Hag"));
    evaluate (__LINE__, __func__, 38, filter_passage_interpret_book_v1 ("Zech"));
    evaluate (__LINE__, __func__, 39, filter_passage_interpret_book_v1 ("Mal"));
    evaluate (__LINE__, __func__, 40, filter_passage_interpret_book_v1 ("Matt"));
    evaluate (__LINE__, __func__, 41, filter_passage_interpret_book_v1 ("Mk"));
    evaluate (__LINE__, __func__, 42, filter_passage_interpret_book_v1 ("Lk"));
    evaluate (__LINE__, __func__, 32, filter_passage_interpret_book_v1 ("Jn"));
    evaluate (__LINE__, __func__, 44, filter_passage_interpret_book_v1 ("Acts"));
    evaluate (__LINE__, __func__, 45, filter_passage_interpret_book_v1 ("Rom"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1 Co"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1Co"));
    evaluate (__LINE__, __func__, 47, filter_passage_interpret_book_v1 ("2 Co"));
    evaluate (__LINE__, __func__, 48, filter_passage_interpret_book_v1 ("Gal"));
    evaluate (__LINE__, __func__, 49, filter_passage_interpret_book_v1 ("Eph"));
    evaluate (__LINE__, __func__, 50, filter_passage_interpret_book_v1 ("Phil"));
    evaluate (__LINE__, __func__, 51, filter_passage_interpret_book_v1 ("Col"));
    evaluate (__LINE__, __func__, 52, filter_passage_interpret_book_v1 ("1 Thess"));
    evaluate (__LINE__, __func__, 53, filter_passage_interpret_book_v1 ("2 Thess"));
    evaluate (__LINE__, __func__, 54, filter_passage_interpret_book_v1 ("1 Tim"));
    evaluate (__LINE__, __func__, 55, filter_passage_interpret_book_v1 ("2 Tim"));
    evaluate (__LINE__, __func__, 56, filter_passage_interpret_book_v1 ("Tit"));
    evaluate (__LINE__, __func__, 57, filter_passage_interpret_book_v1 ("Phlm"));
    evaluate (__LINE__, __func__, 58, filter_passage_interpret_book_v1 ("Heb"));
    evaluate (__LINE__, __func__, 59, filter_passage_interpret_book_v1 ("Jas"));
    evaluate (__LINE__, __func__, 60, filter_passage_interpret_book_v1 ("1 Pet"));
    evaluate (__LINE__, __func__, 61, filter_passage_interpret_book_v1 ("2 Pet"));
    evaluate (__LINE__, __func__, 62, filter_passage_interpret_book_v1 ("1 Jn"));
    evaluate (__LINE__, __func__, 63, filter_passage_interpret_book_v1 ("2 Jn"));
    evaluate (__LINE__, __func__, 64, filter_passage_interpret_book_v1 ("3 Jn"));
    evaluate (__LINE__, __func__, 65, filter_passage_interpret_book_v1 ("Jude"));
    evaluate (__LINE__, __func__, 66, filter_passage_interpret_book_v1 ("Rev"));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_genesis), static_cast<int>(filter_passage_interpret_book_v2 ("Gen")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_exodus), static_cast<int>(filter_passage_interpret_book_v2 ("Exod")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_leviticus), static_cast<int>(filter_passage_interpret_book_v2 ("Lev")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_numbers), static_cast<int>(filter_passage_interpret_book_v2 ("Num")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_deuteronomy), static_cast<int>(filter_passage_interpret_book_v2 ("Deut")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_joshua), static_cast<int>(filter_passage_interpret_book_v2 ("Jos")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_judges), static_cast<int>(filter_passage_interpret_book_v2 ("Jdg")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_ruth), static_cast<int>(filter_passage_interpret_book_v2 ("Ruth")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_samuel), static_cast<int>(filter_passage_interpret_book_v2 ("1 Sam")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_samuel), static_cast<int>(filter_passage_interpret_book_v2 ("2 Sam")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_kings), static_cast<int>(filter_passage_interpret_book_v2 ("1 Ki")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_kings), static_cast<int>(filter_passage_interpret_book_v2 ("2 Ki")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_chronicles), static_cast<int>(filter_passage_interpret_book_v2 ("1 Chr")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_chronicles), static_cast<int>(filter_passage_interpret_book_v2 ("2 Chr")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_ezra), static_cast<int>(filter_passage_interpret_book_v2 ("Ezr")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_nehemiah), static_cast<int>(filter_passage_interpret_book_v2 ("Neh")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_esther), static_cast<int>(filter_passage_interpret_book_v2 ("Est")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_job), static_cast<int>(filter_passage_interpret_book_v2 ("Job")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_psalms), static_cast<int>(filter_passage_interpret_book_v2 ("Ps")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_proverbs), static_cast<int>(filter_passage_interpret_book_v2 ("Prov")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_ecclesiastes), static_cast<int>(filter_passage_interpret_book_v2 ("Eccl")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_song_of_solomon), static_cast<int>(filter_passage_interpret_book_v2 ("Sol")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_isaiah), static_cast<int>(filter_passage_interpret_book_v2 ("Isa")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_jeremiah), static_cast<int>(filter_passage_interpret_book_v2 ("Jer")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_lamentations), static_cast<int>(filter_passage_interpret_book_v2 ("Lam")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_ezekiel), static_cast<int>(filter_passage_interpret_book_v2 ("Ezek")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_daniel), static_cast<int>(filter_passage_interpret_book_v2 ("Dan")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_hosea), static_cast<int>(filter_passage_interpret_book_v2 ("Hos")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_joel), static_cast<int>(filter_passage_interpret_book_v2 ("Joel")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_amos), static_cast<int>(filter_passage_interpret_book_v2 ("Amos")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_obadiah), static_cast<int>(filter_passage_interpret_book_v2 ("Obad")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_jonah), static_cast<int>(filter_passage_interpret_book_v2 ("Jon")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_micah), static_cast<int>(filter_passage_interpret_book_v2 ("Mic")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_nahum), static_cast<int>(filter_passage_interpret_book_v2 ("Nah")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_habakkuk), static_cast<int>(filter_passage_interpret_book_v2 ("Hab")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_zephaniah), static_cast<int>(filter_passage_interpret_book_v2 ("Zeph")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_haggai), static_cast<int>(filter_passage_interpret_book_v2 ("Hag")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_zechariah), static_cast<int>(filter_passage_interpret_book_v2 ("Zech")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_malachi), static_cast<int>(filter_passage_interpret_book_v2 ("Mal")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_matthew), static_cast<int>(filter_passage_interpret_book_v2 ("Matt")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_mark), static_cast<int>(filter_passage_interpret_book_v2 ("Mk")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_luke), static_cast<int>(filter_passage_interpret_book_v2 ("Lk")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_john), static_cast<int>(filter_passage_interpret_book_v2 ("Jn")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_acts), static_cast<int>(filter_passage_interpret_book_v2 ("Acts")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_romans), static_cast<int>(filter_passage_interpret_book_v2 ("Rom")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1 Co")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("1Co")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_corinthians), static_cast<int>(filter_passage_interpret_book_v2 ("2 Co")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_galatians), static_cast<int>(filter_passage_interpret_book_v2 ("Gal")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_ephesians), static_cast<int>(filter_passage_interpret_book_v2 ("Eph")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_philippians), static_cast<int>(filter_passage_interpret_book_v2 ("Phil")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_colossians), static_cast<int>(filter_passage_interpret_book_v2 ("Col")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_thessalonians), static_cast<int>(filter_passage_interpret_book_v2 ("1 Thess")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_thessalonians), static_cast<int>(filter_passage_interpret_book_v2 ("2 Thess")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_timothy), static_cast<int>(filter_passage_interpret_book_v2 ("1 Tim")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_timothy), static_cast<int>(filter_passage_interpret_book_v2 ("2 Tim")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_titus), static_cast<int>(filter_passage_interpret_book_v2 ("Tit")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_philemon), static_cast<int>(filter_passage_interpret_book_v2 ("Phlm")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_hebrews), static_cast<int>(filter_passage_interpret_book_v2 ("Heb")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_james), static_cast<int>(filter_passage_interpret_book_v2 ("Jas")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_peter), static_cast<int>(filter_passage_interpret_book_v2 ("1 Pet")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_peter), static_cast<int>(filter_passage_interpret_book_v2 ("2 Pet")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_1_john), static_cast<int>(filter_passage_interpret_book_v2 ("1 Jn")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_2_john), static_cast<int>(filter_passage_interpret_book_v2 ("2 Jn")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_3_john), static_cast<int>(filter_passage_interpret_book_v2 ("3 Jn")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_jude), static_cast<int>(filter_passage_interpret_book_v2 ("Jude")));
    evaluate (__LINE__, __func__, static_cast<int>(book_id::_revelation), static_cast<int>(filter_passage_interpret_book_v2 ("Rev")));

    cout << "Todo" << endl; // Todo
  }

  // Interpret book Online Bible.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("Ge"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1Co"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book_v1 ("So"));
  }

  // Interpret book partial names.
  {
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("G"));
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("g"));
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("ge"));
    evaluate (__LINE__, __func__, 1, filter_passage_interpret_book_v1 ("gene"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1 Cori"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1 cori"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1 corint"));
    evaluate (__LINE__, __func__, 46, filter_passage_interpret_book_v1 ("1cor"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book_v1 ("song"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book_v1 ("song of"));
    evaluate (__LINE__, __func__, 22, filter_passage_interpret_book_v1 ("song of sol"));
    evaluate (__LINE__, __func__, 11, filter_passage_interpret_book_v1 ("1ki"));
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
