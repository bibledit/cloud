/*
 Copyright (©) 2003-2021 Teus Benschop.
 
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


using namespace pugi;


void test_dev ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);

  // Test the verse markup finder.
  {
    bool at_passage = false;
    string paragraph;

    at_passage = false;
    paragraph = "Verse 13 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 13, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verse 13: Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 13, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verse 14";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 14, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 15, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verses 15-17 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 16, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 18, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verse 8-11 Text";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 9, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 12, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

    at_passage = false;
    paragraph = "Verses 3 – 4: DAVID LOOKS TO THE LORD";
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 3, at_passage);
    evaluate (__LINE__, __func__, true, at_passage);
    at_passage = false;
    resource_logic_easy_english_bible_handle_verse_marker (paragraph, 5, at_passage);
    evaluate (__LINE__, __func__, false, at_passage);

  }

  {
    int book = 19; // Psalms
    string text = resource_logic_easy_english_bible_get (book, 3, 6);
    text = filter_string_html2text (text);
    cout << text << endl; // Todo
    size_t pos = text.find ("There the writer says that God uses those that are nothing.");
    evaluate (__LINE__, __func__, 1693, pos);
  }
  exit (0);

  refresh_sandbox (true);
}
