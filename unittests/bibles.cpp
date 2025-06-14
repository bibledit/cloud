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
#include <webserver/request.h>
#include <database/state.h>
#include <database/login.h>
#include <database/bibleactions.h>
#include <filter/usfm.h>
#include <filter/string.h>
#include <bb/logic.h>


constexpr const auto bible {"bible"};


static void test_store_bible_data_safely_setup (const std::string& usfm)
{
  refresh_sandbox (false);
  database::bibles::create_bible (bible);
  database::bibles::store_chapter (bible, 1, 1, usfm);
}


// USFM with separate verses.
const std::string usfm_separate =
R"(\c 1)" "\n"
R"(\p)" "\n"
R"(\v 1 Verse 1.)" "\n"
R"(\v 2 Verse two two two two two two.)" "\n"
R"(\p)" "\n"
R"(\v 3 Verse 3.)" "\n"
R"(\v 4 Verse 4.)" "\n"
R"(\v 5 Verse 5.)"
;


// USFM with combined verses.
const std::string usfm_combined =
R"(\c 1)" "\n"
R"(\p)" "\n"
R"(\v 1 Verse 1.)" "\n"
R"(\v 2-3 Verse 2 and 3.)" "\n"
R"(\p)" "\n"
R"(\v 4-5 Verse 4 and 5.)" "\n"
R"(\v 6 Verse 6.)";


// Safely store a chapter.
TEST (bibles, safely_store_a_chapter_1)
{
  Webserver_Request webserver_request;
  test_store_bible_data_safely_setup (usfm_separate);
  const std::string data =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 Verse 1.\n"
  "\\v 2 Verse two two two two.\n"
  "\\v 3 Verse 3.\n"
  "\\v 4 Verse 4.\n"
  "\\v 5 Verse 5.";
  std::string explanation;
  std::string stored = filter::usfm::safely_store_chapter (webserver_request, bible, 1, 1, data, explanation);
  EXPECT_EQ (std::string(), stored);
  EXPECT_EQ (std::string(), explanation);
  const std::string result = database::bibles::get_chapter (bible, 1, 1);
  EXPECT_EQ (data, result);
}


// Safely store a chapter.
TEST (bibles, safely_store_a_chapter_2)
{
  Webserver_Request webserver_request;
  test_store_bible_data_safely_setup (usfm_separate);
  const std::string data =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 Verse 1.\n"
  "\\v 2 Verse two two two two two.\n"
  "\\p\n"
  "\\v 3 Verse 3.\n"
  "\\v 4 Verse 4.";
  std::string explanation;
  std::string stored = filter::usfm::safely_store_chapter (webserver_request, bible, 1, 1, data, explanation);
  EXPECT_EQ (std::string(), stored);
  EXPECT_EQ (std::string(), explanation);
  const std::string result = database::bibles::get_chapter (bible, 1, 1);
  EXPECT_EQ (data, result);
}


// Safely store chapter with length error
TEST (bibles, safely_store_chapter_with_length_error)
{
  Webserver_Request webserver_request;
  test_store_bible_data_safely_setup (usfm_separate);
  const std::string data =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 Verse 1.\n"
  "\\v 2 Verse 2.\n"
  "\\v 3 Verse 3.\n";
  std::string explanation;
  const std::string stored = filter::usfm::safely_store_chapter (webserver_request, bible, 1, 1, data, explanation);
  EXPECT_EQ ("Text length differs too much", stored);
  EXPECT_EQ ("The text was not saved for safety reasons. The length differs 50% from the existing text. Make fewer changes at a time and wait till the editor has saved the text. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
  const std::string result = database::bibles::get_chapter (bible, 1, 1);
  EXPECT_EQ (usfm_separate, result);
}


// Safely store chapter with text similarity error.
TEST (bibles, safely_store_chapter_with_text_similarity_error)
{
  Webserver_Request webserver_request;
  test_store_bible_data_safely_setup (usfm_separate);
  const std::string data =
  "\\c 1\n"
  "\\p\n"
  "\\v 1 Verse 1.\n"
  "\\v 3 Verse 3.\n"
  "\\v 2 Verse two two two two two two two.\n"
  "\\v 4 Verse 4.\n";
  std::string explanation;
  const std::string stored = filter::usfm::safely_store_chapter (webserver_request, bible, 1, 1, data, explanation);
  EXPECT_EQ ("Text content differs too much", stored);
  EXPECT_EQ ("The text was not saved for safety reasons. The new text is 59% similar to the existing text. Make fewer changes at a time and wait till the editor has saved the text. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
  const std::string result = database::bibles::get_chapter (bible, 1, 1);
  EXPECT_EQ (usfm_separate, result);
}


// Safely store a chapter with no change in the text.
TEST (bibles, safely_store_chapter_without_text_change)
{
  Webserver_Request webserver_request;
  test_store_bible_data_safely_setup (usfm_separate);
  std::string explanation;
  const int currentId = database::bibles::get_chapter_id (bible, 1, 1);
  const std::string stored = filter::usfm::safely_store_chapter (webserver_request, bible, 1, 1, usfm_separate, explanation);
  EXPECT_EQ (std::string(), stored);
  EXPECT_EQ (std::string(), explanation);
  const std::string result = database::bibles::get_chapter (bible, 1, 1);
  EXPECT_EQ (usfm_separate, result);
  const int currentId2 = database::bibles::get_chapter_id (bible, 1, 1);
  EXPECT_EQ (currentId, currentId2);
}


// Safely store verse 0 without a change.
TEST (bibles, safely_store_verse_0_without_change)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data =
    "\\c 1\n"
    "\\p\n";
    std::string explanation;
    constexpr const auto quill {false};
    const std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 0, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data =
    "\\c 1\n";
    std::string explanation;
    constexpr const auto quill {true};
    const std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 0, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
}


// Safely store verse 0 with a change.
TEST (bibles, safely_store_verse_0_with_a_change)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data =
    "\\c 1\n"
    "\\p xx\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 0, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("\\p\n\\v 1", "\\p xx\n\\v 1", usfm_separate);
    EXPECT_EQ (newusfm, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data =
    "\\c 1x\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 0, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("c 1", "c 1x", usfm_separate);
    EXPECT_EQ (newusfm, result);
  }
}


// Safely store verse 2 with a change.
TEST (bibles, safely_store_verse_2_with_a_change)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 Verse two two two two.\n\\p\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("two two two two two two", "two two two two", usfm_separate);
    EXPECT_EQ (newusfm, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 Verse two.\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("two two two two two two", "two", usfm_separate);
    EXPECT_EQ (newusfm, result);
  }
}


// Safely store verse 3 with a change.
TEST (bibles, safely_store_verse_3_with_a_change)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 3 Verse three.\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 3, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("Verse 3", "Verse three", usfm_separate);
    EXPECT_EQ (newusfm, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\p\n\\v 3 Verse three.\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 3, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("Verse 3", "Verse three", usfm_separate);
    EXPECT_EQ (newusfm, result);
  }
}


// Safely store the USFM for verse two to verse one: Fails.
TEST (bibles, safely_store_verse_2_to_verse_1_fails)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 Verse 2.\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 1, data, explanation, quill);
    EXPECT_EQ ("Verse mismatch", stored);
    EXPECT_EQ ("The USFM contains verse(s) 2 while it wants to save to verse 1", explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 Verse 2.\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 1, data, explanation, quill);
    EXPECT_EQ ("Verse mismatch", stored);
    EXPECT_EQ ("The USFM contains verse(s) 2 while it wants to save to verse 1", explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
}


// Safely store a verse with too much length difference.
// If the new text is longer, it always accepts that.
// If the new test is shorter, it should fail.
TEST (bibles, safely_store_verse_with_too_large_length_difference)
{
  Webserver_Request webserver_request;
  bool quill;
  {
    quill = false;
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 Verse two two two two two two two two two.\n\\p\n";
    std::string explanation;
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ (true, stored.empty());
    EXPECT_EQ (true, explanation.empty());
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string standard = filter::strings::replace("two two two two two two", "two two two two two two two two two", usfm_separate);
    EXPECT_EQ (standard, result);
  }
  {
    quill = true;
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 Verse two two two two two two two two two.\n";
    std::string explanation;
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ (true, stored.empty());
    EXPECT_EQ (true, explanation.empty());
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string standard = filter::strings::replace("two two two two two two", "two two two two two two two two two", usfm_separate);
    EXPECT_EQ (standard, result);
  }
  {
    quill = false;
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 two";
    std::string explanation;
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ ("Text length differs too much", stored);
    EXPECT_EQ ("The text was not saved for safety reasons. The length differs 78% from the existing text. Make fewer changes at a time and wait till the editor has saved the text. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
  {
    quill = true;
    test_store_bible_data_safely_setup (usfm_separate);
    const std::string data = "\\v 2 two";
    std::string explanation;
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ ("Text length differs too much", stored);
    EXPECT_EQ ("The text was not saved for safety reasons. The length differs 77% from the existing text. Make fewer changes at a time and wait till the editor has saved the text. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
}


// Safely store a verse with too much of content difference: Fails.
TEST (bibles, safely_store_verse_with_too_large_content_difference)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (40);
    const std::string data = "\\v 2 vERSE TWO TWO two two two two.\n";
    std::string explanation;
    const std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, false);
    EXPECT_EQ ("Text content differs too much", stored);
    EXPECT_EQ ("The text was not saved for safety reasons. The new text is 49% similar to the existing text. Make fewer changes at a time and wait till the editor has saved the text. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (40);
    const std::string data = "\\v 2 vERSE TWO TWO two two two two.\n";
    std::string explanation;
    const std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, true);
    EXPECT_EQ ("Text content differs too much", stored);
    EXPECT_EQ ("The text was not saved for safety reasons. The new text is 52% similar to the existing text. Make fewer changes at a time and wait till the editor has saved the text. Or relax the restriction in the editing settings. See menu Settings - Preferences.", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
}


// Safely store USFM without any verse to verse 2: Fails.
TEST (bibles, safely_store_usfm_without_verse_to_verse_2_fails)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (40);
    const std::string data = "\\p Verse 2.\n";
    std::string explanation;
    constexpr const auto quill {false};
    const std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ ("Missing verse number", stored);
    EXPECT_EQ ("The USFM contains no verse information", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (40);
    const std::string data = "\\p Verse 2.\n";
    std::string explanation;
    constexpr const auto quill {true};
    const std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ ("Missing verse number", stored);
    EXPECT_EQ ("The USFM contains no verse information", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
}


// Safely store USFM with two verses: Fails.
TEST (bibles, safely_store_usfm_with_two_verses_fails)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (40);
    const std::string data = "\\v 2 Verse 2.\n\\v 3 3";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ ("Cannot overwrite another verse", stored);
    EXPECT_EQ ("The USFM contains verse(s) 0 2 3 which would overwrite a fragment that contains verse(s) 0 2", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (40);
    const std::string data = "\\v 2 Verse 2.\n\\v 3 3";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ ("Cannot overwrite another verse", stored);
    EXPECT_EQ ("The USFM contains verse(s) 0 2 3 which would overwrite a fragment that contains verse(s) 0 2", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_separate, result);
  }
}


// Safely store USFM with incorrect chapter: Fails.
TEST (bibles, safely_store_usfm_with_invalid_chapter_fails)
{
  Webserver_Request webserver_request;
  constexpr int book{1};
  constexpr int chapter{1};
  constexpr int verse0{0};
  constexpr int verse1{1};
  
  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (100);
    constexpr const auto data =
    R"(\c 1)" "\n"
    R"(\p)" "\n"
    R"(\c)"
    ;
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, book, chapter, verse0, data, explanation, quill);
    EXPECT_EQ ("Incorrect chapter", stored);
    EXPECT_EQ ("Cannot save 2 chapters", explanation);
    const std::string result = database::bibles::get_chapter (bible, book, chapter);
    EXPECT_EQ (usfm_separate, result);
  }

  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (100);
    constexpr const auto data =
    R"(\c 2)" "\n"
    R"(\p)"
    ;
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, book, chapter, verse0, data, explanation, quill);
    EXPECT_EQ ("Incorrect chapter", stored);
    EXPECT_EQ ("Incorrect chapter 2", explanation);
    const std::string result = database::bibles::get_chapter (bible, book, chapter);
    EXPECT_EQ (usfm_separate, result);
  }

  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (100);
    constexpr const auto data =
    R"(\c 1)" "\n"
    R"(\p)" "\n"
    R"(\v 1 Verse 1.)" "\n"
    R"(\c 1)"
    ;
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, book, chapter, verse1, data, explanation, quill);
    EXPECT_EQ ("Incorrect chapter", stored);
    EXPECT_EQ ("Cannot save 3 chapters", explanation);
    const std::string result = database::bibles::get_chapter (bible, book, chapter);
    EXPECT_EQ (usfm_separate, result);
  }

  {
    test_store_bible_data_safely_setup (usfm_separate);
    webserver_request.database_config_user ()->set_editing_allowed_difference_verse (100);
    constexpr const auto data =
    R"(\c 1)" "\n"
    R"(\p)" "\n"
    R"(\v 1 Verse 1.)" "\n"
    R"(\c 2)"
    ;
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, book, chapter, verse1, data, explanation, quill);
    EXPECT_EQ ("Incorrect chapter", stored);
    EXPECT_EQ ("Cannot save 3 chapters", explanation);
    const std::string result = database::bibles::get_chapter (bible, book, chapter);
    EXPECT_EQ (usfm_separate, result);
  }
}


// Safely store combined verse without any change.
TEST (bibles, safely_store_combined_verse_without_any_change)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\v 2-3 Verse 2 and 3.\n\\p\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_combined, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\v 2-3 Verse 2 and 3.\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 2, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_combined, result);
  }
}


// Safely store combined verse before the \p with a change.
TEST (bibles, safely_store_combined_verse_before_the_p_with_a_change)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\v 2-3 Verse 2 andx 3.\n\\p\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 3, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    const std::string newusfm = filter::strings::replace ("2 and 3", "2 andx 3", usfm_combined);
    EXPECT_EQ (newusfm, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\v 2-3 Verse 2 andx 3.\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 3, data, explanation, quill);
    EXPECT_EQ (std::string(), stored);
    EXPECT_EQ (std::string(), explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    const std::string newusfm = filter::strings::replace ("2 and 3", "2 andx 3", usfm_combined);
    EXPECT_EQ (newusfm, result);
  }
}


// Safely store combined verse after the \p with a change.
TEST (bibles, safely_store_combined_verse_after_the_p_with_a_change)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\v 4-5 Verse 4 andx 5.\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 4, data, explanation, quill);
    EXPECT_EQ ("", stored);
    EXPECT_EQ ("", explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("4 and 5", "4 andx 5", usfm_combined);
    EXPECT_EQ (newusfm, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\p\n\\v 4-5 Verse 4 andx 5.\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 4, data, explanation, quill);
    EXPECT_EQ ("", stored);
    EXPECT_EQ ("", explanation);
    std::string result = database::bibles::get_chapter (bible, 1, 1);
    std::string newusfm = filter::strings::replace ("4 and 5", "4 andx 5", usfm_combined);
    EXPECT_EQ (newusfm, result);
  }
}


// Safely store combined verse with a change and wrong verses: Fails.
TEST (bibles, safely_store_combined_verse_with_a_change_and_wrong_verses_fails)
{
  Webserver_Request webserver_request;
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\v 2-4 Verse 2 andx 3.\n";
    std::string explanation;
    constexpr const auto quill {false};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 3, data, explanation, quill);
    EXPECT_EQ ("Cannot overwrite another verse", stored);
    EXPECT_EQ ("The USFM contains verse(s) 0 2 3 4 which would overwrite a fragment that contains verse(s) 0 2 3", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_combined, result);
  }
  {
    test_store_bible_data_safely_setup (usfm_combined);
    const std::string data = "\\v 2-4 Verse 2 andx 3.\n";
    std::string explanation;
    constexpr const auto quill {true};
    std::string stored = filter::usfm::safely_store_verse (webserver_request, bible, 1, 1, 3, data, explanation, quill);
    EXPECT_EQ ("Cannot overwrite another verse", stored);
    EXPECT_EQ ("The USFM contains verse(s) 0 2 3 4 which would overwrite a fragment that contains verse(s) 0 2 3", explanation);
    const std::string result = database::bibles::get_chapter (bible, 1, 1);
    EXPECT_EQ (usfm_combined, result);
  }
}


TEST (bibles, condense_editor_update)
{
  // Condense a simple editor update to format a paragraph.
  {
    const std::vector <int> positions_in       { 6,     6 };
    const std::vector <int> sizes_in           { 1,     1 };
    const std::vector <bool> additions_in      { false, true };
    const std::vector <std::string> content_in { "\np", "\ns" };
    std::vector <int> positions_out;
    std::vector <int> sizes_out;
    std::vector <std::string> operators_out;
    std::vector <std::string> content_out;
    bible_logic::condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                          positions_out, sizes_out, operators_out, content_out);
    EXPECT_EQ (std::vector<int>{6}, positions_out);
    EXPECT_EQ (std::vector<int>{1}, sizes_out);
    EXPECT_EQ (std::vector <std::string>{"p"}, operators_out);
    EXPECT_EQ (std::vector <std::string>{"s"}, content_out);
  }

  // No condensing of any editor updates.
  {
    const std::vector <int> positions_in       { 6,      6 };
    const std::vector <int> sizes_in           { 1,      1 };
    const std::vector <bool> additions_in      { false,  true };
    const std::vector <std::string> content_in { "ladd", "kadd" };
    std::vector <int> positions_out;
    std::vector <int> sizes_out;
    std::vector <std::string> operators_out;
    std::vector <std::string> content_out;
    bible_logic::condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                          positions_out, sizes_out, operators_out, content_out);
    EXPECT_EQ (positions_in, positions_out);
    EXPECT_EQ (sizes_in, sizes_out);
    
    std::vector<std::string> standard_operators_out {"d","i"};
    EXPECT_EQ (standard_operators_out, operators_out);
    std::vector<std::string> standard_content_out {"ladd", "kadd"};
    EXPECT_EQ (standard_content_out, content_out);
  }
  
  // No condensing of any editor updates.
  // Handle smileys as examples of characters that are 4 bytes long in UTF-16.
  {
    std::vector <int> positions_in {6, 6};
    std::vector <int> sizes_in {2, 2};
    std::vector <bool> additions_in {false, true};
    std::vector <std::string> content_in {"😀add", "😁add"};
    std::vector <int> positions_out;
    std::vector <int> sizes_out;
    std::vector <std::string> operators_out;
    std::vector <std::string> content_out;
    bible_logic::condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                          positions_out, sizes_out, operators_out, content_out);
    EXPECT_EQ (positions_in, positions_out);
    EXPECT_EQ (sizes_in, sizes_out);
    std::vector <std::string> standard_operators_out {"d", "i"};
    EXPECT_EQ (standard_operators_out, operators_out);
    std::vector <std::string> standard_content_out {"😀add", "😁add"};
    EXPECT_EQ (standard_content_out, content_out);
  }
  
  // Condense new line and leave other edits as they are.
  {
    std::vector <int> positions_in {6, 6, 7, 8};
    std::vector <int> sizes_in {1, 1, 1, 1};
    std::vector <bool> additions_in {false, true,  false,  true};
    std::vector <std::string> content_in {"\np", "\ns", "ladd", "kadd"};
    std::vector <int> positions_out;
    std::vector <int> sizes_out;
    std::vector <std::string> operators_out;
    std::vector <std::string> content_out;
    bible_logic::condense_editor_updates (positions_in, sizes_in, additions_in, content_in,
                                          positions_out, sizes_out, operators_out, content_out);
    std::vector <int> standard_positions_out {6, 7, 8};
    EXPECT_EQ (standard_positions_out, positions_out);
    std::vector <int> standard_sizes_out {1, 1, 1};
    EXPECT_EQ (standard_sizes_out, sizes_out);
    std::vector <std::string> standard_operators_out {"p", "d", "i"};
    EXPECT_EQ (standard_operators_out, operators_out);
    std::vector <std::string> standard_content_out {"s", "ladd", "kadd"};
    EXPECT_EQ (standard_content_out, content_out);
  }
}


TEST (bibles, entire_pipeline_for_editor_updates)
{
  // Test entire pipeline for generating editor updates.
  {
    // The server text has inserted an exclamation mark after "nations!".
    const std::string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    const std::string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!!</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q2"><span>And so on the third line.</span></p>)";
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <std::string> operators;
    std::vector <std::string> content;
    bible_logic::html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    EXPECT_EQ (std::vector <int>{34}, positions);
    EXPECT_EQ (std::vector <int>{1}, sizes);
    EXPECT_EQ (std::vector <std::string>{"i"}, operators);
    EXPECT_EQ (std::vector <std::string>{"!"}, content);
  }
  
  // Test entire pipeline for generating editor updates.
  {
    // The server text has removed two exclamation marks from "nations!!" and inserted a full stop.
    const std::string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    const std::string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations.</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q2"><span>And so on the third line.</span></p>)";
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <std::string> operators;
    std::vector <std::string> content;
    bible_logic::html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    std::vector <int> standard_positions {33, 33, 33};
    std::vector <int> standard_sizes {1, 1, 1};
    std::vector <std::string> standard_operators {"d", "d", "i"};
    std::vector <std::string> standard_content {"!", "!", "."};
    EXPECT_EQ (standard_positions, positions);
    EXPECT_EQ (standard_sizes, sizes);
    EXPECT_EQ (standard_operators, operators);
    EXPECT_EQ (standard_content, content);
  }
  
  // Test entire pipeline for generating editor updates.
  {
    // The server text has joined the second paragraph to the first.
    const std::string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    const std::string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!</span> <span>Extol him, all you peoples!</span></p><p class="b-q2"><span>And so on the third line.</span></p>)";
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <std::string> operators;
    std::vector <std::string> content;
    bible_logic::html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    std::vector <int> standard_positions {34, 34, 0, 62};
    std::vector <int> standard_sizes {1, 1, 1, 1};
    std::vector <std::string> standard_operators {"d", "i", "p", "p"};
    std::vector <std::string> standard_content {"\np", " ", "q1", "q2"};
    EXPECT_EQ (standard_positions, positions);
    EXPECT_EQ (standard_sizes, sizes);
    EXPECT_EQ (standard_operators, operators);
    EXPECT_EQ (standard_content, content);
  }
  
  // Test entire pipeline for generating editor updates.
  {
    // The server text has added an extra paragraph with a certain paragraph style.
    const std::string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples!</p><p class="b-q2">And so on the third line.</p>)";
    const std::string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q3">P.</p><p class="b-q2"><span>And so on the third line.</span></p>)";
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <std::string> operators;
    std::vector <std::string> content;
    bible_logic::html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    std::vector <int> standard_positions {62, 63, 64, 0, 34, 62, 65};
    std::vector <int> standard_sizes {1, 1, 1, 1, 1, 1, 1};
    std::vector <std::string> standard_operators {"i", "i", "i", "p", "p", "p",  "p"};
    std::vector <std::string> standard_content {"\nq3", "P", ".", "q1", "p", "q3", "q2"};
    EXPECT_EQ (standard_positions, positions);
    EXPECT_EQ (standard_sizes, sizes);
    EXPECT_EQ (standard_operators, operators);
    EXPECT_EQ (standard_content, content);
  }
  
  // Test entire pipeline for generating editor updates using 4-bytes UTF-16 characters.
  {
    // The server text has added an extra paragraph with a certain paragraph style.
    const std::string editor_html = R"(<p class="b-q1"><span class="i-v">1</span> Praise Yahweh, all you nations!</p><p class="b-p">Extol him, all you peoples😀!</p><p class="b-q2">And so on the third line.</p>)";
    const std::string server_html = R"(<p class="b-q1"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations😀!</span></p><p class="b-p"><span>Extol him, all you peoples!</span></p><p class="b-q3">P😀.</p><p class="b-q2"><span>And so on the third line😀.</span></p>)";
    std::vector <int> positions;
    std::vector <int> sizes;
    std::vector <std::string> operators;
    std::vector <std::string> content;
    bible_logic::html_to_editor_updates (editor_html, server_html, positions, sizes, operators, content);
    EXPECT_EQ ((std::vector <int>{33, 63, 64, 65, 68, 69, 94, 0, 36, 64, 69}), positions);
    EXPECT_EQ ((std::vector <int>{2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1}), sizes);
    EXPECT_EQ ((std::vector <std::string>{"i", "i", "i", "i", "i", "d", "i", "p", "p", "p", "p"}), operators);
    std::vector <std::string> standard_content {"😀", "!", "\nq3", "P", ".", "!", "😀", "q1", "p", "q3", "q2"};
    EXPECT_EQ (standard_content, content);
  }
}


TEST (bibles, database_bibleactions)
{
  refresh_sandbox (false);
  database::bible_actions::create ();
  
  database::bible_actions::optimize ();
  
  std::vector <std::string> s_standard{};
  std::vector <std::string> bibles = database::bible_actions::get_bibles ();
  s_standard = {};
  EXPECT_EQ (s_standard, bibles);
  
  database::bible_actions::record ("unittest1", 1, 2, "data112");
  database::bible_actions::record ("unittest1", 1, 3, "data113");
  database::bible_actions::record ("unittest1", 2, 4, "data124");
  database::bible_actions::record ("unittest2", 5, 6, "data256");
  database::bible_actions::record ("unittest2", 5, 6, "data256: Not to be stored");
  
  bibles = database::bible_actions::get_bibles ();
  s_standard = {"unittest1", "unittest2"};
  EXPECT_EQ (s_standard, bibles);
  
  std::vector <int> i_standard{};
  
  std::vector <int> books = database::bible_actions::get_books ("unittest1");
  i_standard = {1, 2};
  EXPECT_EQ (i_standard, books);
  
  std::vector <int> chapters = database::bible_actions::get_chapters ("unittest1", 1);
  i_standard = {2, 3};
  EXPECT_EQ (i_standard, chapters);
  
  chapters = database::bible_actions::get_chapters ("unittest1", 2);
  i_standard = {4};
  EXPECT_EQ (i_standard, chapters);
  
  database::bible_actions::erase ("unittest1", 2, 3);
  
  chapters = database::bible_actions::get_chapters ("unittest1", 2);
  i_standard = {4};
  EXPECT_EQ (i_standard, chapters);
  
  database::bible_actions::erase ("unittest1", 2, 4);
  
  chapters = database::bible_actions::get_chapters ("unittest1", 2);
  i_standard = {};
  EXPECT_EQ (i_standard, chapters);
  
  std::string usfm = database::bible_actions::get_usfm ("unittest2", 5, 5);
  EXPECT_EQ ("", usfm);
  
  usfm = database::bible_actions::get_usfm ("unittest2", 5, 6);
  EXPECT_EQ ("data256", usfm);
}


TEST (bibles, database_bibles)
{
  std::string testbible = "testbible";
  std::string testbible2 = testbible + "2";

  // It should get the standard default Bible when there's no Bibles created by the user yet.
  {
    refresh_sandbox (false);
    Database_State::create ();
    std::vector <std::string> standard;
    std::vector <std::string> bibles = database::bibles::get_bibles ();
    EXPECT_EQ (standard, bibles);
  }

  // Test whether optimizing works without errors.
  {
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible (testbible);
    database::bibles::store_chapter (testbible, 2, 3, "a");
    database::bibles::store_chapter (testbible, 2, 3, "b");
    database::bibles::store_chapter (testbible, 2, 3, "c");
    database::bibles::store_chapter (testbible, 2, 3, "d");
    database::bibles::store_chapter (testbible, 2, 3, "e");
    database::bibles::store_chapter (testbible, 2, 3, "f");
    database::bibles::store_chapter (testbible, 2, 3, "g");
    database::bibles::optimize ();
    const std::string usfm = database::bibles::get_chapter (testbible, 2, 3);
    EXPECT_EQ ("g", usfm);
  }

  // Test whether optimizing removes files with 0 size.
  {
    refresh_sandbox (true);
    Database_State::create ();
    database::bibles::create_bible (testbible);
    database::bibles::store_chapter (testbible, 2, 3, "a");
    database::bibles::store_chapter (testbible, 2, 3, "b");
    database::bibles::store_chapter (testbible, 2, 3, "c");
    database::bibles::store_chapter (testbible, 2, 3, "d");
    database::bibles::store_chapter (testbible, 2, 3, "e");
    database::bibles::store_chapter (testbible, 2, 3, "f");
    database::bibles::store_chapter (testbible, 2, 3, "");
    std::string usfm = database::bibles::get_chapter (testbible, 2, 3);
    EXPECT_EQ (std::string(), usfm);
    database::bibles::optimize ();
    usfm = database::bibles::get_chapter (testbible, 2, 3);
    EXPECT_EQ ("f", usfm);
  }

  // Test create / get / delete Bibles.
  {
    refresh_sandbox (true);
    Database_State::create ();
    
    database::bibles::create_bible (testbible);
    
    std::vector <std::string> bibles = database::bibles::get_bibles ();
    std::vector <std::string> standard {testbible};
    EXPECT_EQ (standard, bibles);
    
    database::bibles::delete_bible (testbible);
    
    bibles = database::bibles::get_bibles ();
    standard = {};
    EXPECT_EQ (standard, bibles);
  }

  // Test storing a chapter, and getting it again.
  {
    refresh_sandbox (true);
    Database_State::create ();
    
    database::bibles::create_bible (testbible);
    const std::string usfm = "\\c 1\n\\p\n\\v 1 Verse 1";
    database::bibles::store_chapter (testbible, 2, 1, usfm);
    std::string result = database::bibles::get_chapter (testbible, 2, 1);
    EXPECT_EQ (usfm, result);
    result = database::bibles::get_chapter (testbible2, 2, 1);
    EXPECT_EQ (std::string(), result);
    result = database::bibles::get_chapter (testbible, 1, 1);
    EXPECT_EQ (std::string(), result);
  }

  // Test books.
  {
    refresh_sandbox (true);
    Database_State::create ();
    std::vector <int> standard{};
    
    database::bibles::create_bible (testbible);
    std::vector <int> books = database::bibles::get_books (testbible);
    EXPECT_EQ (std::vector <int>{}, books);
    
    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    books = database::bibles::get_books (testbible);
    EXPECT_EQ (std::vector <int>{1}, books);
    
    database::bibles::store_chapter (testbible, 2, 3, "\\c 0");
    books = database::bibles::get_books (testbible);
    standard = {1, 2};
    EXPECT_EQ (standard, books);
    
    database::bibles::delete_book (testbible, 3);
    books = database::bibles::get_books (testbible);
    standard = {1, 2};
    EXPECT_EQ (standard, books);
    
    database::bibles::delete_book (testbible, 1);
    books = database::bibles::get_books (testbible);
    EXPECT_EQ (std::vector <int>{2}, books);
    
    database::bibles::delete_book (testbible2, 2);
    books = database::bibles::get_books (testbible);
    EXPECT_EQ (std::vector <int>{2}, books);
  }

  // Test chapters.
  {
    refresh_sandbox (true);
    Database_State::create ();
    std::vector<int>standard{};
    
    database::bibles::create_bible (testbible);
    std::vector <int> chapters = database::bibles::get_chapters (testbible, 1);
    standard = { };
    EXPECT_EQ (standard, chapters);
    
    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    chapters = database::bibles::get_chapters (testbible, 1);
    standard = { 2 };
    EXPECT_EQ (standard, chapters);
    
    chapters = database::bibles::get_chapters (testbible, 2);
    standard = { };
    EXPECT_EQ (standard, chapters);
    
    database::bibles::store_chapter (testbible, 1, 3, "\\c 1");
    chapters = database::bibles::get_chapters (testbible, 1);
    standard = { 2, 3 };
    EXPECT_EQ (standard, chapters);
    
    database::bibles::delete_chapter (testbible, 3, 3);
    chapters = database::bibles::get_chapters (testbible, 1);
    standard = { 2, 3 };
    EXPECT_EQ (standard, chapters);
    
    database::bibles::delete_chapter (testbible, 1, 2);
    chapters = database::bibles::get_chapters (testbible, 1);
    standard = { 3 };
    EXPECT_EQ (standard, chapters);
    
    database::bibles::delete_chapter (testbible, 1, 3);
    chapters = database::bibles::get_chapters (testbible, 1);
    standard = { };
    EXPECT_EQ (standard, chapters);
  }

  // Test chapter identifiers.
  {
    refresh_sandbox (true);
    Database_State::create ();
    
    database::bibles::create_bible (testbible);
    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    int id = database::bibles::get_chapter_id (testbible, 1, 2);
    EXPECT_EQ (100'000'001, id);
    
    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    id = database::bibles::get_chapter_id (testbible, 1, 2);
    EXPECT_EQ (100'000'002, id);
    
    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    id = database::bibles::get_chapter_id (testbible, 1, 2);
    EXPECT_EQ (100'000'004, id);
    
    database::bibles::store_chapter (testbible, 2, 3, "\\c 1");
    id = database::bibles::get_chapter_id (testbible, 1, 2);
    EXPECT_EQ (100'000'004, id);
  }
  
  // Test the age of the newest chapter.
  {
    refresh_sandbox (true);
    Database_State::create ();
    
    database::bibles::create_bible (testbible);
    int age = database::bibles::get_chapter_age (testbible, 1, 2);
    EXPECT_EQ (100'000'000, age);

    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    age = database::bibles::get_chapter_age (testbible, 1, 2);
    EXPECT_EQ (0, age);

    database::bibles::store_chapter (testbible, 1, 2, "\\c 1");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    age = database::bibles::get_chapter_age (testbible, 1, 2);
    EXPECT_TRUE ((age == 1) || (age == 2));
  }
}


#endif
