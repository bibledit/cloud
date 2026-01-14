/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include "checks/issues.h"
#include "locale/translate.h"


namespace checks::issues {


std::string text (const issue issue)
{
  switch (issue) {
    case issue::should_be_followed_by_a_no_break_space_rather_than_a_plain_space_in_french:
      return translate ("Should be followed by a no-break space rather than a plain space in French");
    case issue::should_be_followed_by_a_no_break_space_in_french:
      return translate ("Should be followed by a no-break space in French");
    case issue::should_be_preceded_by_a_no_break_space_rather_than_a_plain_space_in_french:
      return translate ("Should be preceded by a no-break space rather than a plain space in French");
    case issue::should_be_preceded_by_a_no_break_space_in_french:
      return translate ("Should be preceded by a no-break space in French");
    case issue::the_previous_paragraph_contains_a_citation_not_closed_with_a_right_guillemet_therefore_the_current_paragraph_is_expected_to_start_with_a_left_guillemet_to_continue_that_citation_in_french:
      return translate ("The previous paragraph contains a citation not closed with a » therefore the current paragraph is expected to start with a « to continue that citation in French");
    case issue::the_paragraph_contains_more_left_guillements_than_needed:
      return translate ("The paragraph contains more left guillements than needed");
    case issue::the_paragraph_contains_more_right_guillements_than_needed:
      return translate ("The paragraph contains more right guillements than needed");
    case issue::punctuation_at_end_of_heading:
      return translate ("Punctuation at end of heading");
    case issue::closing_character:
      return translate ("Closing character");
    case issue::without_its_matching_opening_character:
      return translate ("without its matching opening character");
    case issue::opening_character:
      return translate ("Opening character");
    case issue::without_its_matching_closing_character:
      return translate ("without its matching closing character");
    case issue::capital_follows_mid_sentence_punctuation_mark:
      return translate ("Capital follows mid-sentence punctuation mark");
    case issue::small_letter_follows_straight_after_a_mid_sentence_punctuation_mark:
      return translate ("Small letter follows straight after a mid-sentence punctuation mark");
    case issue::capital_follows_straight_after_a_mid_sentence_punctuation_mark:
      return translate ("Capital follows straight after a mid-sentence punctuation mark");
    case issue::a_letter_follows_straight_after_an_end_sentence_punctuation_mark:
      return translate ("A letter follows straight after an end-sentence punctuation mark");
    case issue::no_capital_after_an_end_sentence_punctuation_mark:
      return translate ("No capital after an end-sentence punctuation mark");
    case issue::two_punctuation_marks_in_sequence:
      return translate ("Two punctuation marks in sequence");
    case issue::paragraph_does_not_start_with_a_capital:
      return translate ("Paragraph does not start with a capital");
    case issue::paragraph_does_not_end_with_an_end_marker:
      return translate ("Paragraph does not end with an end marker");
    case issue::unknown_character:
      return translate ("Unknown character");
    case issue::double_space:
      return translate ("Double space");
    case issue::space_before_a_comma:
      return translate ("Space before a comma");
    case issue::space_before_a_semicolon:
      return translate ("Space before a semicolon");
    case issue::space_before_a_colon:
      return translate ("Space before a colon");
    case issue::space_before_a_full_stop:
      return translate ("Space before a full stop");
    case issue::space_before_a_question_mark:
      return translate ("Space before a question mark");
    case issue::space_before_an_exclamation_mark:
      return translate ("Space before an exclamation mark");
    case issue::space_at_the_end_of_the_verse:
      return translate ("Space at the end of the verse");
    case issue::space_before_final_note_markup:
      return translate ("Space before final note markup");
    case issue::space_before_final_cross_reference_markup:
      return translate ("Space before final cross reference markup");
    case issue::unclosed_markers:
      return translate ("Unclosed markers");
    case issue::malformed_verse_number:
      return translate ("Malformed verse number");
    case issue::new_line_within_usfm:
      return translate ("New line within USFM");
    case issue::marker_not_in_stylesheet:
      return translate ("Marker not in stylesheet");
    case issue::unknown_id:
      return translate ("Unknown ID");
    case issue::id_is_not_in_uppercase:
      return translate ("ID is not in uppercase");
    case issue::forward_slash_instead_of_backslash:
      return translate ("Forward slash instead of backslash");
    case issue::widow_backslash:
      return translate ("Widow backslash");
    case issue::repeating_opening_marker:
      return translate ("Repeating opening marker");
    case issue::closing_marker_does_not_match_opening_marker:
      return translate ("Closing marker does not match opening marker");
    case issue::embedded_marker_requires_a_plus_sign:
      return translate ("Embedded marker requires a plus sign");
    case issue::the_book_lacks_the_marker_for_the_verbose_book_name:
      return translate ("The book lacks the marker for the verbose book name");
    case issue::the_book_lacks_the_marker_for_the_short_book_name:
      return translate ("The book lacks the marker for the short book name");
    case issue::the_following_marker_belongs_in_chapter_0:
      return translate ("The following marker belongs in chapter 0");
    case issue::empty_figure_source:
      return translate ("Empty figure source");
    case issue::could_not_find_bible_image:
      return translate ("Could not find Bible image");
    case issue::unusual_quotation_mark_found:
      return translate ("Unusual quotation mark found");
    case issue::opening_markup_is_followed_by_closing_markup_without_intervening_text:
      return translate ("Opening markup is followed by closing markup without intervening text");
    case issue::this_sequence_in_the_note_does_not_look_right:
      return translate ("This sequence in the note does not look right");
    case issue::no_punctuation_at_end_of_verse:
      return translate ("No punctuation at end of verse");
    case issue::pattern_found_in_text:
      return translate ("Pattern found in text");
    case issue::this_book_is_absent_from_the_bible:
      return translate ("This book is absent from the Bible");
    case issue::this_book_is_extra_in_the_bible:
      return translate ("This book is extra in the Bible");
    case issue::this_chapter_is_missing:
      return translate ("This chapter is missing");
    case issue::this_chapter_is_extra:
      return translate ("This chapter is extra");
    case issue::this_verse_is_missing_according_to_the_versification_system:
      return translate ("This verse is missing according to the versification system");
    case issue::this_verse_is_extra_according_to_the_versification_system:
      return translate ("This verse is extra according to the versification system");
    case issue::the_verse_is_out_of_sequence:
      return translate ("The verse is out of sequence");
    case issue::start_boundary:
    case issue::stop_boundary:
    default:
      return std::string();
  }
}


} // Namespace.
