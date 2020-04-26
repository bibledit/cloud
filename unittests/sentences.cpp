/*
Copyright (©) 2003-2020 Teus Benschop.

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


#include <unittests/sentences.h>
#include <unittests/utilities.h>
#include <checks/sentences.h>


Checks_Sentences test_sentences_setup ()
{
  trace_unit_tests (__func__);
  
  Checks_Sentences check;
  check.enterCapitals ("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
  check.enterSmallLetters ("a b c d e f g h i j k l m n o p q r s t u v w x y z");
  check.enterEndMarks (". ! ? :");
  check.enterCenterMarks (", ;");
  check.enterDisregards ("( ) [ ] { } ' \" * - 0 1 2 3 4 5 6 7 8 9");
  check.enterNames ("Nkosi Longnamelongnamelongname");
  check.initialize ();
  return check;
}


void test_sentences ()
{
  trace_unit_tests (__func__);
  
  // Test unknown character.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({make_pair (1, "Abc ζ abc.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {make_pair (1, "Unknown character: ζ")};
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test capital after mid-sentence punctuation mark.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({make_pair (2, "He said, Go.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {make_pair (2, "Capital follows mid-sentence punctuation mark: He said, Go.")};
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test capital straight after mid-sentence punctuation mark.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({make_pair (2, "He said,Go.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = { make_pair (2, "Capital follows straight after a mid-sentence punctuation mark: He said,Go.")};
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test small letter straight after mid-sentence punctuation mark.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({make_pair (2, "He said,go.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = { make_pair (2, "Small letter follows straight after a mid-sentence punctuation mark: He said,go.")};
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test two verses okay.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ make_pair (17, "Jezus kwam naar de wereld,"), make_pair (18, "dat hij zou lijden.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard;
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test name after comma several verses okay.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({
      make_pair (17, "Kwasekusithi esebakhuphele ngaphandle, yathi: Baleka ngenxa yempilo yakho, ungakhangeli ngemva kwakho, ungemi egcekeni lonke; balekela entabeni hlezi ubhujiswe."),
      make_pair (18, "ULothi wasesithi kuwo: Kakungabi njalo, Nkosi yami."),
      make_pair (19, "Khangela-ke, inceku yakho ithole umusa emehlweni akho, ukhulisile isihawu sakho, osenze kimi, ukugcina uphila umphefumulo wami; kodwa mina ngingeke ngiphephele entabeni, hlezi ububi bunamathele kimi, besengisifa."),
      make_pair (20, "Khangela-ke, lumuzi useduze ukubalekela kuwo; futhi umncinyane. Ngicela ngibalekele kuwo (kambe kawumncinyane?) Lomphefumulo wami uphile."),
      make_pair (21, "Yasisithi kuye: Khangela, ngibemukele ubuso bakho lakulolu udaba, ukuze ngingawuchithi umuzi okhulume ngawo."),
      make_pair (22, "Phangisa, balekela kuwo; ngoba ngingeze ngenza ulutho uze ufike kuwo. Ngakho babiza ibizo lomuzi ngokuthi yiZowari.")
    });
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard;
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test long name.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({make_pair (17, "O, Longnamelongnamelongname.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard;
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test no space after full stop.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ make_pair (2, "He did that.He went.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (2, "A letter follows straight after an end-sentence punctuation mark: He did that.He went."),
      make_pair (2, "No capital after an end-sentence punctuation mark: did that.He went.")
    };
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test capital full stop.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ make_pair (2, "He did that. he went.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = { make_pair (2, "No capital after an end-sentence punctuation mark: did that. he went.")};
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test paragraph
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p"}, {""}, {{make_pair (1, "he said")}});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
                       make_pair (1, "Paragraph does not start with a capital: he said"),
                       make_pair (1, "Paragraph does not end with an end marker: he said")
    };
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test that a paragraph started by \q and starting with a small letter, is not flagged.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"q"}, {"q"}, {{ make_pair (1, "he said")}});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (1, "Paragraph does not end with an end marker: he said")
    };
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test that a paragraph without proper ending, where the next paragraph starts with e.g. \q, is not flagged.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p", "q"}, {"q"}, {{ make_pair (1, "He said,")}, { make_pair (1, "he is Jesus.")}});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {};
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test flagging a paragraph that starts with a Greek small letter.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p"}, {"q"}, {{ make_pair (1, "εὐθέως")}});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {
      make_pair (1, "Paragraph does not start with a capital: εὐθέως"),
      make_pair (1, "Paragraph does not end with an end marker: εὐθέως")
    };
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test a correct paragraph.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"p"}, {"q"}, {{ make_pair (1, "Immediately εὐθέως.")}});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard;
    evaluate (__LINE__, __func__, standard, results);
  }
  {
    Checks_Sentences check = test_sentences_setup ();
    check.paragraphs ({"q1"}, {"q1", "q"}, {{ make_pair (1, "Immediately εὐθέως.")}});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard;
    evaluate (__LINE__, __func__, standard, results);
  }
  // Test two punctuation marks.
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ make_pair (2, "He did that..")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = { make_pair (2, "Two punctuation marks in sequence: He did that..")};
    evaluate (__LINE__, __func__, standard, results);
  }
  {
    Checks_Sentences check = test_sentences_setup ();
    check.check ({ make_pair (2, "He did ;. That.")});
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = { make_pair (2, "Two punctuation marks in sequence: He did ;. That.")};
    evaluate (__LINE__, __func__, standard, results);
  }
}
