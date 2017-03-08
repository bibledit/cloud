/*
Copyright (©) 2003-2017 Teus Benschop.

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
#include <filter/abbreviations.h>


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
    evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The length differs 37% from the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Personalize.", explanation);
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
    evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The new text is 54% similar to the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Personalize.", explanation);
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
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The length differs 81% from the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Personalize.", explanation);
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
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The length differs 76% from the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Personalize.", explanation);
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
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The new text is 38% similar to the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Personalize.", explanation);
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
      evaluate (__LINE__, __func__, "The text was not saved for safety reasons. The new text is 44% similar to the existing text. Make smaller changes and save more often. Or relax the restriction in the editing settings. See menu Settings - Personalize.", explanation);
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
  
  // Reading Bible book abbreviations.
  {
    string input =
    "Psalms = Ps.\n"
    "\n"
    "Exodus = Exod.\n"
    "\n"
    "Exodu = Exod.\n"
    "\n"
    "Psalms = Psa.\n"
    "\n"
    "Joshua =\n"
    "\n";
    vector <pair <int, string> > output = filter_abbreviations_read (input);
    vector <pair <int, string> > standard = { make_pair (19, "Ps."), make_pair (2, "Exod."), make_pair (19, "Psa.") };
    evaluate (__LINE__, __func__, standard, output);
  }
  
  // Displaying Bible book abbreviations.
  {
    string input =
    "Psalms = Ps.\n"
    "\n"
    "Exodus = Exod.\n"
    "\n";
    string output = filter_abbreviations_display (input);
    string standard =
    "Genesis = \n"
    "Exodus = Exod.\n"
    "Leviticus = \n"
    "Numbers = \n"
    "Deuteronomy = \n"
    "Joshua = \n"
    "Judges = \n"
    "Ruth = \n"
    "1 Samuel = \n"
    "2 Samuel = \n"
    "1 Kings = \n"
    "2 Kings = \n"
    "1 Chronicles = \n"
    "2 Chronicles = \n"
    "Ezra = \n"
    "Nehemiah = \n"
    "Esther = \n"
    "Job = \n"
    "Psalms = Ps.\n"
    "Proverbs = \n"
    "Ecclesiastes = \n"
    "Song of Solomon = \n"
    "Isaiah = \n"
    "Jeremiah = \n"
    "Lamentations = \n"
    "Ezekiel = \n"
    "Daniel = \n"
    "Hosea = \n"
    "Joel = \n"
    "Amos = \n"
    "Obadiah = \n"
    "Jonah = \n"
    "Micah = \n"
    "Nahum = \n"
    "Habakkuk = \n"
    "Zephaniah = \n"
    "Haggai = \n"
    "Zechariah = \n"
    "Malachi = \n"
    "Matthew = \n"
    "Mark = \n"
    "Luke = \n"
    "John = \n"
    "Acts = \n"
    "Romans = \n"
    "1 Corinthians = \n"
    "2 Corinthians = \n"
    "Galatians = \n"
    "Ephesians = \n"
    "Philippians = \n"
    "Colossians = \n"
    "1 Thessalonians = \n"
    "2 Thessalonians = \n"
    "1 Timothy = \n"
    "2 Timothy = \n"
    "Titus = \n"
    "Philemon = \n"
    "Hebrews = \n"
    "James = \n"
    "1 Peter = \n"
    "2 Peter = \n"
    "1 John = \n"
    "2 John = \n"
    "3 John = \n"
    "Jude = \n"
    "Revelation = \n"
    "Front Matter = \n"
    "Back Matter = \n"
    "Other Material = ";
    evaluate (__LINE__, __func__, standard, output);
  }

  // Sorting Bible book abbreviations.
  {
    vector <pair <int, string> > input =
      { make_pair (1, "One"), make_pair (2, "Two"), make_pair (2, "Two."), make_pair (3, "3") };
    vector <pair <int, string> > output = filter_abbreviations_sort (input);
    vector <pair <int, string> > standard =
      { make_pair (2, "Two."), make_pair (1, "One"), make_pair (2, "Two"), make_pair (3, "3") };
    evaluate (__LINE__, __func__, standard, output);
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
  
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    vector <string> standard;
    vector <string> bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, standard, bibles);
  }
  {
    // Test whether optimizing works without errors.
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    database_bibles.createBible ("phpunit");
    database_bibles.storeChapter ("phpunit", 2, 3, "a");
    database_bibles.storeChapter ("phpunit", 2, 3, "b");
    database_bibles.storeChapter ("phpunit", 2, 3, "c");
    database_bibles.storeChapter ("phpunit", 2, 3, "d");
    database_bibles.storeChapter ("phpunit", 2, 3, "e");
    database_bibles.storeChapter ("phpunit", 2, 3, "f");
    database_bibles.storeChapter ("phpunit", 2, 3, "g");
    database_bibles.optimize ();
    string usfm = database_bibles.getChapter ("phpunit", 2, 3);
    evaluate (__LINE__, __func__, "g", usfm);
  }
  {
    // Test whether optimizing removes files with 0 size.
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    database_bibles.createBible ("phpunit");
    database_bibles.storeChapter ("phpunit", 2, 3, "a");
    database_bibles.storeChapter ("phpunit", 2, 3, "b");
    database_bibles.storeChapter ("phpunit", 2, 3, "c");
    database_bibles.storeChapter ("phpunit", 2, 3, "d");
    database_bibles.storeChapter ("phpunit", 2, 3, "e");
    database_bibles.storeChapter ("phpunit", 2, 3, "f");
    database_bibles.storeChapter ("phpunit", 2, 3, "");
    string usfm = database_bibles.getChapter ("phpunit", 2, 3);
    evaluate (__LINE__, __func__, "", usfm);
    database_bibles.optimize ();
    usfm = database_bibles.getChapter ("phpunit", 2, 3);
    evaluate (__LINE__, __func__, "f", usfm);
  }
  // Test create / get / delete Bibles.
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    
    vector <string> bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, {"phpunit"}, bibles);
    
    database_bibles.deleteBible ("phpunit");
    
    bibles = database_bibles.getBibles ();
    evaluate (__LINE__, __func__, {}, bibles);
  }
  // Test storeChapter / getChapter
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    string usfm = "\\c 1\n\\p\n\\v 1 Verse 1";
    database_bibles.storeChapter ("phpunit", 2, 1, usfm);
    string result = database_bibles.getChapter ("phpunit", 2, 1);
    evaluate (__LINE__, __func__, usfm, result);
    result = database_bibles.getChapter ("phpunit2", 2, 1);
    evaluate (__LINE__, __func__, "", result);
    result = database_bibles.getChapter ("phpunit", 1, 1);
    evaluate (__LINE__, __func__, "", result);
  }
  // Test books
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    vector <int> books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { }, books);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 1 }, books);
    
    database_bibles.storeChapter ("phpunit", 2, 3, "\\c 0");
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 1, 2 }, books);
    
    database_bibles.deleteBook ("phpunit", 3);
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 1, 2 }, books);
    
    database_bibles.deleteBook ("phpunit", 1);
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 2 }, books);
    
    database_bibles.deleteBook ("phpunit2", 2);
    books = database_bibles.getBooks ("phpunit");
    evaluate (__LINE__, __func__, { 2 }, books);
  }
  // Test chapters ()
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    vector <int> chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { }, chapters);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 2 }, chapters);
    
    chapters = database_bibles.getChapters ("phpunit", 2);
    evaluate (__LINE__, __func__, { }, chapters);
    
    database_bibles.storeChapter ("phpunit", 1, 3, "\\c 1");
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 2, 3 }, chapters);
    
    database_bibles.deleteChapter ("phpunit", 3, 3);
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 2, 3 }, chapters);
    
    database_bibles.deleteChapter ("phpunit", 1, 2);
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { 3 }, chapters);
    
    database_bibles.deleteChapter ("phpunit", 1, 3);
    chapters = database_bibles.getChapters ("phpunit", 1);
    evaluate (__LINE__, __func__, { }, chapters);
  }
  // Test chapter IDs
  {
    refresh_sandbox (true);
    Database_Bibles database_bibles;
    Database_State::create ();
    
    database_bibles.createBible ("phpunit");
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    int id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000001, id);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000002, id);
    
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    database_bibles.storeChapter ("phpunit", 1, 2, "\\c 1");
    id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000004, id);
    
    database_bibles.storeChapter ("phpunit", 2, 3, "\\c 1");
    id = database_bibles.getChapterId ("phpunit", 1, 2);
    evaluate (__LINE__, __func__, 100000004, id);
  }
}


