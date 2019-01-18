/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


#include <unittests/french.h>
#include <unittests/utilities.h>
#include <checks/french.h>
#include <database/check.h>
#include <filter/string.h>
#include <filter/text.h>
#include <checks/usfm.h>


void test_dev () // Todo
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  
  string bible = "phpunit";
  
  // Test plain text and notes for export to Quick Bible.
  {
    string usfm = R"(
\id GEN
\c 1
\p
\v 1 This is verse one\x + Xref 1\x*\f + Note 1\f*.
\v 2 This is verse two\f + Note 2\f*\x + Xref 2\x*.
\v 3 This is verse three\fe + Endnote 3\fe*.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet ());

    map <int, string> output = filter_text.getVersesText ();
    map <int, string> standard = {
      {1, "This is verse one."},
      {2, "This is verse two."},
      {3, "This is verse three."},
    };
    evaluate (__LINE__, __func__, standard, output);

    int n = 5;
    int size = filter_text.notes_plain_text.size();
    evaluate (__LINE__, __func__, n, size);
    if (size == n) {
      evaluate (__LINE__, __func__, "1", filter_text.notes_plain_text[0].first);
      evaluate (__LINE__, __func__, "1", filter_text.notes_plain_text[1].first);
      evaluate (__LINE__, __func__, "2", filter_text.notes_plain_text[2].first);
      evaluate (__LINE__, __func__, "2", filter_text.notes_plain_text[3].first);
      evaluate (__LINE__, __func__, "3", filter_text.notes_plain_text[4].first);
      evaluate (__LINE__, __func__, "Xref 1", filter_text.notes_plain_text[0].second);
      evaluate (__LINE__, __func__, "Note 1", filter_text.notes_plain_text[1].second);
      evaluate (__LINE__, __func__, "Note 2", filter_text.notes_plain_text[2].second);
      evaluate (__LINE__, __func__, "Xref 2", filter_text.notes_plain_text[3].second);
      evaluate (__LINE__, __func__, "Endnote 3", filter_text.notes_plain_text[4].second);
    }
    
    evaluate (__LINE__, __func__, 3, filter_text.verses_text_note_positions.size());
    evaluate (__LINE__, __func__, {}, filter_text.verses_text_note_positions [0]);
    evaluate (__LINE__, __func__, {17, 17}, filter_text.verses_text_note_positions [1]);
    evaluate (__LINE__, __func__, {17, 17}, filter_text.verses_text_note_positions [2]);
    evaluate (__LINE__, __func__, {19}, filter_text.verses_text_note_positions [3]);
    evaluate (__LINE__, __func__, {}, filter_text.verses_text_note_positions [4]);
  }
  
  
  return; // Todo
  // Test check on matching markers.
  {
    string usfm = R"(
\id GEN
\v 1 Njalo kwakwenziwe \w \+add ng\+add*amakherubhi\w* lezihlahla zelala, kuze kuthi isihlahla selala sasiphakathi kwekherubhi lekherubhi; njalo \add yilelo lalelo\add* ikherubhi lalilobuso obubili.
\v 2 Izembatho zakho zonke \add ziqholiwe\add* \w ngemure\w* lenhlaba \w \+add l\+add*ekhasiya\w*; ezigodlweni zempondo zendlovu abakwenze wathokoza ngazo.
\v 3 Ngasengiyithenga insimu kuHanameli indodana kamalumami, eseAnathothi, ngamlinganisela imali \w \+add eng\+add*amashekeli\w** ayisikhombisa lenhlamvu ezilitshumi zesiliva.
    )";
    Checks_Usfm check = Checks_Usfm ("phpunit");
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {};
    evaluate (__LINE__, __func__, standard, results);
    for (size_t i = 0; i < results.size (); i++) {
      cout << results[i].first << " " << results[i].second << endl;
    }
  }
  
}

