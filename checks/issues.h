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


#pragma once

#include <config/libraries.h>

namespace checks::issues {

enum class issue {
  start_boundary, // This one should remain the first.
  should_be_followed_by_a_no_break_space_rather_than_a_plain_space_in_french,
  should_be_followed_by_a_no_break_space_in_french,
  should_be_preceded_by_a_no_break_space_rather_than_a_plain_space_in_french,
  should_be_preceded_by_a_no_break_space_in_french,
  the_previous_paragraph_contains_a_citation_not_closed_with_a_right_guillemet_therefore_the_current_paragraph_is_expected_to_start_with_a_left_guillemet_to_continue_that_citation_in_french,
  the_paragraph_contains_more_left_guillements_than_needed,
  the_paragraph_contains_more_right_guillements_than_needed,
  punctuation_at_end_of_heading,
  closing_character,
  without_its_matching_opening_character,
  opening_character,
  without_its_matching_closing_character,
  capital_follows_mid_sentence_punctuation_mark,
  small_letter_follows_straight_after_a_mid_sentence_punctuation_mark,
  capital_follows_straight_after_a_mid_sentence_punctuation_mark,
  a_letter_follows_straight_after_an_end_sentence_punctuation_mark,
  no_capital_after_an_end_sentence_punctuation_mark,
  two_punctuation_marks_in_sequence,
  paragraph_does_not_start_with_a_capital,
  paragraph_does_not_end_with_an_end_marker,
  unknown_character,
  double_space,
  space_before_a_comma,
  space_before_a_semicolon,
  space_before_a_colon,
  space_before_a_full_stop,
  space_before_a_question_mark,
  space_before_an_exclamation_mark,
  space_at_the_end_of_the_verse,
  space_before_final_note_markup,
  space_before_final_cross_reference_markup,
  unclosed_markers,
  malformed_verse_number,
  new_line_within_usfm,
  marker_not_in_stylesheet,
  unknown_id,
  id_is_not_in_uppercase,
  forward_slash_instead_of_backslash,
  widow_backslash,
  repeating_opening_marker,
  closing_marker_does_not_match_opening_marker,
  embedded_marker_requires_a_plus_sign,
  the_book_lacks_the_marker_for_the_verbose_book_name,
  the_book_lacks_the_marker_for_the_short_book_name,
  the_following_marker_belongs_in_chapter_0,
  empty_figure_source,
  could_not_find_bible_image,
  unusual_quotation_mark_found,
  opening_markup_is_followed_by_closing_markup_without_intervening_text,
  this_sequence_in_the_note_does_not_look_right,
  no_punctuation_at_end_of_verse,
  pattern_found_in_text,
  this_book_is_absent_from_the_bible,
  this_book_is_extra_in_the_bible,
  this_chapter_is_missing,
  this_chapter_is_extra,
  this_verse_is_missing_according_to_the_versification_system,
  this_verse_is_extra_according_to_the_versification_system,
  the_verse_is_out_of_sequence,
  stop_boundary, // This one should remain the last.
};

std::string text (const issue);


} // Namespace.
