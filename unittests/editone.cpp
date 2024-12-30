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
#include <styles/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <editone/logic.h>


void test_editone_logic_verse_indicator (int verse)
{
  std::cerr << "The above is about verse " << verse << std::endl << std::endl;
}


// Test the logic used in the visual verse editor.
TEST (editone, logic)
{
  const std::string stylesheet = styles_logic_standard_sheet ();
  const std::string directory = filter_url_create_root_path ({"unittests", "tests"});
  
  // Prefix.
  {
    std::string usfm = filter_url_file_get_contents (filter_url_create_path ({directory, "editone01.usfm"}));
    std::string html;
    std::string last_paragraph_style;
    editone_logic_prefix_html (usfm, stylesheet, html, last_paragraph_style);
    std::string standard = filter_url_file_get_contents (filter_url_create_path ({directory, "editone01.html"}));
    EXPECT_EQ (standard, html);
    EXPECT_EQ ("p", last_paragraph_style);
  }
  
  // Suffix.
  {
    std::string usfm = filter_url_file_get_contents (filter_url_create_path ({directory, "editone03.usfm"}));
    std::string html;
    std::string last_paragraph_style;
    editone_logic_suffix_html ("q1", usfm, stylesheet, html);
    std::string standard = filter_url_file_get_contents (filter_url_create_path ({directory, "editone03.html"}));
    EXPECT_EQ (standard, html);
    EXPECT_EQ ("", last_paragraph_style);
  }
  
  // Removing notes from the prefix and appending them to the notes in the suffix.
  {
    std::string usfm_prefix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone01prefix.usfm"}));
    std::string usfm_suffix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone01suffix.usfm"}));
    std::string html_prefix;
    std::string html_suffix;
    std::string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes (html_prefix, html_suffix);
    std::string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him</span><span class="i-notecall2">2</span><span>, all you peoples!</span></p><p class="b-p"><br/></p>)";
    std::string standard_suffix = R"(<p class="p"><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness</span><span class="i-notecall1">1</span><span>, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p><p class="b-notes"> <p class="b-f"><span class="i-notebody1">1</span><span/><span>+ Note one.</span></p><p class="b-f"><span class="i-notebody2">2</span><span/><span>+ Note two.</span></p></p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ Note four.</span></p>)";
    EXPECT_EQ (standard_prefix, html_prefix);
    EXPECT_EQ (standard_suffix, html_suffix);
  }

  // Prefix without notes, so moving nothing to the notes in the suffix.
  {
    std::string usfm_prefix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone02prefix.usfm"}));
    std::string usfm_suffix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone02suffix.usfm"}));
    std::string html_prefix;
    std::string html_suffix;
    std::string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes (html_prefix, html_suffix);
    std::string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him, all you peoples!</span></p><p class="b-p"><br/></p>)";
    std::string standard_suffix = R"(<p class="p"><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p>)";
    EXPECT_EQ (standard_prefix, html_prefix);
    EXPECT_EQ (standard_suffix, html_suffix);
  }

  // Move notes from the prefix to a suffix that does not have notes of its own.
  {
    std::string usfm_prefix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone03prefix.usfm"}));
    std::string usfm_suffix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone03suffix.usfm"}));
    std::string html_prefix;
    std::string html_suffix;
    std::string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes (html_prefix, html_suffix);
    std::string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him</span><span class="i-notecall2">2</span><span>, all you peoples!</span></p><p class="b-p"><br/></p>)";
    std::string standard_suffix = R"(<p class="p"><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p><p class="b-notes"><br/><p class="b-f"><span class="i-notebody1">1</span><span/><span>+ Note one.</span></p><p class="b-f"><span class="i-notebody2">2</span><span/><span>+ Note two.</span></p></p>)";
    EXPECT_EQ (standard_prefix, html_prefix);
    EXPECT_EQ (standard_suffix, html_suffix);
  }

  // Test that a empty prefix works fine when trying to move notes from prefix to suffix.
  {
    std::string usfm_prefix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone04prefix.usfm"}));
    std::string usfm_suffix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone04suffix.usfm"}));
    std::string html_prefix;
    std::string html_suffix;
    std::string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes (html_prefix, html_suffix);
    std::string standard_prefix = R"()";
    std::string standard_suffix = R"(<p><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness</span><span class="i-notecall1">1</span><span>, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ Note four.</span></p>)";
    EXPECT_EQ (standard_prefix, html_prefix);
    EXPECT_EQ (standard_suffix, html_suffix);
  }

  // Test that notes from the prefix get moved even to an empty suffix.
  {
    std::string usfm_prefix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone05prefix.usfm"}));
    std::string usfm_suffix = filter_url_file_get_contents (filter_url_create_path ({directory, "editone05suffix.usfm"}));
    std::string html_prefix;
    std::string html_suffix;
    std::string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes (html_prefix, html_suffix);
    std::string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him</span><span class="i-notecall2">2</span><span>, all you peoples!</span></p><p class="b-p"><br/></p>)";
    std::string standard_suffix = R"(<p class="b-notes"><br/><p class="b-f"><span class="i-notebody1">1</span><span/><span>+ Note one.</span></p><p class="b-f"><span class="i-notebody2">2</span><span/><span>+ Note two.</span></p></p>)";
    EXPECT_EQ (standard_prefix, html_prefix);
    EXPECT_EQ (standard_suffix, html_suffix);
  }

  // Regression test for case of a chapter with references and combined verse.
  // See issue https://github.com/bibledit/bibledit/issues/496
  {
    std::string chapter_usfm = filter_url_file_get_contents (filter_url_create_path ({directory, "editone06.usfm"}));
    int highest_verse = 4;
    std::string reference;
    
    // Test the range of verses found in the USFM fragment.
    for (int verse = 0; verse <= 4; verse++) {

      std::string number = std::to_string (verse);
      
      // Test the editable USFM fragment.
      std::string editable_usfm = filter::usfm::get_verse_text_quill (chapter_usfm, verse);
      reference = filter_url_file_get_contents (filter_url_create_path ({directory, "editone06verse" + number + "edit.usfm"}));
      EXPECT_EQ (reference, editable_usfm);
      if (reference != editable_usfm) test_editone_logic_verse_indicator (verse);
      
      // Test the USFM fragment before the editable verse.
      std::string prefix_usfm = filter::usfm::get_verse_range_text (chapter_usfm, 0, verse - 1, editable_usfm, true);
      reference = filter_url_file_get_contents (filter_url_create_path ({directory, "editone06verse" + number + "prefix.usfm"}));
      EXPECT_EQ (reference, prefix_usfm);
      if (reference != prefix_usfm) test_editone_logic_verse_indicator (verse);
      
      // Test the USFM fragment that follows the editable verse.
      std::string suffix_usfm = filter::usfm::get_verse_range_text (chapter_usfm, verse + 1, highest_verse, editable_usfm, true);
      reference = filter_url_file_get_contents (filter_url_create_path ({directory, "editone06verse" + number + "suffix.usfm"}));
      EXPECT_EQ (reference, suffix_usfm);
      if (reference != suffix_usfm) test_editone_logic_verse_indicator (verse);

      // The rendered html of the prefix to the editable verse.
      std::string prefix_html;
      std::string not_used;
      editone_logic_prefix_html (prefix_usfm, stylesheet, prefix_html, not_used);
      reference = filter_url_file_get_contents (filter_url_create_path ({directory, "editone06verse" + number + "prefix.html"}));
      EXPECT_EQ (reference, prefix_html);
      if (reference != prefix_html) test_editone_logic_verse_indicator (verse);

      // The rendered html of the editable verse.
      std::string editable_html;
      editone_logic_editable_html (editable_usfm, stylesheet, editable_html);
      reference = filter_url_file_get_contents (filter_url_create_path ({directory, "editone06verse" + number + "edit.html"}));
      EXPECT_EQ (reference, editable_html);
      if (reference != editable_html) test_editone_logic_verse_indicator (verse);

      // The html rendering of the suffix of the editable verse.
      std::string suffix_html;
      editone_logic_suffix_html ("", suffix_usfm, stylesheet, suffix_html);
      reference = filter_url_file_get_contents (filter_url_create_path ({directory, "editone06verse" + number + "suffix.html"}));
      EXPECT_EQ (reference, suffix_html);
      if (reference != suffix_html) test_editone_logic_verse_indicator (verse);
    }
  }
}

#endif

