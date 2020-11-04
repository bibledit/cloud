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


#include <unittests/bibles.h>
#include <unittests/utilities.h>
#include <webserver/request.h>
#include <database/state.h>
#include <database/login.h>
#include <database/bibleactions.h>
#include <filter/usfm.h>
#include <filter/string.h>
#include <bb/logic.h>


void test_store_bible_data_safely_setup (Webserver_Request * request, string usfm)
{
  refresh_sandbox (true);
  Database_State::create ();
  Database_Login::create ();
  request->database_bibles()->createBible ("phpunit");
  request->database_bibles()->storeChapter ("phpunit", 1, 1, usfm);
}


void test_bibles ()
{
  trace_unit_tests (__func__);
  
  Webserver_Request request;
  test_store_bible_data_safely_setup (&request, "");
  request.database_users ()->create ();
  request.session_logic ()->setUsername ("phpunit");
  string usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 Verse 1.\n"
  "\\v 2 Verse 2.\n"
  "\\p\n"
  "\\v 3 Verse 3.\n"
  "\\v 4 Verse 4.\n"
  "\\v 5 Verse 5.";
  {
    // Safely store a chapter.
    test_store_bible_data_safely_setup (&request, usfm);
    string data =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse 1.\n"
    "\\v 2 Verse 2.\n"
    "\\v 3 Verse 3.\n"
    "\\v 4 Verse 4.\n"
    "\\v 5 Verse 5.";
    string explanation;
    string stored = usfm_safely_store_chapter (&request, "phpunit", 1, 1, data, explanation);
    evaluate (__LINE__, __func__, "", stored);
    evaluate (__LINE__, __func__, "", explanation);
    string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, data, result);
    refresh_sandbox (false);
  }
  {
    // Safely store chapter.
    test_store_bible_data_safely_setup (&request, usfm);
    string data =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse 1.\n"
    "\\v 2 Verse 2.\n"
    "\\p\n"
    "\\v 3 Verse 3.\n"
    "\\v 4 Verse 4.";
    string explanation;
    string stored = usfm_safely_store_chapter (&request, "phpunit", 1, 1, data, explanation);
    evaluate (__LINE__, __func__, "", stored);
    evaluate (__LINE__, __func__, "", explanation);
    string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, data, result);
    refresh_sandbox (false);
  }
  {
    // Safely store chapter with length error
    test_store_bible_data_safely_setup (&request, usfm);
    string data =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse 1.\n"
    "\\v 2 Verse 2.\n"
    "\\v 3 Verse 3.\n";
    string explanation;
    string stored = usfm_safely_store_chapter (&request, "phpunit", 1, 1, data, explanation);
    evaluate (__LINE__, __func__, "Text length differs too much", stored);
    evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The length differs 37% from the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
    string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, usfm, result);
    refresh_sandbox (false);
  }
  // SafeStoreChapterSimilarity
  {
    test_store_bible_data_safely_setup (&request, usfm);
    string data =
    "\\c 1\n"
    "\\p\n"
    "\\v 1 Verse 1.\n"
    "\\v 3 Verse 3.\n"
    "\\v 2 Verse 2.\n"
    "\\v 4 Verse 4.\n";
    string explanation;
    string stored = usfm_safely_store_chapter (&request, "phpunit", 1, 1, data, explanation);
    evaluate (__LINE__, __func__, "Text content differs too much", stored);
    evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The new text is 54% similar to the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
    string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, usfm, result);
    refresh_sandbox (false);
  }
  // SafeStoreChapterNoChange
  {
    test_store_bible_data_safely_setup (&request, usfm);
    string explanation;
    int currentId = request.database_bibles()->getChapterId ("phpunit", 1, 1);
    string stored = usfm_safely_store_chapter (&request, "phpunit", 1, 1, usfm, explanation);
    evaluate (__LINE__, __func__, "", stored);
    evaluate (__LINE__, __func__, "", explanation);
    string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, usfm, result);
    int currentId2 = request.database_bibles()->getChapterId ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, currentId, currentId2);
    refresh_sandbox (false);
  }
  // Safely store verse 0 without a change.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data =
      "\\c 1\n"
      "\\p\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 0, data, explanation, false);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data =
      "\\c 1\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 0, data, explanation, true);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store verse 0 with a change.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data =
      "\\c 1\n"
      "\\p xx\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 0, data, explanation, false);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("\\p\n\\v 1", "\\p xx\n\\v 1", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data =
      "\\c 1x\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 0, data, explanation, true);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("c 1", "c 1x", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store verse 2 with a change.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2 Verse two.\n\\p\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, false);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("Verse 2", "Verse two", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2 Verse two.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, true);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("Verse 2", "Verse two", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store verse 3 with a change.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 3 Verse three.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 3, data, explanation, false);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("Verse 3", "Verse three", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\p\n\\v 3 Verse three.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 3, data, explanation, true);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("Verse 3", "Verse three", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store the USFM for verse two to verse one: Fails.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2 Verse 2.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 1, data, explanation, false);
      evaluate (__LINE__, __func__, "Verse mismatch", stored);
      evaluate (__LINE__, __func__, "The USFM contains verse(s) 2 while it wants to save to verse 1", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2 Verse 2.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 1, data, explanation, true);
      evaluate (__LINE__, __func__, "Verse mismatch", stored);
      evaluate (__LINE__, __func__, "The USFM contains verse(s) 2 while it wants to save to verse 1", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store a verse with too much length difference: Fails.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2 Verse two two to to two.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, false);
      evaluate (__LINE__, __func__, "Text length differs too much", stored);
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The length differs 81% from the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2 Verse two two two.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, true);
      evaluate (__LINE__, __func__, "Text length differs too much", stored);
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The length differs 76% from the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store a verse with too much of content difference: Fails.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      request.database_config_user ()->setEditingAllowedDifferenceVerse (40);
      string data = "\\v 2 vERSE 2.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, false);
      evaluate (__LINE__, __func__, "Text content differs too much", stored);
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The new text is 38% similar to the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      request.database_config_user ()->setEditingAllowedDifferenceVerse (40);
      string data = "\\v 2 vERSE 2.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, true);
      evaluate (__LINE__, __func__, "Text content differs too much", stored);
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The new text is 44% similar to the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store USFM without any verse to verse 2: Fails.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      request.database_config_user ()->setEditingAllowedDifferenceVerse (40);
      string data = "\\p Verse 2.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, false);
      evaluate (__LINE__, __func__, "Missing verse number", stored);
      evaluate (__LINE__, __func__, "The USFM contains no verse information", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      request.database_config_user ()->setEditingAllowedDifferenceVerse (40);
      string data = "\\p Verse 2.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, true);
      evaluate (__LINE__, __func__, "Missing verse number", stored);
      evaluate (__LINE__, __func__, "The USFM contains no verse information", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store USFM with two verses: Fails.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      request.database_config_user ()->setEditingAllowedDifferenceVerse (40);
      string data = "\\v 2 Verse 2.\n\\v 3 3";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, false);
      evaluate (__LINE__, __func__, "Cannot overwrite another verse", stored);
      evaluate (__LINE__, __func__, "The USFM contains verse(s) 0 2 3 which would overwrite a fragment that contains verse(s) 0 2", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      request.database_config_user ()->setEditingAllowedDifferenceVerse (40);
      string data = "\\v 2 Verse 2.\n\\v 3 3";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, true);
      evaluate (__LINE__, __func__, "Cannot overwrite another verse", stored);
      evaluate (__LINE__, __func__, "The USFM contains verse(s) 0 2 3 which would overwrite a fragment that contains verse(s) 0 2", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
  }
  // The USFM is going to have combined verses.
  usfm =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 Verse 1.\n"
  "\\v 2-3 Verse 2 and 3.\n"
  "\\p\n"
  "\\v 4-5 Verse 4 and 5.\n"
  "\\v 6 Verse 6.";
  // Safely store combined verse without any change.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2-3 Verse 2 and 3.\n\\p\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, false);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2-3 Verse 2 and 3.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 2, data, explanation, true);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
    }
  }
  // Safely store combined verse before the \p with a change.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2-3 Verse 2 andx 3.\n\\p\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 3, data, explanation, false);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("2 and 3", "2 andx 3", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2-3 Verse 2 andx 3.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 3, data, explanation, true);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("2 and 3", "2 andx 3", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store combined verse after the \p with a change.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 4-5 Verse 4 andx 5.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 4, data, explanation, false);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("4 and 5", "4 andx 5", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\p\n\\v 4-5 Verse 4 andx 5.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 4, data, explanation, true);
      evaluate (__LINE__, __func__, "", stored);
      evaluate (__LINE__, __func__, "", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      string newusfm = filter_string_str_replace ("4 and 5", "4 andx 5", usfm);
      evaluate (__LINE__, __func__, newusfm, result);
      refresh_sandbox (false);
    }
  }
  // Safely store combined verse with a change and wrong verses: Fails.
  {
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2-4 Verse 2 andx 3.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 3, data, explanation, false);
      evaluate (__LINE__, __func__, "Cannot overwrite another verse", stored);
      evaluate (__LINE__, __func__, "The USFM contains verse(s) 0 2 3 4 which would overwrite a fragment that contains verse(s) 0 2 3", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
    {
      test_store_bible_data_safely_setup (&request, usfm);
      string data = "\\v 2-4 Verse 2 andx 3.\n";
      string explanation;
      string stored = usfm_safely_store_verse (&request, "phpunit", 1, 1, 3, data, explanation, true);
      evaluate (__LINE__, __func__, "Cannot overwrite another verse", stored);
      evaluate (__LINE__, __func__, "The USFM contains verse(s) 0 2 3 4 which would overwrite a fragment that contains verse(s) 0 2 3", explanation);
      string result = request.database_bibles()->getChapter ("phpunit", 1, 1);
      evaluate (__LINE__, __func__, usfm, result);
      refresh_sandbox (false);
    }
  }
  
  // Condense a simple editor update to format a paragraph.
  {
    vector <int> positions_in =  { 6,     6 };
    vector <int> sizes_in     =  { 1,     1 };
    vector <bool> additions_in = { false, true };
    vector <string> content_in = { "\np", "\ns" };
    vector <int> positions_out;
    vector <int> sizes_out;
    vector <string> operators_out;
    vector <string> content_out;
    bible_logic_condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                         positions_out, sizes_out, operators_out, content_out);
    evaluate (__LINE__, __func__, {6  }, positions_out);
    evaluate (__LINE__, __func__, {1  }, sizes_out);
    evaluate (__LINE__, __func__, {"p"}, operators_out);
    evaluate (__LINE__, __func__, {"s"}, content_out);
  }

  // No condensing of any editor updates.
  {
    vector <int> positions_in =  { 6,     6 };
    vector <int> sizes_in     =  { 1,     1 };
    vector <bool> additions_in = { false, true };
    vector <string> content_in = { "ladd", "kadd" };
    vector <int> positions_out;
    vector <int> sizes_out;
    vector <string> operators_out;
    vector <string> content_out;
    bible_logic_condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                         positions_out, sizes_out, operators_out, content_out);
    evaluate (__LINE__, __func__, {6,      6    }, positions_out);
    evaluate (__LINE__, __func__, {1,      1    }, sizes_out);
    evaluate (__LINE__, __func__, {"d",    "i"  }, operators_out);
    evaluate (__LINE__, __func__, {"ladd", "kadd"}, content_out);
  }

  // No condensing of any editor updates.
  // Handle smileys as examples of characters that are 4 bytes long in UTF-16.
  {
    vector <int> positions_in =  { 6,     6 };
    vector <int> sizes_in     =  { 2,     2 };
    vector <bool> additions_in = { false, true };
    vector <string> content_in = { "😀add", "😁add" };
    vector <int> positions_out;
    vector <int> sizes_out;
    vector <string> operators_out;
    vector <string> content_out;
    bible_logic_condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                         positions_out, sizes_out, operators_out, content_out);
    evaluate (__LINE__, __func__, {6,      6       }, positions_out);
    evaluate (__LINE__, __func__, {2,      2       }, sizes_out);
    evaluate (__LINE__, __func__, {"d",    "i"     }, operators_out);
    evaluate (__LINE__, __func__, {"😀add", "😁add"}, content_out);
  }

  // Condense new line and leave other edits as they are.
  {
    vector <int> positions_in =  { 6,     6,     7,      8 };
    vector <int> sizes_in     =  { 1,     1,     1,      1 };
    vector <bool> additions_in = { false, true,  false,  true };
    vector <string> content_in = { "\np", "\ns", "ladd", "kadd" };
    vector <int> positions_out;
    vector <int> sizes_out;
    vector <string> operators_out;
    vector <string> content_out;
    bible_logic_condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                         positions_out, sizes_out, operators_out, content_out);
    evaluate (__LINE__, __func__, {6,   7,      8     }, positions_out);
    evaluate (__LINE__, __func__, {1,   1,      1     }, sizes_out);
    evaluate (__LINE__, __func__, {"p", "d",    "i"   }, operators_out);
    evaluate (__LINE__, __func__, {"s", "ladd", "kadd"}, content_out);
  }
  
  // Test entire pipeline for generating editor updates.
  {
    // The server text has inserted an exclamation mark after "nations!".
    string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!!</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q2"><span>And so on the third line.</span></p>)";
    vector <int> positions;
    vector <int> sizes;
    vector <string> operators;
    vector <string> content;
    bible_logic_html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    evaluate (__LINE__, __func__, { 34  }, positions);
    evaluate (__LINE__, __func__, { 1   }, sizes);
    evaluate (__LINE__, __func__, { "i" }, operators);
    evaluate (__LINE__, __func__, { "!" }, content);
  }

  // Test entire pipeline for generating editor updates.
  {
    // The server text has removed two exclamation marks from "nations!!" and inserted a full stop.
    string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations.</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q2"><span>And so on the third line.</span></p>)";
    vector <int> positions;
    vector <int> sizes;
    vector <string> operators;
    vector <string> content;
    bible_logic_html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    evaluate (__LINE__, __func__, { 33,  33,  33  }, positions);
    evaluate (__LINE__, __func__, { 1,   1,   1   }, sizes);
    evaluate (__LINE__, __func__, { "d", "d", "i" }, operators);
    evaluate (__LINE__, __func__, { "!", "!", "." }, content);
  }

  // Test entire pipeline for generating editor updates.
  {
    // The server text has joined the second paragraph to the first.
    string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!</span> <span>Extol him, all you peoples!</span></p><p class="b-q2"><span>And so on the third line.</span></p>)";
    vector <int> positions;
    vector <int> sizes;
    vector <string> operators;
    vector <string> content;
    bible_logic_html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    evaluate (__LINE__, __func__, { 34,    34,  0,    62   }, positions);
    evaluate (__LINE__, __func__, { 1,     1,   1,    1    }, sizes);
    evaluate (__LINE__, __func__, { "d",   "i", "p",  "p"  }, operators);
    evaluate (__LINE__, __func__, { "\np", " ", "q1", "q2" }, content);
  }

  // Test entire pipeline for generating editor updates.
  {
    // The server text has added an extra paragraph with a certain paragraph style.
    string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q3">P.</p><p class="b-q2"><span>And so on the third line.</span></p>)";
    vector <int> positions;
    vector <int> sizes;
    vector <string> operators;
    vector <string> content;
    bible_logic_html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    evaluate (__LINE__, __func__, { 62,     63,  64,  0,    34,  62,   65   }, positions);
    evaluate (__LINE__, __func__, { 1,      1,   1,   1,    1,   1,    1    }, sizes);
    evaluate (__LINE__, __func__, { "i",    "i", "i", "p",  "p", "p",  "p"  }, operators);
    evaluate (__LINE__, __func__, { "\nq3", "P", ".", "q1", "p", "q3", "q2" }, content);
  }

  // Test entire pipeline for generating editor updates using 4-bytes UTF-16 characters.
  {
    // The server text has added an extra paragraph with a certain paragraph style.
    string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples😀!</p><p class="b-q2">And so on the third line.</p>)";
    string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations😀!</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q3">P😀.</p><p class="b-q2"><span>And so on the third line😀.</span></p>)";
    vector <int> positions;
    vector <int> sizes;
    vector <string> operators;
    vector <string> content;
    bible_logic_html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    //evaluate (__LINE__, __func__, { 62,     63,  64,  0,    34,  62,   65   }, positions);
    //evaluate (__LINE__, __func__, { 1,      1,   1,   1,    1,   1,    1    }, sizes);
    //evaluate (__LINE__, __func__, { "i",    "i", "i", "p",  "p", "p",  "p"  }, operators);
    evaluate (__LINE__, __func__, { "😀", "!", "\nq3", "P", ".", "!", "😀", "q1", "p", "q3", "q2" }, content);
  }

}


void test_database_bibleactions ()
{
  trace_unit_tests (__func__);
  
  refresh_sandbox (true);
  Database_BibleActions database_bibleactions;
  database_bibleactions.create ();
  
  database_bibleactions.optimize ();
  
  vector <string> bibles = database_bibleactions.getBibles ();
  evaluate (__LINE__, __func__, { }, bibles);
  
  database_bibleactions.record ("phpunit1", 1, 2, "data112");
  database_bibleactions.record ("phpunit1", 1, 3, "data113");
  database_bibleactions.record ("phpunit1", 2, 4, "data124");
  database_bibleactions.record ("phpunit2", 5, 6, "data256");
  database_bibleactions.record ("phpunit2", 5, 6, "data256: Not to be stored");
  
  bibles = database_bibleactions.getBibles ();
  evaluate (__LINE__, __func__, {"phpunit1", "phpunit2"}, bibles);
  
  vector <int> books = database_bibleactions.getBooks ("phpunit1");
  evaluate (__LINE__, __func__, {1, 2}, books);
  
  vector <int> chapters = database_bibleactions.getChapters ("phpunit1", 1);
  evaluate (__LINE__, __func__, {2, 3}, chapters);
  
  chapters = database_bibleactions.getChapters ("phpunit1", 2);
  evaluate (__LINE__, __func__, {4}, chapters);
  
  database_bibleactions.erase ("phpunit1", 2, 3);
  
  chapters = database_bibleactions.getChapters ("phpunit1", 2);
  evaluate (__LINE__, __func__, {4}, chapters);
  
  database_bibleactions.erase ("phpunit1", 2, 4);
  
  chapters = database_bibleactions.getChapters ("phpunit1", 2);
  evaluate (__LINE__, __func__, { }, chapters);
  
  string usfm = database_bibleactions.getUsfm ("phpunit2", 5, 5);
  evaluate (__LINE__, __func__, "", usfm);
  
  usfm = database_bibleactions.getUsfm ("phpunit2", 5, 6);
  evaluate (__LINE__, __func__, "data256", usfm);
}


void test_database_bibles ()
{
  trace_unit_tests (__func__);
  
  string testbible = "testbible";
  string testbible2 = testbible + "2";

  // It should get the standard default Bible when there's no Bibles created by the user yet.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    vector <string> standard;
    vector <string> bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, standard, bibles);
  }

  // Test whether optimizing works without errors.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    database_bibles.createBible (testbible);
    database_bibles.storeChapter (testbible, 2, 3, "a");
    database_bibles.storeChapter (testbible, 2, 3, "b");
    database_bibles.storeChapter (testbible, 2, 3, "c");
    database_bibles.storeChapter (testbible, 2, 3, "d");
    database_bibles.storeChapter (testbible, 2, 3, "e");
    database_bibles.storeChapter (testbible, 2, 3, "f");
    database_bibles.storeChapter (testbible, 2, 3, "g");
    database_bibles.optimize ();
    string usfm = database_bibles.getChapter (testbible, 2, 3);
    evaluate (__LINE__, __func__, "g", usfm);
  }

  // Test whether optimizing removes files with 0 size.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    database_bibles.createBible (testbible);
    database_bibles.storeChapter (testbible, 2, 3, "a");
    database_bibles.storeChapter (testbible, 2, 3, "b");
    database_bibles.storeChapter (testbible, 2, 3, "c");
    database_bibles.storeChapter (testbible, 2, 3, "d");
    database_bibles.storeChapter (testbible, 2, 3, "e");
    database_bibles.storeChapter (testbible, 2, 3, "f");
    database_bibles.storeChapter (testbible, 2, 3, "");
    string usfm = database_bibles.getChapter (testbible, 2, 3);
    evaluate (__LINE__, __func__, "", usfm);
    database_bibles.optimize ();
    usfm = database_bibles.getChapter (testbible, 2, 3);
    evaluate (__LINE__, __func__, "f", usfm);
  }

  // Test create / get / delete Bibles.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible (testbible);
    
    vector <string> bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, {testbible}, bibles);
    
    database_bibles.deleteBible (testbible);
    
    bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, {}, bibles);
  }

  // Test storing a chapter, and getting it again.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible (testbible);
    string usfm = "\\c 1\n\\p\n\\v 1 Verse 1";
    database_bibles.storeChapter (testbible, 2, 1, usfm);
    string result = database_bibles.getChapter (testbible, 2, 1);
    evaluate (__LINE__, __func__, usfm, result);
    result = database_bibles.getChapter (testbible2, 2, 1);
    evaluate (__LINE__, __func__, "", result);
    result = database_bibles.getChapter (testbible, 1, 1);
    evaluate (__LINE__, __func__, "", result);
  }

  // Test books.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible (testbible);
    vector <int> books = database_bibles.getBooks (testbible);
    evaluate (__LINE__, __func__, { }, books);
    
    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    books = database_bibles.getBooks (testbible);
    evaluate (__LINE__, __func__, { 1 }, books);
    
    database_bibles.storeChapter (testbible, 2, 3, "\\c 0");
    books = database_bibles.getBooks (testbible);
    evaluate (__LINE__, __func__, { 1, 2 }, books);
    
    database_bibles.deleteBook (testbible, 3);
    books = database_bibles.getBooks (testbible);
    evaluate (__LINE__, __func__, { 1, 2 }, books);
    
    database_bibles.deleteBook (testbible, 1);
    books = database_bibles.getBooks (testbible);
    evaluate (__LINE__, __func__, { 2 }, books);
    
    database_bibles.deleteBook (testbible2, 2);
    books = database_bibles.getBooks (testbible);
    evaluate (__LINE__, __func__, { 2 }, books);
  }

  // Test chapters.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible (testbible);
    vector <int> chapters = database_bibles.getChapters (testbible, 1);
    evaluate (__LINE__, __func__, { }, chapters);
    
    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    chapters = database_bibles.getChapters (testbible, 1);
    evaluate (__LINE__, __func__, { 2 }, chapters);
    
    chapters = database_bibles.getChapters (testbible, 2);
    evaluate (__LINE__, __func__, { }, chapters);
    
    database_bibles.storeChapter (testbible, 1, 3, "\\c 1");
    chapters = database_bibles.getChapters (testbible, 1);
    evaluate (__LINE__, __func__, { 2, 3 }, chapters);
    
    database_bibles.deleteChapter (testbible, 3, 3);
    chapters = database_bibles.getChapters (testbible, 1);
    evaluate (__LINE__, __func__, { 2, 3 }, chapters);
    
    database_bibles.deleteChapter (testbible, 1, 2);
    chapters = database_bibles.getChapters (testbible, 1);
    evaluate (__LINE__, __func__, { 3 }, chapters);
    
    database_bibles.deleteChapter (testbible, 1, 3);
    chapters = database_bibles.getChapters (testbible, 1);
    evaluate (__LINE__, __func__, { }, chapters);
  }

  // Test chapter identifiers.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible (testbible);
    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    int id = database_bibles.getChapterId (testbible, 1, 2);
    evaluate (__LINE__, __func__, 100000001, id);
    
    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    id = database_bibles.getChapterId (testbible, 1, 2);
    evaluate (__LINE__, __func__, 100000002, id);
    
    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    id = database_bibles.getChapterId (testbible, 1, 2);
    evaluate (__LINE__, __func__, 100000004, id);
    
    database_bibles.storeChapter (testbible, 2, 3, "\\c 1");
    id = database_bibles.getChapterId (testbible, 1, 2);
    evaluate (__LINE__, __func__, 100000004, id);
  }
  
  // Test the age of the newest chapter.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible (testbible);
    int age = database_bibles.getChapterAge (testbible, 1, 2);
    evaluate (__LINE__, __func__, 100000000, age);

    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    age = database_bibles.getChapterAge (testbible, 1, 2);
    evaluate (__LINE__, __func__, 0, age);

    database_bibles.storeChapter (testbible, 1, 2, "\\c 1");
    this_thread::sleep_for(chrono::milliseconds(1000));
    age = database_bibles.getChapterAge (testbible, 1, 2);
    evaluate (__LINE__, __func__, 1, age);
  }
  
}


