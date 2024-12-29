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
#include <checks/sentences.h>


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


TEST (check, sentences)
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


#endif

