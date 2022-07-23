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

class Checks_Sentences
{
public:
  void enterCapitals (string capitals_in);
  void enterSmallLetters (string small_letters_in);
  void enterEndMarks (string end_marks_in);
  void enterCenterMarks (string center_marks_in);
  void enterDisregards (string disregards_in);
  void enterNames (string names_in);
  void initialize ();
  vector <pair<int, string>> getResults ();
  void check (map <int, string> texts);
  void paragraphs (vector <string> paragraph_start_markers,
                   vector <string> within_sentence_paragraph_markers,
                   vector <map <int, string>> verses_paragraphs);

private:
  // Sentence structure parameters.
  vector <string> capitals {};
  vector <string> small_letters {};
  vector <string> end_marks {};
  vector <string> center_marks {};
  vector <string> disregards {};
  vector <string> names {};
  
  // State.
  int verseNumber {0};
  int currentPosition {0};
  
  // Grapheme analysis.
  string character {};
  bool isSpace {false};
  int spacePosition {0};
  bool isCapital {false};
  int capitalPosition {0};
  bool isSmallLetter {false};
  int smallLetterPosition {0};
  bool isEndMark {false};
  int endMarkPosition {0};
  bool isCenterMark {false};
  int centerMarkPosition {0};
  int punctuationMarkPosition {0};
  int previousMarkPosition {0};
  
  // Context.
  string fullText {};
  
  // Results of the checks.
  vector <pair<int, string>> checkingResults {};
  static constexpr int displayCharacterOnly {1};
  static constexpr int displayContext {2};
  static constexpr int skipNames {3};
  
  void addResult (string text, int modifier);
  void checkUnknownCharacter ();
  void analyzeCharacters ();
  void checkCharacter ();
};
