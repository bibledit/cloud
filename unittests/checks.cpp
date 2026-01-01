/*
Copyright (©) 2003-2025 Teus Benschop.

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
#include <database/check.h>
#include <database/state.h>
#include <database/bibles.h>
#include <filter/passage.h>
#include <checks/issues.h>
#include <checks/french.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/text.h>
#include <checks/pairs.h>
#include <checks/sentences.h>
#include <checks/space.h>
#include <checks/verses.h>


TEST (checks, database)
{
  {
    // Test optimize.
    refresh_sandbox (false);
    database::check::optimize ();
  }

  {
    // Test record get truncate.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (0, static_cast<int> (hits.size()));
    
    database::check::record_output ("phpunit", 1, 2, 3, "test");
    hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    
    database::check::delete_output ("");
    hits = database::check::get_hits ();
    EXPECT_EQ (0, static_cast<int> (hits.size()));
  }

  {
    // Test getting details.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 5, 2, 3, "test");
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    EXPECT_EQ ("phpunit", hits [0].bible);
    EXPECT_EQ (5, hits [0].book);
    EXPECT_EQ (2, hits [0].chapter);
    EXPECT_EQ ("test", hits [0].data);
  }

  {
    // Test approvals.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    
    database::check::record_output ("phpunit", 3, 4, 5, "test1");
    database::check::record_output ("phpunit", 3, 4, 5, "test2");
    
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
    
    int id = hits [0].rowid;
    database::check::approve (id);
    hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
    
    std::vector <database::check::Hit> suppressions = database::check::get_suppressions ();
    EXPECT_EQ (1, static_cast<int>(suppressions.size()));
    
    id = suppressions [0].rowid;
    EXPECT_EQ (1, id);

    database::check::release (1);
    hits = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
  }

  {
    // Test delete.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 3, 4, 5, "test1");
    database::check::record_output ("phpunit", 3, 4, 5, "test2");
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int> (hits.size()));
    const int id = hits [0].rowid;
    database::check::delete_id (id);
    hits = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int> (hits.size()));
  }

  {
    // Test passage.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 3, 4, 5, "test1");
    database::check::record_output ("phpunit", 6, 7, 8, "test2");
    Passage passage = database::check::get_passage (2);
    EXPECT_EQ (6, passage.m_book);
    EXPECT_EQ (7, passage.m_chapter);
    EXPECT_EQ ("8", passage.m_verse);
  }

  {
    // Test same checks overflow.
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible ("phpunit");
    database::check::create ();
    database::check::record_output ("phpunit", 3, 4, 5, "once");
    for (int i = 0; i < 100; i++) {
      database::check::record_output ("phpunit", i, i, i, "multiple");
    }
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (12, static_cast<int> (hits.size()));
  }
}


// Check that each issue has a translation.
// Check there's no duplicate translations.
TEST (checks, issues)
{
  using namespace checks::issues;
  constexpr auto start {static_cast<int>(issue::start_boundary)};
  constexpr auto stop {static_cast<int>(issue::stop_boundary)};
  std::set<std::string> translations;
  for (int i {start + 1}; i < stop; i++) {
    const auto issue = static_cast<enum issue>(i);
    const auto translation = text(issue);
    EXPECT_FALSE(translation.empty());
    EXPECT_FALSE(translations.count(translation));
    translations.insert(translation);
  }
}


TEST (checks, french)
{
  refresh_sandbox (false);
  database::check::create ();
  std::string bible = "unit test";
  std::string nbsp = filter::string::non_breaking_space_u00A0 ();
  std::string nnbsp = filter::string::narrow_non_breaking_space_u202F ();
  
  // Test reporting lacking no-break space at French square brackets and other punctuation.
  {
    database::check::delete_output (bible);
    std::map <int, std::string> texts;
    texts [1] = "This is «French» text.";
    texts [2] = "This is « French » text.";
    texts [3] = "This is «" + nbsp + "French" + nbsp + "» with non-breaking spaces.";
    texts [4] = "This is it ;";
    texts [5] = "This is it;";
    texts [6] = "This is it" + nbsp + ";";
    texts [7] = "This is «" + nnbsp + "French" + nnbsp + "» with narrow non-breaking spaces.";
    checks_french::space_before_after_punctuation (bible, 2, 3, texts);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    int hitcount = 6;
    EXPECT_EQ (hitcount, hits.size ());
    if (static_cast<int> (hits.size ()) == hitcount) {
      std::string standard;
      standard = "« - Should be followed by a no-break space in French";
      EXPECT_EQ (standard, hits [0].data);
      EXPECT_EQ (1, hits [0].verse);
      standard = "» - Should be preceded by a no-break space in French";
      EXPECT_EQ (standard, hits [1].data);
      EXPECT_EQ (1, hits [1].verse);
      standard = "« - Should be followed by a no-break space rather than a plain space in French";
      EXPECT_EQ (standard, hits [2].data);
      EXPECT_EQ (2, hits [2].verse);
      standard = "» - Should be preceded by a no-break space rather than a plain space in French";
      EXPECT_EQ (standard, hits [3].data);
      EXPECT_EQ (2, hits [3].verse);
      standard = "; - Should be preceded by a no-break space rather than a plain space in French";
      EXPECT_EQ (standard, hits [4].data);
      EXPECT_EQ (4, hits [4].verse);
      standard = "; - Should be preceded by a no-break space in French";
      EXPECT_EQ (standard, hits [5].data);
      EXPECT_EQ (5, hits [5].verse);
      standard = "» - Should be preceded by a no-break space in French";
    }
  }
  
  // Test French citation style.
  {
    database::check::delete_output (bible);
    std::string usfm = R"(
\c 1
\p
\v 1 This is a «citation».
\v 2 Verse text.
\v 3 Verse text and « citation.
\p
\v 4 « New paragraph with continued citation.
\p
\v 5 « New paragraph with continued citation.
\p
\v 6 New paragraph without continued citation and with end of citation ».
\p
\v 7 New «citation.
\p
\v 8 Forgot citation opener.
\v 9 End of citation ».
)";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.add_usfm_code (usfm);
    filter_text.run (stylesv2::standard_sheet ());
    std::vector <std::map <int, std::string>> verses_paragraphs = filter_text.verses_paragraphs;
    checks_french::citation_style (bible, 2, 3, verses_paragraphs);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    int size = 4;
    EXPECT_EQ (size, hits.size ());
    if (static_cast<int> (hits.size ()) == size) {
      std::string standard1 = "The previous paragraph contains a citation not closed with a » therefore the current paragraph is expected to start with a « to continue that citation in French";
      std::string standard2 = "The paragraph contains more right guillements than needed";
      EXPECT_EQ (6, hits [0].verse);
      EXPECT_EQ (standard1, hits [0].data);
      EXPECT_EQ (6, hits [1].verse);
      EXPECT_EQ (standard2, hits [1].data);
      EXPECT_EQ (8, hits [2].verse);
      EXPECT_EQ (standard1, hits [2].data);
      EXPECT_EQ (9, hits [3].verse);
      EXPECT_EQ (standard2, hits [3].data);
    }
  }
  
  // Real-life data, fixed, regression test.
  {
    database::check::delete_output (bible);
    // In the example following, there is the « in the beginning of the verse 13, it needs to begin a new paragraph, or be considered as an extra «, but the checks don't find it.
    std::string usfm = R"(
    \p
    \v 9 Yezu taka kingana yai na bantu yankaka vandaka kudisikisa nde bo kele bantu ya masonga na meso ya Nzambi, kansi bo vandaka kuvweza bantu yankaka yonso :
    \v 10 « Bantu zole kwendaka na Tempelo na kusamba. Mosi vandaka mfarizi ; yina yankaka, kalaki ya kitari.
    \v 11 Mfarizi telamaka ; yandi sambaka mutindu yai : « E Nzambi, mono ke pesa nge matondo, sambu mono ke fwanana ve na bantu yankaka yonso : bo kele bamiyibi, bantu ya nku, bantu ya bizumba ; mono ke fwanana mpi ve na kalaki yai ya kitari.
    \v 12 Konso mposo, mono ke salaka kibansa ya kulala nzala mbala zole ; ibuna bima yonso ya mono ke zwaka, mono ke kabisaka yo na ndambu kumi, sambu na kupesa nge makabu ndambu mosi. »
    \v 13 « Kalaki ya kitari telamaka kwa yandi mwa-ntama. Nsoni simbaka yandi na kunangula meso na zulu, kansi yandi vandaka kudibula ntulu na kusonga ntantu. Yandi nde : « E Nzambi na mono, mono nsumuki, wila no mawa ! » Yezu yikaka nde :
    \v 14 « Mono ke tubila beno nde : ntangu kalaki ya kitari vutukaka na nzo na yandi, Nzambi talaka yandi na disu ya mbote, kansi yandi talaka ve mfarizi mutindu yina, sambu konso muntu ke kudinangula, Nzambi ta kulumusa yandi ; kansi yina ke kudikulumusa, Nzambi ta nangula yandi. »
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.add_usfm_code (usfm);
    filter_text.run (stylesv2::standard_sheet ());
    std::vector <std::map <int, std::string>> verses_paragraphs = filter_text.verses_paragraphs;
    checks_french::citation_style (bible, 2, 3, verses_paragraphs);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    int size = 1;
    EXPECT_EQ (size, hits.size ());
    if (static_cast<int> (hits.size ()) == size) {
      std::string standard = "The paragraph contains more left guillements than needed";
      EXPECT_EQ (14, hits [0].verse);
      EXPECT_EQ (standard, hits [0].data);
    }
  }
}


TEST (checks, pairs)
{
  refresh_sandbox (false);
  database::check::create ();
  
  const std::string bible = "bible";
  const int book = 2;
  const int chapter = 3;
  std::map <int, std::string> verses;
  const std::vector <std::pair <std::string, std::string> > pairs = {
    std::pair ("[", "]"),
    std::pair ("(", ")"),
    std::pair ("“", "”"),
  };
  std::vector <database::check::Hit> results;
  
  {
    verses = {
      std::pair (2, "Verse two."),
      std::pair (3, "Verse three."),
      std::pair (4, "Verse four.")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (0, results.size());
    database::check::delete_output ("");
  }
  
  {
    verses = {
      std::pair (2, "Verse [two."),
      std::pair (3, "Verse three]."),
      std::pair (4, "Verse four.")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (0, results.size());
    database::check::delete_output ("");
  }
  
  {
    verses = {
      std::pair (2, "Verse [two."),
      std::pair (3, "Verse (three."),
      std::pair (4, "Verse four.")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (2, results.size());
    if (results.size () == 2) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ (bible, hit.bible);
      EXPECT_EQ (book, hit.book);
      EXPECT_EQ (chapter, hit.chapter);
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ (R"(Opening character "[" without its matching closing character "]")", hit.data);
      hit = results[1];
      EXPECT_EQ ("Opening character \"(\" without its matching closing character \")\"", hit.data);
    }
    database::check::delete_output ("");
  }
  
  {
    verses = {
      std::pair (2, "Verse [two."),
      std::pair (3, "Verse three."),
      std::pair (4, "Verse four).")
    };
    checks_pairs::run (bible, book, chapter, verses, pairs, false);
    results = database::check::get_hits ();
    EXPECT_EQ (2, results.size());
    if (results.size () == 2) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ (bible, hit.bible);
      EXPECT_EQ (book, hit.book);
      EXPECT_EQ (chapter, hit.chapter);
      EXPECT_EQ (4, hit.verse);
      EXPECT_EQ ("Closing character \")\" without its matching opening character \"(\"", hit.data);
      hit = results[1];
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ (R"(Opening character "[" without its matching closing character "]")", hit.data);
    }
    database::check::delete_output ("");
  }
}


Checks_Sentences test_sentences_setup ()
{
  Checks_Sentences check;
  check.enter_capitals ("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
  check.enter_small_letters ("a b c d e f g h i j k l m n o p q r s t u v w x y z");
  check.enter_end_marks (". ! ? :");
  check.enter_center_marks (", ;");
  check.enter_disregards ("( ) [ ] { } ' \" * - 0 1 2 3 4 5 6 7 8 9");
  check.enter_names ("Nkosi Longnamelongnamelongname");
  check.initialize ();
  return check;
}


TEST (checks, sentences)
{
  // Test unknown character.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({std::pair (1, "Abc ζ abc.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {std::pair (1, "Unknown character: ζ")};
    EXPECT_EQ (standard, results);
  }
  // Test capital after mid-sentence punctuation mark.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({std::pair (2, "He said, Go.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {std::pair (2, "Capital follows mid-sentence punctuation mark: He said, Go.")};
    EXPECT_EQ (standard, results);
  }
  // Test capital straight after mid-sentence punctuation mark.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({std::pair (2, "He said,Go.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { std::pair (2, "Capital follows straight after a mid-sentence punctuation mark: He said,Go.")};
    EXPECT_EQ (standard, results);
  }
  // Test small letter straight after mid-sentence punctuation mark.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({std::pair (2, "He said,go.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { std::pair (2, "Small letter follows straight after a mid-sentence punctuation mark: He said,go.")};
    EXPECT_EQ (standard, results);
  }
  // Test two verses okay.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ std::pair (17, "Jezus kwam naar de wereld,"), std::pair (18, "dat hij zou lijden.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard;
    EXPECT_EQ (standard, results);
  }
  // Test name after comma several verses okay.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({
      std::pair (17, "Kwasekusithi esebakhuphele ngaphandle, yathi: Baleka ngenxa yempilo yakho, ungakhangeli ngemva kwakho, ungemi egcekeni lonke; balekela entabeni hlezi ubhujiswe."),
      std::pair (18, "ULothi wasesithi kuwo: Kakungabi njalo, Nkosi yami."),
      std::pair (19, "Khangela-ke, inceku yakho ithole umusa emehlweni akho, ukhulisile isihawu sakho, osenze kimi, ukugcina uphila umphefumulo wami; kodwa mina ngingeke ngiphephele entabeni, hlezi ububi bunamathele kimi, besengisifa."),
      std::pair (20, "Khangela-ke, lumuzi useduze ukubalekela kuwo; futhi umncinyane. Ngicela ngibalekele kuwo (kambe kawumncinyane?) Lomphefumulo wami uphile."),
      std::pair (21, "Yasisithi kuye: Khangela, ngibemukele ubuso bakho lakulolu udaba, ukuze ngingawuchithi umuzi okhulume ngawo."),
      std::pair (22, "Phangisa, balekela kuwo; ngoba ngingeze ngenza ulutho uze ufike kuwo. Ngakho babiza ibizo lomuzi ngokuthi yiZowari.")
    });
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard;
    EXPECT_EQ (standard, results);
  }
  // Test long name.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({std::pair (17, "O, Longnamelongnamelongname.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard;
    EXPECT_EQ (standard, results);
  }
  // Test no space after full stop.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ std::pair (2, "He did that.He went.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {
      std::pair (2, "A letter follows straight after an end-sentence punctuation mark: He did that.He went."),
      std::pair (2, "No capital after an end-sentence punctuation mark: did that.He went.")
    };
    EXPECT_EQ (standard, results);
  }
  // Test capital full stop.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ std::pair (2, "He did that. he went.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { std::pair (2, "No capital after an end-sentence punctuation mark: did that. he went.")};
    EXPECT_EQ (standard, results);
  }
  // Test paragraph
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p"}, {""}, {{std::pair (1, "he said")}});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {
      std::pair (1, "Paragraph does not start with a capital: he said"),
      std::pair (1, "Paragraph does not end with an end marker: he said")
    };
    EXPECT_EQ (standard, results);
  }
  // Test that a paragraph started by \q and starting with a small letter, is not flagged.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"q"}, {"q"}, {{ std::pair (1, "he said")}});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {
      std::pair (1, "Paragraph does not end with an end marker: he said")
    };
    EXPECT_EQ (standard, results);
  }
  // Test that a paragraph without proper ending, where the next paragraph starts with e.g. \q, is not flagged.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p", "q"}, {"q"}, {{ std::pair (1, "He said,")}, { std::pair (1, "he is Jesus.")}});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {};
    EXPECT_EQ (standard, results);
  }
  // Test flagging a paragraph that starts with a Greek small letter.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p"}, {"q"}, {{ std::pair (1, "εὐθέως")}});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = {
      std::pair (1, "Paragraph does not start with a capital: εὐθέως"),
      std::pair (1, "Paragraph does not end with an end marker: εὐθέως")
    };
    EXPECT_EQ (standard, results);
  }
  // Test a correct paragraph.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p"}, {"q"}, {{ std::pair (1, "Immediately εὐθέως.")}});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard;
    EXPECT_EQ (standard, results);
  }
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"q1"}, {"q1", "q"}, {{ std::pair (1, "Immediately εὐθέως.")}});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard;
    EXPECT_EQ (standard, results);
  }
  // Test two punctuation marks.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ std::pair (2, "He did that..")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { std::pair (2, "Two punctuation marks in sequence: He did that..")};
    EXPECT_EQ (standard, results);
  }
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ std::pair (2, "He did ;. That.")});
    std::vector <std::pair<int, std::string>> results = check.get_results ();
    std::vector <std::pair<int, std::string>> standard = { std::pair (2, "Two punctuation marks in sequence: He did ;. That.")};
    EXPECT_EQ (standard, results);
  }
}


TEST (checks, space)
{
  refresh_sandbox (false);
  database::check::create ();
  std::string bible = "unit test";
  
  // Test reporting space at end of verse: No spaces here.
  {
    database::check::delete_output (bible);
    std::string usfm =
    "\\v 2 This is verse 2.\n"
    "\\v 3 This is verse 3.\n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (0, hits.size ());
  }
  
  // Test reporting space at end of verse: One space here.
  {
    database::check::delete_output (bible);
    std::string usfm =
    "\\v 4 This is verse 4. \n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  
  // Test reporting space at end of verse: One space here.
  // The space is not right at the end in the USFM.
  // The space is followed by empty embedded markup.
  // So in the pure text stream, the space is at the end of a verse.
  // Check that it gets reported.
  {
    database::check::delete_output (bible);
    std::string usfm =
    "\\v 5 This is verse \\add 5. \\add*\n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  
  // Test there's no space in the cleaned text at the end of the verse.
  {
    database::check::delete_output (bible);
    std::string usfm =
    "\\v 6 This is verse \\add 6.\\add*\n"
    ;
    checks::space::space_end_verse (bible, 2, 3, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (0, hits.size ());
  }
  
  // Check that it catches a double space in USFM.
  {
    database::check::delete_output (bible);
    std::string usfm = R"(\v 1 This contains  a double space.)";
    checks::space::double_space_usfm (bible, 2, 3, 4, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  
  // Check that it transposes spaces in notes.
  {
    std::string usfm = R"(\v 1 Verse\f + \fr 3.1\fk  keyword\ft  Text.\f* one.)";
    std::string standard = R"(\v 1 Verse\f + \fr 3.1 \fk keyword \ft Text.\f* one.)";
    bool transposed = checks::space::transpose_note_space (usfm);
    EXPECT_EQ (true, transposed);
    EXPECT_EQ (standard, usfm);
  }
  {
    std::string usfm = R"(\v 2 Verse\x + \xo 3.2\xt  Text.\x* two.)";
    std::string standard = R"(\v 2 Verse\x + \xo 3.2 \xt Text.\x* two.)";
    bool transposed = checks::space::transpose_note_space (usfm);
    EXPECT_EQ (true, transposed);
    EXPECT_EQ (standard, usfm);
  }
  
  // Check on a space before final note and cross reference markup.
  {
    database::check::delete_output (bible);
    int verse = 4;
    std::string usfm = R"(\v 1 Note \f ... \f*.)";
    checks::space::space_before_final_note_markup (bible, 2, 3, verse, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  {
    database::check::delete_output (bible);
    int verse = 5;
    std::string usfm = R"(\v 2 Endnote \fe ... \fe*.)";
    checks::space::space_before_final_note_markup (bible, 2, 3, verse, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
  {
    database::check::delete_output (bible);
    int verse = 6;
    std::string usfm = R"(\v 3 Cross reference \x ... \x*.)";
    checks::space::space_before_final_note_markup (bible, 2, 3, verse, usfm);
    std::vector <database::check::Hit> hits = database::check::get_hits ();
    EXPECT_EQ (1, hits.size ());
  }
}


TEST (verses, basic)
{
  refresh_sandbox (false);
  database::check::create ();
  // Test Missing Punctuation At End
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said."),
      std::pair (5, "He said: “Jesus.”")
    };
    checks_verses::missing_punctuation_at_end ("1", 1, 1, verses, ", ;", ". ! ? :", "”");
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int>(results.size()));
    if (results.size ()) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (3, hit.verse);
      EXPECT_EQ ("No punctuation at end of verse: y", hit.data);
    }
  }
  database::check::delete_output ("");
  // Test Pattern
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said.")
    };
    checks_verses::patterns ("1", 1, 1, verses, {"did"});
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (1, static_cast<int>(results.size()));
    if (results.size ()) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (3, hit.verse);
      EXPECT_EQ ("Pattern found in text: did", hit.data);
    }
  }
  database::check::delete_output ("");
  // Test Pattern
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said.")
    };
    checks_verses::patterns ("1", 1, 1, verses, {"Did"});
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (0, static_cast<int>(results.size()));
  }
  database::check::delete_output ("");
  // Test Pattern
  {
    std::map <int, std::string> verses = {
      std::pair (2, "He said."),
      std::pair (3, "He didn't say"),
      std::pair (4, "He said.")
    };
    checks_verses::patterns ("1", 1, 1, verses, {"said"});
    std::vector <database::check::Hit> results = database::check::get_hits ();
    EXPECT_EQ (2, static_cast<int>(results.size()));
    if (results.size () == 2) {
      database::check::Hit hit = results[0];
      EXPECT_EQ (1, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (2, hit.verse);
      EXPECT_EQ ("Pattern found in text: said", hit.data);
      hit = results[1];
      EXPECT_EQ (2, hit.rowid);
      EXPECT_EQ ("1", hit.bible);
      EXPECT_EQ (1, hit.book);
      EXPECT_EQ (1, hit.chapter);
      EXPECT_EQ (4, hit.verse);
      EXPECT_EQ ("Pattern found in text: said", hit.data);
    }
  }
  database::check::delete_output ("");
}


#endif
