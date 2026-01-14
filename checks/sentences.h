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


#pragma once

#include <config/libraries.h>

class Checks_Sentences
{
public:
  void enter_capitals (const std::string & capitals);
  void enter_small_letters (const std::string & small_letters);
  void enter_end_marks (const std::string & end_marks);
  void enter_center_marks (const std::string & center_marks);
  void enter_disregards (const std::string & disregards);
  void enter_names (std::string names);
  void initialize ();
  std::vector <std::pair<int, std::string>> get_results ();
  void check (const std::map <int, std::string> & texts);
  void paragraphs (const std::vector <std::string> & paragraph_start_markers,
                   const std::vector <std::string> & within_sentence_paragraph_markers,
                   const std::vector <std::map <int, std::string>> & verses_paragraphs);

private:
  // Sentence structure parameters.
  std::vector <std::string> m_capitals {};
  std::vector <std::string> m_small_letters {};
  std::vector <std::string> m_end_marks {};
  std::vector <std::string> m_center_marks {};
  std::vector <std::string> m_disregards {};
  std::vector <std::string> m_names {};
  
  // State.
  int verse_number {0};
  int current_position {0};
  
  // Grapheme analysis.
  std::string character {};
  bool is_space {false};
  int space_position {0};
  bool is_capital {false};
  int capital_position {0};
  bool is_small_letter {false};
  int small_letter_position {0};
  bool is_end_mark {false};
  int end_mark_position {0};
  bool is_center_mark {false};
  int center_mark_position {0};
  int punctuation_mark_position {0};
  int previous_mark_position {0};
  
  // Context.
  std::string full_text {};
  
  // Results of the checks.
  std::vector <std::pair<int, std::string>> checking_results {};
  static constexpr int display_character_only {1};
  static constexpr int display_context {2};
  static constexpr int skip_names {3};
  
  void add_result (std::string text, int modifier);
  void check_unknown_character ();
  void analyze_characters ();
  void check_character ();
};
