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


#include <database/config/bible.h>
#include <filter/url.h>
#include <filter/string.h>
#include <styles/logic.h>
#include <database/logic.h>


namespace database::config::bible {


// Cache values in memory for better speed.
// The speed improvement is supposed to come from reading a value from disk only once,
// and after that to read the value straight from the memory cache.
static std::map <std::string, std::string> cache;


// Functions for getting and setting values or lists of values follow now:


// The path to the folder for storing the settings for the $bible.
static std::string file (const std::string& bible)
{
  return filter_url_create_root_path ({database_logic_databases (), "config", "bible", bible});
}


// The path to the file that contains this setting.
static std::string file (const std::string& bible, const char * key)
{
  return filter_url_create_path ({file (bible), key});
}


// The key in the cache for this setting.
static std::string mapkey (const std::string& bible, const char * key)
{
  return bible + key;
}


std::string get_value (const std::string& bible, const char * key, const char * default_value)
{
  // Check the memory cache.
  const std::string cachekey = mapkey (bible, key);
  if (cache.count (cachekey)) {
    return cache [cachekey];
  }
  // Get the setting from file.
  std::string value;
  const std::string filename = file (bible, key);
  if (file_or_dir_exists (filename))
    value = filter_url_file_get_contents (filename);
  else 
    value = default_value;
  // Cache it.
  cache [cachekey] = value;
  // Done.
  return value;
}


static void set_value (const std::string& bible, const char * key, const std::string& value)
{
  if (bible.empty ()) 
    return;
  // Store in memory cache.
  cache [mapkey (bible, key)] = value;
  // Store on disk.
  const std::string filename = file (bible, key);
  const std::string dirname = filter_url_dirname (filename);
  if (!file_or_dir_exists (dirname))
    filter_url_mkdir (dirname);
  filter_url_file_put_contents (filename, value);
}


static bool get_boolean_value (const std::string& bible, const char * key, bool default_value)
{
  return filter::strings::convert_to_bool (get_value (bible, key, filter::strings::convert_to_string (default_value).c_str()));
}


static void set_boolean_value (const std::string& bible, const char * key, bool value)
{
  set_value (bible, key, filter::strings::convert_to_string (value));
}


static int get_integer_value (const std::string& bible, const char * key, int default_value)
{
  return filter::strings::convert_to_int (get_value (bible, key, std::to_string (default_value).c_str()));
}


static void set_integer_value (const std::string& bible, const char * key, int value)
{
  set_value (bible, key, std::to_string (value));
}


void remove (const std::string& bible)
{
  // Remove from disk.
  const std::string folder = file (bible);
  filter_url_rmdir (folder);
  // Clear cache.
  cache.clear ();
}


// Named configuration functions.


constexpr const auto remote_repo_url_key {"remote-repo-url"};
std::string get_remote_repository_url (const std::string& bible)
{
  return get_value (bible, remote_repo_url_key, "");
}
void set_remote_repository_url (const std::string& bible, const std::string& url)
{
  set_value (bible, remote_repo_url_key, url);
}


constexpr const auto double_spaces_usfm_key {"double-spaces-usfm"};
bool get_check_double_spaces_usfm (const std::string& bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return get_boolean_value (bible, double_spaces_usfm_key, standard);
}
void set_check_double_spaces_usfm (const std::string& bible, bool value)
{
  set_boolean_value (bible, double_spaces_usfm_key, value);
}


constexpr const auto full_stop_headings_key {"full-stop-headings"};
bool get_check_full_stop_in_headings (const std::string& bible)
{
  return get_boolean_value (bible, full_stop_headings_key, false);
}
void set_check_full_stop_in_headings (const std::string& bible, bool value)
{
  set_boolean_value (bible, full_stop_headings_key, value);
}


constexpr const auto space_before_punctuation_key {"space-before-punctuation"};
bool get_check_space_before_punctuation (const std::string& bible)
{
  return get_boolean_value (bible, space_before_punctuation_key, false);
}
void set_check_space_before_punctuation (const std::string& bible, bool value)
{
  set_boolean_value (bible, space_before_punctuation_key, value);
}


constexpr const auto space_before_final_note_marker_key {"space-before-final-note-marker"};
bool get_check_space_before_final_note_marker (const std::string& bible)
{
  return get_boolean_value (bible, space_before_final_note_marker_key, false);
}
void set_check_space_before_final_note_marker (const std::string& bible, bool value)
{
  set_boolean_value (bible, space_before_final_note_marker_key, value);
}


constexpr const auto sentence_structure_key {"sentence-structure"};
bool get_check_sentence_structure (const std::string& bible)
{
  return get_boolean_value (bible, sentence_structure_key, false);
}
void set_check_sentence_structure (const std::string& bible, bool value)
{
  set_boolean_value (bible, sentence_structure_key, value);
}


constexpr const auto paragraph_structure_key {"paragraph-structure"};
bool get_check_paragraph_structure (const std::string& bible)
{
  return get_boolean_value (bible, paragraph_structure_key, false);
}
void set_check_paragraph_structure (const std::string& bible, bool value)
{
  set_boolean_value (bible, paragraph_structure_key, value);
}


constexpr const auto check_books_versification_key {"check-books-versification"};
bool get_check_books_versification (const std::string& bible)
{
  return get_boolean_value (bible, check_books_versification_key, false);
}
void set_check_books_versification (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_books_versification_key, value);
}


constexpr const auto check_chapters_verses_versification_key {"check-chapters-verses-versification"};
bool get_check_chaptes_verses_versification (const std::string& bible)
{
  return get_boolean_value (bible, check_chapters_verses_versification_key, false);
}
void set_check_chaptes_verses_versification (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_chapters_verses_versification_key, value);
}


constexpr const auto check_well_formed_usfm_key {"check-well-formed-usfm"};
bool get_check_well_formed_usfm (const std::string& bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return get_boolean_value (bible, check_well_formed_usfm_key, standard);
}
void set_check_well_formed_usfm (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_well_formed_usfm_key, value);
}


constexpr const auto missing_punctuation_end_verse_key {"missing-punctuation-end-verse"};
bool get_check_missing_punctuation_end_verse (const std::string& bible)
{
  return get_boolean_value (bible, missing_punctuation_end_verse_key, false);
}
void set_check_missing_punctuation_end_verse (const std::string& bible, bool value)
{
  set_boolean_value (bible, missing_punctuation_end_verse_key, value);
}


constexpr const auto check_patterns_key {"check-patterns"};
bool get_check_patterns (const std::string& bible)
{
  return get_boolean_value (bible, check_patterns_key, false);
}
void set_check_patterns (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_patterns_key, value);
}


constexpr const auto checking_patterns_key {"checking-patterns"};
std::string get_checking_patterns (const std::string& bible)
{
  return get_value (bible, checking_patterns_key, "");
}
void set_checking_patterns (const std::string& bible, const std::string& value)
{
  set_value (bible, checking_patterns_key, value);
}


constexpr const auto sentence_structure_capitals_key {"sentence-structure-capitals"};
std::string get_sentence_structure_capitals (const std::string& bible)
{
  return get_value (bible, sentence_structure_capitals_key, "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
}
void set_sentence_structure_capitals (const std::string& bible, const std::string& value)
{
  set_value (bible, sentence_structure_capitals_key, value);
}


constexpr const auto sentence_structure_small_letters_key {"sentence-structure-small-letters"};
std::string get_sentence_structure_small_letters (const std::string& bible)
{
  return get_value (bible, sentence_structure_small_letters_key, "a b c d e f g h i j k l m n o p q r s t u v w x y z");
}
void set_sentence_structure_small_letters (const std::string& bible, const std::string& value)
{
  set_value (bible, sentence_structure_small_letters_key, value);
}


constexpr const auto sentence_structure_end_punctuation_key {"sentence-structure-end-punctuation"};
std::string get_sentence_structure_end_punctuation (const std::string& bible)
{
  return get_value (bible, sentence_structure_end_punctuation_key, ". ! ? :");
}
void set_sentence_structure_end_punctuation (const std::string& bible, const std::string& value)
{
  set_value (bible, sentence_structure_end_punctuation_key, value);
}


constexpr const auto sentence_structure_middle_punctuation_key {"sentence-structure-middle-punctuation"};
std::string get_sentence_structure_middle_punctuation (const std::string& bible)
{
  return get_value (bible, sentence_structure_middle_punctuation_key, ", ;");
}
void set_sentence_structure_middle_punctuation (const std::string& bible, const std::string& value)
{
  set_value (bible, sentence_structure_middle_punctuation_key, value);
}


constexpr const auto sentence_structure_disregards_key {"sentence-structure-disregards"};
std::string get_sentence_structure_disregards (const std::string& bible)
{
  return get_value (bible, sentence_structure_disregards_key, "( ) [ ] { } ' \" * - 0 1 2 3 4 5 6 7 8 9");
}
void set_sentence_structure_disregards (const std::string& bible, const std::string& value)
{
  set_value (bible, sentence_structure_disregards_key, value);
}


constexpr const auto sentence_structure_names_key {"sentence-structure-names"};
std::string get_sentence_structure_names (const std::string& bible)
{
  return get_value (bible, sentence_structure_names_key, "");
}
void set_sentence_structure_names (const std::string& bible, const std::string& value)
{
  set_value (bible, sentence_structure_names_key, value);
}


constexpr const auto sentence_structure_within_sentence_markers_key {"sentence-structure-within-sentence-markers"};
std::string get_sentence_structure_within_sentence_markers (const std::string& bible)
{
  return get_value (bible, sentence_structure_within_sentence_markers_key, "q q1 q2 q3");
}
void set_sentence_structure_within_sentence_markers (const std::string& bible, const std::string& value)
{
  set_value (bible, sentence_structure_within_sentence_markers_key, value);
}


constexpr const auto check_matching_pairs_key {"check-matching-pairs"};
bool get_check_matching_pairs (const std::string& bible)
{
  return get_boolean_value (bible, check_matching_pairs_key, false);
}
void set_check_matching_pairs (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_matching_pairs_key, value);
}


constexpr const auto matching_pairs_key {"matching-pairs"};
std::string get_matching_pairs (const std::string& bible)
{
  return get_value (bible, matching_pairs_key, "[] () {} “” ‘’ «» ‹›");
}
void set_matching_pairs (const std::string& bible, const std::string& value)
{
  set_value (bible, matching_pairs_key, value);
}


constexpr const auto check_space_end_verse_key {"check-space-end-verse"};
bool get_check_space_end_verse (const std::string& bible)
{
  return get_boolean_value (bible, check_space_end_verse_key, true);
}
void set_check_space_end_verse (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_space_end_verse_key, value);
}


constexpr const auto check_french_punctuation_key {"check-french-punctuation"};
bool get_check_french_punctuation (const std::string& bible)
{
  return get_boolean_value (bible, check_french_punctuation_key, false);
}
void set_check_french_punctuation (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_french_punctuation_key, value);
}


constexpr const auto check_french_citation_style_key {"check-french-citation-style"};
bool get_check_french_citation_style (const std::string& bible)
{
  return get_boolean_value (bible, check_french_citation_style_key, false);
}
void set_check_french_citation_style (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_french_citation_style_key, value);
}


constexpr const auto transpose_fix_spaces_notes_key {"transpose-fix-spaces-notes"};
bool get_transpose_fix_spaces_notes (const std::string& bible)
{
  return get_boolean_value (bible, transpose_fix_spaces_notes_key, false);
}
void set_transpose_fix_spaces_notes (const std::string& bible, bool value)
{
  set_boolean_value (bible, transpose_fix_spaces_notes_key, value);
}


constexpr const auto check_valid_utf8_text_key {"check-valid-utf8-text"};
bool get_check_valid_utf8_text (const std::string& bible)
{
  return get_boolean_value (bible, check_valid_utf8_text_key, false);
}
void set_check_valid_utf8_text (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_valid_utf8_text_key, value);
}


constexpr const auto sprint_task_completion_categories_key {"sprint-task-completion-categories"};
std::string get_sprint_task_completion_categories (const std::string& bible)
{
  return get_value (bible, sprint_task_completion_categories_key, "Translate\nCheck\nHebrew/Greek\nDiscussions");
}
void set_sprint_task_completion_categories (const std::string& bible, const std::string& value)
{
  set_value (bible, sprint_task_completion_categories_key, value);
}


constexpr const auto repeat_send_receive_key {"repeat-send-receive"};
int get_repeat_send_receive (const std::string& bible)
{
  return get_integer_value (bible, repeat_send_receive_key, 0);
}
void set_repeat_send_receive (const std::string& bible, int value)
{
  set_integer_value (bible, repeat_send_receive_key, value);
}


constexpr const auto export_chapter_drop_caps_frames_key {"export-chapter-drop-caps-frames"};
bool get_export_chapter_drop_caps_frames (const std::string& bible)
{
  return get_boolean_value (bible, export_chapter_drop_caps_frames_key, false);
}
void set_export_chapter_drop_caps_frames (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_chapter_drop_caps_frames_key, value);
}


constexpr const auto page_width_key {"page-width"};
std::string get_page_width (const std::string& bible)
{
  return get_value (bible, page_width_key, "210");
}
void set_page_width  (const std::string& bible, const std::string& value)
{
  set_value (bible, page_width_key, value);
}


constexpr const auto page_height_key {"page-height"};
std::string get_page_height (const std::string& bible)
{
  return get_value (bible, page_height_key, "297");
}
void set_page_height  (const std::string& bible, const std::string& value)
{
  set_value (bible, page_height_key, value);
}


constexpr const auto inner_margin_key {"inner-margin"};
std::string get_inner_margin (const std::string& bible)
{
  return get_value (bible, inner_margin_key, "20");
}
void set_inner_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, inner_margin_key, value);
}


constexpr const auto outer_margin_key {"outer-margin"};
std::string get_outer_margin (const std::string& bible)
{
  return get_value (bible, outer_margin_key, "10");
}
void set_outer_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, outer_margin_key, value);
}


constexpr const auto top_margin_key {"top-margin"};
std::string get_top_margin (const std::string& bible)
{
  return get_value (bible, top_margin_key, "10");
}
void set_top_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, top_margin_key, value);
}


constexpr const auto bottom_margin_key {"bottom-margin"};
std::string get_bottom_margin (const std::string& bible)
{
  return get_value (bible, bottom_margin_key, "10");
}
void set_bottom_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, bottom_margin_key, value);
}


constexpr const auto date_in_header_key {"date-in-header"};
bool get_date_in_header (const std::string& bible)
{
  return get_boolean_value (bible, date_in_header_key, false);
}
void set_date_in_header  (const std::string& bible, bool value)
{
  set_boolean_value (bible, date_in_header_key, value);
}


constexpr const auto hyphenation_first_set_key {"hyphenation-first-set"};
std::string get_hyphenation_first_set (const std::string& bible)
{
  return get_value (bible, hyphenation_first_set_key, "");
}
void set_hyphenation_first_set (const std::string& bible, const std::string& value)
{
  set_value (bible, hyphenation_first_set_key, value);
}


constexpr const auto hyphenation_second_set_key {"hyphenation-second-set"};
std::string get_hyphenation_second_set (const std::string& bible)
{
  return get_value (bible, hyphenation_second_set_key, "");
}
void set_hyphenation_second_set (const std::string& bible, const std::string& value)
{
  set_value (bible, hyphenation_second_set_key, value);
}


constexpr const auto editor_stylesheet_key {"editor-stylesheet"};
std::string get_editor_stylesheet (const std::string& bible)
{
  return get_value (bible, editor_stylesheet_key, styles_logic_standard_sheet ().c_str());
}
void set_editor_stylesheet (const std::string& bible, const std::string& value)
{
  set_value (bible, editor_stylesheet_key, value);
}


constexpr const auto export_stylesheet_key {"export-stylesheet"};
std::string get_export_stylesheet (const std::string& bible)
{
  return get_value (bible, export_stylesheet_key, styles_logic_standard_sheet ().c_str());
}
void set_export_stylesheet (const std::string& bible, const std::string& value)
{
  set_value (bible, export_stylesheet_key, value);
}


constexpr const auto versification_system_key {"versification-system"};
std::string get_versification_system (const std::string& bible)
{
  return get_value (bible, versification_system_key, filter::strings::english ());
}
void set_versification_system (const std::string& bible, const std::string& value)
{
  set_value (bible, versification_system_key, value);
}


constexpr const auto export_web_during_night_key {"export-web-during-night"};
bool get_export_web_during_night (const std::string& bible)
{
  return get_boolean_value (bible, export_web_during_night_key, false);
}
void set_export_web_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_web_during_night_key, value);
}


constexpr const auto export_html_during_night_key {"export-html-during-night"};
bool get_export_hml_during_night (const std::string& bible)
{
  return get_boolean_value (bible, export_html_during_night_key, false);
}
void set_export_hml_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_html_during_night_key, value);
}


constexpr const auto export_html_notes_on_hover_key {"export-html-notes-on-hover"};
bool get_export_html_notes_on_hover (const std::string& bible)
{
  return get_boolean_value (bible, export_html_notes_on_hover_key, false);
}
void set_export_html_notes_on_hover (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_html_notes_on_hover_key, value);
}


constexpr const auto export_usfm_during_night_key {"export-usfm-during-night"};
bool get_export_usfm_during_night (const std::string& bible)
{
  return get_boolean_value (bible, export_usfm_during_night_key, false);
}
void set_export_usfm_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_usfm_during_night_key, value);
}


constexpr const auto export_text_during_night_key {"export-text-during-night"};
bool get_export_text_during_night (const std::string& bible)
{
  return get_boolean_value (bible, export_text_during_night_key, false);
}
void set_export_text_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_text_during_night_key, value);
}


constexpr const auto export_odt_during_night_key {"export-odt-during-night"};
bool get_export_odt_during_night (const std::string& bible)
{
  return get_boolean_value (bible, export_odt_during_night_key, false);
}
void set_export_odt_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_odt_during_night_key, value);
}


constexpr const auto generate_info_during_night_key {"generate-info-during-night"};
bool get_generate_info_during_night (const std::string& bible)
{
  return get_boolean_value (bible, generate_info_during_night_key, false);
}
void set_generate_info_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, generate_info_during_night_key, value);
}


constexpr const auto export_esword_during_night_key {"export-esword-during-night"};
bool get_export_e_sword_during_night (const std::string& bible)
{
  return get_boolean_value (bible, export_esword_during_night_key, false);
}
void set_export_e_sword_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_esword_during_night_key, value);
}


constexpr const auto export_onlinebible_during_night_key {"export-onlinebible-during-night"};
bool get_export_online_bible_during_night (const std::string& bible)
{
  return get_boolean_value (bible, export_onlinebible_during_night_key, false);
}
void set_export_online_bible_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_onlinebible_during_night_key, value);
}


constexpr const auto export_password_key {"export-password"};
std::string get_export_password (const std::string& bible)
{
  return get_value (bible, export_password_key, "");
}
void set_export_password (const std::string& bible, const std::string& value)
{
  set_value (bible, export_password_key, value);
}


constexpr const auto secure_usfm_export_key {"secure-usfm-export"};
bool get_secure_usfm_export (const std::string& bible)
{
  return get_boolean_value (bible, secure_usfm_export_key, false);
}
void set_secure_usfm_export (const std::string& bible, bool value)
{
  set_boolean_value (bible, secure_usfm_export_key, value);
}


constexpr const auto secure_odt_export_key {"secure-odt-export"};
bool get_secure_odt_export (const std::string& bible)
{
  return get_boolean_value (bible, secure_odt_export_key, false);
}
void set_secure_odt_export (const std::string& bible, bool value)
{
  set_boolean_value (bible, secure_odt_export_key, value);
}


constexpr const auto export_font_key {"export-font"};
std::string get_export_font (const std::string& bible)
{
  return get_value (bible, export_font_key, "");
}
void set_export_font (const std::string& bible, const std::string& value)
{
  set_value (bible, export_font_key, value);
}


constexpr const auto export_feedback_email_key {"export-feedback-email"};
std::string get_export_feedback_email (const std::string& bible)
{
  return get_value (bible, export_feedback_email_key, "");
}
void set_export_feedback_email (const std::string& bible, const std::string& value)
{
  set_value (bible, export_feedback_email_key, value);
}


constexpr const auto book_order_key {"book-order"};
std::string get_book_order (const std::string& bible)
{
  return get_value (bible, book_order_key, "");
}
void set_book_order (const std::string& bible, const std::string& value)
{
  set_value (bible, book_order_key, value);
}


constexpr const auto text_direction_key {"text-direction"};
int get_text_direction (const std::string& bible)
{
  return get_integer_value (bible, text_direction_key, 0);
}
void set_text_direction (const std::string& bible, int value)
{
  set_integer_value (bible, text_direction_key, value);
}


constexpr const auto text_font_key {"text-font"};
std::string get_text_font (const std::string& bible)
{
  return get_value (bible, text_font_key, "");
}
void set_text_font (const std::string& bible, const std::string& value)
{
  set_value (bible, text_font_key, value);
}


constexpr const auto text_font_client_key {"text-font-client"};
std::string get_text_font_client (const std::string& bible)
{
  return get_value (bible, text_font_client_key, "");
}
void set_text_font_client (const std::string& bible, const std::string& value)
{
  set_value (bible, text_font_client_key, value);
}


constexpr const auto paratext_project_key {"paratext-project"};
std::string get_paratext_project (const std::string& bible)
{
  return get_value (bible, paratext_project_key, "");
}
void set_paratext_project (const std::string& bible, const std::string& value)
{
  set_value (bible, paratext_project_key, value);
}


constexpr const auto paratext_collaboration_enabled_key {"paratext-collaboration-enabled"};
bool get_paratext_collaboration_enabled (const std::string& bible)
{
  return get_boolean_value (bible, paratext_collaboration_enabled_key, false);
}
void set_paratext_collaboration_enabled (const std::string& bible, bool value)
{
  set_boolean_value (bible, paratext_collaboration_enabled_key, value);
}

constexpr const auto line_height_key {"line-height"};
int get_line_height (const std::string& bible)
{
  return get_integer_value (bible, line_height_key, 100);
}
void set_line_height (const std::string& bible, int value)
{
  set_integer_value (bible, line_height_key, value);
}


constexpr const auto letter_spacing_key {"letter-spacing"};
int get_letter_spacing (const std::string& bible)
{
  return get_integer_value (bible, letter_spacing_key, 0);
}
void set_letter_spacing (const std::string& bible, int value)
{
  set_integer_value (bible, letter_spacing_key, value);
}


constexpr const auto public_feedback_enabled_key {"public-feedback-enabled"};
bool get_public_feedback_enabled (const std::string& bible)
{
  return get_boolean_value (bible, public_feedback_enabled_key, true);
}
void set_public_feedback_enabled (const std::string& bible, bool value)
{
  set_boolean_value (bible, public_feedback_enabled_key, value);
}


constexpr const auto read_from_git_key {"read-from-git"};
bool get_read_from_git (const std::string& bible)
{
  return get_boolean_value (bible, read_from_git_key, false);
}
void set_read_from_git (const std::string& bible, bool value)
{
  set_boolean_value (bible, read_from_git_key, value);
}


constexpr const auto send_changes_to_rss_key {"send-changes-to-rss"};
bool get_send_changes_to_rss (const std::string& bible)
{
  return get_boolean_value (bible, send_changes_to_rss_key, false);
}
void set_send_changes_to_rss (const std::string& bible, bool value)
{
  set_boolean_value (bible, send_changes_to_rss_key, value);
}


constexpr const auto odt_space_after_verse_key {"odt-space-after-verse"};
std::string get_odt_space_after_verse (const std::string& bible)
{
  return get_value (bible, odt_space_after_verse_key, " ");
}
void set_odt_space_after_verse (const std::string& bible, const std::string& value)
{
  set_value (bible, odt_space_after_verse_key, value);
}


constexpr const auto daily_checks_enabled_key {"daily-checks-enabled"};
bool get_daily_checks_enabled (const std::string& bible)
{
  return get_boolean_value (bible, daily_checks_enabled_key, true);
}
void set_daily_checks_enabled (const std::string& bible, bool value)
{
  set_boolean_value (bible, daily_checks_enabled_key, value);
}


constexpr const auto odt_poetry_verses_left_key {"odt-poetry-verses-left"};
bool get_odt_poetry_verses_left (const std::string& bible)
{
  return get_boolean_value (bible, odt_poetry_verses_left_key, false);
}
void set_odt_poetry_verses_left (const std::string& bible, bool value)
{
  set_boolean_value (bible, odt_poetry_verses_left_key, value);
}


constexpr const auto odt_automatic_note_caller_key {"odt-automatic-note-caller"};
bool get_odt_automatic_note_caller (const std::string& bible)
{
  return get_boolean_value (bible, odt_automatic_note_caller_key, false);
}
void set_odt_automatic_note_caller (const std::string& bible, bool value)
{
  set_boolean_value (bible, odt_automatic_note_caller_key, value);
}

}
