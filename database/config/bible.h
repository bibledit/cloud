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


#pragma once

#include <config/libraries.h>

namespace database::config::bible {

void remove (const std::string& bible);
std::string get_remote_repository_url (const std::string& bible);
void set_remote_repository_url (const std::string& bible, const std::string& url);
bool get_check_double_spaces_usfm (const std::string& bible);
void set_check_double_spaces_usfm (const std::string& bible, bool value);
bool get_check_full_stop_in_headings (const std::string& bible);
void set_check_full_stop_in_headings (const std::string& bible, bool value);
bool get_check_space_before_punctuation (const std::string& bible);
void set_check_space_before_punctuation (const std::string& bible, bool value);
bool get_check_space_before_final_note_marker (const std::string& bible);
void set_check_space_before_final_note_marker (const std::string& bible, bool value);
bool get_check_sentence_structure (const std::string& bible);
void set_check_sentence_structure (const std::string& bible, bool value);
bool get_check_paragraph_structure (const std::string& bible);
void set_check_paragraph_structure (const std::string& bible, bool value);
bool get_check_books_versification (const std::string& bible);
void set_check_books_versification (const std::string& bible, bool value);
bool get_check_chaptes_verses_versification (const std::string& bible);
void set_check_chaptes_verses_versification (const std::string& bible, bool value);
bool get_check_well_formed_usfm (const std::string& bible);
void set_check_well_formed_usfm (const std::string& bible, bool value);
bool get_check_missing_punctuation_end_verse (const std::string& bible);
void set_check_missing_punctuation_end_verse (const std::string& bible, bool value);
bool get_check_patterns (const std::string& bible);
void set_check_patterns (const std::string& bible, bool value);
std::string get_checking_patterns (const std::string& bible);
void set_checking_patterns (const std::string& bible, const std::string& value);
std::string get_sentence_structure_capitals (const std::string& bible);
void set_sentence_structure_capitals (const std::string& bible, const std::string& value);
std::string get_sentence_structure_small_letters (const std::string& bible);
void set_sentence_structure_small_letters (const std::string& bible, const std::string& value);
std::string get_sentence_structure_end_punctuation (const std::string& bible);
void set_sentence_structure_end_punctuation (const std::string& bible, const std::string& value);
std::string get_sentence_structure_middle_punctuation (const std::string& bible);
void set_sentence_structure_middle_punctuation (const std::string& bible, const std::string& value);
std::string get_sentence_structure_disregards (const std::string& bible);
void set_sentence_structure_disregards (const std::string& bible, const std::string& value);
std::string get_sentence_structure_names (const std::string& bible);
void set_sentence_structure_names (const std::string& bible, const std::string& value);
std::string get_sentence_structure_within_sentence_markers (const std::string& bible);
void set_sentence_structure_within_sentence_markers (const std::string& bible, const std::string& value);
bool get_check_matching_pairs (const std::string& bible);
void set_check_matching_pairs (const std::string& bible, bool value);
std::string get_matching_pairs (const std::string& bible);
void set_matching_pairs (const std::string& bible, const std::string& value);
bool get_check_space_end_verse (const std::string& bible);
void set_check_space_end_verse (const std::string& bible, bool value);
bool get_check_french_punctuation (const std::string& bible);
void set_check_french_punctuation (const std::string& bible, bool value);
bool get_check_french_citation_style (const std::string& bible);
void set_check_french_citation_style (const std::string& bible, bool value);
bool get_transpose_fix_spaces_notes (const std::string& bible);
void set_transpose_fix_spaces_notes (const std::string& bible, bool value);
bool get_check_valid_utf8_text (const std::string& bible);
void set_check_valid_utf8_text (const std::string& bible, bool value);
std::string get_sprint_task_completion_categories (const std::string& bible);
void set_sprint_task_completion_categories (const std::string& bible, const std::string& value);
int get_repeat_send_receive (const std::string& bible);
void set_repeat_send_receive (const std::string& bible, int value);
bool get_export_chapter_drop_caps_frames (const std::string& bible);
void set_export_chapter_drop_caps_frames (const std::string& bible, bool value);
std::string get_page_width (const std::string& bible);
void set_page_width  (const std::string& bible, const std::string& value);
std::string get_page_height (const std::string& bible);
void set_page_height  (const std::string& bible, const std::string& value);
std::string get_inner_margin (const std::string& bible);
void set_inner_margin  (const std::string& bible, const std::string& value);
std::string get_outer_margin (const std::string& bible);
void set_outer_margin  (const std::string& bible, const std::string& value);
std::string get_top_margin (const std::string& bible);
void set_top_margin  (const std::string& bible, const std::string& value);
std::string get_bottom_margin (const std::string& bible);
void set_bottom_margin  (const std::string& bible, const std::string& value);
bool get_date_in_header (const std::string& bible);
void set_date_in_header  (const std::string& bible, bool value);
std::string get_hyphenation_first_set (const std::string& bible);
void set_hyphenation_first_set (const std::string& bible, const std::string& value);
std::string get_hyphenation_second_set (const std::string& bible);
void set_hyphenation_second_set (const std::string& bible, const std::string& value);
std::string get_editor_stylesheet (const std::string& bible);
void set_editor_stylesheet (const std::string& bible, const std::string& value);
std::string get_export_stylesheet (const std::string& bible);
void set_export_stylesheet (const std::string& bible, const std::string& value);
std::string get_versification_system (const std::string& bible);
void set_versification_system (const std::string& bible, const std::string& value);
bool get_export_web_during_night (const std::string& bible);
void set_export_web_during_night (const std::string& bible, bool value);
bool get_export_hml_during_night (const std::string& bible);
void set_export_hml_during_night (const std::string& bible, bool value);
bool get_export_html_notes_on_hover (const std::string& bible);
void set_export_html_notes_on_hover (const std::string& bible, bool value);
bool get_export_usfm_during_night (const std::string& bible);
void set_export_usfm_during_night (const std::string& bible, bool value);
bool get_export_text_during_night (const std::string& bible);
void set_export_text_during_night (const std::string& bible, bool value);
bool get_export_odt_during_night (const std::string& bible);
void set_export_odt_during_night (const std::string& bible, bool value);
bool get_generate_info_during_night (const std::string& bible);
void set_generate_info_during_night (const std::string& bible, bool value);
bool get_export_e_sword_during_night (const std::string& bible);
void set_export_e_sword_during_night (const std::string& bible, bool value);
bool get_export_online_bible_during_night (const std::string& bible);
void set_export_online_bible_during_night (const std::string& bible, bool value);
std::string get_export_password (const std::string& bible);
void set_export_password (const std::string& bible, const std::string& value);
bool get_secure_usfm_export (const std::string& bible);
void set_secure_usfm_export (const std::string& bible, bool value);
bool get_secure_odt_export (const std::string& bible);
void set_secure_odt_export (const std::string& bible, bool value);
std::string get_export_font (const std::string& bible);
void set_export_font (const std::string& bible, const std::string& value);
std::string get_export_feedback_email (const std::string& bible);
void set_export_feedback_email (const std::string& bible, const std::string& value);
std::string get_book_order (const std::string& bible);
void set_book_order (const std::string& bible, const std::string& value);
int get_text_direction (const std::string& bible);
void set_text_direction (const std::string& bible, int value);
std::string get_text_font (const std::string& bible);
void set_text_font (const std::string& bible, const std::string& value);
std::string get_text_font_client (const std::string& bible);
void set_text_font_client (const std::string& bible, const std::string& value);
std::string get_paratext_project (const std::string& bible);
void set_paratext_project (const std::string& bible, const std::string& value);
bool get_paratext_collaboration_enabled (const std::string& bible);
void set_paratext_collaboration_enabled (const std::string& bible, bool value);
int get_line_height (const std::string& bible);
void set_line_height (const std::string& bible, int value);
int get_letter_spacing (const std::string& bible);
void set_letter_spacing (const std::string& bible, int value);
bool get_public_feedback_enabled (const std::string& bible);
void set_public_feedback_enabled (const std::string& bible, bool value);
bool get_read_from_git (const std::string& bible);
void set_read_from_git (const std::string& bible, bool value);
bool get_send_changes_to_rss (const std::string& bible);
void set_send_changes_to_rss (const std::string& bible, bool value);
std::string get_odt_space_after_verse (const std::string& bible);
void set_odt_space_after_verse (const std::string& bible, const std::string& value);
bool get_daily_checks_enabled (const std::string& bible);
void set_daily_checks_enabled (const std::string& bible, bool value);
bool get_odt_poetry_verses_left (const std::string& bible);
void set_odt_poetry_verses_left (const std::string& bible, bool value);
bool get_odt_automatic_note_caller (const std::string& bible);
void set_odt_automatic_note_caller (const std::string& bible, bool value);

}
