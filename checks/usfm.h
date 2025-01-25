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
#include <database/styles.h>

class Checks_Usfm
{
public:
  Checks_Usfm (const std::string & bible);
  void initialize (int book, int chapter);
  void finalize ();
  void check (const std::string & usfm);
  std::vector <std::pair<int, std::string>> get_results ();
private:
  // USFM and text.
  std::vector <std::string> usfm_markers_and_text {};
  unsigned int usfm_markers_and_text_pointer {0};
  std::string usfm_item {};
  int book_number {0};
  int chapter_number {0};
  int verse_number {0};
  
  // Results of the checks.
  std::vector <std::pair<int, std::string>> checking_results {};
  static constexpr int display_nothing {0};
  static constexpr int display_current {1};
  static constexpr int display_next {2};
  static constexpr int display_full {3};
  
  // Stylesheet.
  std::vector <std::string> markers_stylesheet {};
  std::map <std::string, database::styles1::Item> style_items {};
  
  // Matching markers.
  std::vector <std::string> markers_requiring_endmarkers {};
  std::vector <std::string> open_matching_markers {};
  
  // Embedded markers.
  std::vector <std::string> embeddable_markers {};
  std::vector <std::string> open_embeddable_markers {};

  // Table of contents markers and flags.
  std::string long_toc1_marker {};
  std::string short_toc2_marker {};
  std::string abbrev_toc3_marker {};

  // Empty markup checking.
  std::string empty_markup_previous_item {};
  
  // Empty note checking.
  bool within_note {false};
  
  // Methods.
  void malformed_verse_number ();
  void new_line_in_usfm (const std::string & usfm);
  void marker_in_stylesheet ();
  void malformed_id ();
  void forward_slash (const std::string & usfm);
  void widow_back_slash ();
  void matching_endmarker ();
  void embedded_marker ();
  void toc (std::string usfm);
  void figure ();
  void add_result (std::string text, int modifier);
  void empty_markup ();
  void note ();
};
