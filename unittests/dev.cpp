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


#include <codecvt>
#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/text.h>
#include <filter/image.h>
#include <editor/html2usfm.h>
#include <editor/html2format.h>
#include <styles/logic.h>
#include <database/state.h>
#include <database/login.h>
#include <database/users.h>
#include <database/bibleimages.h>
#include <webserver/request.h>
#include <user/logic.h>
#include <pugixml/pugixml.hpp>
#include <html/text.h>
#include <checks/usfm.h>
#include <resource/logic.h>
#include <type_traits>


using namespace pugi;


void test_dev () // Todo move into place.
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  
  string bible { "bible" };
  
  // Test export word-level attributes.
  {
    string usfm = R"(
    \id GEN
    \c 1
    \p
    \v 1 This is verse one.
    \v 2 And the \nd \+w Lord|strong="H3068"\+w*\nd* \w said|strong="H0559"\w* unto \w Cain|strong="H7014"\w*:
    \v 3 This is verse three.
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.initializeHeadingsAndTextPerVerse (false);
    filter_text.add_usfm_code (usfm);
    filter_text.run (styles_logic_standard_sheet ());
    
    map <int, string> verses_text = filter_text.getVersesText ();
    for (auto verse_text : verses_text) {
      cout << verse_text.first << " " << verse_text.second << endl;
    }
    
//    map <int, string> standard = {
//      {1, "This is verse one."},
//      {2, "This is verse two."},
//      {3, "This is verse three."},
//    };
//    evaluate (__LINE__, __func__, standard, output);
    
//    int n = 5;
//    size_t size = filter_text.notes_plain_text.size();
//    evaluate (__LINE__, __func__, n, size);
//    if ((int)size == n) {
//      evaluate (__LINE__, __func__, "1", filter_text.notes_plain_text[0].first);
//      evaluate (__LINE__, __func__, "1", filter_text.notes_plain_text[1].first);
//      evaluate (__LINE__, __func__, "2", filter_text.notes_plain_text[2].first);
//      evaluate (__LINE__, __func__, "2", filter_text.notes_plain_text[3].first);
//      evaluate (__LINE__, __func__, "3", filter_text.notes_plain_text[4].first);
//      evaluate (__LINE__, __func__, "Xref 1", filter_text.notes_plain_text[0].second);
//      evaluate (__LINE__, __func__, "Note 1", filter_text.notes_plain_text[1].second);
//      evaluate (__LINE__, __func__, "Note 2", filter_text.notes_plain_text[2].second);
//      evaluate (__LINE__, __func__, "Xref 2", filter_text.notes_plain_text[3].second);
//      evaluate (__LINE__, __func__, "Endnote 3", filter_text.notes_plain_text[4].second);
//    }
    
//    evaluate (__LINE__, __func__, 3, filter_text.verses_text_note_positions.size());
//    evaluate (__LINE__, __func__, {}, filter_text.verses_text_note_positions [0]);
//    evaluate (__LINE__, __func__, {17, 17}, filter_text.verses_text_note_positions [1]);
//    evaluate (__LINE__, __func__, {17, 17}, filter_text.verses_text_note_positions [2]);
//    evaluate (__LINE__, __func__, {19}, filter_text.verses_text_note_positions [3]);
//    evaluate (__LINE__, __func__, {}, filter_text.verses_text_note_positions [4]);
  }

  
  
  
  refresh_sandbox (true);
}
