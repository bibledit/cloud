/*
Copyright (©) 2003-2024 Teus Benschop.

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
  return filter::strings::convert_to_int (get_value (bible, key, filter::strings::convert_to_string (default_value).c_str()));
}


static void set_integer_value (const std::string& bible, const char * key, int value)
{
  set_value (bible, key, filter::strings::convert_to_string (value));
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


std::string get_remote_repository_url (const std::string& bible)
{
  return get_value (bible, "remote-repo-url", "");
}
void set_remote_repository_url (const std::string& bible, const std::string& url)
{
  set_value (bible, "remote-repo-url", url);
}


bool get_check_double_spaces_usfm (const std::string& bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return get_boolean_value (bible, "double-spaces-usfm", standard);
}
void set_check_double_spaces_usfm (const std::string& bible, bool value)
{
  set_boolean_value (bible, "double-spaces-usfm", value);
}


bool get_check_full_stop_in_headings (const std::string& bible)
{
  return get_boolean_value (bible, "full-stop-headings", false);
}
void set_check_full_stop_in_headings (const std::string& bible, bool value)
{
  set_boolean_value (bible, "full-stop-headings", value);
}


bool get_check_space_before_punctuation (const std::string& bible)
{
  return get_boolean_value (bible, "space-before-punctuation", false);
}
void set_check_space_before_punctuation (const std::string& bible, bool value)
{
  set_boolean_value (bible, "space-before-punctuation", value);
}


const char * space_before_final_note_marker_key ()
{
  return "space-before-final-note-marker";
}
bool get_check_space_before_final_note_marker (const std::string& bible)
{
  return get_boolean_value (bible, space_before_final_note_marker_key (), false);
}
void set_check_space_before_final_note_marker (const std::string& bible, bool value)
{
  set_boolean_value (bible, space_before_final_note_marker_key (), value);
}


bool get_check_sentence_structure (const std::string& bible)
{
  return get_boolean_value (bible, "sentence-structure", false);
}
void set_check_sentence_structure (const std::string& bible, bool value)
{
  set_boolean_value (bible, "sentence-structure", value);
}


bool get_check_paragraph_structure (const std::string& bible)
{
  return get_boolean_value (bible, "paragraph-structure", false);
}
void set_check_paragraph_structure (const std::string& bible, bool value)
{
  set_boolean_value (bible, "paragraph-structure", value);
}


bool get_check_books_versification (const std::string& bible)
{
  return get_boolean_value (bible, "check-books-versification", false);
}
void set_check_books_versification (const std::string& bible, bool value)
{
  set_boolean_value (bible, "check-books-versification", value);
}


bool get_check_chaptes_verses_versification (const std::string& bible)
{
  return get_boolean_value (bible, "check-chapters-verses-versification", false);
}
void set_check_chaptes_verses_versification (const std::string& bible, bool value)
{
  set_boolean_value (bible, "check-chapters-verses-versification", value);
}


bool get_check_well_formed_usfm (const std::string& bible)
{
  // Check is on by default in the Cloud, and off on a client.
#ifdef HAVE_CLIENT
  bool standard = false;
#else
  bool standard = true;
#endif
  return get_boolean_value (bible, "check-well-formed-usfm", standard);
}
void set_check_well_formed_usfm (const std::string& bible, bool value)
{
  set_boolean_value (bible, "check-well-formed-usfm", value);
}


bool get_check_missing_punctuation_end_verse (const std::string& bible)
{
  return get_boolean_value (bible, "missing-punctuation-end-verse", false);
}
void set_check_missing_punctuation_end_verse (const std::string& bible, bool value)
{
  set_boolean_value (bible, "missing-punctuation-end-verse", value);
}


bool get_check_patterns (const std::string& bible)
{
  return get_boolean_value (bible, "check_patterns", false);
}
void set_check_patterns (const std::string& bible, bool value)
{
  set_boolean_value (bible, "check_patterns", value);
}


std::string get_checking_patterns (const std::string& bible)
{
  return get_value (bible, "checking-patterns", "");
}
void set_checking_patterns (const std::string& bible, const std::string& value)
{
  set_value (bible, "checking-patterns", value);
}


std::string get_sentence_structure_capitals (const std::string& bible)
{
  return get_value (bible, "sentence-structure-capitals", "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
}
void set_sentence_structure_capitals (const std::string& bible, const std::string& value)
{
  set_value (bible, "sentence-structure-capitals", value);
}


std::string get_sentence_structure_small_letters (const std::string& bible)
{
  return get_value (bible, "sentence-structure-small-letters", "a b c d e f g h i j k l m n o p q r s t u v w x y z");
}
void set_sentence_structure_small_letters (const std::string& bible, const std::string& value)
{
  set_value (bible, "sentence-structure-small-letters", value);
}


std::string get_sentence_structure_end_punctuation (const std::string& bible)
{
  return get_value (bible, "sentence-structure-end-punctuation", ". ! ? :");
}
void set_sentence_structure_end_punctuation (const std::string& bible, const std::string& value)
{
  set_value (bible, "sentence-structure-end-punctuation", value);
}


std::string get_sentence_structure_middle_punctuation (const std::string& bible)
{
  return get_value (bible, "sentence-structure-middle-punctuation", ", ;");
}
void set_sentence_structure_middle_punctuation (const std::string& bible, const std::string& value)
{
  set_value (bible, "sentence-structure-middle-punctuation", value);
}


std::string get_sentence_structure_disregards (const std::string& bible)
{
  return get_value (bible, "sentence-structure-disregards", "( ) [ ] { } ' \" * - 0 1 2 3 4 5 6 7 8 9");
}
void set_sentence_structure_disregards (const std::string& bible, const std::string& value)
{
  set_value (bible, "sentence-structure-disregards", value);
}


std::string get_sentence_structure_names (const std::string& bible)
{
  return get_value (bible, "sentence-structure-names", "");
}
void set_sentence_structure_names (const std::string& bible, const std::string& value)
{
  set_value (bible, "sentence-structure-names", value);
}


std::string get_sentence_structure_within_sentence_markers (const std::string& bible)
{
  return get_value (bible, "sentence-structure-within_sentence-markers", "q q1 q2 q3");
}
void set_sentence_structure_within_sentence_markers (const std::string& bible, const std::string& value)
{
  set_value (bible, "sentence-structure-within_sentence-markers", value);
}


bool get_check_matching_pairs (const std::string& bible)
{
  return get_boolean_value (bible, "check-matching-pairs", false);
}
void set_check_matching_pairs (const std::string& bible, bool value)
{
  set_boolean_value (bible, "check-matching-pairs", value);
}


std::string get_matching_pairs (const std::string& bible)
{
  return get_value (bible, "matching-pairs", "[] () {} “” ‘’ «» ‹›");
}
void set_matching_pairs (const std::string& bible, const std::string& value)
{
  set_value (bible, "matching-pairs", value);
}


bool get_check_space_end_verse (const std::string& bible)
{
  return get_boolean_value (bible, "check-space-end-verse", true);
}
void set_check_space_end_verse (const std::string& bible, bool value)
{
  set_boolean_value (bible, "check-space-end-verse", value);
}


bool get_check_french_punctuation (const std::string& bible)
{
  return get_boolean_value (bible, "check-french-punctuation", false);
}
void set_check_french_punctuation (const std::string& bible, bool value)
{
  set_boolean_value (bible, "check-french-punctuation", value);
}


const char * check_french_citation_style_key ()
{
  return "check-french-citation-style";
}
bool get_check_french_citation_style (const std::string& bible)
{
  return get_boolean_value (bible, check_french_citation_style_key (), false);
}
void set_check_french_citation_style (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_french_citation_style_key (), value);
}


const char * transpose_fix_spaces_notes_key ()
{
  return "transpose-fix-spaces-notes";
}
bool get_transpose_fix_spaces_notes (const std::string& bible)
{
  return get_boolean_value (bible, transpose_fix_spaces_notes_key (), false);
}
void set_transpose_fix_spaces_notes (const std::string& bible, bool value)
{
  set_boolean_value (bible, transpose_fix_spaces_notes_key (), value);
}


const char * check_valid_utf8_text_key ()
{
  return "check-valid-utf8-text";
}
bool get_check_valid_utf8_text (const std::string& bible)
{
  return get_boolean_value (bible, check_valid_utf8_text_key (), false);
}
void set_check_valid_utf8_text (const std::string& bible, bool value)
{
  set_boolean_value (bible, check_valid_utf8_text_key (), value);
}


std::string get_sprint_task_completion_categories (const std::string& bible)
{
  return get_value (bible, "sprint-task-completion-categories", "Translate\nCheck\nHebrew/Greek\nDiscussions");
}
void set_sprint_task_completion_categories (const std::string& bible, const std::string& value)
{
  set_value (bible, "sprint-task-completion-categories", value);
}


int get_repeat_send_receive (const std::string& bible)
{
  return get_integer_value (bible, "repeat-send-receive", 0);
}
void set_repeat_send_receive (const std::string& bible, int value)
{
  set_integer_value (bible, "repeat-send-receive", value);
}


bool get_export_chapter_drop_caps_frames (const std::string& bible)
{
  return get_boolean_value (bible, "export-chapter-drop-caps-frames", false);
}
void set_export_chapter_drop_caps_frames (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-chapter-drop-caps-frames", value);
}


std::string get_page_width (const std::string& bible)
{
  return get_value (bible, "page-width", "210");
}
void set_page_width  (const std::string& bible, const std::string& value)
{
  set_value (bible, "page-width", value);
}


std::string get_page_height (const std::string& bible)
{
  return get_value (bible, "page-height", "297");
}
void set_page_height  (const std::string& bible, const std::string& value)
{
  set_value (bible, "page-height", value);
}


std::string get_inner_margin (const std::string& bible)
{
  return get_value (bible, "inner-margin", "20");
}
void set_inner_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, "inner-margin", value);
}


std::string get_outer_margin (const std::string& bible)
{
  return get_value (bible, "outer-margin", "10");
}
void set_outer_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, "outer-margin", value);
}


std::string get_top_margin (const std::string& bible)
{
  return get_value (bible, "top-margin", "10");
}
void set_top_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, "top-margin", value);
}


std::string get_bottom_margin (const std::string& bible)
{
  return get_value (bible, "bottom-margin", "10");
}
void set_bottom_margin  (const std::string& bible, const std::string& value)
{
  set_value (bible, "bottom-margin", value);
}


bool get_date_in_header (const std::string& bible)
{
  return get_boolean_value (bible, "date-in-header", false);
}
void set_date_in_header  (const std::string& bible, bool value)
{
  set_boolean_value (bible, "date-in-header", value);
}


std::string get_hyphenation_first_set (const std::string& bible)
{
  return get_value (bible, "hyphenation-first-set", "");
}
void set_hyphenation_first_set (const std::string& bible, const std::string& value)
{
  set_value (bible, "hyphenation-first-set", value);
}


std::string get_hyphenation_second_set (const std::string& bible)
{
  return get_value (bible, "hyphenation-second-set", "");
}
void set_hyphenation_second_set (const std::string& bible, const std::string& value)
{
  set_value (bible, "hyphenation-second-set", value);
}


std::string get_editor_stylesheet (const std::string& bible)
{
  return get_value (bible, "editor-stylesheet", styles_logic_standard_sheet ().c_str());
}
void set_editor_stylesheet (const std::string& bible, const std::string& value)
{
  set_value (bible, "editor-stylesheet", value);
}


std::string get_export_stylesheet (const std::string& bible)
{
  return get_value (bible, "export-stylesheet", styles_logic_standard_sheet ().c_str());
}
void set_export_stylesheet (const std::string& bible, const std::string& value)
{
  set_value (bible, "export-stylesheet", value);
}


std::string get_versification_system (const std::string& bible)
{
  return get_value (bible, "versification-system", filter::strings::english ());
}
void set_versification_system (const std::string& bible, const std::string& value)
{
  set_value (bible, "versification-system", value);
}


bool get_export_web_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "export-web-during-night", false);
}
void set_export_web_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-web-during-night", value);
}


bool get_export_hml_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "export-html-during-night", false);
}
void set_export_hml_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-html-during-night", value);
}


const char * export_html_notes_on_hover_key ()
{
  return "export-html-notes-on-hover";
}
bool get_export_html_notes_on_hover (const std::string& bible)
{
  return get_boolean_value (bible, export_html_notes_on_hover_key (), false);
}
void set_export_html_notes_on_hover (const std::string& bible, bool value)
{
  set_boolean_value (bible, export_html_notes_on_hover_key (), value);
}


bool get_export_usfm_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "export-usfm-during-night", false);
}
void set_export_usfm_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-usfm-during-night", value);
}


bool get_export_text_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "export-text-during-night", false);
}
void set_export_text_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-text-during-night", value);
}


bool get_export_odt_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "export-odt-during-night", false);
}
void set_export_odt_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-odt-during-night", value);
}


bool get_generate_info_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "generate-info-during-night", false);
}
void set_generate_info_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "generate-info-during-night", value);
}


bool get_export_e_sword_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "export-esword-during-night", false);
}
void set_export_e_sword_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-esword-during-night", value);
}


bool get_export_online_bible_during_night (const std::string& bible)
{
  return get_boolean_value (bible, "export-onlinebible-during-night", false);
}
void set_export_online_bible_during_night (const std::string& bible, bool value)
{
  set_boolean_value (bible, "export-onlinebible-during-night", value);
}


std::string get_export_password (const std::string& bible)
{
  return get_value (bible, "export-password", "");
}
void set_export_password (const std::string& bible, const std::string& value)
{
  set_value (bible, "export-password", value);
}


bool get_secure_usfm_export (const std::string& bible)
{
  return get_boolean_value (bible, "secure-usfm-export", false);
}
void set_secure_usfm_export (const std::string& bible, bool value)
{
  set_boolean_value (bible, "secure-usfm-export", value);
}


bool get_secure_odt_export (const std::string& bible)
{
  return get_boolean_value (bible, "secure-odt-export", false);
}
void set_secure_odt_export (const std::string& bible, bool value)
{
  set_boolean_value (bible, "secure-odt-export", value);
}


const char * export_font_key ()
{
  return "export-font";
}
std::string get_export_font (const std::string& bible)
{
  return get_value (bible, export_font_key (), "");
}
void set_export_font (const std::string& bible, const std::string& value)
{
  set_value (bible, export_font_key (), value);
}


const char * export_feedback_email_key ()
{
  return "export-feedback-email";
}
std::string get_export_feedback_email (const std::string& bible)
{
  return get_value (bible, export_feedback_email_key (), "");
}
void set_export_feedback_email (const std::string& bible, const std::string& value)
{
  set_value (bible, export_feedback_email_key (), value);
}


std::string get_book_order (const std::string& bible)
{
  return get_value (bible, "book-order", "");
}
void set_book_order (const std::string& bible, const std::string& value)
{
  set_value (bible, "book-order", value);
}


int get_text_direction (const std::string& bible)
{
  return get_integer_value (bible, "text-direction", 0);
}
void set_text_direction (const std::string& bible, int value)
{
  set_integer_value (bible, "text-direction", value);
}


std::string get_text_font (const std::string& bible)
{
  return get_value (bible, "text-font", "");
}
void set_text_font (const std::string& bible, const std::string& value)
{
  set_value (bible, "text-font", value);
}


std::string get_text_font_client (const std::string& bible)
{
  return get_value (bible, "text-font-client", "");
}
void set_text_font_client (const std::string& bible, const std::string& value)
{
  set_value (bible, "text-font-client", value);
}


std::string get_paratext_project (const std::string& bible)
{
  return get_value (bible, "paratext-project", "");
}
void set_paratext_project (const std::string& bible, const std::string& value)
{
  set_value (bible, "paratext-project", value);
}


bool get_paratext_collaboration_enabled (const std::string& bible)
{
  return get_boolean_value (bible, "paratext-collaboration-enabled", false);
}
void set_paratext_collaboration_enabled (const std::string& bible, bool value)
{
  set_boolean_value (bible, "paratext-collaboration-enabled", value);
}


int get_line_height (const std::string& bible)
{
  return get_integer_value (bible, "line-height", 100);
}
void set_line_height (const std::string& bible, int value)
{
  set_integer_value (bible, "line-height", value);
}


int get_letter_spacing (const std::string& bible)
{
  return get_integer_value (bible, "letter-spacing", 0);
}
void set_letter_spacing (const std::string& bible, int value)
{
  set_integer_value (bible, "letter-spacing", value);
}


bool get_public_feedback_enabled (const std::string& bible)
{
  return get_boolean_value (bible, "public-feedback-enabled", true);
}
void set_public_feedback_enabled (const std::string& bible, bool value)
{
  set_boolean_value (bible, "public-feedback-enabled", value);
}


bool get_read_from_git (const std::string& bible)
{
  return get_boolean_value (bible, "read-from-git", false);
}
void set_read_from_git (const std::string& bible, bool value)
{
  set_boolean_value (bible, "read-from-git", value);
}


const char * send_changes_to_rss_key ()
{
  return "send-changes-to-rss";
}
bool get_send_changes_to_rss (const std::string& bible)
{
  return get_boolean_value (bible, send_changes_to_rss_key (), false);
}
void set_send_changes_to_rss (const std::string& bible, bool value)
{
  set_boolean_value (bible, send_changes_to_rss_key (), value);
}


const char * odt_space_after_verse_key ()
{
  return "odt-space-after-verse";
}
std::string get_odt_space_after_verse (const std::string& bible)
{
  return get_value (bible, odt_space_after_verse_key (), " ");
}
void set_odt_space_after_verse (const std::string& bible, const std::string& value)
{
  set_value (bible, odt_space_after_verse_key (), value);
}


const char * daily_checks_enabled_key ()
{
  return "daily-checks-enabled";
}
bool get_daily_checks_enabled (const std::string& bible)
{
  return get_boolean_value (bible, daily_checks_enabled_key (), true);
}
void set_daily_checks_enabled (const std::string& bible, bool value)
{
  set_boolean_value (bible, daily_checks_enabled_key (), value);
}


const char * odt_poetry_verses_left_key ()
{
  return "odt-poetry-verses-left";
}
bool get_odt_poetry_verses_left (const std::string& bible)
{
  return get_boolean_value (bible, odt_poetry_verses_left_key(), false);
}
void set_odt_poetry_verses_left (const std::string& bible, bool value)
{
  set_boolean_value (bible, odt_poetry_verses_left_key(), value);
}


const char * odt_automatic_note_caller_key ()
{
  return "odt-automatic-note-caller";
}
bool get_odt_automatic_note_caller (const std::string& bible)
{
  return get_boolean_value (bible, odt_automatic_note_caller_key(), false);
}
void set_odt_automatic_note_caller (const std::string& bible, bool value)
{
  set_boolean_value (bible, odt_automatic_note_caller_key(), value);
}

}
