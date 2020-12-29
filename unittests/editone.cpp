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


#include <unittests/editone.h>
#include <unittests/utilities.h>
#include <styles/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <editone2/logic.h>


void test_editone_logic_verse_indicator (int verse)
{
  cerr << "The above is about verse " << verse << endl << endl;
}


// Test the logic used in the visual verse editor.
void test_editone_logic ()
{
  trace_unit_tests (__func__);

  string stylesheet = styles_logic_standard_sheet ();
  string directory = filter_url_create_root_path ("unittests", "tests");
  
  // Prefix.
  {
    string usfm = filter_url_file_get_contents (filter_url_create_path (directory, "editone01.usfm"));
    string html;
    string last_paragraph_style;
    editone_logic_prefix_html (usfm, stylesheet, html, last_paragraph_style);
    string standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone01.html"));
    evaluate (__LINE__, __func__, standard, html);
    evaluate (__LINE__, __func__, "p", last_paragraph_style);
  }
  
  // Suffix.
  {
    string usfm = filter_url_file_get_contents (filter_url_create_path (directory, "editone03.usfm"));
    string html;
    string last_paragraph_style;
    editone_logic_suffix_html ("q1", usfm, stylesheet, html);
    string standard = filter_url_file_get_contents (filter_url_create_path (directory, "editone03.html"));
    evaluate (__LINE__, __func__, standard, html);
    evaluate (__LINE__, __func__, "", last_paragraph_style);
  }
  
  // Removing notes from the prefix and appending them to the notes in the suffix.
  {
    string usfm_prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone01prefix.usfm"));
    string usfm_suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone01suffix.usfm"));
    string html_prefix;
    string html_suffix;
    string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes_v2 (html_prefix, html_suffix);
    string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him</span><span class="i-notecall2">2</span><span>, all you peoples!</span></p><p class="b-p"><br /></p>)";
    string standard_suffix = R"(<p class="p"><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness</span><span class="i-notecall1">1</span><span>, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p><p class="b-notes"> <p class="b-f"><span class="i-notebody1">1</span><span /><span>+ Note one.</span></p><p class="b-f"><span class="i-notebody2">2</span><span /><span>+ Note two.</span></p></p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ Note four.</span></p>)";
    evaluate (__LINE__, __func__, standard_prefix, html_prefix);
    evaluate (__LINE__, __func__, standard_suffix, html_suffix);
  }

  // Prefix without notes, so moving nothing to the notes in the suffix.
  {
    string usfm_prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone02prefix.usfm"));
    string usfm_suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone02suffix.usfm"));
    string html_prefix;
    string html_suffix;
    string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes_v2 (html_prefix, html_suffix);
    string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him, all you peoples!</span></p><p class="b-p"><br /></p>)";
    string standard_suffix = R"(<p class="p"><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p>)";
    evaluate (__LINE__, __func__, standard_prefix, html_prefix);
    evaluate (__LINE__, __func__, standard_suffix, html_suffix);
  }

  // Move notes from the prefix to a suffix that does not have notes of its own.
  {
    string usfm_prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone03prefix.usfm"));
    string usfm_suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone03suffix.usfm"));
    string html_prefix;
    string html_suffix;
    string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes_v2 (html_prefix, html_suffix);
    string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him</span><span class="i-notecall2">2</span><span>, all you peoples!</span></p><p class="b-p"><br /></p>)";
    string standard_suffix = R"(<p class="p"><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p><p class="b-notes"><br /><p class="b-f"><span class="i-notebody1">1</span><span /><span>+ Note one.</span></p><p class="b-f"><span class="i-notebody2">2</span><span /><span>+ Note two.</span></p></p>)";
    evaluate (__LINE__, __func__, standard_prefix, html_prefix);
    evaluate (__LINE__, __func__, standard_suffix, html_suffix);
  }

  // Test that a empty prefix works fine when trying to move notes from prefix to suffix.
  {
    string usfm_prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone04prefix.usfm"));
    string usfm_suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone04suffix.usfm"));
    string html_prefix;
    string html_suffix;
    string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes_v2 (html_prefix, html_suffix);
    string standard_prefix = R"()";
    string standard_suffix = R"(<p><span class="i-v">4</span><span> </span><span>Yahweh’s faithfulness</span><span class="i-notecall1">1</span><span>, endures forever.</span><span> </span><span class="i-v">5</span><span> </span><span>Last verse, without a note.</span></p><p class="b-notes"> </p><p class="b-f"><span class="i-notebody1">1</span><span> </span><span>+ Note four.</span></p>)";
    evaluate (__LINE__, __func__, standard_prefix, html_prefix);
    evaluate (__LINE__, __func__, standard_suffix, html_suffix);
  }

  // Test that notes from the prefix get moved even to an empty suffix.
  {
    string usfm_prefix = filter_url_file_get_contents (filter_url_create_path (directory, "editone05prefix.usfm"));
    string usfm_suffix = filter_url_file_get_contents (filter_url_create_path (directory, "editone05suffix.usfm"));
    string html_prefix;
    string html_suffix;
    string last_paragraph_style;
    editone_logic_prefix_html (usfm_prefix, stylesheet, html_prefix, last_paragraph_style);
    editone_logic_suffix_html (last_paragraph_style, usfm_suffix, stylesheet, html_suffix);
    editone_logic_move_notes_v2 (html_prefix, html_suffix);
    string standard_prefix = R"(<p class="b-c"><span>117</span></p><p class="b-p"><span class="i-v">1</span><span> </span><span>Praise Yahweh</span><span class="i-notecall1">1</span><span>, all you nations!</span><span> </span><span class="i-v">2</span><span> </span><span>Extol him</span><span class="i-notecall2">2</span><span>, all you peoples!</span></p><p class="b-p"><br /></p>)";
    string standard_suffix = R"(<p class="b-notes"><br /><p class="b-f"><span class="i-notebody1">1</span><span /><span>+ Note one.</span></p><p class="b-f"><span class="i-notebody2">2</span><span /><span>+ Note two.</span></p></p>)";
    evaluate (__LINE__, __func__, standard_prefix, html_prefix);
    evaluate (__LINE__, __func__, standard_suffix, html_suffix);
  }

  // Regression test for case of a chapter with references and combined verse.
  // See issue https://github.com/bibledit/bibledit/issues/496
  {
    string chapter_usfm = filter_url_file_get_contents (filter_url_create_path (directory, "editone06.usfm"));
    int highest_verse = 4;
    string reference;
    bool evaluation = false;
    
    // Test the range of verses found in the USFM fragment.
    for (int verse = 0; verse <= 4; verse++) {

      string number = convert_to_string (verse);
      
      // Test the editable USFM fragment.
      string editable_usfm = usfm_get_verse_text_quill (chapter_usfm, verse);
      reference = filter_url_file_get_contents (filter_url_create_path (directory, "editone06verse" + number + "edit.usfm"));
      evaluation = evaluate (__LINE__, __func__, reference, editable_usfm);
      if (!evaluation) test_editone_logic_verse_indicator (verse);
      
      // Test the USFM fragment before the editable verse.
      string prefix_usfm = usfm_get_verse_range_text (chapter_usfm, 0, verse - 1, editable_usfm, true);
      reference = filter_url_file_get_contents (filter_url_create_path (directory, "editone06verse" + number + "prefix.usfm"));
      evaluation = evaluate (__LINE__, __func__, reference, prefix_usfm);
      if (!evaluation) test_editone_logic_verse_indicator (verse);
      
      // Test the USFM fragment that follows the editable verse.
      string suffix_usfm = usfm_get_verse_range_text (chapter_usfm, verse + 1, highest_verse, editable_usfm, true);
      reference = filter_url_file_get_contents (filter_url_create_path (directory, "editone06verse" + number + "suffix.usfm"));
      evaluation = evaluate (__LINE__, __func__, reference, suffix_usfm);
      if (!evaluation) test_editone_logic_verse_indicator (verse);

      // The rendered html of the prefix to the editable verse.
      string prefix_html;
      string not_used;
      editone_logic_prefix_html (prefix_usfm, stylesheet, prefix_html, not_used);
      reference = filter_url_file_get_contents (filter_url_create_path (directory, "editone06verse" + number + "prefix.html"));
      evaluation = evaluate (__LINE__, __func__, reference, prefix_html);
      if (!evaluation) test_editone_logic_verse_indicator (verse);

      // The rendered html of the editable verse.
      string editable_html;
      editone_logic_editable_html (editable_usfm, stylesheet, editable_html);
      reference = filter_url_file_get_contents (filter_url_create_path (directory, "editone06verse" + number + "edit.html"));
      evaluation = evaluate (__LINE__, __func__, reference, editable_html);
      if (!evaluation) test_editone_logic_verse_indicator (verse);

      // The html rendering of the suffix of the editable verse.
      string suffix_html;
      editone_logic_suffix_html ("", suffix_usfm, stylesheet, suffix_html);
      reference = filter_url_file_get_contents (filter_url_create_path (directory, "editone06verse" + number + "suffix.html"));
      evaluation = evaluate (__LINE__, __func__, reference, suffix_html);
      if (!evaluation) test_editone_logic_verse_indicator (verse);
    }
  }
}
