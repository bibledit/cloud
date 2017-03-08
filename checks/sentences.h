/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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


#ifndef INCLUDED_CHECK_SENTENCES_H
#define INCLUDED_CHECK_SENTENCES_H


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
  vector <string> capitals;
  vector <string> small_letters;
  vector <string> end_marks;
  vector <string> center_marks;
  vector <string> disregards;
  vector <string> names;
  
  // State.
  int verseNumber;
  int currentPosition;
  
  // Grapheme analysis.
  string character;
  bool isSpace;
  int spacePosition;
  bool isCapital;
  int capitalPosition;
  bool isSmallLetter;
  int smallLetterPosition;
  bool isEndMark;
  int endMarkPosition;
  bool isCenterMark;
  int centerMarkPosition;
  int punctuationMarkPosition;
  int previousMarkPosition;
  
  // Context.
  string fullText;
  
  // Results of the checks.
  vector <pair<int, string>> checkingResults;
  const int displayCharacterOnly = 1;
  const int displayContext = 2;
  const int skipNames = 3;
  
  void addResult (string text, int modifier);
  void checkUnknownCharacter ();
  void analyzeCharacters ();
  void checkCharacter ();
};


#endif
