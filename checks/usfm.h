/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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
  Checks_Usfm (string bible);
  void initialize (int book, int chapter);
  void finalize ();
  void check (string usfm);
  vector <pair<int, string>> get_results ();
private:
  // USFM and text.
  vector <string> usfm_markers_and_text;
  unsigned int usfm_markers_and_text_pointer;
  string usfm_item;
  int book_number;
  int chapter_number;
  int verse_number;
  
  // Results of the checks.
  vector <pair<int, string>> checking_results;
  static const int display_nothing = 0;
  static const int display_current = 1;
  static const int display_next = 2;
  static const int display_full = 3;
  
  // Stylesheet.
  vector <string> markers_stylesheet;
  map <string, Database_Styles_Item> style_items;
  
  // Matching markers.
  vector <string> markers_requiring_endmarkers;
  vector <string> open_matching_markers;
  
  // Embedded markers.
  vector <string> embeddable_markers;
  vector <string> open_embeddable_markers;

  // Table of contents markers and flags.
  string long_toc1_marker;
  string short_toc2_marker;
  string abbrev_toc3_marker;

  // Empty markup checking.
  string empty_markup_previous_item;
  
  // Empty note checking.
  bool within_note = false;
  
  // Methods.
  void malformed_verse_number ();
  void new_line_in_usfm (string usfm);
  void marker_in_stylesheet ();
  void malformed_id ();
  void forward_slash (string usfm);
  void widow_back_slash ();
  void matching_endmarker ();
  void embedded_marker ();
  void toc (string usfm);
  void figure ();
  void add_result (string text, int modifier);
  void empty_markup ();
  void note ();
};
